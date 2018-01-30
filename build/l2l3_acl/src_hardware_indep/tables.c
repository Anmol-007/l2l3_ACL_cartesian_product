 #include "dataplane.h"// sugar@18
 #include "actions.h"// sugar@19
 #include "data_plane_data.h"// sugar@20
 #include "dpdk_tables.h"

 lookup_table_t table_config[NB_TABLES] = {// sugar@22
 {// sugar@25
  .name= "vlan_ingress_proc",// sugar@26
  .id = TABLE_vlan_ingress_proc,// sugar@27
  .type = LOOKUP_EXACT,// sugar@28
  .key_size = 3,// sugar@29 Shailja
  .val_size = sizeof(struct vlan_ingress_proc_action),// sugar@30
  .min_size = 0, //64,// sugar@31
  .max_size = 2*VLAN_INGRESS //64// sugar@32
 },// sugar@33
 {// sugar@25
  .name= "mac_learning",// sugar@26
  .id = TABLE_mac_learning,// sugar@27
  .type = LOOKUP_EXACT,// sugar@28
  .key_size = 6,// sugar@29
  .val_size = sizeof(struct mac_learning_action),// sugar@30
  .min_size = 0, //4000,// sugar@31
  .max_size = 2*MAC_LEARNING //4000// sugar@32
 },// sugar@33
 {// sugar@25
  .name= "routable",// sugar@26
  .id = TABLE_routable,// sugar@27
  .type = LOOKUP_EXACT,// sugar@28
  .key_size = 14,// sugar@29
  .val_size = sizeof(struct routable_action),// sugar@30
  .min_size = 0, //64,// sugar@31
  .max_size = 2*ROUTABLE //64// sugar@32
 },// sugar@33
 {// sugar@25
  .name= "switching",// sugar@26
  .id = TABLE_switching,// sugar@27
  .type = LOOKUP_EXACT,// sugar@28
  .key_size = 8,// sugar@29
  .val_size = sizeof(struct switching_action),// sugar@30
  .min_size = 0, //4000,// sugar@31
  .max_size = 2*SWITCHING //4000// sugar@32
 },// sugar@33
 {// sugar@25
  .name= "routing",// sugar@26
  .id = TABLE_routing,// sugar@27
  .type = LOOKUP_LPM,// sugar@28
  .key_size = 4,// sugar@29
  .val_size = sizeof(struct routing_action),// sugar@30
  .min_size = 0, //2000,// sugar@31
  .max_size = ROUTING //2000// sugar@32
 },// sugar@33
 {// sugar@25
  .name= "acl",// sugar@26
  .id = TABLE_acl,// sugar@27
  .type = LOOKUP_TERNARY,// sugar@28
  .key_size = 13,// sugar@29
  .val_size = sizeof(struct acl_action),// sugar@30
  .min_size = 0, //1000,// sugar@31
  .max_size = ACL //1000//TODO: uint8_t, can't be more than 255
 },// sugar@33
 {// sugar@25
  .name= "vlan_egress_proc",// sugar@26
  .id = TABLE_vlan_egress_proc,// sugar@27
  .type = LOOKUP_EXACT,// sugar@28
  .key_size = 3,// sugar@29
  .val_size = sizeof(struct vlan_egress_proc_action),// sugar@30
  .min_size = 0, //64,// sugar@31
  .max_size = VLAN_EGRESS //64// sugar@32
 },// sugar@33
 {// sugar@25
  .name= "product1",// sugar@26
  .id = TABLE_product1,// sugar@27
  .type = LOOKUP_EXACT,// sugar@28
  .key_size = KEY1,// sugar@29
  .val_size = sizeof(struct product1_action),// sugar@30
  .min_size = 0, //64,// sugar@31
  .max_size = VLAN_INGRESS*MAC_LEARNING //64// sugar@32
  },// sugar@33
  {// sugar@25
  .name= "product2",// sugar@26
  .id = TABLE_product2,// sugar@27
  .type = LOOKUP_EXACT,// sugar@28
  .key_size = KEY2,// sugar@29
  .val_size = sizeof(struct product2_action),// sugar@30
  .min_size = 0, //64,// sugar@31
  .max_size = VLAN_INGRESS*MAC_LEARNING/4 //64// sugar@32
  },// sugar@33
  {// sugar@25
  .name= "product3",// sugar@26
  .id = TABLE_product3,// sugar@27
  .type = LOOKUP_EXACT,// sugar@28
  .key_size = KEY3,// sugar@29
  .val_size = sizeof(struct product3_action),// sugar@30
  .min_size = 0, //64,// sugar@31
  .max_size = VLAN_INGRESS*MAC_LEARNING/4 //64// sugar@32
  },// sugar@33
  {// sugar@25
  .name= "product4",// sugar@26
  .id = TABLE_product4,// sugar@27
  .type = LOOKUP_EXACT,// sugar@28
  .key_size = KEY4,// sugar@29
  .val_size = sizeof(struct vlan_egress_proc_action),// sugar@30
  .min_size = 0, //64,// sugar@31
  .max_size = ROUTING*VLAN_EGRESS //64// sugar@32
  },// sugar@33
  {// sugar@25
  .name= "product5",// sugar@26
  .id = TABLE_product5,// sugar@27
  .type = LOOKUP_EXACT,// sugar@28
  .key_size = KEY5,// sugar@29
  .val_size = sizeof(struct product5_action),// sugar@30
  .min_size = 0, //64,// sugar@31
  .max_size = VLAN_INGRESS*MAC_LEARNING //64// sugar@32
  },// sugar@33



};// sugar@34
 counter_t counter_config[NB_COUNTERS] = {// sugar@36
 };// sugar@51
 p4_register_t register_config[NB_REGISTERS] = {// sugar@53
 };// sugar@66
