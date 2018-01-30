# Copyright 2016 Eotvos Lorand University, Budapest, Hungary
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import p4_hlir.hlir.p4 as p4
from utils.hlir import *
from utils.misc import addError, addWarning 

def format_state(state):
    generated_code = ""
    if isinstance(state, p4.p4_parse_state):
        generated_code += " return parse_state_" + str(state.name) + "(pd, buf, tables);// sugar@21\n"
    elif isinstance(state, p4.p4_parser_exception):
        print "Parser exceptions are not supported yet."
    else: #Control function (parsing is finished)
        generated_code += " {// sugar@25\n"
        generated_code += "   if(verify_packet(pd)) p4_pe_checksum(pd);// sugar@26\n"
        generated_code += "   " + str(format_p4_node(state)) + "// sugar@27\n"
        generated_code += " }// sugar@28\n"
    return generated_code

def get_key_byte_width(branch_on):
    """
    :param branch_on: list of union(p4_field, tuple)
    :rtype:           int
    """
    key_width = 0
    for switch_ref in branch_on:
        if type(switch_ref) is p4.p4_field:
            if not is_vwf(switch_ref): #Variable width field in parser return select statement is not supported
                key_width += (switch_ref.width+7)/8
        elif type(switch_ref) is tuple:
            key_width += max(4, (switch_ref[1] + 7) / 8)
    return key_width

pe_dict = { "p4_pe_index_out_of_bounds" : None,
            "p4_pe_out_of_packet" : None,
            "p4_pe_header_too_long" : None,
            "p4_pe_header_too_short" : None,
            "p4_pe_unhandled_select" : None,
            "p4_pe_checksum" : None,
            "p4_pe_default" : None }

pe_default = p4.p4_parser_exception(None, None)
pe_default.name = "p4_pe_default"
pe_default.return_or_drop = p4.P4_PARSER_DROP

for pe_name, pe in pe_dict.items():
    pe_dict[pe_name] = pe_default
for pe_name, pe in hlir.p4_parser_exceptions.items():
    pe_dict[pe_name] = pe

generated_code += " #include \"dpdk_lib.h\"// sugar@62\n"
generated_code += " #include \"actions.h\" // apply_table_* and action_code_*// sugar@63\n"
generated_code += "\n"
generated_code += " extern int verify_packet(packet_descriptor_t* pd);// sugar@65\n"
generated_code += "\n"
generated_code += " void print_mac(uint8_t* v) { printf(\"%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX\\n\", v[0], v[1], v[2], v[3], v[4], v[5]); }// sugar@67\n"
generated_code += " void print_ip(uint8_t* v) { printf(\"%d.%d.%d.%d\\n\",v[0],v[1],v[2],v[3]); }// sugar@68\n"
generated_code += " \n"

for pe_name, pe in pe_dict.items():
    generated_code += " static inline void " + str(pe_name) + "(packet_descriptor_t *pd) {// sugar@72\n"
    if pe.return_or_drop == p4.P4_PARSER_DROP:
        generated_code += " pd->dropped = 1;// sugar@74\n"
    else:
        format_p4_node(pe.return_or_drop)
    generated_code += " }// sugar@77\n"

for hi_name, hi in hlir.p4_header_instances.items():
    hi_prefix = hdr_prefix(hi.name)
    generated_code += " static void// sugar@81\n"
    generated_code += " extract_header_" + str(hi) + "(uint8_t* buf, packet_descriptor_t* pd) {// sugar@82\n"
    generated_code += "     pd->headers[" + str(hi_prefix) + "].pointer = buf;// sugar@83\n"
    if isinstance(hi.header_type.length, p4.p4_expression):
        generated_code += "     uint32_t hdr_length = " + str(format_expr(resolve_field_ref(hlir, hi, hi.header_type.length))) + ";// sugar@85\n"
        generated_code += "     pd->headers[" + str(hi_prefix) + "].length = hdr_length;// sugar@86\n"
        generated_code += "     pd->headers[" + str(hi_prefix) + "].var_width_field_bitwidth = hdr_length * 8 - " + str(sum([f[1] if f[1] != p4.P4_AUTO_WIDTH else 0 for f in hi.header_type.layout.items()])) + ";// sugar@87\n"
        generated_code += "     if(hdr_length > " + str(hi.header_type.max_length) + ") //TODO: is this the correct place for the check// sugar@88\n"
        generated_code += "         p4_pe_header_too_long(pd);// sugar@89\n"
    generated_code += " }// sugar@90\n"
    generated_code += " \n"

for state_name, parse_state in hlir.p4_parse_states.items():
    generated_code += " static void parse_state_" + str(state_name) + "(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables);// sugar@94\n"
generated_code += "\n"

for state_name, parse_state in hlir.p4_parse_states.items():
    branch_on = parse_state.branch_on
    if branch_on:
        generated_code += " static inline void build_key_" + str(state_name) + "(packet_descriptor_t *pd, uint8_t *buf, uint8_t *key) {// sugar@100\n"
        for switch_ref in branch_on:
            if type(switch_ref) is p4.p4_field:
                field_instance = switch_ref
                if is_vwf(field_instance):
                    addError("generating build_key_" + state_name, "Variable width field '" + str(field_instance) + "' in parser '" + state_name + "' return select statement is not supported")
                else:
                    byte_width = (field_instance.width + 7) / 8
                    if byte_width <= 4:
                        generated_code += " EXTRACT_INT32_BITS(pd, " + str(fld_id(field_instance)) + ", *(uint32_t*)key)// sugar@109\n"
                        generated_code += " key += sizeof(uint32_t);// sugar@110\n"
                    else:
                        generated_code += " EXTRACT_BYTEBUF(pd, " + str(fld_id(field_instance)) + ", key)// sugar@112\n"
                        generated_code += " key += " + str(byte_width) + ";// sugar@113\n"
            elif type(switch_ref) is tuple:
                generated_code += "     uint8_t* ptr;// sugar@115\n"
                offset, width = switch_ref
                # TODO
                addError("generating parse state %s"%state_name, "current() calls are not supported yet")
        generated_code += " }// sugar@119\n"

for state_name, parse_state in hlir.p4_parse_states.items():
    generated_code += " static void parse_state_" + str(state_name) + "(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables)// sugar@122\n"
    generated_code += " {// sugar@123\n"
    generated_code += "     uint32_t value32;// sugar@124\n"
    generated_code += "     (void)value32;// sugar@125\n"
    
    for call in parse_state.call_sequence:
        if call[0] == p4.parse_call.extract:
            hi = call[1] 
            generated_code += "     extract_header_" + str(hi) + "(buf, pd);// sugar@130\n"
            generated_code += "     buf += pd->headers[" + str(hdr_prefix(hi.name)) + "].length;// sugar@131\n"
            for f in hi.fields:
                if parsed_field(hlir, f):
                    if f.width <= 32:
                        generated_code += " EXTRACT_INT32_AUTO(pd, " + str(fld_id(f)) + ", value32)// sugar@135\n"
                        generated_code += " pd->fields." + str(fld_id(f)) + " = value32;// sugar@136\n"
                        generated_code += " pd->fields.attr_" + str(fld_id(f)) + " = 0;// sugar@137\n"
        elif call[0] == p4.parse_call.set:
            dest_field, src = call[1], call[2]
            if type(src) is int or type(src) is long:
                hex(src)
                # TODO
            elif type(src) is p4.p4_field:
                src
                # TODO
            elif type(src) is tuple:
                offset, width = src
                # TODO
            addError("generating parse state %s"%state_name, "set_metadata during parsing is not supported yet")

    branch_on = parse_state.branch_on
    if not branch_on:
        branch_case, next_state = parse_state.branch_to.items()[0]
        generated_code += " " + str(format_state(next_state)) + "// sugar@154\n"
    else:
        key_byte_width = get_key_byte_width(branch_on)
        generated_code += " uint8_t key[" + str(key_byte_width) + "];// sugar@157\n"
        generated_code += " build_key_" + str(state_name) + "(pd, buf, key);// sugar@158\n"
        has_default_case = False
        for case_num, case in enumerate(parse_state.branch_to.items()):
            branch_case, next_state = case
            mask_name  = "mask_value_%d" % case_num
            value_name  = "case_value_%d" % case_num
            if branch_case == p4.P4_DEFAULT:
                has_default_case = True
                generated_code += " " + str(format_state(next_state)) + "// sugar@166\n"
                continue
            if type(branch_case) is int:
                value = branch_case
                value_len, l = int_to_big_endian_byte_array(value)
                generated_code += "     uint8_t " + str(value_name) + "[" + str(value_len) + "] = {// sugar@171\n"
                for c in l:
                    generated_code += "         " + str(c) + ",// sugar@173\n"
                generated_code += "     };// sugar@174\n"
                generated_code += "     if ( memcmp(key, " + str(value_name) + ", " + str(value_len) + ") == 0)// sugar@175\n"
                generated_code += "         " + str(format_state(next_state)) + "// sugar@176\n"
            elif type(branch_case) is tuple:
                value = branch_case[0]
                mask = branch_case[1]
                # TODO
                addError("generating parse state %s"%state_name, "value masking is not supported yet")
            elif type(branch_case) is p4.p4_parse_value_set:
                value_set = branch_case
                # TODO
                addError("generating parse state %s"%state_name, "value sets are not supported yet")
                continue
        if not has_default_case:
            generated_code += "     return NULL;// sugar@188\n"
    generated_code += " }// sugar@189\n"
    generated_code += " \n"

generated_code += " void parse_packet(packet_descriptor_t* pd, lookup_table_t** tables) {// sugar@192\n"
generated_code += "     parse_state_start(pd, pd->data, tables);// sugar@193\n"
generated_code += " }// sugar@194\n"