 #include "dpdk_lib.h"// sugar@62
 #include "actions.h" // apply_table_* and action_code_*// sugar@63

 extern int verify_packet(packet_descriptor_t* pd);// sugar@65

 void print_mac(uint8_t* v) { printf("%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX\n", v[0], v[1], v[2], v[3], v[4], v[5]); }// sugar@67
 void print_ip(uint8_t* v) { printf("%d.%d.%d.%d\n",v[0],v[1],v[2],v[3]); }// sugar@68
 
 static inline void p4_pe_header_too_short(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 }// sugar@77
 static inline void p4_pe_default(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 }// sugar@77
 static inline void p4_pe_checksum(packet_descriptor_t *pd) {// sugar@72
 //pd->dropped = 1;// sugar@74
 }// sugar@77
 static inline void p4_pe_unhandled_select(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 }// sugar@77
 static inline void p4_pe_index_out_of_bounds(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 }// sugar@77
 static inline void p4_pe_header_too_long(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 }// sugar@77
 static inline void p4_pe_out_of_packet(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 }// sugar@77
 static void// sugar@81
 extract_header_standard_metadata(uint8_t* buf, packet_descriptor_t* pd) {// sugar@82
     pd->headers[header_instance_standard_metadata].pointer = buf;// sugar@83
 }// sugar@90
 
 static void// sugar@81
 extract_header_ethernet_(uint8_t* buf, packet_descriptor_t* pd) {// sugar@82
     pd->headers[header_instance_ethernet_].pointer = buf;// sugar@83
 }// sugar@90
 
 static void// sugar@81
 extract_header_vlan_(uint8_t* buf, packet_descriptor_t* pd) {// sugar@82
     pd->headers[header_instance_vlan_].pointer = buf;// sugar@83
 }// sugar@90
 
 static void// sugar@81
 extract_header_ipv4_(uint8_t* buf, packet_descriptor_t* pd) {// sugar@82
     pd->headers[header_instance_ipv4_].pointer = buf;// sugar@83
 }// sugar@90
 
 static void// sugar@81
 extract_header_tcp_(uint8_t* buf, packet_descriptor_t* pd) {// sugar@82
     pd->headers[header_instance_tcp_].pointer = buf;// sugar@83
 }// sugar@90
 
 static void// sugar@81
 extract_header_udp_(uint8_t* buf, packet_descriptor_t* pd) {// sugar@82
     pd->headers[header_instance_udp_].pointer = buf;// sugar@83
 }// sugar@90
 
 static void// sugar@81
 extract_header_intrinsic_metadata(uint8_t* buf, packet_descriptor_t* pd) {// sugar@82
     pd->headers[header_instance_intrinsic_metadata].pointer = buf;// sugar@83
 }// sugar@90
 
 static void// sugar@81
 extract_header_l4_metadata_(uint8_t* buf, packet_descriptor_t* pd) {// sugar@82
     pd->headers[header_instance_l4_metadata_].pointer = buf;// sugar@83
 }// sugar@90
 
 static void parse_state_start(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@94
 static void parse_state_parse_ethernet(packet_descriptor_t* pd, int batch_size, uint8_t* buf[], lookup_table_t** tables);// sugar@94
 static void parse_state_parse_vlan(packet_descriptor_t* pd, int batch_size, uint8_t* buf[], lookup_table_t** tables);// sugar@94
 static void parse_state_parse_ipv4(packet_descriptor_t* pd, int batch_size, uint8_t* buf[], lookup_table_t** tables);// sugar@94
 static void parse_state_parse_tcp(packet_descriptor_t* pd, int batch_size, uint8_t* buf[], lookup_table_t** tables);// sugar@94

 static inline void build_key_parse_ethernet(packet_descriptor_t *pd, uint8_t *buf, uint8_t *key) {// sugar@100
 EXTRACT_INT32_BITS(pd, field_instance_ethernet__etherType, *(uint32_t*)key)// sugar@109
 key += sizeof(uint32_t);// sugar@110
 }// sugar@119
 static inline void build_key_parse_vlan(packet_descriptor_t *pd, uint8_t *buf, uint8_t *key) {// sugar@100
 EXTRACT_INT32_BITS(pd, field_instance_vlan__etherType, *(uint32_t*)key)// sugar@109
 key += sizeof(uint32_t);// sugar@110
 }// sugar@119
 static inline void build_key_parse_ipv4(packet_descriptor_t *pd, uint8_t *buf, uint8_t *key) {// sugar@100
 EXTRACT_INT32_BITS(pd, field_instance_ipv4__ihl, *(uint32_t*)key)// sugar@109
 key += sizeof(uint32_t);// sugar@110
 EXTRACT_INT32_BITS(pd, field_instance_ipv4__protocol, *(uint32_t*)key)// sugar@109
 key += sizeof(uint32_t);// sugar@110
 }// sugar@119

 static void parse_state_start(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@122
 {// sugar@123
  uint8_t* buf[batch_size];
  for(int i = 0; i < batch_size; i++) {
	buf[i] = (uint8_t*) pd[i].data;
  }
  return parse_state_parse_ethernet(pd, batch_size, buf, tables);// sugar@21
 }// sugar@189
 
 static void parse_state_parse_ethernet(packet_descriptor_t* pd, int batch_size, uint8_t* buf[], lookup_table_t** tables)// sugar@122
 {// sugar@123
     for(int i = 0; i < batch_size; i++) 
     {
	uint32_t value32;
     	extract_header_ethernet_(buf[i], &pd[i]);// sugar@130
     	buf[i] += (&pd[i])->headers[header_instance_ethernet_].length;// sugar@131
 	EXTRACT_INT32_AUTO(&pd[i], field_instance_ethernet__etherType, value32)// sugar@135
 	(&pd[i])->fields.field_instance_ethernet__etherType = value32;// sugar@136
 	(&pd[i])->fields.attr_field_instance_ethernet__etherType = 0;// sugar@137
 	uint8_t key[2];// sugar@157
 	build_key_parse_ethernet(&pd[i], buf[i], key);// sugar@158
     }
     return parse_state_parse_vlan(pd, batch_size, buf, tables);// sugar@21
 }// sugar@189
 
 static void parse_state_parse_vlan(packet_descriptor_t* pd, int batch_size, uint8_t* buf[], lookup_table_t** tables)// sugar@122
 {// sugar@123
    for(int i = 0; i < batch_size; i++) {
     	uint32_t value32;// sugar@124
     	(void)value32;// sugar@125
     	extract_header_vlan_(buf[i], &pd[i]);// sugar@130
     	buf[i] += (&pd[i])->headers[header_instance_vlan_].length;// sugar@131
 	EXTRACT_INT32_AUTO(&pd[i], field_instance_vlan__vid, value32)// sugar@135
 	(&pd[i])->fields.field_instance_vlan__vid = value32;// sugar@136
 	(&pd[i])->fields.attr_field_instance_vlan__vid = 0;// sugar@137
 	EXTRACT_INT32_AUTO(&pd[i], field_instance_vlan__etherType, value32)// sugar@135
 	(&pd[i])->fields.field_instance_vlan__etherType = value32;// sugar@136
 	(&pd[i])->fields.attr_field_instance_vlan__etherType = 0;// sugar@137
 	uint8_t key[2];// sugar@157
 	build_key_parse_vlan(&pd[i], buf[i], key);// sugar@158
    }
    return parse_state_parse_ipv4(pd, batch_size, buf, tables);// sugar@21
 }// sugar@189
 
 static void parse_state_parse_ipv4(packet_descriptor_t* pd, int batch_size, uint8_t* buf[], lookup_table_t** tables)// sugar@122
 {// sugar@123
    for(int i = 0; i < batch_size; i++) {
     	uint32_t value32;// sugar@124
     	(void)value32;// sugar@125
     	extract_header_ipv4_(buf[i], &pd[i]);// sugar@130
     	buf[i] += (&pd[i])->headers[header_instance_ipv4_].length;// sugar@131
 	EXTRACT_INT32_AUTO(&pd[i], field_instance_ipv4__ttl, value32)// sugar@135
	(&pd[i])->fields.field_instance_ipv4__ttl = value32;// sugar@136
	(&pd[i])->fields.attr_field_instance_ipv4__ttl = 0;// sugar@137
 	uint8_t key[2];// sugar@157
 	build_key_parse_ipv4(&pd[i], buf[i], key);// sugar@158
    }
    return parse_state_parse_tcp(pd, batch_size, buf, tables);// sugar@21
 }// sugar@189
 
 static void parse_state_parse_tcp(packet_descriptor_t* pd, int batch_size, uint8_t* buf[], lookup_table_t** tables)// sugar@122
 {// sugar@123
    for(int i = 0 ; i < batch_size; i++) {
     	uint32_t value32;// sugar@124
     	(void)value32;// sugar@125
     	extract_header_tcp_(buf[i], &pd[i]);// sugar@130
     	buf += (&pd[i])->headers[header_instance_tcp_].length;// sugar@131
    }
 //return apply_table_product5(pd, batch_size, tables); //Anmol
 //return apply_table_product1(pd, batch_size, tables); 
 return apply_table_vlan_ingress_proc(pd, batch_size, tables);// sugar@27
 }// sugar@189
 
 void parse_packet(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables) {// sugar@192
     parse_state_start(pd, batch_size, tables);// sugar@193
 }// sugar@194
