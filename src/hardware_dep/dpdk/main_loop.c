/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2015 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
// A large portion of the code in this file comes from
// main.c in the l3fwd example of DPDK 2.2.0.

#include "dpdk_lib.h"
#include "actions.h"
#include <rte_ethdev.h>
#include "key.h"
#include "dpdk_tables.h"

struct rte_mempool *header_pool, *clone_pool;
struct lcore_conf lcore_conf[RTE_MAX_LCORE];

extern void p4_handle_packet(packet* p, unsigned portid);

//=   shared   ================================================================

uint32_t enabled_port_mask = 0;

//=   used only here   ========================================================


extern unsigned int rx_queue_per_lcore;

/* A tsc-based timer responsible for triggering statistics printout */
#define TIMER_MILLISECOND 2000000ULL /* around 1ms at 2 Ghz */
#define MAX_TIMER_PERIOD 86400 /* 1 day max */
int64_t timer_period = 10 * TIMER_MILLISECOND * 1000; /* default period is 10 seconds */

#define MAX_PORTS 16

#define MCAST_CLONE_PORTS       2
#define MCAST_CLONE_SEGS        2

#define RTE_TEST_RX_DESC_DEFAULT 128
#define RTE_TEST_TX_DESC_DEFAULT 512
uint16_t nb_rxd = RTE_TEST_RX_DESC_DEFAULT;
uint16_t nb_txd = RTE_TEST_TX_DESC_DEFAULT;


#define PKT_MBUF_DATA_SIZE      RTE_MBUF_DEFAULT_BUF_SIZE
#define NB_PKT_MBUF     8192

#define HDR_MBUF_DATA_SIZE      (2 * RTE_PKTMBUF_HEADROOM)
#define NB_HDR_MBUF     (NB_PKT_MBUF * MAX_PORTS)

#define NB_CLONE_MBUF   (NB_PKT_MBUF * MCAST_CLONE_PORTS * MCAST_CLONE_SEGS * 2)

#define BURST_TX_DRAIN_US 100 /* TX drain every ~100us */



// note: this much space MUST be able to hold all deparsed content
#define DEPARSE_BUFFER_SIZE 1024
struct rte_mbuf* deparse_mbuf;

static const struct rte_eth_conf port_conf = {
    .rxmode = {
        .split_hdr_size = 0,
        .header_split   = 0, /**< Header Split disabled */
        .hw_ip_checksum = 0, /**< IP checksum offload disabled */
        .hw_vlan_filter = 0, /**< VLAN filtering disabled */
        .jumbo_frame    = 0, /**< Jumbo Frame Support disabled */
        .hw_strip_crc   = 0, /**< CRC stripped by hardware */
    },
    .txmode = {
        .mq_mode = ETH_MQ_TX_NONE,
    },
};

struct rte_mempool * pktmbuf_pool[NB_SOCKETS];

//=============================================================================




/* Send burst of packets on an output interface */
static inline int
send_burst(struct lcore_conf *qconf, uint16_t n, uint8_t port)
{
    struct rte_mbuf **m_table;
    int ret;
    uint16_t queueid;

    queueid = qconf->tx_queue_id[port];
    m_table = (struct rte_mbuf **)qconf->tx_mbufs[port].m_table;

    ret = rte_eth_tx_burst(port, queueid, m_table, n);
    if (unlikely(ret < n)) {
        do {
            rte_pktmbuf_free(m_table[ret]);
        } while (++ret < n);
    }

    return 0;
}

/* Send burst of outgoing packet, if timeout expires. */
static inline void
send_timeout_burst(struct lcore_conf *qconf)
{
        uint64_t cur_tsc;
        uint8_t portid;
        const uint64_t drain_tsc = (rte_get_tsc_hz() + US_PER_S - 1) / US_PER_S * BURST_TX_DRAIN_US;

        cur_tsc = rte_rdtsc();
        if (likely (cur_tsc < qconf->tx_tsc + drain_tsc))
            return;

        for (portid = 0; portid < MAX_PORTS; portid++) {
            if (qconf->tx_mbufs[portid].len != 0) {
                send_burst(qconf, qconf->tx_mbufs[portid].len, portid);
                qconf->tx_mbufs[portid].len = 0; 
            }
        }
        qconf->tx_tsc = cur_tsc;
}



static int
get_socketid(unsigned lcore_id)
{
    if (numa_on)
        return rte_lcore_to_socket_id(lcore_id);
    else
        return 0;
}


static inline void
dbg_print_headers(packet_descriptor_t* pd)
{
    for (int i = 0; i < HEADER_INSTANCE_COUNT; ++i) {
        debug("    :: header %d (type=%d, len=%d) = ", i, pd->headers[i].type, pd->headers[i].length);
        for (int j = 0; j < pd->headers[i].length; ++j) {
            debug("%02x ", ((uint8_t*)(pd->headers[i].pointer))[j]);
        }
        debug("\n");
    }
}

static inline unsigned
deparse_headers(packet_descriptor_t* pd, int socketid)
{
    uint8_t* deparse_buffer = (uint8_t*)rte_pktmbuf_append(deparse_mbuf, DEPARSE_BUFFER_SIZE);
    int len = 0;
    for (int i = 0; i < HEADER_INSTANCE_COUNT; ++i) {
        uint8_t* hdr_ptr = (uint8_t*)(pd->headers[i].pointer);
        unsigned hdr_len = pd->headers[i].length;
        for (int j = 0; j < hdr_len; ++j) {
            *deparse_buffer = *hdr_ptr;
            ++deparse_buffer;
            ++hdr_ptr;
        }
        len += hdr_len;
    }
    return len;
}

/* Get number of bits set. */
static inline uint32_t
bitcnt(uint32_t v)
{
        uint32_t n;

        for (n = 0; v != 0; v &= v - 1, n++)
                ;

        return (n);
}

static void
dpdk_send_packet(struct rte_mbuf *m, uint8_t port, uint32_t lcore_id)
{
    struct lcore_conf *qconf = &lcore_conf[lcore_id];
    uint16_t len = qconf->tx_mbufs[port].len;
    qconf->tx_mbufs[port].m_table[len] = m;
    len++;

    if (unlikely(len == MAX_PKT_BURST)) {
        debug("    :: BURST SENDING DPDK PACKETS - port:%d\n", port);
        send_burst(qconf, MAX_PKT_BURST, port);
        len = 0;
    }

    qconf->tx_mbufs[port].len = len;
}

/* creating replicas of a packet for L2 multicasting */
static inline struct rte_mbuf *
mcast_out_pkt(struct rte_mbuf *pkt, int use_clone)
{
    struct rte_mbuf *pktnew;

    /* If requested, then make a new clone packet. */
    if (use_clone != 0 &&
        (pktnew = rte_pktmbuf_clone(pkt, clone_pool)) == NULL) {
            debug("clone_pool empty\n");
            return (NULL);
    }

    debug("setup cloned packet - metadata-header\n");

    if (use_clone == 0) return pkt;

    /* copy metadata from source packet*/
    pktnew->port = pkt->port;
    pktnew->tx_offload = pkt->tx_offload;
    pktnew->ol_flags = pkt->ol_flags;

    __rte_mbuf_sanity_check(pktnew, 1);

    return pktnew;
}

static void
dpdk_mcast_packet(struct rte_mbuf *m, uint32_t port_mask, uint32_t lcore_id)
{
    struct rte_mbuf *mc;
    uint32_t port_num, use_clone;
    uint8_t port;

    port_num = bitcnt(port_mask);

    /* Should we use rte_pktmbuf_clone() or not. */
    use_clone = (port_num <= MCAST_CLONE_PORTS &&
        m->nb_segs <= MCAST_CLONE_SEGS);

    /* Mark all packet's segments as referenced port_num times */
    if (use_clone == 0)
            rte_pktmbuf_refcnt_update(m, (uint16_t)port_num);

    debug("USE_CLONE = %d\n", use_clone);

    for (port = 0; use_clone != port_mask; port_mask >>= 1, port++) {
        /* Prepare output packet and send it out. */
        if ((port_mask & 1) != 0) {
            debug("MCAST - PORT -%d\n", port);
            if ((mc = mcast_out_pkt(m, use_clone)) != NULL) {
                debug("MCAST mc is ready\n");
                dpdk_send_packet(mc, port, lcore_id);
            } else if (use_clone == 0) {
                rte_pktmbuf_free(m);
            }
        }
    }

    /*
     * If we making clone packets, then, for the last destination port,
     * we can overwrite input packet's metadata.
     */
    if (use_clone != 0)
        dpdk_send_packet(m, port, lcore_id);
    else
        rte_pktmbuf_free(m);
}

static void
dpdk_bcast_packet(struct rte_mbuf *m, uint8_t ingress_port, uint32_t lcore_id)
{
    uint32_t port_num;
    uint8_t port;
    uint32_t port_mask;

    port_num = bitcnt(enabled_port_mask);

    debug("Broadcast - ingress port:%d/%d\n", ingress_port, port_num);

    /* Mark all packet's segments as referenced port_num-1 times */
    rte_pktmbuf_refcnt_update(m, (uint16_t)port_num-1); // it increases the reference num by port_num-1, originally it is 1

    port_mask = enabled_port_mask ^ (1 << ingress_port); // excluding ingress port from outgoing port set 

    for (port = 0; port_mask!=0; port_mask >>= 1, port++) {
        if ((port_mask & 1) != 0) {
                dpdk_send_packet(m, port, lcore_id);
        }
   }

    rte_pktmbuf_free(m); // Since the packet is not sent to the ingress port we should reduce the pointer count by 1

}

#define EXTRACT_EGRESSPORT(p) GET_INT32_AUTO(p, field_instance_standard_metadata_egress_port) 


#define EXTRACT_INGRESSPORT(p) GET_INT32_AUTO(p, field_instance_standard_metadata_ingress_port)


/* Enqueue a single packet, and send burst if queue is filled */
static inline int
send_packets(packet_descriptor_t* pd, int batch_size, int inport)
{
    uint32_t lcore_id = rte_lcore_id();
    int port_list[4] = {1,0,3,2};
    for(int i = 0; i<batch_size; i++) {
        //int port = EXTRACT_EGRESSPORT(&pd[i]);
        //int inport = EXTRACT_INGRESSPORT(&pd[i]);
        //if(port == 100)
        //      debug("Port is 100, broadcast, why ?\n");
	int port = port_list[inport];
        //int port =  (inport + 1) % 4;       //Custom port for forward the packet
        struct rte_mbuf *m = (struct rte_mbuf *)((&pd[i])->wrapper);
        struct lcore_conf *qconf = &lcore_conf[lcore_id];
        uint16_t len = qconf->tx_mbufs[port].len;
        qconf->tx_mbufs[port].m_table[len] = m;
        len++;

        if (unlikely(len == MAX_PKT_BURST)) {
                int ret = send_burst(qconf, MAX_PKT_BURST, port);
                if(ret <= 0)
                        debug("no packet was sent\n");
                len = 0;
        }
        qconf->tx_mbufs[port].len = len;
    }
    return 0;
}

static void
set_metadata_inport(packet_descriptor_t* packet_desc, uint32_t inport)
{
    //modify_field_to_const(packet_desc, field_desc(field_instance_standard_metadata_ingress_port), (uint8_t*)&inport, 2);
    int res32; // needed for the macro
    MODIFY_INT32_INT32_BITS(packet_desc, field_instance_standard_metadata_ingress_port, inport); // TODO fix? LAKI
}

void
packets_received(packet_descriptor_t* pd, int batch_size, packet **p, unsigned portid, struct lcore_conf *conf)
{
    for(int i = 0; i < batch_size; i++) {
        pd[i].data = rte_pktmbuf_mtod(p[i], uint8_t *);
        pd[i].wrapper = p[i];
        set_metadata_inport(&pd[i], portid);
    }
    handle_packet(pd, batch_size, conf->state.tables);
    send_packets(pd, batch_size, portid);
}

void
l2l3_acl_VLAN_ingress_init_table(lookup_table_t** tables) 
{
    struct vlan_ingress_proc_action vvalue = {
  	.action_id = action__nop,
    };

    //Add random entries
    for(int i = 0; i < 4; i++) {
    	for(int j = 0; j < VLAN_INGRESS/4 - 1; j++) {
		uint8_t src[3] = {0};
		src[0] = i;	//ingress port
		src[1] = j;	//vlan id
        	exact_add(tables[TABLE_vlan_ingress_proc], (uint8_t *)&src, (uint8_t *) &vvalue);
	}
    }
    printf("Table 1, vlan ingress, init Done with %d entries\n", VLAN_INGRESS/4 - 1);
}

void
l2l3_acl_MAC_learning_init_table(lookup_table_t** tables) 
{
    struct mac_learning_action mvalue = {
	.action_id = action__nop,
    };

    //Adding random entries
    uint8_t src[6] = {160, 54, 159, 0, 0, 0};
    int j = 0;
    for (j = 0; j < MAC_LEARNING; j++) {
        int number = j, array = 5;
        while (array >= 3) {
            src[array] = number % 256;	//eth src
            number = number / 256;
            array-=1;
        }
        exact_add(tables[TABLE_mac_learning], (uint8_t*) src, (uint8_t *) &mvalue);
    }
    printf("Table 2, mac learning, init Done with %d entries\n", MAC_LEARNING);
}

void
l2l3_acl_routable_init_table(lookup_table_t** tables) 
{
    struct routable_action rvalue = { 
    	.action_id = action_route,
    };
    int divergence = 0;
    //Adding random entries
    uint8_t src[14] = {160, 54, 159, 0, 0, 0, 160, 54, 159, 0, 0, 0, 0, 0};
    for (int j = 0; j < MAC_LEARNING; j++) {
        int number = j, array = 5;
        while (array >= 3) {
            src[array] = number % 256;	//src mac
            number = number / 256;
            array-=1;
        }
        array = 11;
        number = j;
        while (array >= 9) {
            src[array] = number % 256;	//dst mac
            number = number / 256;
            array-=1;
        }
 	for (uint8_t i = 0; i < VLAN_INGRESS/4 - 1; i++) {	
	    src[12] = i;	// vlan id
	    if(divergence % 2 == 0){
		rvalue.action_id = action__nop;
	    }else{
		rvalue.action_id = action_route;
	    }
	    divergence++;
            exact_add(tables[TABLE_routable], (uint8_t*) src, (uint8_t *) &rvalue);
	}
    }
    printf("Table 3, routable, init Done with %d entries\n", ROUTABLE);
}

void
l2l3_acl_switching_init_table(lookup_table_t** tables) 
{
    struct switching_action svalue = {
    	.action_id = action_forward,
    	.forward_params.port[1] = 0,
    };

    uint8_t src[8] = {160, 54, 159, 0, 0, 0, 0, 0};
    for (int j = 0; j < MAC_LEARNING; j++) {
        int number = j, array = 5;
        while (array >= 3) {
            src[array] = number % 256;	//eth dst
            number = number / 256;
            array-=1;
        }
	for (uint8_t i = 0; i < VLAN_INGRESS/4 - 1; i++) {
            src[6] = i;		//vlan id
	    exact_add(tables[TABLE_switching], (uint8_t*) src, (uint8_t *) &svalue); 
       }
    }
    printf("Table 4, switching, init Done with %d entries\n", SWITCHING);
}

void
l2l3_acl_routing_init_table(lookup_table_t** tables) 
{
    struct routing_action rvalue;
    rvalue.action_id = action_set_nhop;
    rvalue.set_nhop_params.smac[0] = rvalue.set_nhop_params.dmac[0] = 160;
    rvalue.set_nhop_params.smac[1] = rvalue.set_nhop_params.dmac[1] = 54;
    rvalue.set_nhop_params.smac[2] = rvalue.set_nhop_params.dmac[2] = 159;
    rvalue.set_nhop_params.smac[3] = rvalue.set_nhop_params.dmac[3] = 0;
    rvalue.set_nhop_params.smac[4] = rvalue.set_nhop_params.dmac[4] = 0;
    rvalue.set_nhop_params.smac[5] = rvalue.set_nhop_params.dmac[5] = 0;

    uint8_t ip[4] = {0,0,0,0};
    uint8_t depth = 24;

    FILE *fd_ipv4_prefix = fopen("/root/workspace/atul/ipv4/uniq_ipv4_rib_201409_1_percent", "r+");
    if(fd_ipv4_prefix == NULL){
         printf("Error: Can't open prefix file\n");
    }

    for(int i = 0;i < ROUTING; i++){
        int oct1, oct2, oct3, oct4, len;
        char temp_buff[64];
        if(fscanf(fd_ipv4_prefix, "%s %d.%d.%d.%d/%d", temp_buff,
             &oct1, &oct2, &oct3, &oct4, &len) == EOF){
             printf("END OF prefix FILE reached\n");
             break;
 	}
	int number = i, array = 5;
	if(number < (MAC_LEARNING)) {
		 while (array >= 3) {
                    rvalue.set_nhop_params.smac[array] = rvalue.set_nhop_params.dmac[array] = number%255;       //TODO:Setting parameters for nexthop in routing
                    number = number / 256;
                    array-=1;
                }
		//rvalue.set_nhop_params.smac[5] = rvalue.set_nhop_params.dmac[5] = i;	//TODO:Setting parameters for nexthop in routing
		rvalue.set_nhop_params.vid[1] = i;
	}

        ip[3] = oct4; ip[2] = oct3; ip[1] = oct2; ip[0] = oct1;	//IP address
	depth = len;	//depth
        lpm_add(tables[TABLE_routing], ip, depth, (uint8_t *) &rvalue);
     }
     fclose(fd_ipv4_prefix);
     printf("Table 5, routing, init Done with %d entries\n", ROUTING);
}

void
l2l3_acl_acl_init_table(lookup_table_t** tables) 
{
    uint8_t src[13] = {0};
    struct acl_action avalue;
    avalue.action_id = action__nop;

/*    uint8_t mask[13] = {255,255,255,255,255,255,255,255,255,255,255,255,255};

    FILE *fd_ipv4_prefix = fopen("/root/workspace/atul/ipv4/uniq_ipv4_rib_201409_1_percent", "r+");
    if(fd_ipv4_prefix == NULL){
         printf("Error: Can't open prefix file\n");
    }
*/
    uint8_t prefix[3] =  {1,100,101};
    uint8_t mask[13] = {255,0,0,0,255,0,0,0,255,255,255,255,255};
    for(int i = 0;i < 3; i++){
        int oct1 = 0, oct2 = 0, oct3 = 0, oct4 = 0, len = 0;

//    for(int i = 0;i < HASH_ENTRIES-2; i++){
//        int oct1, oct2, oct3, oct4, len;
//        char temp_buff[64];
//        if(fscanf(fd_ipv4_prefix, "%s %d.%d.%d.%d/%d", temp_buff,
//             &oct1, &oct2, &oct3, &oct4, &len) == EOF){
//             printf("END OF prefix FILE reached\n");
//             break;
//        }
//        
        oct1 = prefix[i];
        src[3] = src[7] = oct4; 
	src[2] = src[6] = oct3; 
	src[1] = src[5] = oct2; 
	src[0] = src[4] = oct1;
	src[8] = 6;	//IP Protocol
	ternary_add(tables[TABLE_acl], src, mask, (uint8_t *) &avalue);
     }
     //fclose(fd_ipv4_prefix);
     printf("Table 6, acl, init Done with %d entries\n", tables[TABLE_acl]->counter);
}

void
l2l3_acl_VLAN_egress_init_table(lookup_table_t** tables) 
{
    struct vlan_egress_proc_action vvalue = {
	    vvalue.action_id = action__nop,
    };

    //Adding random entries
    uint8_t src[3] = {0, 0, 0}; 
    for (int j = 0; j < VLAN_EGRESS/4; j++) {
        src[0] = 0;
        src[1] = j;
        exact_add(tables[TABLE_vlan_egress_proc], (uint8_t *)&src, (uint8_t *) &vvalue);
    }
    printf("Table 7, vlan egress, init Done with %d entries\n", HASH_ENTRIES-2);
}

void
l2l3_acl_product1_init_table(lookup_table_t** tables)
{
     	/*struct routable_action rvalue = { 
    		.action_id = action_route,
    	};*/
	struct product1_action p1value = {
		.action_id_vlan_ingress = action__nop,
		.action_id_mac_learning = action__nop,
		.action_id_routable = action_route,	
	};
  
        uint8_t key[KEY1] = {0};
        int num_vlan_ingress, num_vid, num_mac;
        //assuming VLAN {0,8} and MAC {0,255}
        num_vlan_ingress = 4;
        num_vid = VLAN_INGRESS/4;//Anmol
        num_mac = MAC_LEARNING;
        int divergence = 0;
	//define output action value

        //static real mac entries
         //60:253:254:22:09:72 - Port 2
        //60:253:254:14:09:74 - Port 3
        //160:54:159:62:235:162 - Port 0
        //160:54:159:62:235:164 - Port 1
       //uint8_t real_macs[4][6] = {{160,54,159,14,54,72}, {160, 54, 159, 14, 54, 74}, {160,54,159,62,235,162}, {160,54,159,62,235,164}};

        //indices for all for-loops
/*        int i = 0, j = 0, k = 0, m = 0;

        for(i = 0; i < num_vid; i++){
                uint16_t vid = i;
                memcpy(&key[0], &vid, 2); //added vid to key[0:1]
                for(j = 0; j < num_vlan_ingress; j++){
                        uint16_t ingress = j;
                        memcpy(&key[2], &ingress, 2);
                        for(k = 0; k < num_mac; k++){
                                int number = k, index = 5;
                                uint8_t src[6] = {0};
                                while (index >= 3) {
                                        src[index] = number % 255;
                                        number = number / 255;
                                        index-=1;
                                }
                                src[0] = 160;
                                src[1] = 54;
                                src[2] = 159;
                                //src[0] = 0; src[1] = 0; src[2] = 0;
                                memcpy(&key[4], src, 6);
                                memcpy(&key[10], src, 6);
				if(divergence % 2 == 0){
					p1value.action_id_routable = action__nop;
				}else{
					p1value.action_id_routable = action_route;
				}
				divergence++;
                                exact_add(tables[TABLE_product1], key, (uint8_t *) &p1value);
                       }//mac src and dst
                }//vlan ingress for loop ends
        }//vid for loop ends
*/
        //for real entries
        uint8_t product1_src[3][16] = {{0, 1, 0, 0, 160, 54, 159, 62, 235, 162, 160, 54, 159, 62, 235, 164}, {0, 2, 0, 1, 160, 54, 159, 62, 235, 164, 160, 54, 159, 62, 235, 162}, {0, 3, 0, 2, 60, 253, 254, 22, 9, 72, 60, 253, 254, 22, 9, 74}};
        exact_add(tables[TABLE_product1], (uint8_t*) product1_src[0], (uint8_t *) &p1value);
        exact_add(tables[TABLE_product1], (uint8_t*) product1_src[1], (uint8_t *) &p1value);
        exact_add(tables[TABLE_product1], (uint8_t*) product1_src[2], (uint8_t *) &p1value);
        
        printf("Product1 table created; entries filled %d\n", tables[TABLE_product1]->counter);

}
 
void
l2l3_acl_product3_init_table(lookup_table_t** tables)
{
     	/*struct routable_action rvalue = { 
    		.action_id = action_route,
    	};*/
	struct product3_action p3value = {
		.action_id_mac_learning = action__nop,
		.action_id_routable = action_route,
	};
        uint8_t key[KEY3] = {0};
        int num_vlan_ingress, num_vid, num_mac;
        //assuming VLAN {0,8} and MAC {0,255}
        num_vlan_ingress = 4;
        num_vid = VLAN_INGRESS/4;//Anmol
        num_mac = MAC_LEARNING;
        int divergence = 0;
	//define output action value
        //static real mac entries
        //60:253:254:22:09:72 - Port 2
        //60:253:254:14:09:74 - Port 3
        //160:54:159:62:235:162 - Port 0
        //160:54:159:62:235:164 - Port 1
 
        //uint8_t real_macs[4][6] = {{160,54,159,14,54,72}, {160, 54, 159, 14, 54, 74}, {160,54,159,62,235,162}, {160,54,159,62,235,164}};
        //indices for all for-loops
/*        int i = 0, j = 0, k = 0, m = 0;
        for(i = 0; i < num_vid; i++){
                uint16_t vid = i;
                memcpy(&key[0], &vid, 2); //added vid to key[0:1]
		for(k = 0; k < num_mac; k++){
			int number = k, index = 5;
			uint8_t src[6] = {0};
			while (index >= 3) {
				src[index] = number % 255;
				number = number / 255;
				index-=1;
			}
			src[0] = 160;
			src[1] = 54;
			src[2] = 159;
			//src[0] = 0; src[1] = 0; src[2] = 0;
			memcpy(&key[2], src, 6);
			memcpy(&key[8], src, 6);
			if(divergence % 2 == 0){
				p3value.action_id_routable = action__nop;
			}else{
				p3value.action_id_routable = action_route;
			}
			divergence++;
			exact_add(tables[TABLE_product3], key, (uint8_t *) &p3value);
	       	}//mac src and dst
        }//vid for loop ends
*/
	uint8_t product3_src[3][14] = {{0, 1, 160, 54, 159, 62, 235, 162, 160, 54, 159, 62, 235, 164}, {0, 2, 160, 54, 159, 62, 235, 164, 160, 54, 159, 62, 235, 162}, {0, 3, 60, 253, 254, 14, 9, 72, 60, 253, 254, 14, 9, 74}};
	exact_add(tables[TABLE_product3], (uint8_t*) product3_src[0], (uint8_t *) &p3value);
	exact_add(tables[TABLE_product3], (uint8_t*) product3_src[1], (uint8_t *) &p3value);
	exact_add(tables[TABLE_product3], (uint8_t*) product3_src[2], (uint8_t *) &p3value);
	
	printf("Product3 table created; entries filled %d\n", tables[TABLE_product3]->counter);
}

void
l2l3_acl_product5_init_table(lookup_table_t** tables)
{
        /*struct mac_learning_action mvalue = {
                .action_id = action__nop,
        };*/
	struct product5_action p5value = {
		.action_id_vlan_ingress = action__nop,
		.action_id_mac_learning = action__nop,
	};

        uint8_t key[KEY5] = {0};
        int num_vlan_ingress, num_vid, num_mac;
        //assuming VLAN {0,8} and MAC {0,255}
        num_vlan_ingress = 4;
        num_vid = VLAN_INGRESS/4;//Anmol
        num_mac = MAC_LEARNING;
        //define output action value

        //static real mac entries
         //60:253:254:22:09:72 - Port 2
        //60:253:254:14:09:74 - Port 3
        //160:54:159:62:235:162 - Port 0
        //160:54:159:62:235:164 - Port 1
       //uint8_t real_macs[4][6] = {{160,54,159,14,54,72}, {160, 54, 159, 14, 54, 74}, {160,54,159,62,235,162}, {160,54,159,62,235,164}};
/*
        //indices for all for-loops
        int i = 0, j = 0, k = 0, m = 0;

        for(i = 0; i < num_vid; i++){
                uint16_t vid = i;
                memcpy(&key[0], &vid, 2); //added vid to key[0:1]
                for(j = 0; j < num_vlan_ingress; j++){
                        uint16_t ingress = j;
                        memcpy(&key[2], &ingress, 2);
                        for(k = 0; k < num_mac; k++){
                                int number = k, index = 5;
                                uint8_t src[6] = {0};
                                while (index >= 3) {
                                        src[index] = number % 255;
                                        number = number / 255;
                                        index-=1;
                                }
                                src[0] = 160;
                                src[1] = 54;
                                src[2] = 159;
                                //src[0] = 0; src[1] = 0; src[2] = 0;
                                memcpy(&key[4], src, 6);

                                exact_add(tables[TABLE_product5], key, (uint8_t *) &p5value);
                       }//mac src
                }//vlan ingress for loop ends
        }//vid for loop ends
*/       
	uint8_t product5_src[3][10] = {{0, 1, 0, 0, 160, 54, 159, 62, 235, 162}, {0, 2, 0, 1, 160, 54, 159, 62, 235, 164}, {0, 3, 0, 2, 60, 253, 254, 14, 9, 72}};
        exact_add(tables[TABLE_product5], (uint8_t*) product5_src[0], (uint8_t *) &p5value);
        exact_add(tables[TABLE_product5], (uint8_t*) product5_src[1], (uint8_t *) &p5value);
        exact_add(tables[TABLE_product5], (uint8_t*) product5_src[2], (uint8_t *) &p5value);
 
	printf("Product5 table created; entries filled %d\n", tables[TABLE_product5]->counter);
}
 
void
l2l3_acl_product2_init_table(lookup_table_t** tables)
{
        /*struct switching_action svalue = {
                .action_id = action_forward,
                .forward_params.port[1] = 0,
        };*/
	struct product2_action p2value = {
		.action_id_switching = action_forward,
		.forward_params.port[1] = 0,
		.action_id_vlan_egress = action__nop,
	};

        uint8_t key[KEY2] = {0};
        int num_vid, num_mac, num_ip;
        //assuming VLAN {0,8} and MAC {0,255}
        num_vid = VLAN_INGRESS/4;//Anmol
        num_mac = MAC_LEARNING;
        num_ip = ROUTING;
        //define output action value
        //60:253:254:22:09:72 - Port 2
        //60:253:254:14:09:74 - Port 3
        //160:54:159:62:235:162 - Port 0
        //160:54:159:62:235:164 - Port 1
 
        //static real mac entries
        //uint8_t real_macs[4][6] = {{160,54,159,14,54,72}, {160, 54, 159, 14, 54, 74}, {160,54,159,62,235,162}, {160,54,159,62,235,164}};

        //indices for all for-loops
/*        int i = 0, j = 0, k = 0, m = 0;
	for(i = 0; i < num_vid; i++){
		uint16_t vid = i;
		memcpy(&key[0], &vid, 2); //added vid to key[0:1]
		for(k = 0; k < num_mac; k++){
			int number = k, index = 5;
			uint8_t src[6] = {0};
			while (index >= 3) {
				src[index] = number % 255;
				number = number / 255;
				index-=1;
			}
			src[0] = 160;
			src[1] = 54;
			src[2] = 159;
			//src[0] = 0; src[1] = 0; src[2] = 0;
			memcpy(&key[2], src, 6);
			exact_add(tables[TABLE_product2], key, (uint8_t *) &p2value);
			}//mac src and dst
       }//vid for loop ends
*/
	uint8_t product2_src[3][8] = {{0, 1, 160, 54, 159, 62, 235, 162}, {0, 2, 160, 54, 159, 62, 235, 164}, {0, 3, 60, 253, 254, 14, 9, 72}};
	p2value.forward_params.port[1] = 1;
	exact_add(tables[TABLE_product2], (uint8_t*) product2_src[0], (uint8_t *) &p2value);
	p2value.forward_params.port[1] = 0;
       	exact_add(tables[TABLE_product2], (uint8_t*) product2_src[1], (uint8_t *) &p2value);
        p2value.forward_params.port[1] = 3;
 	exact_add(tables[TABLE_product2], (uint8_t*) product2_src[2], (uint8_t *) &p2value);
 	printf("Product2 table created; entries filled %d\n", tables[TABLE_product2]->counter);
}
                                                                                                                                          
void
l2l3_acl_product4_init_table(lookup_table_t** tables)
{
        struct vlan_egress_proc_action evalue = {
                .action_id = action__nop,
        };
        uint8_t key[KEY4] = {0};
        int num_vlan_egress, num_vid, num_mac, num_ip;
        //assuming VLAN {0,8} and MAC {0,255}
        num_vlan_egress = 4;
        num_vid = VLAN_INGRESS/4;//Anmol
        num_mac = MAC_LEARNING;
        num_ip = ROUTING;
        //define output action value

        //static real mac entries
        //160:54:159:14:54:72
        //160:54:159:14:54:74
        //160:54:159:62:235:162
        //160:54:159:62:235:164
        //uint8_t real_macs[4][6] = {{160,54,159,14,54,72}, {160, 54, 159, 14, 54, 74}, {160,54,159,62,235,162}, {160,54,159,62,235,164}};

        //indices for all for-loops
        int i = 0, k = 0, m = 0;
        for(i = 0; i < num_vid; i++){
                uint16_t vid = i;
                memcpy(&key[0], &vid, 2); //added vid to key[0:1]
		for(k = 0; k < num_vlan_egress; k++){
			uint16_t egress_port = k;	
			memcpy(&key[2], &egress_port, 2);	
			FILE *fd_ipv4_src_prefix = fopen("/root/workspace/atul/ipv4/uniq_ipv4_rib_201409_1_percent", "r+");
			if(fd_ipv4_src_prefix == NULL){
				printf("Error: Can't open prefix file\n");
			}
			//int prefix_count_in_file = 527961;
			for(m = 0; m < num_ip; m++){
				//printf("%d\n", i);
				int oct1, oct2, oct3, oct4, len;
				char temp_buff[64];
				if(fscanf(fd_ipv4_src_prefix, "%s %d.%d.%d.%d/%d", temp_buff,
					&oct1, &oct2, &oct3, &oct4, &len) == EOF){
					printf("END OF prefix FILE reached\n");
					break;
				}
				key[7] = oct4; key[6] = oct3; key[5] = oct2; key[4] = oct1;
				key[11] = oct4; key[10] = oct3; key[9] = oct2; key[8] = oct1;
				//zeroing out ip for debugging
//                                      key[21] = 0; key[20] = 0; key[19] = 0; key[18] = 0;
//                                        key[25] = 0; key[24] = 0; key[23] = 0; key[22] = 0;
				//set ip proto
				//set tcp src and dst        
//				printf("VID: %d %d\n", key[0], key[1]);
//				print_mac(&key[2]); 
//				printf("IP src: %d %d %d %d IP dst: %d %d %d %d IP Proto: %d TCP src: %d %d TCP dst: %d %d\n", key[8], key[9], key[10],key[11],key[12],key[13],key[14],key[15],key[16],key[17],key[18],key[19],key[20]);  
				exact_add(tables[TABLE_product4], key, (uint8_t *) &evalue);
			}//ip loop ends
			fclose(fd_ipv4_src_prefix);
		}//mac src and dst
       }//vid for loop ends
       printf("Product4 table created; entries filled %d\n", tables[TABLE_product4]->counter);

}

void
l2l3_acl_init_all_real(lookup_table_t** tables) 
{
    struct vlan_ingress_proc_action ivalue = {
  	.action_id = action__nop,
    };
    uint8_t vlan_src[3][3] = {{0,0,1}, {1,0,2}, {2,0,3}};
    //Adding VLAN ingress entries
    exact_add(tables[TABLE_vlan_ingress_proc], (uint8_t *)vlan_src[0], (uint8_t *) &ivalue);
    exact_add(tables[TABLE_vlan_ingress_proc], (uint8_t *)vlan_src[1], (uint8_t *) &ivalue);
    exact_add(tables[TABLE_vlan_ingress_proc], (uint8_t *)vlan_src[2], (uint8_t *) &ivalue);
    printf("Table 1, vlan ingress, init Done with %d entries\n", tables[TABLE_vlan_ingress_proc]->counter);

    struct mac_learning_action mvalue = {
	.action_id = action__nop,
    };
    //Adding MAC learning entries
        //60:253:254:22:09:72 - Port 2
        //60:253:254:14:09:74 - Port 3
        //160:54:159:62:235:162 - Port 0
        //160:54:159:62:235:164 - Port 1
 
    uint8_t mac_src[3][6] = {{160, 54, 159, 62, 235, 162}, {160, 54, 159, 62, 235, 164}, {60, 253, 254, 22, 9, 72}};
    exact_add(tables[TABLE_mac_learning], (uint8_t*) mac_src[0], (uint8_t *) &mvalue);
    exact_add(tables[TABLE_mac_learning], (uint8_t*) mac_src[1], (uint8_t *) &mvalue);
    exact_add(tables[TABLE_mac_learning], (uint8_t*) mac_src[2], (uint8_t *) &mvalue);
    printf("Table 2, mac learning, init Done with %d entries\n", tables[TABLE_mac_learning]->counter);

    //Adding Routable entries 
    struct routable_action routable_value = { 
    	.action_id = action_route,
    };
    uint8_t routable_src[3][14] = {{160, 54, 159, 62, 235, 162, 160, 54, 159, 62, 235, 164, 0, 1}, {160, 54, 159, 62, 235, 164, 160, 54, 159, 62, 235, 162, 0, 2}, {60, 253, 254, 22, 9, 72, 60, 253, 254, 22, 9, 74, 0, 3}};
    exact_add(tables[TABLE_routable], (uint8_t*) routable_src[0], (uint8_t *) &routable_value);
    exact_add(tables[TABLE_routable], (uint8_t*) routable_src[1], (uint8_t *) &routable_value);
    exact_add(tables[TABLE_routable], (uint8_t*) routable_src[2], (uint8_t *) &routable_value); 
    printf("Table 3, routable, init Done with %d entries\n", tables[TABLE_routable]->counter);

    //Adding switching table entries
    struct switching_action svalue = {
    	.action_id = action_forward,
    	.forward_params.port[1] = 1,
    };
    uint8_t switching_src[3][8] = {{160, 54, 159, 62, 235, 164, 0, 1}, {160, 54, 159, 62, 235, 162, 0, 2}, {60, 253, 254, 22, 9, 74, 0, 3}};// first six = MAC, last two = VLAN ID
    exact_add(tables[TABLE_switching], (uint8_t*) switching_src[0], (uint8_t *) &svalue);
    svalue.forward_params.port[1] = 0;
    exact_add(tables[TABLE_switching], (uint8_t*) switching_src[1], (uint8_t *) &svalue);
    svalue.forward_params.port[1] = 3;
    exact_add(tables[TABLE_switching], (uint8_t*) switching_src[2], (uint8_t *) &svalue);
    printf("Table 4, switching, init Done with %d entries\n", tables[TABLE_switching]->counter);
    
    //Adding routing entries
    struct routing_action rvalue;
    uint8_t ip[4] = {0,0,0,0};
    uint8_t depth = 24;
    
    rvalue.action_id = action_set_nhop;
    ip[0] = 192; ip[1] = 168; ip[2] = 0; ip[3] = 1;
    rvalue.set_nhop_params.smac[0] = 160;
    rvalue.set_nhop_params.smac[1] = 54;
    rvalue.set_nhop_params.smac[2] = 159;
    rvalue.set_nhop_params.smac[3] = 62;
    rvalue.set_nhop_params.smac[4] = 235;
    rvalue.set_nhop_params.smac[5] = 162;
    
    rvalue.set_nhop_params.dmac[0] = 160;
    rvalue.set_nhop_params.dmac[1] = 54;
    rvalue.set_nhop_params.dmac[2] = 159;
    rvalue.set_nhop_params.dmac[3] = 62;
    rvalue.set_nhop_params.dmac[4] = 235;
    rvalue.set_nhop_params.dmac[5] = 164;

    lpm_add(tables[TABLE_routing], ip, depth, (uint8_t *) &rvalue);
    
    ip[0] = 192; ip[1] = 168; ip[2] = 1; ip[3] = 1;
    rvalue.set_nhop_params.smac[0] = 160;
    rvalue.set_nhop_params.smac[1] = 54;
    rvalue.set_nhop_params.smac[2] = 159;
    rvalue.set_nhop_params.smac[3] = 62;
    rvalue.set_nhop_params.smac[4] = 235;
    rvalue.set_nhop_params.smac[5] = 164;
    
    rvalue.set_nhop_params.dmac[0] = 160;
    rvalue.set_nhop_params.dmac[1] = 54;
    rvalue.set_nhop_params.dmac[2] = 159;
    rvalue.set_nhop_params.dmac[3] = 62;
    rvalue.set_nhop_params.dmac[4] = 235;
    rvalue.set_nhop_params.dmac[5] = 162;

    lpm_add(tables[TABLE_routing], ip, depth, (uint8_t *) &rvalue);
 
    ip[0] = 192; ip[1] = 168; ip[2] = 2; ip[3] = 1;
    rvalue.set_nhop_params.smac[0] = 60;
    rvalue.set_nhop_params.smac[1] = 253;
    rvalue.set_nhop_params.smac[2] = 254;
    rvalue.set_nhop_params.smac[3] = 22;
    rvalue.set_nhop_params.smac[4] = 9;
    rvalue.set_nhop_params.smac[5] = 72;
    
    rvalue.set_nhop_params.dmac[0] = 60;
    rvalue.set_nhop_params.dmac[1] = 253;
    rvalue.set_nhop_params.dmac[2] = 254;
    rvalue.set_nhop_params.dmac[3] = 22;
    rvalue.set_nhop_params.dmac[4] = 9;
    rvalue.set_nhop_params.dmac[5] = 74;

    lpm_add(tables[TABLE_routing], ip, depth, (uint8_t *) &rvalue); 
    printf("Table 5, routing, init Done with %d entries\n", tables[TABLE_routing]->counter);

    //Adding ACL table entries
    uint8_t src[13] = {0};
    struct acl_action avalue;
    avalue.action_id = action__nop;
    uint8_t prefix[3] =  {1,100,101};
    uint8_t mask[13] = {255,255,255,255,255,255,255,255,255,255,255,255,255};
    src[3] =  192;	src[2] =  168; 	src[1] =  0;  	src[0] =  1;       
    src[7] =  192; 	src[6] =  168; 	src[5] =  1; 	src[4] =  1;
    src[8] = 6;	//IP Protocol
    ternary_add(tables[TABLE_acl], src, mask, (uint8_t *) &avalue);

    src[3] =  192;	src[2] =  168; 	src[1] =  1;  	src[0] =  1;       
    src[7] =  192; 	src[6] =  168; 	src[5] =  0; 	src[4] =  1;
    src[8] = 6;	//IP Protocol
    ternary_add(tables[TABLE_acl], src, mask, (uint8_t *) &avalue);

    src[3] =  192;	src[2] =  168; 	src[1] =  2;  	src[0] =  1;       
    src[7] =  192; 	src[6] =  168; 	src[5] =  3; 	src[4] =  1;
    src[8] = 6;	//IP Protocol
    ternary_add(tables[TABLE_acl], src, mask, (uint8_t *) &avalue);
    printf("Table 6, acl, init Done with %d entries\n", tables[TABLE_acl]->counter);

    struct vlan_egress_proc_action vvalue = {
	    vvalue.action_id = action__nop,
    };

    //Adding VLAN egress entries
    uint8_t vlan_egress_src[3][3] = {{1, 0, 1}, {0, 0, 2}, {3, 0, 3}}; 
    exact_add(tables[TABLE_vlan_egress_proc], (uint8_t *)vlan_egress_src[0], (uint8_t *) &vvalue);
    exact_add(tables[TABLE_vlan_egress_proc], (uint8_t *)vlan_egress_src[1], (uint8_t *) &vvalue);
    exact_add(tables[TABLE_vlan_egress_proc], (uint8_t *)vlan_egress_src[2], (uint8_t *) &vvalue);
    printf("Table 7, vlan egress, init Done with %d entries\n", tables[TABLE_vlan_egress_proc]->counter);
}

void 
l2l3_acl_init_table(lookup_table_t** tables) 
{
    //l2l3_acl_product1_init_table(tables);	// Table 8 // 	[1-3]
    //l2l3_acl_product5_init_table(tables);	// Table 12//	[1-2]
    //l2l3_acl_VLAN_ingress_init_table(tables);	// Table 1 //		
    //l2l3_acl_product3_init_table(tables);	// Table 10//	[2-3]
    //l2l3_acl_MAC_learning_init_table(tables);	// Table 2
    //l2l3_acl_routable_init_table(tables);	// Table 3
    //l2l3_acl_routing_init_table(tables);	// Table 5
    //l2l3_acl_product2_init_table(tables);	// Table 9 //	[5-6]
    //l2l3_acl_switching_init_table(tables);	// Table 4 
    //l2l3_acl_VLAN_egress_init_table(tables);	// Table 7
    //l2l3_acl_acl_init_table(tables); 		// Table 6
    l2l3_acl_init_all_real(tables);
}

void
dpdk_main_loop(void)
{
    uint64_t total=0, iterations=0;
    int batch_size = 0;
    packet *pkts_burst[MAX_PKT_BURST];
    packet *p;
    uint64_t prev_tsc, diff_tsc, cur_tsc;
    unsigned i, j, portid, nb_rx;
    const uint64_t drain_tsc = (rte_get_tsc_hz() + US_PER_S - 1) / US_PER_S * BURST_TX_DRAIN_US;
    uint8_t queueid;

    prev_tsc = 0;

    unsigned lcore_id = rte_lcore_id();
    struct lcore_conf *qconf = &lcore_conf[lcore_id];

    if (qconf->n_rx_queue == 0) {
        RTE_LOG(INFO, P4_FWD, "lcore %u has nothing to do\n", lcore_id);
        return;
    }

    RTE_LOG(INFO, P4_FWD, "entering main loop on lcore %u\n", lcore_id);

    for (i = 0; i < qconf->n_rx_queue; i++) {

        portid = qconf->rx_queue_list[i].port_id;
        queueid = qconf->rx_queue_list[i].queue_id;
        RTE_LOG(INFO, P4_FWD, " -- lcoreid=%u portid=%u rxqueueid=%hhu\n", lcore_id, portid, queueid);
    }

    packet_descriptor_t pd[MAX_PKT_BURST];
    for(int i =0; i < MAX_PKT_BURST; i++) {
            init_dataplane(&pd[i], qconf->state.tables);
    }
    if(lcore_id == 2)
    	l2l3_acl_init_table(qconf->state.tables);

    while (1) {
        cur_tsc = rte_rdtsc();
        /*
         * TX burst queue drain
         */
        diff_tsc = cur_tsc - prev_tsc;
        if (unlikely(diff_tsc > drain_tsc)) {

            for (portid = 0; portid < RTE_MAX_ETHPORTS; portid++) {
                if (qconf->tx_mbufs[portid].len == 0)
                    continue;
                send_burst(qconf,
                         qconf->tx_mbufs[portid].len,
                         (uint8_t) portid);
                qconf->tx_mbufs[portid].len = 0;
            }
            prev_tsc = cur_tsc;
        }

        /*
         * Read packet from RX queues
         */
        for (i = 0; i < qconf->n_rx_queue; i++) {
            portid = qconf->rx_queue_list[i].port_id;
            queueid = qconf->rx_queue_list[i].queue_id;
            nb_rx = rte_eth_rx_burst((uint8_t) portid, queueid,
                         pkts_burst, MAX_PKT_BURST);
            batch_size = nb_rx;
            if(unlikely(batch_size == 0)) continue;
            for(int j = 0; j < batch_size; j++) {
                rte_prefetch0(rte_pktmbuf_mtod( pkts_burst[j], void *));
            }
            packets_received(pd, batch_size, pkts_burst, portid, qconf);
        }
    }
}

static int
launch_one_lcore(__attribute__((unused)) void *dummy)
{
    dpdk_main_loop();
    return 0;
}

int launch_dpdk()
{

    /* Needed for L2 multicasting - e.g. acting as a hub
        cloning headers and sometimes packet data*/
    header_pool = rte_pktmbuf_pool_create("header_pool", NB_HDR_MBUF, 32,
            0, HDR_MBUF_DATA_SIZE, rte_socket_id());

    if (header_pool == NULL)
            rte_exit(EXIT_FAILURE, "Cannot init header mbuf pool\n");

    clone_pool = rte_pktmbuf_pool_create("clone_pool", NB_CLONE_MBUF, 32,
            0, 0, rte_socket_id());

    if (clone_pool == NULL)
            rte_exit(EXIT_FAILURE, "Cannot init clone mbuf pool\n");

    rte_eal_mp_remote_launch(launch_one_lcore, NULL, CALL_MASTER);

    unsigned lcore_id;
    RTE_LCORE_FOREACH_SLAVE(lcore_id) {
        if (rte_eal_wait_lcore(lcore_id) < 0)
            return -1;
    }
    return 0;
}
