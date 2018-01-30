 #ifndef __ACTION_INFO_GENERATED_H__// sugar@14
 #define __ACTION_INFO_GENERATED_H__// sugar@15
 

 #define FIELD(name, length) uint8_t name[(length + 7) / 8];// sugar@18

 enum actions {// sugar@21
 action_add_vlan,// sugar@27
 action__nop,// sugar@27
 action_mac_learn,// sugar@27
 action_route,// sugar@27
 action_forward,// sugar@27
 action_broadcast,// sugar@27
 action_set_nhop,// sugar@27
 action__drop,// sugar@27
 action_strip_vlan,// sugar@27
 };// sugar@28
 struct action_forward_params {// sugar@33
 FIELD(port, 9);// sugar@35
 };// sugar@36
 struct action_set_nhop_params {// sugar@33
 FIELD(smac, 48);// sugar@35
 FIELD(dmac, 48);// sugar@35
 FIELD(vid, 12);// sugar@35
 };// sugar@36
 struct vlan_ingress_proc_action {// sugar@39
     int action_id;// sugar@40
     union {// sugar@41
     };// sugar@45
 };// sugar@46
 struct mac_learning_action {// sugar@39
     int action_id;// sugar@40
     union {// sugar@41
     };// sugar@45
 };// sugar@46
 struct routable_action {// sugar@39
     int action_id;// sugar@40
     union {// sugar@41
     };// sugar@45
 };// sugar@46
 struct switching_action {// sugar@39
     int action_id;// sugar@40
     union {// sugar@41
 struct action_forward_params forward_params;// sugar@44
     };// sugar@45
 };// sugar@46
 struct routing_action {// sugar@39
     int action_id;// sugar@40
     union {// sugar@41
 struct action_set_nhop_params set_nhop_params;// sugar@44
     };// sugar@45
 };// sugar@46
 struct acl_action {// sugar@39
     int action_id;// sugar@40
     union {// sugar@41
     };// sugar@45
 };// sugar@46
 struct vlan_egress_proc_action {// sugar@39
     int action_id;// sugar@40
     union {// sugar@41
     };// sugar@45
 };// sugar@46
 
 struct product1_action {
     int action_id_vlan_ingress;
     int action_id_mac_learning;
     int action_id_routable;
     union {
     };
 };
 struct product3_action {
     int action_id_mac_learning;
     int action_id_routable;
     union{
     };
 };
 struct product2_action {
     int action_id_switching;
     union {
	struct action_forward_params forward_params;
     };
     int action_id_vlan_egress;
 };
 struct product5_action {
     int action_id_vlan_ingress;
     int action_id_mac_learning;
     union {
     };
 };

 void apply_table_vlan_ingress_proc(packet_descriptor_t *pd, int batch_size, lookup_table_t** tables);// sugar@49
 void action_code_add_vlan(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@54
 void action_code__nop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@54
 void apply_table_mac_learning(packet_descriptor_t *pd, int batch_size, lookup_table_t** tables);// sugar@49
 void action_code_mac_learn(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@54
 void action_code__nop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@54
 void apply_table_routable(packet_descriptor_t *pd, int batch_size, lookup_table_t** tables);// sugar@49
 void action_code_route(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@54
 void action_code__nop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@54
 void apply_table_switching(packet_descriptor_t *pd, int batch_size, lookup_table_t** tables);// sugar@49
 void action_code_forward(packet_descriptor_t *pd, lookup_table_t **tables, struct action_forward_params);// sugar@52
 void action_code_broadcast(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@54
 void apply_table_routing(packet_descriptor_t **pd, int batch_size, lookup_table_t** tables);// sugar@49
 void action_code_set_nhop(packet_descriptor_t *pd, lookup_table_t **tables, struct action_set_nhop_params);// sugar@52
 void action_code__drop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@54
 void apply_table_acl(packet_descriptor_t *pd, int batch_size, lookup_table_t** tables);// sugar@49
 void action_code__nop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@54
 void action_code__drop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@54
 void apply_table_vlan_egress_proc(packet_descriptor_t *pd, int batch_size, lookup_table_t** tables);// sugar@49
 void action_code_strip_vlan(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@54
 void action_code__nop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@54
 
 void apply_table_product1(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@68
 void apply_table_product2(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@68
 void apply_table_product3(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@30
 void apply_table_product4(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@30
 void apply_table_product5(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@30



 #endif// sugar@56
