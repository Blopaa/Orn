#include "./ir.h"
#include <stdlib.h>
#include "./irHelpers.h"

int binaryConstant(IrInstruction *inst){
    return inst->ar1.type == OPERAND_CONSTANT && inst->ar2.type == OPERAND_CONSTANT;
}

void constantFolding(IrContext *ctx){
    IrInstruction *inst = ctx->instructions;
    while(inst){
        if(binaryConstant(inst)){
            switch(inst->op){
                case IR_ADD: {
                    inst->op = IR_COPY;
                    switch(inst->result.dataType){
                        case IR_TYPE_INT: inst->ar1 = createIntConst(inst->ar1.value.constant.intVal + inst->ar2.value.constant.intVal); break;
                        case IR_TYPE_FLOAT: inst->ar1 = createFloatConst(inst->ar1.value.constant.floatVal + inst->ar2.value.constant.floatVal); break;
                        case IR_TYPE_DOUBLE: inst->ar1 = createDoubleConst(inst->ar1.value.constant.doubleVal + inst->ar2.value.constant.doubleVal); break;
                        default: break;
                    }
                    break;
                }
                case IR_SUB: {
                    inst->op = IR_COPY;
                    switch(inst->result.dataType){
                        case IR_TYPE_INT: inst->ar1 = createIntConst(inst->ar1.value.constant.intVal - inst->ar2.value.constant.intVal); break;
                        case IR_TYPE_FLOAT: inst->ar1 = createFloatConst(inst->ar1.value.constant.floatVal - inst->ar2.value.constant.floatVal); break;
                        case IR_TYPE_DOUBLE: inst->ar1 = createDoubleConst(inst->ar1.value.constant.doubleVal - inst->ar2.value.constant.doubleVal); break;
                        default: break;
                    }
                    break;
                }
                case IR_MUL: {
                    inst->op = IR_COPY;
                    switch(inst->result.dataType){
                        case IR_TYPE_INT: inst->ar1 = createIntConst(inst->ar1.value.constant.intVal * inst->ar2.value.constant.intVal); break;
                        case IR_TYPE_FLOAT: inst->ar1 = createFloatConst(inst->ar1.value.constant.floatVal * inst->ar2.value.constant.floatVal); break;
                        case IR_TYPE_DOUBLE: inst->ar1 = createDoubleConst(inst->ar1.value.constant.doubleVal * inst->ar2.value.constant.doubleVal); break;
                        default: break;
                    }
                    break;
                }
                case IR_DIV: {
                    inst->op = IR_COPY;
                    switch(inst->result.dataType){
                        case IR_TYPE_INT: inst->ar1 = createIntConst(inst->ar1.value.constant.intVal / inst->ar2.value.constant.intVal); break;
                        case IR_TYPE_FLOAT: inst->ar1 = createFloatConst(inst->ar1.value.constant.floatVal / inst->ar2.value.constant.floatVal); break;
                        case IR_TYPE_DOUBLE: inst->ar1 = createDoubleConst(inst->ar1.value.constant.doubleVal / inst->ar2.value.constant.doubleVal); break;
                        default: break;
                    }
                    break;
                }
                default: break;
            }
            inst->ar2 = createNone();
        }
        inst = inst->next;
    }
}

// todo: still a lot to work here
void copyProp(IrContext *ctx){
    IrInstruction *inst = ctx->instructions;
    while (inst){
        if(inst->op == IR_COPY && inst->result.type == OPERAND_VAR && inst->ar1.type == OPERAND_CONSTANT){
            IrInstruction *scan = inst->next;
            while (scan && !(scan->op == IR_FUNC_BEGIN || scan->op == IR_FUNC_END)) {
                if (scan->ar1.type == OPERAND_VAR &&
                    bufferEqual(scan->ar1.value.var.name, scan->ar1.value.var.nameLen,
                                inst->result.value.var.name, inst->result.value.var.nameLen)) {
                    scan->ar1 = inst->ar1;
                }
                if (scan->ar2.type == OPERAND_VAR &&
                    bufferEqual(scan->ar2.value.var.name, scan->ar2.value.var.nameLen,
                                inst->result.value.var.name, inst->result.value.var.nameLen)) {
                    scan->ar2 = inst->ar1;
                }
                if (scan->result.type == OPERAND_VAR &&
                    bufferEqual(scan->result.value.var.name, scan->result.value.var.nameLen,
                                inst->result.value.var.name, inst->result.value.var.nameLen)) {
                    break;
                }
                scan = scan->next;
            }
        }
        inst = inst->next;
    }
}