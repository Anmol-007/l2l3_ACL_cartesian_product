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
from p4_hlir.hlir.p4_sized_integer import *
from p4_hlir.hlir.p4_headers import p4_field
from utils.hlir import *
from utils.misc import addError, addWarning

generated_code += " #include <stdlib.h>// sugar@20\n"
generated_code += " #include <string.h>// sugar@21\n"
generated_code += " #include \"dpdk_lib.h\"// sugar@22\n"
generated_code += " #include \"actions.h\"// sugar@23\n"
generated_code += " \n"
generated_code += " extern void parse_packet(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@25\n"
generated_code += "\n"
generated_code += " extern void increase_counter (int counterid, int index);// sugar@27\n"
generated_code += "\n"
for table in hlir.p4_tables.values():
    generated_code += " void apply_table_" + str(table.name) + "(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@30\n"
generated_code += "\n"

if len(hlir.p4_tables.values())>0:
    generated_code += " uint8_t reverse_buffer[" + str(max([t[1] for t in map(getTypeAndLength, hlir.p4_tables.values())])) + "];// sugar@34\n"

def match_type_order(t):
    if t is p4.p4_match_type.P4_MATCH_EXACT:   return 0
    if t is p4.p4_match_type.P4_MATCH_LPM:     return 1
    if t is p4.p4_match_type.P4_MATCH_TERNARY: return 2

for table in hlir.p4_tables.values():
    table_type, key_length = getTypeAndLength(table)
    generated_code += " void table_" + str(table.name) + "_key(packet_descriptor_t* pd, uint8_t* key) {// sugar@43\n"
    sortedfields = sorted(table.match_fields, key=lambda field: match_type_order(field[1]))
    for match_field, match_type, match_mask in sortedfields:
        if is_vwf(match_field):
            addError("generating table_" + table.name + "_key", "Variable width field '" + str(match_field) + "' in match key for table '" + table.name + "' is not supported")
        elif match_field.width <= 32:
            generated_code += " EXTRACT_INT32_BITS(pd, " + str(fld_id(match_field)) + ", *(uint32_t*)key)// sugar@49\n"
            generated_code += " key += sizeof(uint32_t);// sugar@50\n"
        elif match_field.width > 32 and match_field.width % 8 == 0:
            byte_width = (match_field.width+7)/8
            generated_code += " EXTRACT_BYTEBUF(pd, " + str(fld_id(match_field)) + ", key)// sugar@53\n"
            generated_code += " key += " + str(byte_width) + ";// sugar@54\n"
        else:
            print "Unsupported field %s ignored in key calculation." % fld_id(match_field)
    if table_type == "LOOKUP_LPM":
        generated_code += " key -= " + str(key_length) + ";// sugar@58\n"
        generated_code += " int c, d;// sugar@59\n"
        generated_code += " for(c = " + str(key_length-1) + ", d = 0; c >= 0; c--, d++) *(reverse_buffer+d) = *(key+c);// sugar@60\n"
        generated_code += " for(c = 0; c < " + str(key_length) + "; c++) *(key+c) = *(reverse_buffer+c);// sugar@61\n"
    generated_code += " }// sugar@62\n"
    generated_code += "\n"

for table in hlir.p4_tables.values():
    table_type, key_length = getTypeAndLength(table)
    lookupfun = {'LOOKUP_LPM':'lpm_lookup', 'LOOKUP_EXACT':'exact_lookup', 'LOOKUP_TERNARY':'ternary_lookup'}
    generated_code += " void apply_table_" + str(table.name) + "(packet_descriptor_t* pd, lookup_table_t** tables)// sugar@68\n"
    generated_code += " {// sugar@69\n"
    generated_code += "     debug(\"  :::: EXECUTING TABLE " + str(table.name) + "\\n\");// sugar@70\n"
    generated_code += "     uint8_t* key[" + str(key_length) + "];// sugar@71\n"
    generated_code += "     table_" + str(table.name) + "_key(pd, (uint8_t*)key);// sugar@72\n"
    generated_code += "     uint8_t* value = " + str(lookupfun[table_type]) + "(tables[TABLE_" + str(table.name) + "], (uint8_t*)key);// sugar@73\n"
    generated_code += "     struct " + str(table.name) + "_action* res = (struct " + str(table.name) + "_action*)value;// sugar@74\n"
    generated_code += "     int index; (void)index;// sugar@75\n"

    # COUNTERS
    generated_code += "     if(res != NULL) {// sugar@78\n"
    generated_code += "       index = *(int*)(value+sizeof(struct " + str(table.name) + "_action));// sugar@79\n"
    for counter in table.attached_counters:
        generated_code += "       increase_counter(COUNTER_" + str(counter.name) + ", index);// sugar@81\n"
    generated_code += "     }// sugar@82\n"

    # ACTIONS
    generated_code += "     if(res == NULL) {// sugar@85\n"
    generated_code += "       debug(\"    :: NO RESULT, NO DEFAULT ACTION.\\n\");// sugar@86\n"
    generated_code += "     } else {// sugar@87\n"
    generated_code += "       switch (res->action_id) {// sugar@88\n"
    for action in table.actions:
        generated_code += "         case action_" + str(action.name) + ":// sugar@90\n"
        generated_code += "           debug(\"    :: EXECUTING ACTION " + str(action.name) + "...\\n\");// sugar@91\n"
        if action.signature:
            generated_code += "           action_code_" + str(action.name) + "(pd, tables, res->" + str(action.name) + "_params);// sugar@93\n"
        else:
            generated_code += "           action_code_" + str(action.name) + "(pd, tables);// sugar@95\n"
        generated_code += "           break;// sugar@96\n"
    generated_code += "       }// sugar@97\n"
    generated_code += "     }// sugar@98\n"

    # NEXT TABLE
    if 'hit' in table.next_:
        generated_code += "     if(res != NULL && index != DEFAULT_ACTION_INDEX) { //Lookup was successful (not with default action)// sugar@102\n"
        if table.next_['hit'] is not None:
            generated_code += "       " + str(format_p4_node(table.next_['hit'])) + "// sugar@104\n"
        generated_code += "     } else {                                           //Lookup failed or returned default action// sugar@105\n"
        if table.next_['miss'] is not None:
            generated_code += "       " + str(format_p4_node(table.next_['miss'])) + "// sugar@107\n"
        generated_code += "     }// sugar@108\n"
    else:
        generated_code += "     if (res != NULL) {// sugar@110\n"
        generated_code += "       switch (res->action_id) {// sugar@111\n"
        for action, nextnode in table.next_.items():
            generated_code += "         case action_" + str(action.name) + ":// sugar@113\n"
            generated_code += "           " + str(format_p4_node(nextnode)) + "// sugar@114\n"
            generated_code += "           break;// sugar@115\n"
        generated_code += "       }// sugar@116\n"
        generated_code += "     } else {// sugar@117\n"
        generated_code += "       debug(\"    :: IGNORING PACKET.\\n\");// sugar@118\n"
        generated_code += "       return;// sugar@119\n"
        generated_code += "     }// sugar@120\n"
    generated_code += " }// sugar@121\n"
    generated_code += "\n"

generated_code += "\n"
generated_code += " uint16_t csum16_add(uint16_t num1, uint16_t num2) {// sugar@125\n"
generated_code += "     if(num1 == 0) return num2;// sugar@126\n"
generated_code += "     uint32_t tmp_num = num1 + num2;// sugar@127\n"
generated_code += "     while(tmp_num > 0xffff)// sugar@128\n"
generated_code += "         tmp_num = ((tmp_num & 0xffff0000) >> 16) + (tmp_num & 0xffff);// sugar@129\n"
generated_code += "     return (uint16_t)tmp_num;// sugar@130\n"
generated_code += " }// sugar@131\n"
generated_code += "\n"
for calc in hlir.p4_field_list_calculations.values():
    generated_code += " uint32_t calculate_" + str(calc.name) + "(packet_descriptor_t* pd) {// sugar@134\n"
    generated_code += "   uint32_t res = 0;// sugar@135\n"
    generated_code += "   void* payload_ptr;// sugar@136\n"

    buff_idx = 0
    fixed_input_width = 0     #Calculates the fixed width of all p4_fields and sized_integers (PAYLOAD width is not included)
    variable_input_width = "" #Calculates the variable width of all p4_fields
    for field_list in calc.input:
        for item in field_list.fields:
            if isinstance(item, p4_field) or isinstance(item, p4_sized_integer):
                if is_vwf(item):
                    if field_max_width(item) % 8 == 0 and item.offset % 8 == 0:
                        variable_input_width += " + field_desc(pd, " + fld_id(item) + ").bitwidth"
                    else:
                        addError("generating field list calculation " + calc.name, "Variable width field '" + str(item) + "' in calculation '" + calc.name + "' is not byte-aligned. Field list calculations are only supported on byte-aligned variable width fields!");
                else:
                    fixed_input_width += item.width
    if fixed_input_width % 8 != 0:
        addError("generating field list calculation", "The bitwidth of the field_lists for the calculation '" + calc.name + "' is incorrect.")
    generated_code += "   uint8_t* buf = malloc((" + str(fixed_input_width) + "" + str(variable_input_width) + ") / 8);// sugar@153\n"
    generated_code += "   memset(buf, 0, (" + str(fixed_input_width) + "" + str(variable_input_width) + ") / 8);// sugar@154\n"

    tmp_list = []
    fixed_bitoffset = 0
    variable_bitoffset = ""
    for field_list in calc.input:
        item_index = 0
        while item_index < len(field_list.fields):
            start_item = field_list.fields[item_index]
            if isinstance(start_item, p4_field): #Processing field block (multiple continuous fields in a row)
                inst = start_item.instance
                if is_vwf(start_item):
                    fixed_bitwidth = 0
                    variable_bitwidth = " + field_desc(pd, " + fld_id(start_item) + ").bitwidth"
                else:
                    fixed_bitwidth = start_item.width
                    variable_bitwidth = ""

                inst_index = 0 #The index of the field in the header instance
                while start_item != inst.fields[inst_index]: inst_index += 1

                while inst_index + 1 < len(inst.fields) and item_index + 1 < len(field_list.fields) and inst.fields[inst_index + 1] == field_list.fields[item_index + 1]:
                    item_index += 1
                    inst_index += 1
                    if is_vwf(field_list.fields[item_index]):
                        variable_bitwidth += " + field_desc(pd, " + fld_id(field_list.fields[item_index]) + ").bitwidth"
                    else:
                        fixed_bitwidth += field_list.fields[item_index].width

                if (not variable_bitwidth) and fixed_bitwidth % 8 != 0: addError("generating field list calculation", "The bitwidth of a field block is incorrenct!")
                tmp_list.append(("((" + str(fixed_bitoffset) + variable_bitoffset + ") / 8)", start_item, "((" + str(fixed_bitwidth) + variable_bitwidth + ") / 8)"))

                fixed_bitoffset += fixed_bitwidth
                variable_bitoffset += variable_bitwidth
            elif isinstance(start_item, p4_sized_integer):
                if start_item.width % 8 != 0:
                    addError("generating field list calculation", "Only byte-wide constants are supported in field lists.")
                else:
                    buff_idx += 1
                    byte_array = int_to_big_endian_byte_array_with_length(start_item, start_item.width / 8)
                    generated_code += "   uint8_t buffer_" + str(buff_idx) + "[" + str(start_item.width / 8) + "] = {" + str(reduce((lambda a, b: a + ', ' + b), map(lambda x: str(x), byte_array))) + "};// sugar@194\n"
                    generated_code += "   memcpy(buf + ((" + str(fixed_bitoffset) + "" + str(variable_bitoffset) + ") / 8), &buffer_" + str(buff_idx) + ", " + str(start_item.width / 8) + ");// sugar@195\n"
                    fixed_bitoffset += start_item.width
            else:
                if item_index == 0 or not isinstance(field_list.fields[item_index - 1], p4_field):
                    addError("generating field list calculation", "Payload element must follow a regular field instance in the field list.")
                elif calc.algorithm == "csum16":
                    hi_name = hdr_prefix(field_list.fields[item_index - 1].instance.name)
                    generated_code += "   payload_ptr = (((void*)pd->headers[" + str(hi_name) + "].pointer) + (pd->headers[" + str(hi_name) + "].length));// sugar@202\n"
                    generated_code += "   res = csum16_add(res, calculate_csum16(payload_ptr, packet_length(pd) - (payload_ptr - ((void*) pd->data))));// sugar@203\n"
            item_index += 1

    while len(tmp_list) > 0:
        inst = tmp_list[0][1].instance
        list_index = 0
        generated_code += "   if(" + str(format_expr(valid_expression(inst))) + ") {// sugar@209\n"
        while list_index < len(tmp_list):
            item = tmp_list[list_index]
            if item[1].instance == inst:
                generated_code += "     memcpy(buf + " + str(item[0]) + ", field_desc(pd, " + str(fld_id(item[1])) + ").byte_addr, " + str(item[2]) + ");// sugar@213\n"
                del tmp_list[list_index]
            else: list_index += 1;
        generated_code += "   }// sugar@216\n"

    if calc.algorithm == "csum16":
        generated_code += "   res = csum16_add(res, calculate_csum16(buf, (" + str(fixed_input_width) + "" + str(variable_input_width) + ") / 8));// sugar@219\n"
        generated_code += "   res = (res == 0xffff) ? res : ((~res) & 0xffff);// sugar@220\n"
    else:
        #If a new calculation implementation is added, new PAYLOAD handling should also be added.
        addError("generating field list calculation", "Unsupported field list calculation algorithm: " + calc.algorithm)
    generated_code += "   free(buf);// sugar@224\n"
    generated_code += "   return res & " + str(hex((2 ** calc.output_width) - 1)) + ";// sugar@225\n"
    generated_code += " }// sugar@226\n"
    generated_code += "\n"

generated_code += " void reset_headers(packet_descriptor_t* packet_desc) {// sugar@229\n"
for hi in header_instances(hlir):
    n = hdr_prefix(hi.name)
    if hi.metadata:
        generated_code += " memset(packet_desc->headers[" + str(n) + "].pointer, 0, header_info(" + str(n) + ").bytewidth * sizeof(uint8_t));// sugar@233\n"
    else:
        generated_code += " packet_desc->headers[" + str(n) + "].pointer = NULL;// sugar@235\n"
generated_code += " }// sugar@236\n"
generated_code += " void init_headers(packet_descriptor_t* packet_desc) {// sugar@237\n"
for hi in header_instances(hlir):
    n = hdr_prefix(hi.name)
    if hi.metadata:
        generated_code += " packet_desc->headers[" + str(n) + "] = (header_descriptor_t) { .type = " + str(n) + ", .length = header_info(" + str(n) + ").bytewidth,// sugar@241\n"
        generated_code += "                               .pointer = malloc(header_info(" + str(n) + ").bytewidth * sizeof(uint8_t)),// sugar@242\n"
        generated_code += "                               .var_width_field_bitwidth = 0 };// sugar@243\n"
    else:
        generated_code += " packet_desc->headers[" + str(n) + "] = (header_descriptor_t) { .type = " + str(n) + ", .length = header_info(" + str(n) + ").bytewidth, .pointer = NULL,// sugar@245\n"
        generated_code += "                               .var_width_field_bitwidth = 0 };// sugar@246\n"
generated_code += " }// sugar@247\n"
generated_code += "\n"
for table in hlir.p4_tables.values():
    table_type, key_length = getTypeAndLength(table)
    if key_length == 0 and len(table.actions) == 1:
        action = table.actions[0]
        generated_code += " extern void " + str(table.name) + "_setdefault(struct " + str(table.name) + "_action);// sugar@253\n"
generated_code += "\n"
generated_code += " void init_keyless_tables() {// sugar@255\n"
for table in hlir.p4_tables.values():
    table_type, key_length = getTypeAndLength(table)
    if key_length == 0 and len(table.actions) == 1:
        action = table.actions[0]
        generated_code += " struct " + str(table.name) + "_action " + str(table.name) + "_a;// sugar@260\n"
        generated_code += " " + str(table.name) + "_a.action_id = action_" + str(action.name) + ";// sugar@261\n"
        generated_code += " " + str(table.name) + "_setdefault(" + str(table.name) + "_a);// sugar@262\n"
generated_code += " }// sugar@263\n"
generated_code += "\n"
generated_code += " void init_dataplane(packet_descriptor_t* pd, lookup_table_t** tables) {// sugar@265\n"
generated_code += "     init_headers(pd);// sugar@266\n"
generated_code += "     reset_headers(pd);// sugar@267\n"
generated_code += "     init_keyless_tables();// sugar@268\n"
generated_code += "     pd->dropped=0;// sugar@269\n"
generated_code += " }// sugar@270\n"

generated_code += "\n"
generated_code += " void update_packet(packet_descriptor_t* pd) {// sugar@273\n"
generated_code += "     uint32_t value32, res32;// sugar@274\n"
generated_code += "     (void)value32, (void)res32;// sugar@275\n"
for f in hlir.p4_fields.values():
    if parsed_field(hlir, f):
        if f.width <= 32:
#            #[ if(pd->headers[${hdr_prefix(f.instance.name)}].pointer != NULL) {
            generated_code += " if(pd->fields.attr_" + str(fld_id(f)) + " == MODIFIED) {// sugar@280\n"
            generated_code += "     value32 = pd->fields." + str(fld_id(f)) + ";// sugar@281\n"
            generated_code += "     MODIFY_INT32_INT32_AUTO(pd, " + str(fld_id(f)) + ", value32)// sugar@282\n"
            generated_code += " }// sugar@283\n"
generated_code += "\n"
for f in hlir.p4_fields.values():
    for calc in f.calculation:
        if calc[0] == "update":
            if calc[2] is not None:
                generated_code += " if(" + str(format_expr(calc[2])) + ")// sugar@289\n"
            elif not f.instance.metadata:
                generated_code += " if(" + str(format_expr(valid_expression(f))) + ")// sugar@291\n"
            generated_code += " {// sugar@292\n"
            generated_code += "     value32 = calculate_" + str(calc[1].name) + "(pd);// sugar@293\n"
            generated_code += "     MODIFY_INT32_INT32_BITS(pd, " + str(fld_id(f)) + ", value32);// sugar@294\n"
            generated_code += " }// sugar@295\n"
generated_code += " }// sugar@296\n"
generated_code += "\n"

generated_code += "\n"
generated_code += " int verify_packet(packet_descriptor_t* pd) {// sugar@300\n"
generated_code += "   uint32_t value32;// sugar@301\n"
for f in hlir.p4_fields.values():
    for calc in f.calculation:
        if calc[0] == "verify":
            if calc[2] is not None:
                generated_code += "   if(" + str(format_expr(calc[2])) + ")// sugar@306\n"
            elif not f.instance.metadata:
                generated_code += "   if(" + str(format_expr(valid_expression(f))) + ")// sugar@308\n"
            generated_code += "   {// sugar@309\n"
            generated_code += "     EXTRACT_INT32_BITS(pd, " + str(fld_id(f)) + ", value32);// sugar@310\n"
            generated_code += "     if(value32 != calculate_" + str(calc[1].name) + "(pd)) {// sugar@311\n"
            generated_code += "       debug(\"       Checksum verification on field '" + str(f) + "' by '" + str(calc[1].name) + "': FAILED\\n\");// sugar@312\n"
            generated_code += "       return 1;// sugar@313\n"
            generated_code += "     }// sugar@314\n"
            generated_code += "     else debug(\"       Checksum verification on field '" + str(f) + "' by '" + str(calc[1].name) + "': SUCCESSFUL\\n\");// sugar@315\n"
            generated_code += "   }// sugar@316\n"
generated_code += "   return 0;// sugar@317\n"
generated_code += " }// sugar@318\n"
generated_code += "\n"

generated_code += " \n"
generated_code += " void handle_packet(packet_descriptor_t* pd, lookup_table_t** tables)// sugar@322\n"
generated_code += " {// sugar@323\n"
generated_code += "     int value32;// sugar@324\n"
generated_code += "     EXTRACT_INT32_BITS(pd, field_instance_standard_metadata_ingress_port, value32)// sugar@325\n"
generated_code += "     debug(\"### HANDLING PACKET ARRIVING AT PORT %\" PRIu32 \"...\\n\", value32);// sugar@326\n"
generated_code += "     parse_packet(pd, tables);// sugar@327\n"
generated_code += "     update_packet(pd);// sugar@328\n"
generated_code += " }// sugar@329\n"