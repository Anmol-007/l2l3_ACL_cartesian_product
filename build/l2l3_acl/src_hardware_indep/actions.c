 #include "dpdk_lib.h"// sugar@20
 #include "actions.h"// sugar@21
 #include <unistd.h>// sugar@22
 #include <arpa/inet.h>// sugar@23

 extern backend bg;// sugar@25

  void action_code_no_op(packet_descriptor_t* pd, lookup_table_t** tables ) {// sugar@439
     uint32_t value32, res32, mask32;// sugar@440
     (void)value32; (void)res32; (void)mask32;// sugar@441
 }// sugar@451

  void action_code_drop(packet_descriptor_t* pd, lookup_table_t** tables ) {// sugar@439
     uint32_t value32, res32, mask32;// sugar@440
     (void)value32; (void)res32; (void)mask32;// sugar@441
 }// sugar@451

  void action_code__drop(packet_descriptor_t* pd, lookup_table_t** tables ) {// sugar@439
     uint32_t value32, res32, mask32;// sugar@440
     (void)value32; (void)res32; (void)mask32;// sugar@441
  debug("    :: SETTING PACKET TO BE DROPPED\n");// sugar@392
 pd->dropped=1;// sugar@393
// sugar@447
 }// sugar@451

  void action_code__nop(packet_descriptor_t* pd, lookup_table_t** tables ) {// sugar@439
     uint32_t value32, res32, mask32;// sugar@440
     (void)value32; (void)res32; (void)mask32;// sugar@441
 }// sugar@451

  void action_code_add_vlan(packet_descriptor_t* pd, lookup_table_t** tables ) {// sugar@439
     uint32_t value32, res32, mask32;// sugar@440
     (void)value32; (void)res32; (void)mask32;// sugar@441
  EXTRACT_INT32_BITS(pd, field_instance_ethernet__etherType, value32)// sugar@161
 MODIFY_INT32_INT32_BITS(pd, field_instance_vlan__etherType, value32)// sugar@162
// sugar@447
  value32 = 33024;// sugar@144
  pd->fields.field_instance_ethernet__etherType = value32;// sugar@38
 pd->fields.attr_field_instance_ethernet__etherType = MODIFIED;// sugar@39
// sugar@145
// sugar@447
 }// sugar@451

  void action_code_mac_learn(packet_descriptor_t* pd, lookup_table_t** tables ) {// sugar@439
     uint32_t value32, res32, mask32;// sugar@440
     (void)value32; (void)res32; (void)mask32;// sugar@441
   struct type_field_list fields;// sugar@369
    fields.fields_quantity = 3;// sugar@371
    fields.field_offsets = malloc(sizeof(uint8_t*)*fields.fields_quantity);// sugar@372
    fields.field_widths = malloc(sizeof(uint8_t*)*fields.fields_quantity);// sugar@373
    fields.field_offsets[0] = (uint8_t*) field_desc(pd, field_instance_standard_metadata_ingress_port).byte_addr;// sugar@377
    fields.field_widths[0]  =            field_desc(pd, field_instance_standard_metadata_ingress_port).bitwidth;// sugar@378
    fields.field_offsets[1] = (uint8_t*) field_desc(pd, field_instance_ethernet__srcAddr).byte_addr;// sugar@377
    fields.field_widths[1]  =            field_desc(pd, field_instance_ethernet__srcAddr).bitwidth;// sugar@378
    fields.field_offsets[2] = (uint8_t*) field_desc(pd, field_instance_vlan__vid).byte_addr;// sugar@377
    fields.field_widths[2]  =            field_desc(pd, field_instance_vlan__vid).bitwidth;// sugar@378

    generate_digest(bg,"mac_learn_digest",0,&fields); sleep(1);// sugar@384
// sugar@447
 }// sugar@451

  void action_code_route(packet_descriptor_t* pd, lookup_table_t** tables ) {// sugar@439
     uint32_t value32, res32, mask32;// sugar@440
     (void)value32; (void)res32; (void)mask32;// sugar@441
 }// sugar@451

  void action_code_forward(packet_descriptor_t* pd, lookup_table_t** tables , struct action_forward_params parameters) {// sugar@439
     uint32_t value32, res32, mask32;// sugar@440
     (void)value32; (void)res32; (void)mask32;// sugar@441
//     printf("Switching set port: %d %d\n", parameters.port[0], parameters.port[1]); 
//  MODIFY_INT32_BYTEBUF(pd, field_instance_standard_metadata_egress_spec, parameters.port, 2)// sugar@187
   memcpy(field_desc(pd, field_instance_standard_metadata_egress_spec).byte_addr, parameters.port, 2); 
  // uint8_t val[2];
  // EXTRACT_BYTEBUF(pd, field_instance_standard_metadata_egress_spec, val);
  // printf("Egress port after setting: %d %d\n", val[0], val[1] );


// sugar@447
 }// sugar@451

  void action_code_broadcast(packet_descriptor_t* pd, lookup_table_t** tables ) {// sugar@439
     uint32_t value32, res32, mask32;// sugar@440
     (void)value32; (void)res32; (void)mask32;// sugar@441
  value32 = 1;// sugar@144
  MODIFY_INT32_INT32_AUTO(pd, field_instance_intrinsic_metadata_mcast_grp, value32)// sugar@41
// sugar@145
// sugar@447
 }// sugar@451

  void action_code_set_nhop(packet_descriptor_t* pd, lookup_table_t** tables , struct action_set_nhop_params parameters) {// sugar@439
     uint32_t value32, res32, mask32;// sugar@440
     (void)value32; (void)res32; (void)mask32;// sugar@441
  if(6 < field_desc(pd, field_instance_ethernet__srcAddr).bytewidth) {// sugar@191
     MODIFY_BYTEBUF_BYTEBUF(pd, field_instance_ethernet__srcAddr, parameters.smac, 6);                // sugar@192
 } else {// sugar@193
     MODIFY_BYTEBUF_BYTEBUF(pd, field_instance_ethernet__srcAddr, parameters.smac + (6 - field_desc(pd, field_instance_ethernet__srcAddr).bytewidth), field_desc(pd, field_instance_ethernet__srcAddr).bytewidth)// sugar@194
 }// sugar@195
// sugar@447
  if(6 < field_desc(pd, field_instance_ethernet__dstAddr).bytewidth) {// sugar@191
     MODIFY_BYTEBUF_BYTEBUF(pd, field_instance_ethernet__dstAddr, parameters.dmac, 6);                // sugar@192
 } else {// sugar@193
     MODIFY_BYTEBUF_BYTEBUF(pd, field_instance_ethernet__dstAddr, parameters.dmac + (6 - field_desc(pd, field_instance_ethernet__dstAddr).bytewidth), field_desc(pd, field_instance_ethernet__dstAddr).bytewidth)// sugar@194
 }// sugar@195
// sugar@447
  //MODIFY_INT32_BYTEBUF(pd, field_instance_vlan__vid, parameters.vid, 2)// sugar@187 //TODO:Shailja
// sugar@447
  value32 = -1;// sugar@214
  res32 = pd->fields.field_instance_ipv4__ttl;// sugar@50
 pd->fields.attr_field_instance_ipv4__ttl = MODIFIED;// sugar@51
// sugar@216
 value32 += res32;// sugar@220
  pd->fields.field_instance_ipv4__ttl = value32;// sugar@38
 pd->fields.attr_field_instance_ipv4__ttl = MODIFIED;// sugar@39
// sugar@221
// sugar@447
 }// sugar@451

  void action_code_strip_vlan(packet_descriptor_t* pd, lookup_table_t** tables ) {// sugar@439
     uint32_t value32, res32, mask32;// sugar@440
     (void)value32; (void)res32; (void)mask32;// sugar@441
  EXTRACT_INT32_BITS(pd, field_instance_vlan__etherType, value32)// sugar@161
 MODIFY_INT32_INT32_BITS(pd, field_instance_ethernet__etherType, value32)// sugar@162
// sugar@447
 }// sugar@451

