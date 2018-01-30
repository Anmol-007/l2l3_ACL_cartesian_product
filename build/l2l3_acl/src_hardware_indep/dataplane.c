 #include <stdlib.h>// sugar@20
 #include <string.h>// sugar@21
 #include "dpdk_lib.h"// sugar@22
 #include "actions.h"// sugar@23
 #include "key.h"
 #include <rte_ip.h> 
 extern void parse_packet(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@25

 extern void increase_counter (int counterid, int index);// sugar@27

 void apply_table_vlan_ingress_proc(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@30
 void apply_table_mac_learning(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@30
 void apply_table_routable(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@30
 void apply_table_switching(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@30
 void apply_table_routing(packet_descriptor_t **pd, int batch_size, lookup_table_t** tables);// sugar@30
 void apply_table_acl(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@30
 void apply_table_vlan_egress_proc(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@30
 void apply_table_product1(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@30
 void apply_table_product2(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@30
 void apply_table_product3(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@30
 void apply_table_product4(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@30
 void apply_table_product5(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@30

 uint8_t reverse_buffer[14];// sugar@34
 void table_vlan_ingress_proc_key(packet_descriptor_t* pd, int batch_size, uint8_t key[][3]) {// sugar@43
    for(int i = 0; i < batch_size; i++) {
	 int index = 0;
	 EXTRACT_BYTEBUF(&pd[i], field_instance_standard_metadata_ingress_port, &key[i][index]);
	 index += sizeof(uint8_t);
	 EXTRACT_BYTEBUF(&pd[i], field_instance_vlan__vid, &key[i][index])
	 index += sizeof(uint16_t);
    }
 }// sugar@62

 void table_mac_learning_key(packet_descriptor_t* pd, int batch_size, uint8_t key[][6]) {// sugar@43
     for(int i = 0; i < batch_size; i++) {
	 int index = 0;
	 EXTRACT_BYTEBUF(&pd[i], field_instance_ethernet__srcAddr, &key[i][index])// sugar@53
	 index += 6;// sugar@54
    }
 }// sugar@62

 void table_routable_key(packet_descriptor_t* pd, int batch_size, uint8_t key[][14]) {// sugar@43
    for(int i = 0; i < batch_size; i++) {
	 int index = 0;
	 EXTRACT_BYTEBUF(&pd[i], field_instance_ethernet__srcAddr, &key[i][index])// sugar@53
	 index += 6;// sugar@54
	 EXTRACT_BYTEBUF(&pd[i], field_instance_ethernet__dstAddr, &key[i][index])// sugar@53
	 index += 6;// sugar@54
	 EXTRACT_BYTEBUF(&pd[i], field_instance_vlan__vid, &key[i][index])
	 index += sizeof(uint16_t);// sugar@50
    }
 }// sugar@62

 void table_switching_key(packet_descriptor_t* pd, int batch_size, uint8_t key[][8]) {// sugar@43
    for(int i = 0; i < batch_size; i++) {
	 int index = 0;
	 EXTRACT_BYTEBUF(&pd[i], field_instance_ethernet__dstAddr, &key[i][index])// sugar@53
	 index += 6;// sugar@54
	 EXTRACT_BYTEBUF(&pd[i], field_instance_vlan__vid, &key[i][index])// sugar@49
	 index += sizeof(uint16_t);// sugar@50
    }
 }// sugar@62

 void table_routing_key(packet_descriptor_t** pd, int batch_size, uint8_t key[][4]) {// sugar@43
    for(int i = 0 ; i < batch_size; i++) {
// 	EXTRACT_INT32_BITS(pd[i], field_instance_ipv4__dstAddr, *(uint32_t*)key[i])// sugar@49
 	EXTRACT_INT32_BITS(pd[i], field_instance_ipv4__srcAddr, *(uint32_t*)key[i])// sugar@49
 	//key += sizeof(uint32_t);// sugar@50
    }
 }// sugar@62

 void table_acl_key(packet_descriptor_t* pd, int batch_size, uint8_t key[][13]) {// sugar@43
    for(int i = 0; i < batch_size; i++) {
	 int index = 0;
	 EXTRACT_BYTEBUF(&pd[i], field_instance_ipv4__srcAddr, &key[i][index])// sugar@49
	 index += sizeof(uint32_t);// sugar@50
	 EXTRACT_BYTEBUF(&pd[i], field_instance_ipv4__srcAddr, &key[i][index])// sugar@49
	 index += sizeof(uint32_t);// sugar@50
	 EXTRACT_BYTEBUF(&pd[i], field_instance_ipv4__protocol, &key[i][index])// sugar@49
	 index += sizeof(uint8_t);// sugar@50
	 EXTRACT_BYTEBUF(&pd[i], field_instance_l4_metadata__srcPort, &key[i][index])// sugar@49
	 index += sizeof(uint16_t);// sugar@50
	 EXTRACT_BYTEBUF(&pd[i], field_instance_l4_metadata__dstPort, &key[i][index])// sugar@49
	 index += sizeof(uint16_t);// sugar@50
    }
 }// sugar@62

 void table_vlan_egress_proc_key(packet_descriptor_t* pd, int batch_size, uint8_t key[][3]) {// sugar@43
    for(int i = 0; i < batch_size; i++) {
         int index = 0;
         uint8_t egress[2];
	 EXTRACT_BYTEBUF(&pd[i], field_instance_standard_metadata_egress_spec, egress)// sugar@49
         key[i][index] = egress[1];
	 index += sizeof(uint8_t);
	 EXTRACT_BYTEBUF(&pd[i], field_instance_vlan__vid, &key[i][index])// sugar@49
	 index += sizeof(uint16_t);
         //printf("Egress port inside table key extract: %d %d\n"
     }
 }// sugar@62


 void table_product1_key(packet_descriptor_t* pd, int batch_size, uint8_t key[][16]) {// sugar@43
    for(int i = 0; i < batch_size; i++) {
	 int index = 0;
 	 EXTRACT_BYTEBUF(&pd[i], field_instance_vlan__vid, &key[i][index])
	 index += sizeof(uint16_t);
	 EXTRACT_BYTEBUF(&pd[i], field_instance_standard_metadata_ingress_port, &key[i][index]);
	 index += sizeof(uint16_t);
         EXTRACT_BYTEBUF(&pd[i], field_instance_ethernet__srcAddr, &key[i][index])// sugar@53
	 index += 6;// sugar@54
	 EXTRACT_BYTEBUF(&pd[i], field_instance_ethernet__dstAddr, &key[i][index])// sugar@53
	 index += 6;// sugar@54
    }
 }// sugar@62

 void table_product3_key(packet_descriptor_t* pd, int batch_size, uint8_t key[][14]) {// sugar@43
    for(int i = 0; i < batch_size; i++) {
	 int index = 0;
 	 EXTRACT_BYTEBUF(&pd[i], field_instance_vlan__vid, &key[i][index])
	 index += sizeof(uint16_t);
 	 EXTRACT_BYTEBUF(&pd[i], field_instance_ethernet__srcAddr, &key[i][index])// sugar@53
	 index += 6;// sugar@54
	 EXTRACT_BYTEBUF(&pd[i], field_instance_ethernet__dstAddr, &key[i][index])// sugar@53
	 index += 6;// sugar@54
    }
 }// sugar@62


 void table_product2_key(packet_descriptor_t* pd, int batch_size, uint8_t key[][8]) {// sugar@43
    for(int i = 0; i < batch_size; i++) {
	 int index = 0;
	 EXTRACT_BYTEBUF(&pd[i], field_instance_vlan__vid, &key[i][index])
	 index += sizeof(uint16_t);	
	 EXTRACT_BYTEBUF(&pd[i], field_instance_ethernet__dstAddr, &key[i][index])// sugar@53
	 index += 6;// sugar@54
    }
 }// sugar@62

 void table_product4_key(packet_descriptor_t* pd, int batch_size, uint8_t key[batch_size][17]) {// sugar@43
    for(int i = 0; i < batch_size; i++) {
	 int index = 0;
	 EXTRACT_BYTEBUF(&pd[i], field_instance_vlan__vid, &key[i][index])// sugar@49
	 index += sizeof(uint16_t);
	 EXTRACT_BYTEBUF(&pd[i], field_instance_standard_metadata_egress_spec, &key[i][index])// sugar@49
	 index += sizeof(uint16_t);
	 EXTRACT_BYTEBUF(&pd[i], field_instance_ipv4__srcAddr, &key[i][index])// sugar@49
	 index += sizeof(uint32_t);// sugar@50
	 EXTRACT_BYTEBUF(&pd[i], field_instance_ipv4__srcAddr, &key[i][index])// sugar@49
	 index += sizeof(uint32_t);// sugar@50
	 //EXTRACT_BYTEBUF(&pd[i], field_instance_ipv4__protocol, &key[i][index])// sugar@49
	 key[i][index] = 0;
	 index += sizeof(uint8_t);// sugar@50
	 EXTRACT_BYTEBUF(&pd[i], field_instance_l4_metadata__srcPort, &key[i][index])// sugar@49
	 index += sizeof(uint16_t);// sugar@50
	 EXTRACT_BYTEBUF(&pd[i], field_instance_l4_metadata__dstPort, &key[i][index])// sugar@49
	 index += sizeof(uint16_t);// sugar@50
    }
 }// sugar@62

 void table_product5_key(packet_descriptor_t* pd, int batch_size, uint8_t key[][10]) {// sugar@43
    for(int i = 0; i < batch_size; i++) {
         int index = 0;
         EXTRACT_BYTEBUF(&pd[i], field_instance_vlan__vid, &key[i][index])
         index += sizeof(uint16_t);
         EXTRACT_BYTEBUF(&pd[i], field_instance_standard_metadata_ingress_port, &key[i][index]);
         index += sizeof(uint16_t);
         EXTRACT_BYTEBUF(&pd[i], field_instance_ethernet__srcAddr, &key[i][index])// sugar@53
         index += 6;// sugar@54
        }
 }// sugar@62

 void apply_table_vlan_ingress_proc(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@68
 {// sugar@69
     debug("  :::: EXECUTING TABLE vlan_ingress_proc\n");// sugar@70
     uint8_t key[batch_size][3];
     uint8_t* values[batch_size];
     table_vlan_ingress_proc_key(pd, batch_size, key);// sugar@72
     exact_lookup(tables[TABLE_vlan_ingress_proc], batch_size, tables[TABLE_vlan_ingress_proc]->key_size, key, values);
     for(int i = 0; i < batch_size; i++) 
     {
	     struct vlan_ingress_proc_action* res = (struct vlan_ingress_proc_action*)values[i];// sugar@74
	     if(res == NULL) {// sugar@85
	       printf("TABLE_vlan_ingress_proc    :: NO RESULT, NO DEFAULT ACTION\n");// sugar@86
	       printf("vlan extracted %d %d, ingress %d\n", key[i][1], key[i][2], key[i][0]);
	     } else {// sugar@87
	       switch (res->action_id) {// sugar@88
		 case action_add_vlan:// sugar@90
		   debug("    :: EXECUTING ACTION add_vlan...\n");// sugar@91
		   action_code_add_vlan(&pd[i], tables);// sugar@95
		   break;// sugar@96
		 case action__nop:// sugar@90
		   debug("    :: EXECUTING ACTION _nop...\n");// sugar@91
		   action_code__nop(&pd[i], tables);// sugar@95
		   break;// sugar@96
	       }// sugar@97
	     }// sugar@9
    }
    return apply_table_mac_learning(pd, batch_size, tables);// sugar@114
    //return apply_table_product3(pd, batch_size, tables);
 }// sugar@121

 void apply_table_mac_learning(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@68
 {// sugar@69
     debug("  :::: EXECUTING TABLE mac_learning\n");// sugar@70
     uint8_t key[batch_size][6];// sugar@71
     uint8_t* values[batch_size];
     table_mac_learning_key(pd, batch_size, key);// sugar@72
     exact_lookup(tables[TABLE_mac_learning], batch_size, tables[TABLE_mac_learning]->key_size, key, values);
     for(int i = 0; i < batch_size; i++) 
     {
	     struct mac_learning_action* res = (struct mac_learning_action*)values[i];// sugar@74
	     if(res == NULL) {// sugar@85
	       printf("TABLE_mac_learning   :: NO RESULT, NO DEFAULT ACTION.\n");// sugar@86
	     } else {// sugar@87
	       switch (res->action_id) {// sugar@88
		 case action_mac_learn:// sugar@90
		   debug("    :: EXECUTING ACTION mac_learn...\n");// sugar@91
		   action_code_mac_learn(&pd[i], tables);// sugar@95
		   break;// sugar@96
		 case action__nop:// sugar@90
		   debug("    :: EXECUTING ACTION _nop...\n");// sugar@91
		   action_code__nop(&pd[i], tables);// sugar@95
		   break;// sugar@96
	       }// sugar@97
	     }// sugar@98
     }
     return apply_table_routable(pd, batch_size, tables);// sugar@114
 }// sugar@121

 void apply_table_product5(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@68
 {// sugar@69
     debug("  :::: EXECUTING TABLE product1\n");// sugar@70
     uint8_t key[batch_size][10];
     uint8_t* values[batch_size];
     table_product5_key(pd, batch_size, key);// sugar@72
     exact_lookup(tables[TABLE_product5], batch_size, tables[TABLE_product5]->key_size, key, values);// sugar@73
     for(int i = 0; i < batch_size; i++)
     {
             //struct mac_learning_action* res = (struct mac_learning_action*)values[i];// sugar@74
             struct product5_action* res = (struct product5_action*) values[i];
	     if(res == NULL) {// sugar@85
                printf("TABLE_product5    :: NO RESULT, NO DEFAULT ACTION.\n");// sugar@86
                printf("VID: %d %d, Ingress Port: %d %d, mac src", key[i][0], key[i][1], key[i][2], key[i][3]);
                print_mac(&key[i][4]);
             } else {// sugar@87
       	        switch (res->action_id_vlan_ingress) {// sugar@88
		 case action_add_vlan:// sugar@90
		   debug("    :: EXECUTING ACTION add_vlan...\n");// sugar@91
		   action_code_add_vlan(&pd[i], tables);// sugar@95
		   break;// sugar@96
		 case action__nop:// sugar@90
		   debug("    :: EXECUTING ACTION _nop...\n");// sugar@91
		   action_code__nop(&pd[i], tables);// sugar@95
		   break;// sugar@96
	        }// sugar@97
	
	        switch (res->action_id_mac_learning) {// sugar@88
                 case action_mac_learn:// sugar@90
                   debug("    :: EXECUTING ACTION mac_learn...\n");// sugar@91
                   action_code_mac_learn(&pd[i], tables);// sugar@95
                   break;// sugar@96
                 case action__nop:// sugar@90
                   debug("    :: EXECUTING ACTION _nop...\n");// sugar@91
                   action_code__nop(&pd[i], tables);// sugar@95
                   break;// sugar@96
                }// sugar@97
             }// sugar@98
     }
     return apply_table_routable(pd, batch_size, tables);// sugar@114
}// sugar@121

 void apply_table_routable(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@68
 {// sugar@69
     debug("  :::: EXECUTING TABLE routable\n");// sugar@70
     uint8_t key[batch_size][14];
     uint8_t* values[batch_size];
     packet_descriptor_t* diversionQ[batch_size];
     int diversionCount = 0;

     table_routable_key(pd, batch_size, key);// sugar@72
     exact_lookup(tables[TABLE_routable], batch_size, tables[TABLE_routable]->key_size, key, values);// sugar@73
     for(int i = 0; i < batch_size; i++)
     {
             struct routable_action* res = (struct routable_action*)values[i];// sugar@74
             if(res == NULL) {// sugar@85
                printf("TABLE_routable    :: NO RESULT, NO DEFAULT ACTION.\n");// sugar@86
             } else {// sugar@87
               switch (res->action_id) {// sugar@88
                 case action_route:// sugar@90
                   debug("    :: EXECUTING ACTION route...\n");// sugar@91
                   action_code_route(&pd[i], tables);// sugar@95
                   diversionQ[diversionCount] = &pd[i];
                   diversionCount++;
                   break;// sugar@96
                 case action__nop:// sugar@90
                   debug("    :: EXECUTING ACTION _nop...\n");// sugar@91
                   action_code__nop(&pd[i], tables);// sugar@95
                   break;// sugar@96
               }// sugar@97
             }// sugar@98
     }
     if(diversionCount) {
       apply_table_routing(diversionQ, diversionCount, tables);// sugar@114
      }
  //return apply_table_product2(pd, batch_size, tables);
  return apply_table_switching(pd, batch_size, tables);
 }// sugar@121

 void apply_table_product1(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@68
 {// sugar@69
     debug("  :::: EXECUTING TABLE product1\n");// sugar@70
     uint8_t key[batch_size][16];
     uint8_t* values[batch_size];
     packet_descriptor_t* diversionQ[batch_size];
     int diversionCount = 0;

     table_product1_key(pd, batch_size, key);// sugar@72
     exact_lookup(tables[TABLE_product1], batch_size, tables[TABLE_product1]->key_size, key, values);// sugar@73
     for(int i = 0; i < batch_size; i++)
     {
             //struct routable_action* res = (struct routable_action*)values[i];// sugar@74
             struct product1_action* res = (struct product1_action*)values[i];
	     if(res == NULL) {// sugar@85
                printf("TABLE_product1    :: NO RESULT, NO DEFAULT ACTION.\n");// sugar@86
             	printf("VLAN ID %d %d  INGRESS PORT: %d %d and Mac src and dst: ", key[i][0], key[i][1], key[i][2], key[i][3]);
		print_mac(&key[i][4]);
		print_mac(&key[i][10]);
	     } else {// sugar@87
               
		switch (res->action_id_vlan_ingress) {// sugar@88
		 case action_add_vlan:// sugar@90
		   debug("    :: EXECUTING ACTION add_vlan...\n");// sugar@91
		   action_code_add_vlan(&pd[i], tables);// sugar@95
		   break;// sugar@96
		 case action__nop:// sugar@90
		   debug(" Product1:VLAN action   :: EXECUTING ACTION _nop...\n");// sugar@91
		   action_code__nop(&pd[i], tables);// sugar@95
		   break;// sugar@96
	        }// sugar@97
		
		switch (res->action_id_mac_learning) {// sugar@88
		 case action_mac_learn:// sugar@90
		   debug("    :: EXECUTING ACTION mac_learn...\n");// sugar@91
		   action_code_mac_learn(&pd[i], tables);// sugar@95
		   break;// sugar@96
		 case action__nop:// sugar@90
		   debug("    :: EXECUTING ACTION _nop...\n");// sugar@91
		   action_code__nop(&pd[i], tables);// sugar@95
		   break;// sugar@96
	       }// sugar@97

		switch (res->action_id_routable) {// sugar@88
                 case action_route:// sugar@90
                   debug("    :: EXECUTING ACTION route...\n");// sugar@91
                   action_code_route(&pd[i], tables);// sugar@95
                   diversionQ[diversionCount] = &pd[i];
                   diversionCount++;
                   break;// sugar@96
                 case action__nop:// sugar@90
                   debug("    :: EXECUTING ACTION _nop...\n");// sugar@91
                   action_code__nop(&pd[i], tables);// sugar@95
                   break;// sugar@96
               }// sugar@97
             }// sugar@98
     }
     if(diversionCount) {
       apply_table_routing(diversionQ, diversionCount, tables);// sugar@114
      }
  //return apply_table_product2(pd, batch_size, tables);
  return apply_table_switching(pd, batch_size, tables);

}// sugar@121

 void apply_table_product3(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@68
 {// sugar@69
     debug("  :::: EXECUTING TABLE product3\n");// sugar@70
     uint8_t key[batch_size][14];
     uint8_t* values[batch_size];
     packet_descriptor_t* diversionQ[batch_size];
     int diversionCount = 0;

     table_product3_key(pd, batch_size, key);// sugar@72
     exact_lookup(tables[TABLE_product3], batch_size, tables[TABLE_product3]->key_size, key, values);// sugar@73
     for(int i = 0; i < batch_size; i++)
     {
             //struct routable_action* res = (struct routable_action*)values[i];// sugar@74
	     struct product3_action* res = (struct product3_action*)values[i];
             if(res == NULL) {// sugar@85
                printf("TABLE_product3    :: NO RESULT, NO DEFAULT ACTION.\n");// sugar@86
		printf("VID: %d %d Mac src and dst ", key[i][0], key[i][1]);
		print_mac(&key[i][2]);
		print_mac(&key[i][8]);
             } else {// sugar@87
		switch (res->action_id_mac_learning) {// sugar@88
		 case action_mac_learn:// sugar@90
		   debug("    :: EXECUTING ACTION mac_learn...\n");// sugar@91
		   action_code_mac_learn(&pd[i], tables);// sugar@95
		   break;// sugar@96
		 case action__nop:// sugar@90
		   debug("    :: EXECUTING ACTION _nop...\n");// sugar@91
		   action_code__nop(&pd[i], tables);// sugar@95
		   break;// sugar@96
	       }// sugar@97

               switch (res->action_id_routable) {// sugar@88
                 case action_route:// sugar@90
                   debug("    :: EXECUTING ACTION route...\n");// sugar@91
                   action_code_route(&pd[i], tables);// sugar@95
                   diversionQ[diversionCount] = &pd[i];
                   diversionCount++;
                   break;// sugar@96
                 case action__nop:// sugar@90
                   debug("    :: EXECUTING ACTION _nop...\n");// sugar@91
                   action_code__nop(&pd[i], tables);// sugar@95
                   break;// sugar@96
               }// sugar@97
             }// sugar@98
     }
     if(diversionCount) {
       apply_table_routing(diversionQ, diversionCount, tables);// sugar@114
      }
  return apply_table_product2(pd, batch_size, tables);
  //return apply_table_switching(pd, batch_size, tables);

}// sugar@121

 void apply_table_switching(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@68
 {// sugar@69
     debug("  :::: EXECUTING TABLE switching\n");// sugar@70
     uint8_t key[batch_size][8];	// sugar@71
     uint8_t* values[batch_size];
     table_switching_key(pd, batch_size, key);// sugar@72
     exact_lookup(tables[TABLE_switching], batch_size, tables[TABLE_switching]->key_size, key, values);// sugar@73
     for(int i = 0; i < batch_size; i++) 
     {
	     struct switching_action* res = (struct switching_action*)values[i];// sugar@74
	     if(res == NULL) {// sugar@85
	       printf("TABLE_switching    :: NO RESULT, NO DEFAULT ACTION.\n");// sugar@86
	       printf("vlan id %d %d Mac ", key[i][6], key[i][7]);
	       print_mac(key[i]);
	     } else {// sugar@87
	       switch (res->action_id) {// sugar@88
		 case action_forward:// sugar@90
		   debug("    :: EXECUTING ACTION forward...\n");// sugar@91
		   action_code_forward(&pd[i], tables, res->forward_params);// sugar@93
		   break;// sugar@96
		 case action_broadcast:// sugar@90
		   debug("    :: EXECUTING ACTION broadcast...\n");// sugar@91
		   action_code_broadcast(&pd[i], tables);// sugar@95
		   break;// sugar@96
	       }// sugar@97
	     }// sugar@98
     }
     //return apply_table_product4(pd, batch_size, tables); 
     //return apply_table_acl(pd, batch_size, tables);// sugar@114 //TODO:
     return apply_table_vlan_egress_proc(pd, batch_size, tables);
 }// sugar@121

 void apply_table_routing(packet_descriptor_t** pd, int batch_size, lookup_table_t** tables)// sugar@68
 {// sugar@69
     debug("  :::: EXECUTING TABLE routing\n");// sugar@70
     uint8_t key[batch_size][4];// sugar@71
     uint8_t* values[batch_size];
     table_routing_key(pd, batch_size, key);// sugar@72
     lpm_lookup(tables[TABLE_routing], batch_size, tables[TABLE_routing]->key_size, key, values);// sugar@73
     for(int i = 0; i < batch_size; i++)
     {
	     struct routing_action* res = (struct routing_action*)values[i];// sugar@74
	     if(res == NULL) {// sugar@85
	       printf("TABLE_routing    :: NO RESULT, NO DEFAULT ACTION.\n");// sugar@86
	       printf("IP Dst :%d.%d.%d.%d\n", key[i][0], key[i][1], key[i][2], key[i][3] );
	     } else {// sugar@87
	       switch (res->action_id) {// sugar@88
		 case action_set_nhop:// sugar@90
		   debug("    :: EXECUTING ACTION set_nhop...\n");// sugar@91
		   action_code_set_nhop(pd[i], tables, res->set_nhop_params);// sugar@93
		   break;// sugar@96
		 case action__drop:// sugar@90
		   debug("    :: EXECUTING ACTION _drop...\n");// sugar@91
		   action_code__drop(pd[i], tables);// sugar@95
		   break;// sugar@96
	       }// sugar@97
	     }// sugar@98
     }
     //return apply_table_switching(pd, batch_size, tables);// sugar@114
     
 }// sugar@121


static struct acl_action action = {
        .action_id = action__nop,
};

 void apply_table_acl(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@68
 {// sugar@69
     debug("  :::: EXECUTING TABLE acl\n");// sugar@70
     uint8_t key[batch_size][13];// sugar@71
     table_acl_key(pd, batch_size, key);// sugar@72
     for(int i = 0; i < batch_size; i++)
     {
	     key[i][8] = 6; key[i][9] = 0; key[i][10] = 0; key[i][11] = 0; key[i][12] = 0;
	     uint8_t* value = ternary_lookup(tables[TABLE_acl], (uint8_t*)key[i]);// sugar@73
	     struct acl_action* res = (struct acl_action*)value;// sugar@74
	     res = &action;
	     if(res == NULL) {// sugar@85
	       printf("TABLE_acl    :: NO RESULT, NO DEFAULT ACTION.\n");// sugar@86
	     } else {// sugar@87
	       switch (res->action_id) {// sugar@88
		 case action__nop:// sugar@90
		   debug("    :: EXECUTING ACTION _nop...\n");// sugar@91
		   action_code__nop(&pd[i], tables);// sugar@95
		   break;// sugar@96
		 case action__drop:// sugar@90
		   debug("    :: EXECUTING ACTION _drop...\n");// sugar@91
		   action_code__drop(&pd[i], tables);// sugar@95
		   break;// sugar@96
	       }// sugar@97
	     }// sugar@98
     }
     //return apply_table_vlan_egress_proc(pd, batch_size, tables);// sugar@114
 }// sugar@121

 void apply_table_vlan_egress_proc(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@68
 {// sugar@69
     debug("  :::: EXECUTING TABLE vlan_egress_proc\n");// sugar@70
     uint8_t key[batch_size][3];// sugar@71
     uint8_t* values[batch_size];
     table_vlan_egress_proc_key(pd, batch_size, key);// sugar@72
     exact_lookup(tables[TABLE_vlan_egress_proc], batch_size, tables[TABLE_vlan_egress_proc]->key_size, key, values);
     for(int i = 0; i < batch_size; i++)
     {
	     struct vlan_egress_proc_action* res = (struct vlan_egress_proc_action*)values[i];// sugar@74
	     if(res == NULL) {// sugar@85
	       	printf("TABLE_vlan_egress_proc    :: NO RESULT, NO DEFAULT ACTION.\n");// sugar@86
		printf("Egress %d vlan id %d %d\n", key[i][0], key[i][1], key[i][2]);
	     } else {// sugar@87
	       switch (res->action_id) {// sugar@88
		 case action_strip_vlan:// sugar@90
		   debug("    :: EXECUTING ACTION strip_vlan...\n");// sugar@91
		   action_code_strip_vlan(&pd[i], tables);// sugar@95
		   break;// sugar@96
		 case action__nop:// sugar@90
		   debug("    :: EXECUTING ACTION _nop...\n");// sugar@91
		   action_code__nop(&pd[i], tables);// sugar@95
		   break;// sugar@96
	       }// sugar@97
	     }// sugar@98
     }
     return apply_table_acl(pd, batch_size, tables);
 }// sugar@121


 void apply_table_product4(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@68
 {// sugar@69
     debug("  :::: EXECUTING TABLE product4\n");// sugar@70
     uint8_t key[batch_size][17];// sugar@71
     uint8_t* values[batch_size];
     table_product4_key(pd, batch_size, key);// sugar@72
     exact_lookup(tables[TABLE_product4], batch_size, tables[TABLE_product4]->key_size, key, values);
     for(int i = 0; i < batch_size; i++)
     {
	     struct vlan_egress_proc_action* res = (struct vlan_egress_proc_action*)values[i];// sugar@74
	     if(res == NULL) {// sugar@85
	       	printf("TABLE_vlan_egress_proc    :: NO RESULT, NO DEFAULT ACTION.\n");// sugar@86
		printf("VID %d %d egress %d %d\n", key[i][0], key[i][1], key[i][2], key[i][3]);
	     } else {// sugar@87
	       switch (res->action_id) {// sugar@88
		 case action_strip_vlan:// sugar@90
		   debug("    :: EXECUTING ACTION strip_vlan...\n");// sugar@91
		   action_code_strip_vlan(&pd[i], tables);// sugar@95
		   break;// sugar@96
		 case action__nop:// sugar@90
		   debug("    :: EXECUTING ACTION _nop...\n");// sugar@91
		   action_code__nop(&pd[i], tables);// sugar@95
		   break;// sugar@96
	       }// sugar@97
	     }// sugar@98
     }
 }// sugar@121


void apply_table_product2(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@68
{// sugar@69
    debug("  :::: EXECUTING TABLE product2 [5-6]\n");// sugar@70
    uint8_t key[batch_size][8];	// sugar@71
    uint8_t* values[batch_size];
    table_product2_key(pd, batch_size, key);// sugar@72
    exact_lookup(tables[TABLE_product2], batch_size, tables[TABLE_product2]->key_size, key, values);// sugar@73
    for(int i = 0; i < batch_size; i++) 
    {
	    //struct switching_action* res = (struct switching_action*)values[i];// sugar@74
	    struct product2_action* res = (struct product2_action*) values[i];
	    if(res == NULL) {// sugar@85
	      printf("TABLE_product2    :: NO RESULT, NO DEFAULT ACTION.\n");// sugar@86
	      printf("vlan id %d %d Mac ", key[i][0], key[i][1]);
	      print_mac(&key[i][2]);
	    } else {// sugar@87
	      switch (res->action_id_switching) {// sugar@88
		case action_forward:// sugar@90
		  debug("    :: EXECUTING ACTION forward...\n");// sugar@91
		  action_code_forward(&pd[i], tables, res->forward_params);// sugar@93
		  break;// sugar@96
		case action_broadcast:// sugar@90
		  debug("    :: EXECUTING ACTION broadcast...\n");// sugar@91
		  action_code_broadcast(&pd[i], tables);// sugar@95
		  break;// sugar@96
	       }// sugar@97
	       switch (res->action_id_vlan_egress) {// sugar@88
		 case action_strip_vlan:// sugar@90
		   debug("    :: EXECUTING ACTION strip_vlan...\n");// sugar@91
		   action_code_strip_vlan(&pd[i], tables);// sugar@95
		   break;// sugar@96
		 case action__nop:// sugar@90
		   debug("    :: EXECUTING ACTION _nop...\n");// sugar@91
		   action_code__nop(&pd[i], tables);// sugar@95
		   break;// sugar@96
	       }// sugar@97

	    }// sugar@98
    }
    //return apply_table_vlan_egress_proc(pd, batch_size, tables);// sugar@114
    return apply_table_acl(pd, batch_size, tables);
}// sugar@121



 uint16_t csum16_add(uint16_t num1, uint16_t num2) {// sugar@125
     if(num1 == 0) return num2;// sugar@126
     uint32_t tmp_num = num1 + num2;// sugar@127
     while(tmp_num > 0xffff)// sugar@128
         tmp_num = ((tmp_num & 0xffff0000) >> 16) + (tmp_num & 0xffff);// sugar@129
     return (uint16_t)tmp_num;// sugar@130
 }// sugar@131

 uint32_t calculate_ipv4_checksum(packet_descriptor_t* pd) {// sugar@134
   uint32_t res = 0;// sugar@135
   void* payload_ptr;// sugar@136
   uint8_t* buf = malloc((144) / 8);// sugar@153
   memset(buf, 0, (144) / 8);// sugar@154
   if(pd->headers[header_instance_ipv4_].pointer != NULL) {// sugar@209
     memcpy(buf + ((0) / 8), field_desc(pd, field_instance_ipv4__version).byte_addr, ((80) / 8));// sugar@213
     memcpy(buf + ((80) / 8), field_desc(pd, field_instance_ipv4__srcAddr).byte_addr, ((64) / 8));// sugar@213
   }// sugar@216
   res = csum16_add(res, calculate_csum16(buf, (144) / 8));// sugar@219
   res = (res == 0xffff) ? res : ((~res) & 0xffff);// sugar@220
   free(buf);// sugar@224
   return res & 0xffff;// sugar@225
 }// sugar@226

 void reset_headers(packet_descriptor_t* packet_desc) {// sugar@229
 memset(packet_desc->headers[header_instance_standard_metadata].pointer, 0, header_info(header_instance_standard_metadata).bytewidth * sizeof(uint8_t));// sugar@233
 packet_desc->headers[header_instance_ethernet_].pointer = NULL;// sugar@235
 packet_desc->headers[header_instance_vlan_].pointer = NULL;// sugar@235
 packet_desc->headers[header_instance_ipv4_].pointer = NULL;// sugar@235
 packet_desc->headers[header_instance_tcp_].pointer = NULL;// sugar@235
 packet_desc->headers[header_instance_udp_].pointer = NULL;// sugar@235
 memset(packet_desc->headers[header_instance_intrinsic_metadata].pointer, 0, header_info(header_instance_intrinsic_metadata).bytewidth * sizeof(uint8_t));// sugar@233
 memset(packet_desc->headers[header_instance_l4_metadata_].pointer, 0, header_info(header_instance_l4_metadata_).bytewidth * sizeof(uint8_t));// sugar@233
 }// sugar@236
 void init_headers(packet_descriptor_t* packet_desc) {// sugar@237
 packet_desc->headers[header_instance_standard_metadata] = (header_descriptor_t) { .type = header_instance_standard_metadata, .length = header_info(header_instance_standard_metadata).bytewidth,// sugar@241
                               .pointer = malloc(header_info(header_instance_standard_metadata).bytewidth * sizeof(uint8_t)),// sugar@242
                               .var_width_field_bitwidth = 0 };// sugar@243
 packet_desc->headers[header_instance_ethernet_] = (header_descriptor_t) { .type = header_instance_ethernet_, .length = header_info(header_instance_ethernet_).bytewidth, .pointer = NULL,// sugar@245
                               .var_width_field_bitwidth = 0 };// sugar@246
 packet_desc->headers[header_instance_vlan_] = (header_descriptor_t) { .type = header_instance_vlan_, .length = header_info(header_instance_vlan_).bytewidth, .pointer = NULL,// sugar@245
                               .var_width_field_bitwidth = 0 };// sugar@246
 packet_desc->headers[header_instance_ipv4_] = (header_descriptor_t) { .type = header_instance_ipv4_, .length = header_info(header_instance_ipv4_).bytewidth, .pointer = NULL,// sugar@245
                               .var_width_field_bitwidth = 0 };// sugar@246
 packet_desc->headers[header_instance_tcp_] = (header_descriptor_t) { .type = header_instance_tcp_, .length = header_info(header_instance_tcp_).bytewidth, .pointer = NULL,// sugar@245
                               .var_width_field_bitwidth = 0 };// sugar@246
 packet_desc->headers[header_instance_udp_] = (header_descriptor_t) { .type = header_instance_udp_, .length = header_info(header_instance_udp_).bytewidth, .pointer = NULL,// sugar@245
                               .var_width_field_bitwidth = 0 };// sugar@246
 packet_desc->headers[header_instance_intrinsic_metadata] = (header_descriptor_t) { .type = header_instance_intrinsic_metadata, .length = header_info(header_instance_intrinsic_metadata).bytewidth,// sugar@241
                               .pointer = malloc(header_info(header_instance_intrinsic_metadata).bytewidth * sizeof(uint8_t)),// sugar@242
                               .var_width_field_bitwidth = 0 };// sugar@243
 packet_desc->headers[header_instance_l4_metadata_] = (header_descriptor_t) { .type = header_instance_l4_metadata_, .length = header_info(header_instance_l4_metadata_).bytewidth,// sugar@241
                               .pointer = malloc(header_info(header_instance_l4_metadata_).bytewidth * sizeof(uint8_t)),// sugar@242
                               .var_width_field_bitwidth = 0 };// sugar@243
 }// sugar@247


 void init_keyless_tables() {// sugar@255
 }// sugar@263

 void init_dataplane(packet_descriptor_t* pd, lookup_table_t** tables) {// sugar@265
     init_headers(pd);// sugar@266
     reset_headers(pd);// sugar@267
     init_keyless_tables();// sugar@268
     pd->dropped=0;// sugar@269
 }// sugar@270

 void update_packet(packet_descriptor_t* pd) {// sugar@273
     uint32_t value32, res32;// sugar@274
     (void)value32, (void)res32;// sugar@275
 if(pd->fields.attr_field_instance_ethernet__etherType == MODIFIED) {// sugar@280
     value32 = pd->fields.field_instance_ethernet__etherType;// sugar@281
     MODIFY_INT32_INT32_AUTO(pd, field_instance_ethernet__etherType, value32)// sugar@282
 }// sugar@283
 if(pd->fields.attr_field_instance_vlan__vid == MODIFIED) {// sugar@280
     value32 = pd->fields.field_instance_vlan__vid;// sugar@281
     MODIFY_INT32_INT32_AUTO(pd, field_instance_vlan__vid, value32)// sugar@282
 }// sugar@283
 if(pd->fields.attr_field_instance_vlan__etherType == MODIFIED) {// sugar@280
     value32 = pd->fields.field_instance_vlan__etherType;// sugar@281
     MODIFY_INT32_INT32_AUTO(pd, field_instance_vlan__etherType, value32)// sugar@282
 }// sugar@283
 if(pd->fields.attr_field_instance_ipv4__ttl == MODIFIED) {// sugar@280
     value32 = pd->fields.field_instance_ipv4__ttl;// sugar@281
     MODIFY_INT32_INT32_AUTO(pd, field_instance_ipv4__ttl, value32)// sugar@282
 }// sugar@283

 if((GET_INT32_AUTO(pd, field_instance_ipv4__ihl))==(5))// sugar@289
 {// sugar@292
     struct ipv4_hdr* ipv4_hdr = pd->headers[header_instance_ipv4_].pointer;
     uint16_t checksum = rte_ipv4_cksum(ipv4_hdr);
     value32 = checksum;
     MODIFY_INT32_INT32_BITS(pd, field_instance_ipv4__hdrChecksum, value32);

 }// sugar@295
 }// sugar@296


 int verify_packet(packet_descriptor_t* pd) {// sugar@300
   uint32_t value32;// sugar@301
   if((GET_INT32_AUTO(pd, field_instance_ipv4__ihl))==(5))// sugar@306
   {// sugar@309
     EXTRACT_INT32_BITS(pd, field_instance_ipv4__hdrChecksum, value32);// sugar@310
     if(value32 != calculate_ipv4_checksum(pd)) {// sugar@311
       debug("       Checksum verification on field 'ipv4_.hdrChecksum' by 'ipv4_checksum': FAILED\n");// sugar@312
       return 1;// sugar@313
     }// sugar@314
     else debug("       Checksum verification on field 'ipv4_.hdrChecksum' by 'ipv4_checksum': SUCCESSFUL\n");// sugar@315
   }// sugar@316
   return 0;// sugar@317
 }// sugar@318

 void handle_packet(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@322
 {// sugar@323
     
     parse_packet(pd, batch_size, tables);// sugar@327
     for(int i = 0; i < batch_size; i++) {
     	update_packet(&pd[i]);// sugar@328
     }
 }// sugar@329
