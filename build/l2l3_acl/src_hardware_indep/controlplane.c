 #include "dpdk_lib.h"// sugar@22
 #include "actions.h"// sugar@23
 
 extern void table_setdefault_promote  (int tableid, uint8_t* value);// sugar@25
 extern void exact_add_promote  (int tableid, uint8_t* key, uint8_t* value);// sugar@26
 extern void lpm_add_promote    (int tableid, uint8_t* key, uint8_t depth, uint8_t* value);// sugar@27
 extern void ternary_add_promote(int tableid, uint8_t* key, uint8_t* mask, uint8_t* value);// sugar@28

 extern void table_vlan_ingress_proc_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31
 extern void table_mac_learning_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31
 extern void table_routable_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31
 extern void table_switching_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31
 extern void table_routing_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31

 uint8_t reverse_buffer[14];// sugar@35
 void// sugar@39
 vlan_ingress_proc_add(// sugar@40
 uint8_t field_instance_standard_metadata_ingress_port[2],// sugar@43
 uint8_t field_instance_vlan__etherType[2],// sugar@43
 uint8_t field_instance_vlan__vid[2],// sugar@43
 struct vlan_ingress_proc_action action)// sugar@49
 {// sugar@50
     uint8_t key[5];// sugar@51
 memcpy(key+0, field_instance_vlan__etherType, 2);// sugar@56
 memcpy(key+2, field_instance_standard_metadata_ingress_port, 2);// sugar@56
 memcpy(key+4, field_instance_vlan__vid, 2);// sugar@56
 exact_add_promote(TABLE_vlan_ingress_proc, (uint8_t*)key, (uint8_t*)&action);// sugar@74
 }// sugar@75

 void// sugar@77
 vlan_ingress_proc_setdefault(struct vlan_ingress_proc_action action)// sugar@78
 {// sugar@79
     table_setdefault_promote(TABLE_vlan_ingress_proc, (uint8_t*)&action);// sugar@80
 }// sugar@81
 void// sugar@39
 mac_learning_add(// sugar@40
 uint8_t field_instance_ethernet__srcAddr[6],// sugar@43
 struct mac_learning_action action)// sugar@49
 {// sugar@50
     uint8_t key[6];// sugar@51
 memcpy(key+0, field_instance_ethernet__srcAddr, 6);// sugar@56
 exact_add_promote(TABLE_mac_learning, (uint8_t*)key, (uint8_t*)&action);// sugar@74
 }// sugar@75

 void// sugar@77
 mac_learning_setdefault(struct mac_learning_action action)// sugar@78
 {// sugar@79
     table_setdefault_promote(TABLE_mac_learning, (uint8_t*)&action);// sugar@80
 }// sugar@81
 void// sugar@39
 routable_add(// sugar@40
 uint8_t field_instance_ethernet__srcAddr[6],// sugar@43
 uint8_t field_instance_ethernet__dstAddr[6],// sugar@43
 uint8_t field_instance_vlan__vid[2],// sugar@43
 struct routable_action action)// sugar@49
 {// sugar@50
     uint8_t key[14];// sugar@51
 memcpy(key+0, field_instance_ethernet__srcAddr, 6);// sugar@56
 memcpy(key+6, field_instance_ethernet__dstAddr, 6);// sugar@56
 memcpy(key+12, field_instance_vlan__vid, 2);// sugar@56
 exact_add_promote(TABLE_routable, (uint8_t*)key, (uint8_t*)&action);// sugar@74
 }// sugar@75

 void// sugar@77
 routable_setdefault(struct routable_action action)// sugar@78
 {// sugar@79
     table_setdefault_promote(TABLE_routable, (uint8_t*)&action);// sugar@80
 }// sugar@81
 void// sugar@39
 switching_add(// sugar@40
 uint8_t field_instance_ethernet__dstAddr[6],// sugar@43
 uint8_t field_instance_vlan__vid[2],// sugar@43
 struct switching_action action)// sugar@49
 {// sugar@50
     uint8_t key[8];// sugar@51
 memcpy(key+0, field_instance_ethernet__dstAddr, 6);// sugar@56
 memcpy(key+6, field_instance_vlan__vid, 2);// sugar@56
 exact_add_promote(TABLE_switching, (uint8_t*)key, (uint8_t*)&action);// sugar@74
 }// sugar@75

 void// sugar@77
 switching_setdefault(struct switching_action action)// sugar@78
 {// sugar@79
     table_setdefault_promote(TABLE_switching, (uint8_t*)&action);// sugar@80
 }// sugar@81
 void// sugar@39
 routing_add(// sugar@40
 uint8_t field_instance_ipv4__dstAddr[4],// sugar@43
 uint8_t field_instance_ipv4__dstAddr_prefix_length,// sugar@48
 struct routing_action action)// sugar@49
 {// sugar@50
     uint8_t key[4];// sugar@51
 memcpy(key+0, field_instance_ipv4__dstAddr, 4);// sugar@56
 uint8_t prefix_length = 0;// sugar@59
 prefix_length += field_instance_ipv4__dstAddr_prefix_length;// sugar@66
 int c, d;// sugar@67
 for(c = 3, d = 0; c >= 0; c--, d++) *(reverse_buffer+d) = *(key+c);// sugar@68
 for(c = 0; c < 4; c++) *(key+c) = *(reverse_buffer+c);// sugar@69
 lpm_add_promote(TABLE_routing, (uint8_t*)key, prefix_length, (uint8_t*)&action);// sugar@70
 }// sugar@75

 void// sugar@77
 routing_setdefault(struct routing_action action)// sugar@78
 {// sugar@79
     table_setdefault_promote(TABLE_routing, (uint8_t*)&action);// sugar@80
 }// sugar@81
 void// sugar@85
 vlan_ingress_proc_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@86
 uint8_t* field_instance_standard_metadata_ingress_port = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[0])->bitmap);// sugar@90
 uint8_t* field_instance_vlan__vid = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[2])->bitmap);// sugar@90
 if(strcmp("add_vlan", ctrl_m->action_name)==0) {// sugar@95
     struct vlan_ingress_proc_action action;// sugar@96
     action.action_id = action_add_vlan;// sugar@97
     debug("Reply from the control plane arrived.\n");// sugar@101
     debug("Adding new entry to vlan_ingress_proc with action add_vlan\n");// sugar@102
     vlan_ingress_proc_add(// sugar@103
 field_instance_standard_metadata_ingress_port,// sugar@106
 field_instance_vlan__etherType,// sugar@106
 field_instance_vlan__vid,// sugar@106
     action);// sugar@109
 } else// sugar@110
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@95
     struct vlan_ingress_proc_action action;// sugar@96
     action.action_id = action__nop;// sugar@97
     debug("Reply from the control plane arrived.\n");// sugar@101
     debug("Adding new entry to vlan_ingress_proc with action _nop\n");// sugar@102
     vlan_ingress_proc_add(// sugar@103
 field_instance_standard_metadata_ingress_port,// sugar@106
 field_instance_vlan__etherType,// sugar@106
 field_instance_vlan__vid,// sugar@106
     action);// sugar@109
 } else// sugar@110
 debug("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@111
 }// sugar@112
 void// sugar@85
 mac_learning_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@86
 uint8_t* field_instance_ethernet__srcAddr = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[0])->bitmap);// sugar@90
 if(strcmp("mac_learn", ctrl_m->action_name)==0) {// sugar@95
     struct mac_learning_action action;// sugar@96
     action.action_id = action_mac_learn;// sugar@97
     debug("Reply from the control plane arrived.\n");// sugar@101
     debug("Adding new entry to mac_learning with action mac_learn\n");// sugar@102
     mac_learning_add(// sugar@103
 field_instance_ethernet__srcAddr,// sugar@106
     action);// sugar@109
 } else// sugar@110
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@95
     struct mac_learning_action action;// sugar@96
     action.action_id = action__nop;// sugar@97
     debug("Reply from the control plane arrived.\n");// sugar@101
     debug("Adding new entry to mac_learning with action _nop\n");// sugar@102
     mac_learning_add(// sugar@103
 field_instance_ethernet__srcAddr,// sugar@106
     action);// sugar@109
 } else// sugar@110
 debug("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@111
 }// sugar@112
 void// sugar@85
 routable_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@86
 uint8_t* field_instance_ethernet__srcAddr = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[0])->bitmap);// sugar@90
 uint8_t* field_instance_ethernet__dstAddr = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[1])->bitmap);// sugar@90
 uint8_t* field_instance_vlan__vid = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[2])->bitmap);// sugar@90
 if(strcmp("route", ctrl_m->action_name)==0) {// sugar@95
     struct routable_action action;// sugar@96
     action.action_id = action_route;// sugar@97
     debug("Reply from the control plane arrived.\n");// sugar@101
     debug("Adding new entry to routable with action route\n");// sugar@102
     routable_add(// sugar@103
 field_instance_ethernet__srcAddr,// sugar@106
 field_instance_ethernet__dstAddr,// sugar@106
 field_instance_vlan__vid,// sugar@106
     action);// sugar@109
 } else// sugar@110
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@95
     struct routable_action action;// sugar@96
     action.action_id = action__nop;// sugar@97
     debug("Reply from the control plane arrived.\n");// sugar@101
     debug("Adding new entry to routable with action _nop\n");// sugar@102
     routable_add(// sugar@103
 field_instance_ethernet__srcAddr,// sugar@106
 field_instance_ethernet__dstAddr,// sugar@106
 field_instance_vlan__vid,// sugar@106
     action);// sugar@109
 } else// sugar@110
 debug("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@111
 }// sugar@112
 void// sugar@85
 switching_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@86
 uint8_t* field_instance_ethernet__dstAddr = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[0])->bitmap);// sugar@90
 uint8_t* field_instance_vlan__vid = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[1])->bitmap);// sugar@90
 if(strcmp("forward", ctrl_m->action_name)==0) {// sugar@95
     struct switching_action action;// sugar@96
     action.action_id = action_forward;// sugar@97
 uint8_t* port = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;// sugar@99
 memcpy(action.forward_params.port, port, 2);// sugar@100
     debug("Reply from the control plane arrived.\n");// sugar@101
     debug("Adding new entry to switching with action forward\n");// sugar@102
     switching_add(// sugar@103
 field_instance_ethernet__dstAddr,// sugar@106
 field_instance_vlan__vid,// sugar@106
     action);// sugar@109
 } else// sugar@110
 if(strcmp("broadcast", ctrl_m->action_name)==0) {// sugar@95
     struct switching_action action;// sugar@96
     action.action_id = action_broadcast;// sugar@97
     debug("Reply from the control plane arrived.\n");// sugar@101
     debug("Adding new entry to switching with action broadcast\n");// sugar@102
     switching_add(// sugar@103
 field_instance_ethernet__dstAddr,// sugar@106
 field_instance_vlan__vid,// sugar@106
     action);// sugar@109
 } else// sugar@110
 debug("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@111
 }// sugar@112
 void// sugar@85
 routing_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@86
 uint8_t* field_instance_ipv4__dstAddr = (uint8_t*)(((struct p4_field_match_lpm*)ctrl_m->field_matches[0])->bitmap);// sugar@92
 uint16_t field_instance_ipv4__dstAddr_prefix_length = ((struct p4_field_match_lpm*)ctrl_m->field_matches[0])->prefix_length;// sugar@93
 if(strcmp("set_nhop", ctrl_m->action_name)==0) {// sugar@95
     struct routing_action action;// sugar@96
     action.action_id = action_set_nhop;// sugar@97
 uint8_t* smac = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;// sugar@99
 memcpy(action.set_nhop_params.smac, smac, 6);// sugar@100
 uint8_t* dmac = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[1])->bitmap;// sugar@99
 memcpy(action.set_nhop_params.dmac, dmac, 6);// sugar@100
 uint8_t* vid = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[2])->bitmap;// sugar@99
 memcpy(action.set_nhop_params.vid, vid, 2);// sugar@100
     debug("Reply from the control plane arrived.\n");// sugar@101
     debug("Adding new entry to routing with action set_nhop\n");// sugar@102
     routing_add(// sugar@103
 field_instance_ipv4__dstAddr,// sugar@106
 field_instance_ipv4__dstAddr_prefix_length,// sugar@108
     action);// sugar@109
 } else// sugar@110
 if(strcmp("_drop", ctrl_m->action_name)==0) {// sugar@95
     struct routing_action action;// sugar@96
     action.action_id = action__drop;// sugar@97
     debug("Reply from the control plane arrived.\n");// sugar@101
     debug("Adding new entry to routing with action _drop\n");// sugar@102
     routing_add(// sugar@103
 field_instance_ipv4__dstAddr,// sugar@106
 field_instance_ipv4__dstAddr_prefix_length,// sugar@108
     action);// sugar@109
 } else// sugar@110
 debug("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@111
 }// sugar@112
 void// sugar@115
 vlan_ingress_proc_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@116
 debug("Action name: %s\n", ctrl_m->action_name);// sugar@117
 if(strcmp("add_vlan", ctrl_m->action_name)==0) {// sugar@119
     struct vlan_ingress_proc_action action;// sugar@120
     action.action_id = action_add_vlan;// sugar@121
     debug("Message from the control plane arrived.\n");// sugar@125
     debug("Set default action for vlan_ingress_proc with action add_vlan\n");// sugar@126
     vlan_ingress_proc_setdefault( action );// sugar@127
 } else// sugar@128
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@119
     struct vlan_ingress_proc_action action;// sugar@120
     action.action_id = action__nop;// sugar@121
     debug("Message from the control plane arrived.\n");// sugar@125
     debug("Set default action for vlan_ingress_proc with action _nop\n");// sugar@126
     vlan_ingress_proc_setdefault( action );// sugar@127
 } else// sugar@128
 debug("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@129
 }// sugar@130
 void// sugar@115
 mac_learning_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@116
 debug("Action name: %s\n", ctrl_m->action_name);// sugar@117
 if(strcmp("mac_learn", ctrl_m->action_name)==0) {// sugar@119
     struct mac_learning_action action;// sugar@120
     action.action_id = action_mac_learn;// sugar@121
     debug("Message from the control plane arrived.\n");// sugar@125
     debug("Set default action for mac_learning with action mac_learn\n");// sugar@126
     mac_learning_setdefault( action );// sugar@127
 } else// sugar@128
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@119
     struct mac_learning_action action;// sugar@120
     action.action_id = action__nop;// sugar@121
     debug("Message from the control plane arrived.\n");// sugar@125
     debug("Set default action for mac_learning with action _nop\n");// sugar@126
     mac_learning_setdefault( action );// sugar@127
 } else// sugar@128
 debug("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@129
 }// sugar@130
 void// sugar@115
 routable_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@116
 debug("Action name: %s\n", ctrl_m->action_name);// sugar@117
 if(strcmp("route", ctrl_m->action_name)==0) {// sugar@119
     struct routable_action action;// sugar@120
     action.action_id = action_route;// sugar@121
     debug("Message from the control plane arrived.\n");// sugar@125
     debug("Set default action for routable with action route\n");// sugar@126
     routable_setdefault( action );// sugar@127
 } else// sugar@128
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@119
     struct routable_action action;// sugar@120
     action.action_id = action__nop;// sugar@121
     debug("Message from the control plane arrived.\n");// sugar@125
     debug("Set default action for routable with action _nop\n");// sugar@126
     routable_setdefault( action );// sugar@127
 } else// sugar@128
 debug("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@129
 }// sugar@130
 void// sugar@115
 switching_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@116
 debug("Action name: %s\n", ctrl_m->action_name);// sugar@117
 if(strcmp("forward", ctrl_m->action_name)==0) {// sugar@119
     struct switching_action action;// sugar@120
     action.action_id = action_forward;// sugar@121
 uint8_t* port = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;// sugar@123
 memcpy(action.forward_params.port, port, 2);// sugar@124
     debug("Message from the control plane arrived.\n");// sugar@125
     debug("Set default action for switching with action forward\n");// sugar@126
     switching_setdefault( action );// sugar@127
 } else// sugar@128
 if(strcmp("broadcast", ctrl_m->action_name)==0) {// sugar@119
     struct switching_action action;// sugar@120
     action.action_id = action_broadcast;// sugar@121
     debug("Message from the control plane arrived.\n");// sugar@125
     debug("Set default action for switching with action broadcast\n");// sugar@126
     switching_setdefault( action );// sugar@127
 } else// sugar@128
 debug("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@129
 }// sugar@130
 void// sugar@115
 routing_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@116
 debug("Action name: %s\n", ctrl_m->action_name);// sugar@117
 if(strcmp("set_nhop", ctrl_m->action_name)==0) {// sugar@119
     struct routing_action action;// sugar@120
     action.action_id = action_set_nhop;// sugar@121
 uint8_t* smac = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;// sugar@123
 memcpy(action.set_nhop_params.smac, smac, 6);// sugar@124
 uint8_t* dmac = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[1])->bitmap;// sugar@123
 memcpy(action.set_nhop_params.dmac, dmac, 6);// sugar@124
 uint8_t* vid = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[2])->bitmap;// sugar@123
 memcpy(action.set_nhop_params.vid, vid, 2);// sugar@124
     debug("Message from the control plane arrived.\n");// sugar@125
     debug("Set default action for routing with action set_nhop\n");// sugar@126
     routing_setdefault( action );// sugar@127
 } else// sugar@128
 if(strcmp("_drop", ctrl_m->action_name)==0) {// sugar@119
     struct routing_action action;// sugar@120
     action.action_id = action__drop;// sugar@121
     debug("Message from the control plane arrived.\n");// sugar@125
     debug("Set default action for routing with action _drop\n");// sugar@126
     routing_setdefault( action );// sugar@127
 } else// sugar@128
 debug("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@129
 }// sugar@130
 void recv_from_controller(struct p4_ctrl_msg* ctrl_m) {// sugar@133
     debug("MSG from controller %d %s\n", ctrl_m->type, ctrl_m->table_name);// sugar@134
     if (ctrl_m->type == P4T_ADD_TABLE_ENTRY) {// sugar@135
 if (strcmp("vlan_ingress_proc", ctrl_m->table_name) == 0)// sugar@137
     vlan_ingress_proc_add_table_entry(ctrl_m);// sugar@138
 else// sugar@139
 if (strcmp("mac_learning", ctrl_m->table_name) == 0)// sugar@137
     mac_learning_add_table_entry(ctrl_m);// sugar@138
 else// sugar@139
 if (strcmp("routable", ctrl_m->table_name) == 0)// sugar@137
     routable_add_table_entry(ctrl_m);// sugar@138
 else// sugar@139
 if (strcmp("switching", ctrl_m->table_name) == 0)// sugar@137
     switching_add_table_entry(ctrl_m);// sugar@138
 else// sugar@139
 if (strcmp("routing", ctrl_m->table_name) == 0)// sugar@137
     routing_add_table_entry(ctrl_m);// sugar@138
 else// sugar@139
 debug("Table add entry: table name mismatch (%s).\n", ctrl_m->table_name);// sugar@140
     }// sugar@141
     else if (ctrl_m->type == P4T_SET_DEFAULT_ACTION) {// sugar@142
 if (strcmp("vlan_ingress_proc", ctrl_m->table_name) == 0)// sugar@144
     vlan_ingress_proc_set_default_table_action(ctrl_m);// sugar@145
 else// sugar@146
 if (strcmp("mac_learning", ctrl_m->table_name) == 0)// sugar@144
     mac_learning_set_default_table_action(ctrl_m);// sugar@145
 else// sugar@146
 if (strcmp("routable", ctrl_m->table_name) == 0)// sugar@144
     routable_set_default_table_action(ctrl_m);// sugar@145
 else// sugar@146
 if (strcmp("switching", ctrl_m->table_name) == 0)// sugar@144
     switching_set_default_table_action(ctrl_m);// sugar@145
 else// sugar@146
 if (strcmp("routing", ctrl_m->table_name) == 0)// sugar@144
     routing_set_default_table_action(ctrl_m);// sugar@145
 else// sugar@146
 debug("Table setdefault: table name mismatch (%s).\n", ctrl_m->table_name);// sugar@147
     }// sugar@148
 }// sugar@149
 backend bg;// sugar@153
 void init_control_plane()// sugar@154
 {// sugar@155
     debug("Creating control plane connection...\n");// sugar@156
     bg = create_backend(3, 1000, "localhost", 11111, recv_from_controller);// sugar@157
     launch_backend(bg);// sugar@158
 /*// sugar@159
 */// sugar@170
 }// sugar@171
