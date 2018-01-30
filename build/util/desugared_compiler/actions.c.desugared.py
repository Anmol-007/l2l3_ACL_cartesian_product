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
from p4_hlir.hlir.p4_headers import p4_field, p4_field_list, p4_header_keywords
from p4_hlir.hlir.p4_imperatives import p4_signature_ref
from utils.misc import addError, addWarning 
from utils.hlir import *
import math

generated_code += " #include \"dpdk_lib.h\"// sugar@20\n"
generated_code += " #include \"actions.h\"// sugar@21\n"
generated_code += " #include <unistd.h>// sugar@22\n"
generated_code += " #include <arpa/inet.h>// sugar@23\n"
generated_code += "\n"
generated_code += " extern backend bg;// sugar@25\n"
generated_code += "\n"

# =============================================================================
# Helpers for field access and update
# (read/write the cached/pre-parsed value where possible)

# TODO now that these abstractions have been made, we might decide to
#   get rid of _AUTO versions of macros and implement the branching here

def modify_int32_int32(f):
    generated_code = ""
    if parsed_field(hlir, f):
        generated_code += " pd->fields." + str(fld_id(f)) + " = value32;// sugar@38\n"
        generated_code += " pd->fields.attr_" + str(fld_id(f)) + " = MODIFIED;// sugar@39\n"
    else:
        generated_code += " MODIFY_INT32_INT32_AUTO(pd, " + str(fld_id(f)) + ", value32)// sugar@41\n"
    return generated_code

def extract_int32(f, var, mask = None):
    generated_code = ""
    if parsed_field(hlir, f):
        if mask:
            generated_code += " " + str(var) + " = pd->fields." + str(fld_id(f)) + " " + str(mask) + ";// sugar@48\n"
        else:
            generated_code += " " + str(var) + " = pd->fields." + str(fld_id(f)) + ";// sugar@50\n"
        generated_code += " pd->fields.attr_" + str(fld_id(f)) + " = MODIFIED;// sugar@51\n"
    else:
        generated_code += " EXTRACT_INT32_AUTO(pd, " + str(fld_id(f)) + ", " + str(var) + ");// sugar@53\n"
        if mask:
             generated_code += " " + str(var) + " = " + str(var) + "" + str(mask) + ";// sugar@55\n"
    return generated_code

# =============================================================================
# Helpers for saturating in add_to_field

def max_value(bitwidth, signed):
    if signed:
        return long(math.pow(2,bitwidth-1)) - 1
    else:
        return long(math.pow(2,bitwidth)) - 1

def min_value(bitwidth, signed):
    if signed:
        return -long(math.pow(2,bitwidth-1)) + 1
    else:
        return 0

# dst += src;
def add_with_saturating(dst, src, bitwidth, signed):
    generated_code = ""
    upper = max_value(bitwidth, signed)
    lower = min_value(bitwidth, signed)
    generated_code += " if (" + str(upper) + " - " + str(dst) + " < " + str(src) + ") " + str(dst) + " = " + str(upper) + ";// sugar@78\n"
    generated_code += " else if (" + str(lower) + " - " + str(dst) + " > " + str(src) + ") " + str(dst) + " = " + str(lower) + ";// sugar@79\n"
    generated_code += " else " + str(dst) + " += " + str(src) + ";// sugar@80\n"
    return generated_code

# =============================================================================
# Helpers for big numbers

buff = 0

# *valuebuff = value-in-width
def write_int_to_bytebuff(value, width):
    global buff
    generated_code = ""
    l = int_to_big_endian_byte_array_with_length(value, width)
    generated_code += " uint8_t buffer_" + str(buff) + "[" + str(len(l)) + "] = {// sugar@93\n"
    for c in l:
        generated_code += "     " + str(c) + ",// sugar@95\n"
    generated_code += " };// sugar@96\n"
    buff = buff + 1
    return generated_code

# =============================================================================
# MASK_GEN

def modify_field_mask( mask ):
    generated_code = ""
    mask_code = ""
    if isinstance(mask, int):
        mask_code = '0x%s' % format(mask,'x')
    elif isinstance(mask, p4_field):
        if mask.width <= 32:
            generated_code += " EXTRACT_INT32_BITS(pd, " + str(fld_id(mask)) + ", mask32)// sugar@110\n"
            mask_code = 'mask32'
        else:
            addError("generating modify_field_mask", "Modify field mask is not supported.")
    elif isinstance(mask, p4_signature_ref):
        p = "parameters.%s" % str(fun.signature[mask.idx])
        l = fun.signature_widths[mask.idx]
        if l <= 32:
            generated_code += " mask32 = *" + str(p) + ";// sugar@118\n"
            mask_code = 'mask32'
        else:
            addError("generating modify_field_mask", "Modify field mask is not supported.")
    else:
        addError("generating modify_field_mask", "Modify field mask cannot be recognized.")
    return (generated_code, mask_code)

# =============================================================================
# MODIFY_FIELD

def modify_field(fun, call):
    generated_code = ""
    args = call[1]
    dst = args[0]
    src = args[1]
    mask = ''
    if len(args)==3:
       (gc,m) = modify_field_mask( args[2] )
       print "MASK: %s %s" % (fun.name, m)
       generated_code += " " + str(gc) + "// sugar@138\n"
       mask = ' & %s' % m
    if not isinstance(dst, p4_field):
        addError("generating modify_field", "We do not allow changing an R-REF yet")
    if isinstance(src, int):
        if not is_vwf(dst) and dst.width <= 32:
            generated_code += " value32 = " + str(src) + "" + str(mask) + ";// sugar@144\n"
            generated_code += " " + str(modify_int32_int32(dst) ) + "// sugar@145\n"
        else:
            if is_field_byte_aligned(dst):
                generated_code += " " + str(write_int_to_bytebuff(src, field_max_width(dst)/8) ) + "// sugar@148\n"
                if is_vwf(dst):
                    generated_code += " MODIFY_BYTEBUF_BYTEBUF(pd, " + str(fld_id(dst)) + ", buffer_" + str(buff-1) + "+(" + str(field_max_width(dst)/8) + "-field_desc(pd, " + str(fld_id(dst)) + ").bytewidth), field_desc(pd, " + str(fld_id(dst)) + ").bytewidth)// sugar@150\n"
                else:
                    generated_code += " MODIFY_BYTEBUF_BYTEBUF(pd, " + str(fld_id(dst)) + ", buffer_" + str(buff-1) + ", " + str(dst.width/8) + ")// sugar@152\n"
            else:
                if is_vwf(dst):
                    addError("generating modify_field", "Modifying non byte-wide variable width field '" + str(dst) + "' with int is not supported")
                else:
                    addError("generating modify_field", "Improper bytebufs cannot be modified yet.")
    elif isinstance(src, p4_field):
        if not is_vwf(dst) and not is_vwf(src) and dst.width <= 32 and src.width <= 32:
            if src.instance.metadata == dst.instance.metadata:
                generated_code += " EXTRACT_INT32_BITS(pd, " + str(fld_id(src)) + ", value32)// sugar@161\n"
                generated_code += " MODIFY_INT32_INT32_BITS(pd, " + str(fld_id(dst)) + ", value32" + str(mask) + ")// sugar@162\n"
            else:
                generated_code += " " + str(extract_int32(src, 'value32', mask) ) + "// sugar@164\n"
                generated_code += " " + str(modify_int32_int32(dst) ) + "// sugar@165\n"
        else:
            if is_field_byte_aligned(dst) and is_field_byte_aligned(src) and src.instance.metadata == dst.instance.metadata:
                if mask: # TODO: Mask handling is missing
                    addError("generating modify_field", "Using masks in modify_field on fields longer than 4 bytes is not supported")
                src_fd = "field_desc(pd, " + fld_id(src) + ")"
                dst_fd = "field_desc(pd, " + fld_id(dst) + ")"
                generated_code += " if(" + str(src_fd) + ".bytewidth < " + str(dst_fd) + ".bytewidth) {// sugar@172\n"
                generated_code += "     MODIFY_BYTEBUF_BYTEBUF(pd, " + str(fld_id(dst)) + ", " + str(src_fd) + ".byte_addr, " + str(src_fd) + ".bytewidth);// sugar@173\n"
                generated_code += " } else {// sugar@174\n"
                generated_code += "     MODIFY_BYTEBUF_BYTEBUF(pd, " + str(fld_id(dst)) + ", " + str(src_fd) + ".byte_addr + (" + str(src_fd) + ".bytewidth - " + str(dst_fd) + ".bytewidth), " + str(dst_fd) + ".bytewidth);// sugar@175\n"
                generated_code += " }// sugar@176\n"
            else:
                if is_vwf(dst):
                    addError("generating modify_field", "Modifying field '" + str(dst) + "' with field '" + str(src) + "' (one of which is a non byte-wide variable width field) is not supported")
                else:
                    addError("generating modify_field", "Improper bytebufs cannot be modified yet.")
    elif isinstance(src, p4_signature_ref):
        p = "parameters.%s" % str(fun.signature[src.idx])
        l = fun.signature_widths[src.idx]
        # TODO: Mask handling
        if not is_vwf(dst) and dst.width <= 32 and l <= 32:
            generated_code += " MODIFY_INT32_BYTEBUF(pd, " + str(fld_id(dst)) + ", " + str(p) + ", " + str((l+7)/8) + ")// sugar@187\n"
        else:
            if is_field_byte_aligned(dst) and l % 8 == 0: #and dst.instance.metadata:
                dst_fd = "field_desc(pd, " + fld_id(dst) + ")"
                generated_code += " if(" + str(l/8) + " < " + str(dst_fd) + ".bytewidth) {// sugar@191\n"
                generated_code += "     MODIFY_BYTEBUF_BYTEBUF(pd, " + str(fld_id(dst)) + ", " + str(p) + ", " + str(l/8) + ");                // sugar@192\n"
                generated_code += " } else {// sugar@193\n"
                generated_code += "     MODIFY_BYTEBUF_BYTEBUF(pd, " + str(fld_id(dst)) + ", " + str(p) + " + (" + str(l/8) + " - " + str(dst_fd) + ".bytewidth), " + str(dst_fd) + ".bytewidth)// sugar@194\n"
                generated_code += " }// sugar@195\n"
            else:
                if is_vwf(dst):
                    addError("generating modify_field", "Modifying non byte-wide variable width field '" + str(src) + "' with p4_signature_ref is not supported")
                else:
                    addError("generating modify_field", "Improper bytebufs cannot be modified yet.")        
    return generated_code

# =============================================================================
# ADD_TO_FIELD

def add_to_field(fun, call):
    generated_code = ""
    args = call[1]
    dst = args[0]
    val = args[1]
    if not isinstance(dst, p4_field):
        addError("generating add_to_field", "We do not allow changing an R-REF yet")
    if isinstance(val, int):
        generated_code += " value32 = " + str(val) + ";// sugar@214\n"
        if not is_vwf(dst) and dst.width <= 32:
            generated_code += " " + str(extract_int32(dst, 'res32') ) + "// sugar@216\n"
            if (p4_header_keywords.saturating in dst.attributes):
               generated_code += " " + str(add_with_saturating('value32', 'res32', dst.width, (p4_header_keywords.signed in dst.attributes)) ) + "// sugar@218\n"
            else:
                generated_code += " value32 += res32;// sugar@220\n"
            generated_code += " " + str(modify_int32_int32(dst) ) + "// sugar@221\n"
        else:
            if is_vwf(dst):
                addError("generating add_to_field", "add_to_field on variable width field '" + str(dst) + "' is not supported yet")
            else:
                addError("generating add_to_field", "add_to_field on bytebufs (with int) is not supported yet (field: " + str(dst) + ")")
    elif isinstance(val, p4_field):
        if not is_vwf(val) and not is_vwf(dst) and dst.width <= 32 and val.width <= 32:
            generated_code += " " + str(extract_int32(val, 'value32') ) + "// sugar@229\n"
            generated_code += " " + str(extract_int32(dst, 'res32') ) + "// sugar@230\n"
            if (p4_header_keywords.saturating in dst.attributes):
               generated_code += " " + str(add_with_saturating('value32', 'res32', dst.width, (p4_header_keywords.signed in dst.attributes)) ) + "// sugar@232\n"
            else:
                generated_code += " value32 += res32;// sugar@234\n"
            generated_code += " " + str(modify_int32_int32(dst) ) + "// sugar@235\n"
        else:
            if is_vwf(val) or is_vwf(dst):
                addError("generating add_to_field", "add_to_field on field '" + str(dst) + "' with field '" + str(val) + "' is not supported yet. One of the fields is a variable width field!")
            else:
                addError("generating add_to_field", "add_to_field on/with bytebufs is not supported yet (fields: " + str(val) + ", " + str(dst) + ")")
    elif isinstance(val, p4_signature_ref):
        p = "parameters.%s" % str(fun.signature[val.idx])
        l = fun.signature_widths[val.idx]
        if not is_vwf(dst) and dst.width <= 32 and l <= 32:
            generated_code += " " + str(extract_int32(dst, 'res32') ) + "// sugar@245\n"
            generated_code += " TODO// sugar@246\n"
        else:
            if is_vwf(dst):
                addError("generating add_to_field", "add_to_field on variable width field '" + str(dst) + "' with p4_signature_ref is not supported yet")
            else:
                addError("generating add_to_field", "add_to_field on bytebufs (with p4_signature_ref) is not supported yet (field: " + str(dst) + ")")
    return generated_code

# =============================================================================
# COUNT

def count(fun, call):
    generated_code = ""
    args = call[1]
    counter = args[0]
    index = args[1]
    if isinstance(index, int): # TODO
        generated_code += " value32 = " + str(index) + ";// sugar@263\n"
    elif isinstance(index, p4_field): # TODO
        generated_code += " " + str(extract_int32(index, 'value32') ) + "// sugar@265\n"
    elif isinstance(val, p4_signature_ref):
        generated_code += " value32 = TODO;// sugar@267\n"
    generated_code += " increase_counter(COUNTER_" + str(counter.name) + ", value32);// sugar@268\n"
    return generated_code

# =============================================================================
# REGISTER_READ

rc = 0

def register_read(fun, call):
    global rc
    generated_code = ""
    args = call[1]
    dst = args[0] # field
    register = args[1]
    index = args[2]
    if isinstance(index, int): # TODO
        generated_code += " value32 = " + str(index) + ";// sugar@284\n"
    elif isinstance(index, p4_field): # TODO
        generated_code += " " + str(extract_int32(index, 'value32') ) + "// sugar@286\n"
    elif isinstance(val, p4_signature_ref):
        generated_code += " value32 = TODO;// sugar@288\n"
    if (register.width+7)/8 < 4:
        generated_code += " uint8_t register_value_" + str(rc) + "[4];// sugar@290\n"
    else:
        generated_code += " uint8_t register_value_" + str(rc) + "[" + str((register.width+7)/8) + "];// sugar@292\n"
    generated_code += " read_register(REGISTER_" + str(register.name) + ", value32, register_value_" + str(rc) + ");// sugar@293\n"
    if not is_vwf(dst) and dst.width <= 32:
        generated_code += " memcpy(&value32, register_value_" + str(rc) + ", 4);// sugar@295\n"
        generated_code += " " + str(modify_int32_int32(dst) ) + "// sugar@296\n"
    else:
        if is_field_byte_aligned(dst) and register.width % 8 == 0:
            dst_fd = "field_desc(pd, " + fld_id(dst) + ")"
            reg_bw = register.width / 8
            generated_code += " if(" + str(reg_bw) + " < " + str(dst_fd) + ".bytewidth) {// sugar@301\n"
            generated_code += "     MODIFY_BYTEBUF_BYTEBUF(pd, " + str(fld_id(dst)) + ", register_value_" + str(rc) + ", " + str(reg_bw) + ");// sugar@302\n"
            generated_code += " } else {// sugar@303\n"
            generated_code += "     MODIFY_BYTEBUF_BYTEBUF(pd, " + str(fld_id(dst)) + ", register_value_" + str(rc) + " + (" + str(reg_bw) + " - " + str(dst_fd) + ".bytewidth), " + str(dst_fd) + ".bytewidth);// sugar@304\n"
            generated_code += " }// sugar@305\n"
        else:
            addError("generating register_read", "Improper bytebufs cannot be modified yet.")
    rc = rc + 1
    return generated_code

# =============================================================================
# REGISTER_WRITE

def register_write(fun, call):
    global rc
    generated_code = ""
    args = call[1]
    register = args[0] # field
    index = args[1]
    src = args[2]
    if isinstance(index, int): # TODO
        generated_code += " res32 = " + str(index) + ";// sugar@322\n"
    elif isinstance(index, p4_field): # TODO
        generated_code += " " + str(extract_int32(index, 'res32') ) + "// sugar@324\n"
    elif isinstance(val, p4_signature_ref):
        generated_code += " res32 = TODO;// sugar@326\n"
    if (register.width+7)/8 < 4:
        generated_code += " uint8_t register_value_" + str(rc) + "[4];// sugar@328\n"
    else:
        generated_code += " uint8_t register_value_" + str(rc) + "[" + str((register.width+7)/8) + "];// sugar@330\n"
    if isinstance(src, int):
        generated_code += " value32 = " + str(src) + ";// sugar@332\n"
        generated_code += " memcpy(register_value_" + str(rc) + ", &value32, 4);// sugar@333\n"
    elif isinstance(src, p4_field):
        if is_vwf(src):
            addError("generating register_write", "Variable width field '" + str(src) + "' in register_write is not supported yet")
        elif register.width <= 32 and src.width <= 32:
            generated_code += " " + str(extract_int32(src, 'value32') ) + "// sugar@338\n"
            generated_code += " memcpy(register_value_" + str(rc) + ", &value32, 4);// sugar@339\n"
        else:
            if src.width == register.width:
                if src.width % 8 == 0 and src.offset % 8 == 0: # and src.instance.metadata == dst.instance.metadata:
                    generated_code += " EXTRACT_BYTEBUF(pd, " + str(fld_id(src)) + ", register_value_" + str(rc) + ")// sugar@343\n"
                else:
                    addError("generating register_write", "Improper bytebufs cannot be modified yet.")
            else:
                addError("generating register_write", "Write register-to-field of different widths is not supported yet.")
    generated_code += " write_register(REGISTER_" + str(register.name) + ", res32, register_value_" + str(rc) + ");// sugar@348\n"
    rc = rc + 1
    return generated_code

# =============================================================================
# GENERATE_DIGEST

def generate_digest(fun, call):
    generated_code = ""
    
    ## TODO make this proper
    extracted_params = []
    for p in call[1]:
        if isinstance(p, int):
            extracted_params += "0" #[str(p)]
        elif isinstance(p, p4_field_list):
            field_list = p
            extracted_params += ["&fields"]
        else:
            addError("generating actions.c", "Unhandled parameter type in generate_digest: " + str(p))
    fun_params = ["bg"] + ["\""+field_list.name+"\""] + extracted_params
    generated_code += "  struct type_field_list fields;// sugar@369\n"
    quan = str(len(field_list.fields))
    generated_code += "    fields.fields_quantity = " + str(quan) + ";// sugar@371\n"
    generated_code += "    fields.field_offsets = malloc(sizeof(uint8_t*)*fields.fields_quantity);// sugar@372\n"
    generated_code += "    fields.field_widths = malloc(sizeof(uint8_t*)*fields.fields_quantity);// sugar@373\n"
    for i,field in enumerate(field_list.fields):
        j = str(i)
        if isinstance(field, p4_field):
            generated_code += "    fields.field_offsets[" + str(j) + "] = (uint8_t*) field_desc(pd, " + str(fld_id(field)) + ").byte_addr;// sugar@377\n"
            generated_code += "    fields.field_widths[" + str(j) + "]  =            field_desc(pd, " + str(fld_id(field)) + ").bitwidth;// sugar@378\n"
        else:
            addError("generating actions.c", "Unhandled parameter type in field_list: " + name + ", " + str(field))

    params = ",".join(fun_params)
    generated_code += "\n"
    generated_code += "    generate_digest(" + str(params) + "); sleep(1);// sugar@384\n"
    return generated_code

# =============================================================================
# DROP

def drop(fun, call):
    generated_code = ""
    generated_code += " debug(\"    :: SETTING PACKET TO BE DROPPED\\n\");// sugar@392\n"
    generated_code += " pd->dropped=1;// sugar@393\n"
    return generated_code;

# =============================================================================
# RESUBMIT

def resubmit(fun, call):
    generated_code = ""
    generated_code += " debug(\"    :: RESUBMITTING PACKET\\n\");// sugar@401\n"
    generated_code += " handle_packet(pd, tables);// sugar@402\n"
    return generated_code;

# =============================================================================
# NO_OP

def no_op(fun, call):
    return "no_op(); // no_op"

# =============================================================================
# PUSH

def push(fun, call):
    generated_code = ""
    args = call[1]
    i = args[0]
    generated_code += " push(pd, header_stack_" + str(i.base_name) + ");// sugar@418\n"
    return generated_code

# =============================================================================
# POP

def pop(fun, call):
    generated_code = ""
    args = call[1]
    i = args[0]
    generated_code += " pop(pd, header_stack_" + str(i.base_name) + ");// sugar@428\n"
    return generated_code

# =============================================================================

for fun in userActions(hlir):
    hasParam = fun.signature
    modifiers = ""
    ret_val_type = "void"
    name = fun.name
    params = ", struct action_%s_params parameters" % (name) if hasParam else ""
    generated_code += " " + str(modifiers) + " " + str(ret_val_type) + " action_code_" + str(name) + "(packet_descriptor_t* pd, lookup_table_t** tables " + str(params) + ") {// sugar@439\n"
    generated_code += "     uint32_t value32, res32, mask32;// sugar@440\n"
    generated_code += "     (void)value32; (void)res32; (void)mask32;// sugar@441\n"
    for i,call in enumerate(fun.call_sequence):
        name = call[0].name 

        # Generates a primitive action call to `name'
        if name in locals().keys():
            generated_code += " " + str(locals()[name](fun, call)) + "// sugar@447\n"
        else:
            addWarning("generating actions.c", "Unhandled primitive function: " +  name)

    generated_code += " }// sugar@451\n"
    generated_code += "\n"