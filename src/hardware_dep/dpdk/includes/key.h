#include<stdio.h>
#include<linux/types.h>
#define ETH_ALEN 6
struct VLAN {
	uint8_t vid; //vlan id
        uint8_t ingress_port; //vlan in port
        uint8_t egress_port;  //vlan out port
};

struct ETH {
	uint8_t* src;   /* Ethernet source address. */
        uint8_t* dst;   /* Ethernet destination address. */
};

struct IP {
	uint8_t proto;   /* IP protocol or lower 8 bits of ARP opcode. */
        uint8_t* src;  /*Source IP*/
        uint8_t* dst;    /*Destination IP*/
};
        
struct TP {
	uint16_t src_port;             /* TCP/UDP/SCTP source port. */
        uint16_t dst_port;             /* TCP/UDP/SCTP destination port. */
};

//define key struct
struct universal_key {
	struct VLAN vlan;
	struct ETH eth;
	struct IP ip;
	struct TP tp;
};/* Ensure that we can do comparisons as longs. */

