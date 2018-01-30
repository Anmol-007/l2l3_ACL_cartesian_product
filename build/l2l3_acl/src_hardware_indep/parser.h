 #ifndef __HEADER_INFO_H__// sugar@95
 #define __HEADER_INFO_H__// sugar@96

 #define MODIFIED 1// sugar@98

 typedef struct parsed_fields_s {// sugar@100
 uint32_t field_instance_ethernet__etherType;// sugar@104
 uint8_t attr_field_instance_ethernet__etherType;// sugar@105
 uint32_t field_instance_vlan__vid;// sugar@104
 uint8_t attr_field_instance_vlan__vid;// sugar@105
 uint32_t field_instance_vlan__etherType;// sugar@104
 uint8_t attr_field_instance_vlan__etherType;// sugar@105
 uint32_t field_instance_ipv4__ttl;// sugar@104
 uint8_t attr_field_instance_ipv4__ttl;// sugar@105
 } parsed_fields_t;// sugar@106
 
 #define HEADER_INSTANCE_COUNT 8// sugar@108
 #define HEADER_STACK_COUNT 0// sugar@109
 #define FIELD_INSTANCE_COUNT 47// sugar@110
 
 enum header_stack_e {
  header_stack_
};

// sugar@112
 enum header_instance_e {
  header_instance_standard_metadata,
  header_instance_ethernet_,
  header_instance_vlan_,
  header_instance_ipv4_,
  header_instance_tcp_,
  header_instance_udp_,
  header_instance_intrinsic_metadata,
  header_instance_l4_metadata_
};

// sugar@113
 enum field_instance_e {
  field_instance_standard_metadata_ingress_port,
  field_instance_standard_metadata_packet_length,
  field_instance_standard_metadata_egress_spec,
  field_instance_standard_metadata_egress_port,
  field_instance_standard_metadata_egress_instance,
  field_instance_standard_metadata_instance_type,
  field_instance_standard_metadata_clone_spec,
  field_instance_standard_metadata__padding,
  field_instance_ethernet__dstAddr,
  field_instance_ethernet__srcAddr,
  field_instance_ethernet__etherType,
  field_instance_vlan__pcp,
  field_instance_vlan__cfi,
  field_instance_vlan__vid,
  field_instance_vlan__etherType,
  field_instance_ipv4__version,
  field_instance_ipv4__ihl,
  field_instance_ipv4__diffserv,
  field_instance_ipv4__totalLen,
  field_instance_ipv4__identification,
  field_instance_ipv4__flags,
  field_instance_ipv4__fragOffset,
  field_instance_ipv4__ttl,
  field_instance_ipv4__protocol,
  field_instance_ipv4__hdrChecksum,
  field_instance_ipv4__srcAddr,
  field_instance_ipv4__dstAddr,
  field_instance_tcp__srcPort,
  field_instance_tcp__dstPort,
  field_instance_tcp__seqNo,
  field_instance_tcp__ackNo,
  field_instance_tcp__dataOffset,
  field_instance_tcp__res,
  field_instance_tcp__flags,
  field_instance_tcp__window,
  field_instance_tcp__checksum,
  field_instance_tcp__urgentPtr,
  field_instance_udp__srcPort,
  field_instance_udp__dstPort,
  field_instance_udp__length_,
  field_instance_udp__checksum,
  field_instance_intrinsic_metadata_mcast_grp,
  field_instance_intrinsic_metadata_egress_rid,
  field_instance_intrinsic_metadata_mcast_hash,
  field_instance_intrinsic_metadata_lf_field_list,
  field_instance_l4_metadata__srcPort,
  field_instance_l4_metadata__dstPort
};

// sugar@114
 typedef enum header_instance_e header_instance_t;// sugar@115
 typedef enum field_instance_e field_instance_t;// sugar@116
 static const int header_instance_byte_width[HEADER_INSTANCE_COUNT] = {
  20 /* header_instance_standard_metadata */,
  14 /* header_instance_ethernet_ */,
  4 /* header_instance_vlan_ */,
  20 /* header_instance_ipv4_ */,
  20 /* header_instance_tcp_ */,
  8 /* header_instance_udp_ */,
  7 /* header_instance_intrinsic_metadata */,
  4 /* header_instance_l4_metadata_ */
};

// sugar@117
 static const int header_instance_is_metadata[HEADER_INSTANCE_COUNT] = {
  1 /* header_instance_standard_metadata */,
  0 /* header_instance_ethernet_ */,
  0 /* header_instance_vlan_ */,
  0 /* header_instance_ipv4_ */,
  0 /* header_instance_tcp_ */,
  0 /* header_instance_udp_ */,
  1 /* header_instance_intrinsic_metadata */,
  1 /* header_instance_l4_metadata_ */
};

// sugar@118
 static const int header_instance_var_width_field[HEADER_INSTANCE_COUNT] = {
  -1 /* fixed-width header */ /* header_instance_standard_metadata */,
  -1 /* fixed-width header */ /* header_instance_ethernet_ */,
  -1 /* fixed-width header */ /* header_instance_vlan_ */,
  -1 /* fixed-width header */ /* header_instance_ipv4_ */,
  -1 /* fixed-width header */ /* header_instance_tcp_ */,
  -1 /* fixed-width header */ /* header_instance_udp_ */,
  -1 /* fixed-width header */ /* header_instance_intrinsic_metadata */,
  -1 /* fixed-width header */ /* header_instance_l4_metadata_ */
};

// sugar@119
 static const int field_instance_bit_width[FIELD_INSTANCE_COUNT] = {
  9 /* field_instance_standard_metadata_ingress_port */,
  32 /* field_instance_standard_metadata_packet_length */,
  9 /* field_instance_standard_metadata_egress_spec */,
  9 /* field_instance_standard_metadata_egress_port */,
  32 /* field_instance_standard_metadata_egress_instance */,
  32 /* field_instance_standard_metadata_instance_type */,
  32 /* field_instance_standard_metadata_clone_spec */,
  5 /* field_instance_standard_metadata__padding */,
  48 /* field_instance_ethernet__dstAddr */,
  48 /* field_instance_ethernet__srcAddr */,
  16 /* field_instance_ethernet__etherType */,
  3 /* field_instance_vlan__pcp */,
  1 /* field_instance_vlan__cfi */,
  12 /* field_instance_vlan__vid */,
  16 /* field_instance_vlan__etherType */,
  4 /* field_instance_ipv4__version */,
  4 /* field_instance_ipv4__ihl */,
  8 /* field_instance_ipv4__diffserv */,
  16 /* field_instance_ipv4__totalLen */,
  16 /* field_instance_ipv4__identification */,
  3 /* field_instance_ipv4__flags */,
  13 /* field_instance_ipv4__fragOffset */,
  8 /* field_instance_ipv4__ttl */,
  8 /* field_instance_ipv4__protocol */,
  16 /* field_instance_ipv4__hdrChecksum */,
  32 /* field_instance_ipv4__srcAddr */,
  32 /* field_instance_ipv4__dstAddr */,
  16 /* field_instance_tcp__srcPort */,
  16 /* field_instance_tcp__dstPort */,
  32 /* field_instance_tcp__seqNo */,
  32 /* field_instance_tcp__ackNo */,
  4 /* field_instance_tcp__dataOffset */,
  4 /* field_instance_tcp__res */,
  8 /* field_instance_tcp__flags */,
  16 /* field_instance_tcp__window */,
  16 /* field_instance_tcp__checksum */,
  16 /* field_instance_tcp__urgentPtr */,
  16 /* field_instance_udp__srcPort */,
  16 /* field_instance_udp__dstPort */,
  16 /* field_instance_udp__length_ */,
  16 /* field_instance_udp__checksum */,
  4 /* field_instance_intrinsic_metadata_mcast_grp */,
  4 /* field_instance_intrinsic_metadata_egress_rid */,
  16 /* field_instance_intrinsic_metadata_mcast_hash */,
  32 /* field_instance_intrinsic_metadata_lf_field_list */,
  16 /* field_instance_l4_metadata__srcPort */,
  16 /* field_instance_l4_metadata__dstPort */
};

// sugar@120
 static const int field_instance_bit_offset[FIELD_INSTANCE_COUNT] = {
  0 /* field_instance_standard_metadata_ingress_port */,
  1 /* field_instance_standard_metadata_packet_length */,
  1 /* field_instance_standard_metadata_egress_spec */,
  2 /* field_instance_standard_metadata_egress_port */,
  3 /* field_instance_standard_metadata_egress_instance */,
  3 /* field_instance_standard_metadata_instance_type */,
  3 /* field_instance_standard_metadata_clone_spec */,
  3 /* field_instance_standard_metadata__padding */,
  0 /* field_instance_ethernet__dstAddr */,
  0 /* field_instance_ethernet__srcAddr */,
  0 /* field_instance_ethernet__etherType */,
  0 /* field_instance_vlan__pcp */,
  3 /* field_instance_vlan__cfi */,
  4 /* field_instance_vlan__vid */,
  0 /* field_instance_vlan__etherType */,
  0 /* field_instance_ipv4__version */,
  4 /* field_instance_ipv4__ihl */,
  0 /* field_instance_ipv4__diffserv */,
  0 /* field_instance_ipv4__totalLen */,
  0 /* field_instance_ipv4__identification */,
  0 /* field_instance_ipv4__flags */,
  3 /* field_instance_ipv4__fragOffset */,
  0 /* field_instance_ipv4__ttl */,
  0 /* field_instance_ipv4__protocol */,
  0 /* field_instance_ipv4__hdrChecksum */,
  0 /* field_instance_ipv4__srcAddr */,
  0 /* field_instance_ipv4__dstAddr */,
  0 /* field_instance_tcp__srcPort */,
  0 /* field_instance_tcp__dstPort */,
  0 /* field_instance_tcp__seqNo */,
  0 /* field_instance_tcp__ackNo */,
  0 /* field_instance_tcp__dataOffset */,
  4 /* field_instance_tcp__res */,
  0 /* field_instance_tcp__flags */,
  0 /* field_instance_tcp__window */,
  0 /* field_instance_tcp__checksum */,
  0 /* field_instance_tcp__urgentPtr */,
  0 /* field_instance_udp__srcPort */,
  0 /* field_instance_udp__dstPort */,
  0 /* field_instance_udp__length_ */,
  0 /* field_instance_udp__checksum */,
  0 /* field_instance_intrinsic_metadata_mcast_grp */,
  4 /* field_instance_intrinsic_metadata_egress_rid */,
  0 /* field_instance_intrinsic_metadata_mcast_hash */,
  0 /* field_instance_intrinsic_metadata_lf_field_list */,
  0 /* field_instance_l4_metadata__srcPort */,
  0 /* field_instance_l4_metadata__dstPort */
};

// sugar@121
 static const int field_instance_byte_offset_hdr[FIELD_INSTANCE_COUNT] = {
  0 /* field_instance_standard_metadata_ingress_port */,
  1 /* field_instance_standard_metadata_packet_length */,
  5 /* field_instance_standard_metadata_egress_spec */,
  6 /* field_instance_standard_metadata_egress_port */,
  7 /* field_instance_standard_metadata_egress_instance */,
  11 /* field_instance_standard_metadata_instance_type */,
  15 /* field_instance_standard_metadata_clone_spec */,
  19 /* field_instance_standard_metadata__padding */,
  0 /* field_instance_ethernet__dstAddr */,
  6 /* field_instance_ethernet__srcAddr */,
  12 /* field_instance_ethernet__etherType */,
  0 /* field_instance_vlan__pcp */,
  0 /* field_instance_vlan__cfi */,
  0 /* field_instance_vlan__vid */,
  2 /* field_instance_vlan__etherType */,
  0 /* field_instance_ipv4__version */,
  0 /* field_instance_ipv4__ihl */,
  1 /* field_instance_ipv4__diffserv */,
  2 /* field_instance_ipv4__totalLen */,
  4 /* field_instance_ipv4__identification */,
  6 /* field_instance_ipv4__flags */,
  6 /* field_instance_ipv4__fragOffset */,
  8 /* field_instance_ipv4__ttl */,
  9 /* field_instance_ipv4__protocol */,
  10 /* field_instance_ipv4__hdrChecksum */,
  12 /* field_instance_ipv4__srcAddr */,
  16 /* field_instance_ipv4__dstAddr */,
  0 /* field_instance_tcp__srcPort */,
  2 /* field_instance_tcp__dstPort */,
  4 /* field_instance_tcp__seqNo */,
  8 /* field_instance_tcp__ackNo */,
  12 /* field_instance_tcp__dataOffset */,
  12 /* field_instance_tcp__res */,
  13 /* field_instance_tcp__flags */,
  14 /* field_instance_tcp__window */,
  16 /* field_instance_tcp__checksum */,
  18 /* field_instance_tcp__urgentPtr */,
  0 /* field_instance_udp__srcPort */,
  2 /* field_instance_udp__dstPort */,
  4 /* field_instance_udp__length_ */,
  6 /* field_instance_udp__checksum */,
  0 /* field_instance_intrinsic_metadata_mcast_grp */,
  0 /* field_instance_intrinsic_metadata_egress_rid */,
  1 /* field_instance_intrinsic_metadata_mcast_hash */,
  3 /* field_instance_intrinsic_metadata_lf_field_list */,
  0 /* field_instance_l4_metadata__srcPort */,
  2 /* field_instance_l4_metadata__dstPort */
};

// sugar@122
 static const uint32_t field_instance_mask[FIELD_INSTANCE_COUNT] = {
  0x80ff /* field_instance_standard_metadata_ingress_port */,
  0x0 /* field_instance_standard_metadata_packet_length */,
  0xc07f /* field_instance_standard_metadata_egress_spec */,
  0xe03f /* field_instance_standard_metadata_egress_port */,
  0x0 /* field_instance_standard_metadata_egress_instance */,
  0x0 /* field_instance_standard_metadata_instance_type */,
  0x0 /* field_instance_standard_metadata_clone_spec */,
  0x1f /* field_instance_standard_metadata__padding */,
  0x0 /* field_instance_ethernet__dstAddr */,
  0x0 /* field_instance_ethernet__srcAddr */,
  0xffff /* field_instance_ethernet__etherType */,
  0xe0 /* field_instance_vlan__pcp */,
  0x10 /* field_instance_vlan__cfi */,
  0xff0f /* field_instance_vlan__vid */,
  0xffff /* field_instance_vlan__etherType */,
  0xf0 /* field_instance_ipv4__version */,
  0xf /* field_instance_ipv4__ihl */,
  0xff /* field_instance_ipv4__diffserv */,
  0xffff /* field_instance_ipv4__totalLen */,
  0xffff /* field_instance_ipv4__identification */,
  0xe0 /* field_instance_ipv4__flags */,
  0xff1f /* field_instance_ipv4__fragOffset */,
  0xff /* field_instance_ipv4__ttl */,
  0xff /* field_instance_ipv4__protocol */,
  0xffff /* field_instance_ipv4__hdrChecksum */,
  0xffffffff /* field_instance_ipv4__srcAddr */,
  0xffffffff /* field_instance_ipv4__dstAddr */,
  0xffff /* field_instance_tcp__srcPort */,
  0xffff /* field_instance_tcp__dstPort */,
  0xffffffff /* field_instance_tcp__seqNo */,
  0xffffffff /* field_instance_tcp__ackNo */,
  0xf0 /* field_instance_tcp__dataOffset */,
  0xf /* field_instance_tcp__res */,
  0xff /* field_instance_tcp__flags */,
  0xffff /* field_instance_tcp__window */,
  0xffff /* field_instance_tcp__checksum */,
  0xffff /* field_instance_tcp__urgentPtr */,
  0xffff /* field_instance_udp__srcPort */,
  0xffff /* field_instance_udp__dstPort */,
  0xffff /* field_instance_udp__length_ */,
  0xffff /* field_instance_udp__checksum */,
  0xf0 /* field_instance_intrinsic_metadata_mcast_grp */,
  0xf /* field_instance_intrinsic_metadata_egress_rid */,
  0xffff /* field_instance_intrinsic_metadata_mcast_hash */,
  0xffffffff /* field_instance_intrinsic_metadata_lf_field_list */,
  0xffff /* field_instance_l4_metadata__srcPort */,
  0xffff /* field_instance_l4_metadata__dstPort */
};

// sugar@123
 static const header_instance_t field_instance_header[FIELD_INSTANCE_COUNT] = {
  header_instance_standard_metadata /* field_instance_standard_metadata_ingress_port */,
  header_instance_standard_metadata /* field_instance_standard_metadata_packet_length */,
  header_instance_standard_metadata /* field_instance_standard_metadata_egress_spec */,
  header_instance_standard_metadata /* field_instance_standard_metadata_egress_port */,
  header_instance_standard_metadata /* field_instance_standard_metadata_egress_instance */,
  header_instance_standard_metadata /* field_instance_standard_metadata_instance_type */,
  header_instance_standard_metadata /* field_instance_standard_metadata_clone_spec */,
  header_instance_standard_metadata /* field_instance_standard_metadata__padding */,
  header_instance_ethernet_ /* field_instance_ethernet__dstAddr */,
  header_instance_ethernet_ /* field_instance_ethernet__srcAddr */,
  header_instance_ethernet_ /* field_instance_ethernet__etherType */,
  header_instance_vlan_ /* field_instance_vlan__pcp */,
  header_instance_vlan_ /* field_instance_vlan__cfi */,
  header_instance_vlan_ /* field_instance_vlan__vid */,
  header_instance_vlan_ /* field_instance_vlan__etherType */,
  header_instance_ipv4_ /* field_instance_ipv4__version */,
  header_instance_ipv4_ /* field_instance_ipv4__ihl */,
  header_instance_ipv4_ /* field_instance_ipv4__diffserv */,
  header_instance_ipv4_ /* field_instance_ipv4__totalLen */,
  header_instance_ipv4_ /* field_instance_ipv4__identification */,
  header_instance_ipv4_ /* field_instance_ipv4__flags */,
  header_instance_ipv4_ /* field_instance_ipv4__fragOffset */,
  header_instance_ipv4_ /* field_instance_ipv4__ttl */,
  header_instance_ipv4_ /* field_instance_ipv4__protocol */,
  header_instance_ipv4_ /* field_instance_ipv4__hdrChecksum */,
  header_instance_ipv4_ /* field_instance_ipv4__srcAddr */,
  header_instance_ipv4_ /* field_instance_ipv4__dstAddr */,
  header_instance_tcp_ /* field_instance_tcp__srcPort */,
  header_instance_tcp_ /* field_instance_tcp__dstPort */,
  header_instance_tcp_ /* field_instance_tcp__seqNo */,
  header_instance_tcp_ /* field_instance_tcp__ackNo */,
  header_instance_tcp_ /* field_instance_tcp__dataOffset */,
  header_instance_tcp_ /* field_instance_tcp__res */,
  header_instance_tcp_ /* field_instance_tcp__flags */,
  header_instance_tcp_ /* field_instance_tcp__window */,
  header_instance_tcp_ /* field_instance_tcp__checksum */,
  header_instance_tcp_ /* field_instance_tcp__urgentPtr */,
  header_instance_udp_ /* field_instance_udp__srcPort */,
  header_instance_udp_ /* field_instance_udp__dstPort */,
  header_instance_udp_ /* field_instance_udp__length_ */,
  header_instance_udp_ /* field_instance_udp__checksum */,
  header_instance_intrinsic_metadata /* field_instance_intrinsic_metadata_mcast_grp */,
  header_instance_intrinsic_metadata /* field_instance_intrinsic_metadata_egress_rid */,
  header_instance_intrinsic_metadata /* field_instance_intrinsic_metadata_mcast_hash */,
  header_instance_intrinsic_metadata /* field_instance_intrinsic_metadata_lf_field_list */,
  header_instance_l4_metadata_ /* field_instance_l4_metadata__srcPort */,
  header_instance_l4_metadata_ /* field_instance_l4_metadata__dstPort */
};

// sugar@124
 
 static const header_instance_t header_stack_elements[HEADER_STACK_COUNT][10] = {
  
};

// sugar@126
 static const unsigned header_stack_size[HEADER_STACK_COUNT] = {
  
};

// sugar@127
 typedef enum header_stack_e header_stack_t;// sugar@128
 
 #endif // __HEADER_INFO_H__// sugar@130
