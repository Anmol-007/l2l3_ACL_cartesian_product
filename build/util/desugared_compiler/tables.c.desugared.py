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
from utils.misc import addError, addWarning
from utils.hlir import getTypeAndLength
import p4_hlir.hlir.p4_stateful as p4_stateful

generated_code += " #include \"dataplane.h\"// sugar@18\n"
generated_code += " #include \"actions.h\"// sugar@19\n"
generated_code += " #include \"data_plane_data.h\"// sugar@20\n"
generated_code += "\n"
generated_code += " lookup_table_t table_config[NB_TABLES] = {// sugar@22\n"
for table in hlir.p4_tables.values():
    table_type, key_length = getTypeAndLength(table)
    generated_code += " {// sugar@25\n"
    generated_code += "  .name= \"" + str(table.name) + "\",// sugar@26\n"
    generated_code += "  .id = TABLE_" + str(table.name) + ",// sugar@27\n"
    generated_code += "  .type = " + str(table_type) + ",// sugar@28\n"
    generated_code += "  .key_size = " + str(key_length) + ",// sugar@29\n"
    generated_code += "  .val_size = sizeof(struct " + str(table.name) + "_action),// sugar@30\n"
    generated_code += "  .min_size = 0, //" + str(table.min_size) + ",// sugar@31\n"
    generated_code += "  .max_size = 255 //" + str(table.max_size) + "// sugar@32\n"
    generated_code += " },// sugar@33\n"
generated_code += " };// sugar@34\n"

generated_code += " counter_t counter_config[NB_COUNTERS] = {// sugar@36\n"
for counter in hlir.p4_counters.values():
    generated_code += " {// sugar@38\n"
    generated_code += "  .name= \"" + str(counter.name) + "\",// sugar@39\n"
    if counter.instance_count is not None:
        generated_code += " .size = " + str(counter.instance_count) + ",// sugar@41\n"
    elif counter.binding is not None:
        btype, table = counter.binding
        if btype is p4_stateful.P4_DIRECT:
            generated_code += " .size = " + str(table.max_size) + ",// sugar@45\n"
    else:
        generated_code += " .size = 1,// sugar@47\n"
    generated_code += "  .min_width = " + str(32 if counter.min_width is None else counter.min_width) + ",// sugar@48\n"
    generated_code += "  .saturating = " + str(1 if counter.saturating else 0) + "// sugar@49\n"
    generated_code += " },// sugar@50\n"
generated_code += " };// sugar@51\n"

generated_code += " p4_register_t register_config[NB_REGISTERS] = {// sugar@53\n"
for register in hlir.p4_registers.values():
    if register.binding is not None:
        addWarning("", "direct and static registers currently treated as plain registers, no optimization occurs")
        continue
    if register.layout is not None:
        addError("", "registers with custom layouts are not supported yet")
        continue
    generated_code += " {// sugar@61\n"
    generated_code += "  .name= \"" + str(register.name) + "\",// sugar@62\n"
    generated_code += "  .size = " + str(register.instance_count) + ",// sugar@63\n"
    generated_code += "  .width = " + str((register.width+7)/8) + ",// sugar@64\n"
    generated_code += " },// sugar@65\n"
generated_code += " };// sugar@66\n"