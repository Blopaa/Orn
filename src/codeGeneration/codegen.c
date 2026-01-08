#include "./codegen.h"
#include "./emiter.h"

CodeGenContext *createCodeGenContext(void) {
    CodeGenContext *ctx = calloc(1, sizeof(CodeGenContext));
    if (!ctx) return NULL;
    
    ctx->data = sbCreate(4096);
    ctx->text = sbCreate(16384);
    ctx->stringPool = NULL;
    ctx->doublePool = NULL;
    ctx->floatPool = NULL;
    ctx->nextLab = 0;
    ctx->globalVars = NULL;
    ctx->globalTemps = NULL;
    ctx->globalStackOff = 0;
    ctx->currentFn = NULL;
    ctx->inFn = 0;
    ctx->maxTempNum = 0;
    ctx->lastParamType = IR_TYPE_INT;
    
    return ctx;
}

void freeCodeGenContext(CodeGenContext *ctx) {
    if (!ctx) return;
    
    sbFree(&ctx->data);
    sbFree(&ctx->text);

    StringEntry *se = ctx->stringPool;
    while (se) {
        StringEntry *next = se->next;
        free(se);
        se = next;
    }
    
    VarLoc *vl = ctx->globalVars;
    while (vl) {
        VarLoc *next = vl->next;
        free(vl);
        vl = next;
    }
    
    TempLoc *tl = ctx->globalTemps;
    while (tl) {
        TempLoc *next = tl->next;
        free(tl);
        tl = next;
    }
    
    if (ctx->currentFn) {
        VarLoc *loc = ctx->currentFn->locs;
        while (loc) {
            VarLoc *next = loc->next;
            free(loc);
            loc = next;
        }
        TempLoc *temp = ctx->currentFn->temps;
        while (temp) {
            TempLoc *next = temp->next;
            free(temp);
            temp = next;
        }
        free(ctx->currentFn);
    }
    
    free(ctx);
}

void loadOp(CodeGenContext *ctx, IrOperand *op, const char *reg){
    switch(op->type){
        case OPERAND_CONSTANT:
            switch(op->dataType){
                case IR_TYPE_STRING: {
                    int label = addStringLit(ctx, op->value.constant.str.stringVal, op->value.constant.str.len);
                    emitInstruction(ctx, "leaq .LC%d(%%rip), %s", label, getIntReg(reg, IR_TYPE_STRING));
                    break;
                }
                case IR_TYPE_FLOAT: 
                case IR_TYPE_DOUBLE: {
                    int label;
                    if(op->dataType == IR_TYPE_FLOAT){
                        label = addFloatLit(ctx, op->value.constant.floatVal);
                    }else{
                        label = addDoubleLit(ctx, op->value.constant.doubleVal);
                    }
                    
                    emitInstruction(ctx, "mov%s .LC%d(%%rip), %s", getSSESuffix(op->dataType), label, reg);
                    break;
                }
                // ints and bools
                default: 
                    emitInstruction(ctx, "mov%s $%d, %s",
                                getIntSuffix(op->dataType),
                                op->value.constant.intVal,
                                getIntReg(reg, op->dataType));
                    break;
            }
            break;
        case OPERAND_VAR:
        case OPERAND_TEMP: 
            int off = op->type == OPERAND_VAR 
                ? getVarOffset(ctx, op->value.var.name, op->value.var.nameLen) 
                : getTempOffset(ctx, op->value.temp.tempNum, op->dataType);
            
            switch(op->dataType){
                case IR_TYPE_FLOAT:
                case IR_TYPE_DOUBLE:
                    emitInstruction(ctx, "mov%s %d(%%rbp), %s",getSSESuffix(op->dataType), off, reg);
                    break;
                default:
                    emitInstruction(ctx, "mov%s %d(%%rbp), %s",getIntSuffix(op->dataType), off, getIntReg(reg, op->dataType));
                    break;
            }
        break;
        
        default: break;
    }
}

void storeOp(CodeGenContext *ctx, const char *reg, IrOperand *op){
    if(op->type != OPERAND_VAR && op->type != OPERAND_TEMP) return;
    int off;
    if (op->type == OPERAND_VAR) {
        addLocalVar(ctx, op->value.var.name, op->value.var.nameLen, op->dataType);
        off = getVarOffset(ctx, op->value.var.name, op->value.var.nameLen);
    } else {
        off = getTempOffset(ctx, op->value.temp.tempNum, op->dataType);
    }
    
    if (isFloatingPoint(op->dataType)) {
        emitInstruction(ctx, "mov%s %s, %d(%%rbp)", getSSESuffix(op->dataType), reg, off);
    } else {
        emitInstruction(ctx, "mov%s %s, %d(%%rbp)", getIntSuffix(op->dataType), getIntReg(reg, op->dataType), off);
    }
}
void genPointerLoad(CodeGenContext *ctx, IrInstruction *inst) {

    IrOperand *result = &inst->result;
    IrOperand *base = &inst->ar1;
    IrOperand *offset = &inst->ar2;

    if (base->type != OPERAND_VAR) return;

    VarLoc *baseVar = findVar(ctx, base->value.var.name, base->value.var.nameLen);
    if (!baseVar) return;

    IrDataType elemType = result->dataType;
    int elemSize = getTypeSize(elemType);

    loadOp(ctx, offset, "a");

    if (isFloatingPoint(elemType)) {
        emitInstruction(ctx, "mov%s %d(%%rbp,%%rax,%d), %%xmm0", getSSESuffix(elemType),
                        baseVar->stackOffset, elemSize);
        storeOp(ctx, "%xmm0", result);
    } else {
        emitInstruction(ctx, "mov%s %d(%%rbp,%%rax,%d), %s", getIntSuffix(elemType),
                        baseVar->stackOffset, elemSize, getIntReg("a", elemType));
        storeOp(ctx, "a", result);
    }
}

void genPointerStore(CodeGenContext *ctx, IrInstruction *inst) {

    IrOperand *base = &inst->result;
    IrOperand *offset = &inst->ar1;
    IrOperand *value = &inst->ar2;

    if (base->type != OPERAND_VAR) return;

    VarLoc *baseVar = findVar(ctx, base->value.var.name, base->value.var.nameLen);
    if (!baseVar) return;

    IrDataType elemType = value->dataType;
    int elemSize = getTypeSize(elemType);

    if (isFloatingPoint(elemType)) {
        loadOp(ctx, value, "%xmm0");
    } else {
        loadOp(ctx, value, "d");
    }

    loadOp(ctx, offset, "a");
    
    if (isFloatingPoint(elemType)) {
        emitInstruction(ctx, "mov%s %%xmm0, %d(%%rbp,%%rax,%d)", getSSESuffix(elemType),
                        baseVar->stackOffset, elemSize);
    } else {
        emitInstruction(ctx, "mov%s %s, %d(%%rbp,%%rax,%d)", getIntSuffix(elemType),
                        getIntReg("d", elemType), baseVar->stackOffset, elemSize);
    }
}

void genBitwiseOp(CodeGenContext *ctx, IrInstruction *inst){
    IrDataType type = inst->result.dataType;
    
    loadOp(ctx, &inst->ar1, "a");
    loadOp(ctx, &inst->ar2, "c");
    
    const char *suffix = getIntSuffix(type);
    const char *regA = getIntReg("a", type);
    const char *regC = getIntReg("c", type);
    
    switch (inst->op) {
        case IR_BIT_AND:
            emitInstruction(ctx, "and%s %s, %s", suffix, regC, regA);
            break;
        case IR_BIT_OR:
            emitInstruction(ctx, "or%s %s, %s", suffix, regC, regA);
            break;
        case IR_BIT_XOR:
            emitInstruction(ctx, "xor%s %s, %s", suffix, regC, regA);
            break;
        case IR_SHL:
            emitInstruction(ctx, "shl%s %%cl, %s", suffix, regA);
            break;
        case IR_SHR:
            emitInstruction(ctx, "shr%s %%cl, %s", suffix, regA);
            break;
        default:
            break;
    }
    
    storeOp(ctx, "a", &inst->result);
}

void genBinaryOp(CodeGenContext *ctx, IrInstruction *inst){
    IrDataType type = inst->result.dataType;
    if(isFloatingPoint(type)){
        loadOp(ctx, &inst->ar1, "%xmm0");
        loadOp(ctx, &inst->ar2, "%xmm1");
        const char *suffix = getSSESuffix(type);
        switch (inst->op) {
            case IR_ADD: emitInstruction(ctx, "add%s %%xmm1, %%xmm0", suffix); break;
            case IR_SUB: emitInstruction(ctx, "sub%s %%xmm1, %%xmm0", suffix); break;
            case IR_MUL: emitInstruction(ctx, "mul%s %%xmm1, %%xmm0", suffix); break;
            case IR_DIV: emitInstruction(ctx, "div%s %%xmm1, %%xmm0", suffix); break;
            default: break;
        }

        storeOp(ctx, "xmm0", &inst->result);
    }else {
        loadOp(ctx, &inst->ar1, "a");
        loadOp(ctx, &inst->ar2, "c");
        
        const char *suffix = getIntSuffix(type);
        const char *regA = getIntReg("a", type);
        const char *regC = getIntReg("c", type);
        
        switch (inst->op) {
            case IR_ADD:
                emitInstruction(ctx, "add%s %s, %s", suffix, regC, regA);
                break;
            case IR_SUB:
                emitInstruction(ctx, "sub%s %s, %s", suffix, regC, regA);
                break;
            case IR_MUL:
                emitInstruction(ctx, "imul%s %s, %s", suffix, regC, regA);
                break;
            case IR_DIV:
                emitInstruction(ctx, "cltd");
                emitInstruction(ctx, "idiv%s %s", suffix, regC);
                break;
            case IR_MOD:
                emitInstruction(ctx, "cltd");
                emitInstruction(ctx, "idiv%s %s", suffix, regC);
                emitInstruction(ctx, "mov%s %s, %s", suffix, getIntReg("d", type), regA);
                break;
            default:
                break;
        }
        
        storeOp(ctx, "a", &inst->result);
    }
}

void genUnaryOp(CodeGenContext *ctx, IrInstruction *inst) {
    IrDataType type = inst->result.dataType;
    
    if (isFloatingPoint(type)) {
        loadOp(ctx, &inst->ar1, "%xmm0");
        
        if (inst->op == IR_NEG) {
            if (type == IR_TYPE_FLOAT) {
                emitInstruction(ctx, "movl $0x80000000, %%eax");
                emitInstruction(ctx, "movd %%eax, %%xmm1");
                emitInstruction(ctx, "xorps %%xmm1, %%xmm0");
            } else {
                emitInstruction(ctx, "movabsq $0x8000000000000000, %%rax");
                emitInstruction(ctx, "movq %%rax, %%xmm1");
                emitInstruction(ctx, "xorpd %%xmm1, %%xmm0");
            }
        }
        
        storeOp(ctx, "%xmm0", &inst->result);
    }
    else {
        loadOp(ctx, &inst->ar1, "a");
        
        const char *suffix = getIntSuffix(type);
        const char *regA = getIntReg("a", type);
        
        switch (inst->op) {
            case IR_NEG:
                emitInstruction(ctx, "neg%s %s", suffix, regA);
                break;
            case IR_NOT:
                emitInstruction(ctx, "xor%s $1, %s", suffix, regA);
                break;
            default:
                break;
        }
        
        storeOp(ctx, "a", &inst->result);
    }
}

void genReqMem(CodeGenContext *ctx, IrInstruction *inst) {
    if (inst->result.type != OPERAND_VAR || inst->ar1.type != OPERAND_CONSTANT) {
        return;
    }

    addLocalVar(ctx, inst->result.value.var.name, inst->result.value.var.nameLen,
                inst->result.dataType);

    int arraySize = inst->ar1.value.constant.intVal;
    markVarAsArray(ctx, inst->result.value.var.name, inst->result.value.var.nameLen, arraySize);
}

void genCopy(CodeGenContext *ctx, IrInstruction *inst) {
    IrDataType type = inst->result.dataType;
    
    if (inst->result.type == OPERAND_VAR && 
        inst->ar1.type == OPERAND_VAR &&
        inst->ar2.type == OPERAND_CONSTANT &&
        inst->result.value.var.nameLen == inst->ar1.value.var.nameLen &&
        memcmp(inst->result.value.var.name, inst->ar1.value.var.name, inst->result.value.var.nameLen) == 0) {
        
        int paramIndex = inst->ar2.value.constant.intVal;
        
        addLocalVar(ctx, inst->result.value.var.name, inst->result.value.var.nameLen, type);
        int off = getVarOffset(ctx, inst->result.value.var.name, inst->result.value.var.nameLen);
        
        if (isFloatingPoint(type)) {
            if (paramIndex < 8) {
                emitInstruction(ctx, "mov%s %s, %d(%%rbp)", 
                               getSSESuffix(type), 
                               getSSEReg(paramIndex), 
                               off);
            }
        } else {
            if (paramIndex < 6) {
                const char *reg = isFloatingPoint(type) ? getSSEReg(paramIndex)
                                                        : getParamIntReg(paramIndex, type);
                emitInstruction(ctx, "mov%s %s, %d(%%rbp)", 
                               getIntSuffix(type), 
                               reg, 
                               off);
            }
        }
        return;
    }
    
    if (isFloatingPoint(type)) {
        loadOp(ctx, &inst->ar1, "%xmm0");
        storeOp(ctx, "%xmm0", &inst->result);
    } else {
        loadOp(ctx, &inst->ar1, "a");
        storeOp(ctx, "a", &inst->result);
    }
}

void genGoto(CodeGenContext *ctx, IrInstruction *inst) {
    int label = inst->ar1.value.label.labelNum;
    emitInstruction(ctx, "jmp .L%d", label);
}

void genIfFalse(CodeGenContext *ctx, IrInstruction *inst) {
    IrDataType type = inst->ar1.dataType;
    int label = inst->ar2.value.label.labelNum;
    
    if (isFloatingPoint(type)) {
        loadOp(ctx, &inst->ar1, "%xmm0");
        emitInstruction(ctx, "xorpd %%xmm1, %%xmm1");
        if (type == IR_TYPE_FLOAT) {
            emitInstruction(ctx, "ucomiss %%xmm1, %%xmm0");
        } else {
            emitInstruction(ctx, "ucomisd %%xmm1, %%xmm0");
        }
        emitInstruction(ctx, "je .L%d", label);
    } else {
        loadOp(ctx, &inst->ar1, "a");
        emitInstruction(ctx, "test%s %s, %s", 
                       getIntSuffix(type),
                       getIntReg("a", type),
                       getIntReg("a", type));
        emitInstruction(ctx, "je .L%d", label);
    }
}

void genReturn(CodeGenContext *ctx, IrInstruction *inst) {
    if (inst->op == IR_RETURN && inst->ar1.type != OPERAND_NONE) {
        IrDataType type = inst->ar1.dataType;
        if (isFloatingPoint(type)) {
            loadOp(ctx, &inst->ar1, "%xmm0");
        } else {
            loadOp(ctx, &inst->ar1, "a");
        }
    }
    
    emitInstruction(ctx, "movq %%rbp, %%rsp");
    emitInstruction(ctx, "popq %%rbp");
    emitInstruction(ctx, "ret");
}

void genParam(CodeGenContext *ctx, IrInstruction *inst, int paramIndex) {
    ctx->lastParamType = inst->ar1.dataType;
    static const char *intRegs[] = {"di", "si", "d", "c", "8", "9"};
    
    IrDataType type = inst->ar1.dataType;
    
    if (isFloatingPoint(type)) {
        if (paramIndex < 8) {
            loadOp(ctx, &inst->ar1, getSSEReg(paramIndex));
        } else {
            // Spill to stack
            loadOp(ctx, &inst->ar1, "%xmm0");
            emitInstruction(ctx, "sub%s $8, %%rsp", getIntSuffix(IR_TYPE_STRING));
            emitInstruction(ctx, "mov%s %%xmm0, (%%rsp)", getSSESuffix(type));
        }
    } else {
        if (paramIndex < 6) {
            loadOp(ctx, &inst->ar1, intRegs[paramIndex]);
        } else {
            loadOp(ctx, &inst->ar1, "a");
            emitInstruction(ctx, "pushq %%rax");
        }
    }
}

void genCall(CodeGenContext *ctx, IrInstruction *inst) {
        const char *fnName = inst->ar1.value.fn.name;
    size_t fnLen = inst->ar1.value.fn.nameLen;
    
    if (fnLen == 5 && memcmp(fnName, "print", 5) == 0) {
        switch (ctx->lastParamType) {
            case IR_TYPE_STRING:
                emitInstruction(ctx, "call print_str_z");
                break;
            case IR_TYPE_BOOL:
                emitInstruction(ctx, "call print_bool");
                break;
            case IR_TYPE_INT:
                emitInstruction(ctx, "call print_int");
                break;
            case IR_TYPE_FLOAT:
                emitInstruction(ctx, "call print_float");
                break;
            case IR_TYPE_DOUBLE:
                emitInstruction(ctx, "call print_double");
                break;
            default:
                emitInstruction(ctx, "call print_int");
                break;
        }
    }else{
        emitInstruction(ctx, "call %.*s",
                   (int)fnLen,
                   fnName);
    }
    
    if (inst->result.type != OPERAND_NONE) {
        IrDataType type = inst->result.dataType;
        if (isFloatingPoint(type)) {
            storeOp(ctx, "%xmm0", &inst->result);
        } else {
            storeOp(ctx, "a", &inst->result);
        }
    }
}

void genFuncBegin(CodeGenContext *ctx, IrInstruction *inst) {
    FuncInfo *func = calloc(1, sizeof(struct FuncInfo));
    func->name = inst->result.value.fn.name;
    func->nameLen = inst->result.value.fn.nameLen;
    func->stackSize = 0;
    
    ctx->currentFn = func;
    ctx->inFn = 1;
    
    sbAppendf(&ctx->text, "\n    .globl %.*s\n", (int)func->nameLen, func->name);
    sbAppendf(&ctx->text, "    .type %.*s, @function\n", (int)func->nameLen, func->name);
    sbAppendf(&ctx->text, "%.*s:\n", (int)func->nameLen, func->name);
    
    emitInstruction(ctx, "pushq %%rbp");
    emitInstruction(ctx, "movq %%rsp, %%rbp");
    emitInstruction(ctx, "subq $256, %%rsp");
}

void genFuncEnd(CodeGenContext *ctx, IrInstruction *inst) {
    (void)inst;
    
    emitInstruction(ctx, "movq %%rbp, %%rsp");
    emitInstruction(ctx, "popq %%rbp");
    emitInstruction(ctx, "ret");
    
    if (ctx->currentFn) {
        freeVarList(ctx->currentFn->locs);
        freeTempList(ctx->currentFn->temps);
        free(ctx->currentFn);
        ctx->currentFn = NULL;
    }
    ctx->inFn = 0;
}

void genCast(CodeGenContext *ctx, IrInstruction *inst) {
    IrDataType srcType = inst->ar1.dataType;
    IrDataType dstType = inst->result.dataType;
    
    if (srcType == dstType) {
        genCopy(ctx, inst);
        return;
    }
    
    // Int to Float
    if (srcType == IR_TYPE_INT && dstType == IR_TYPE_FLOAT) {
        loadOp(ctx, &inst->ar1, "a");
        emitInstruction(ctx, "cvtsi2ss %%eax, %%xmm0");
        storeOp(ctx, "%xmm0", &inst->result);
    }
    // Int to Double
    else if (srcType == IR_TYPE_INT && dstType == IR_TYPE_DOUBLE) {
        loadOp(ctx, &inst->ar1, "a");
        emitInstruction(ctx, "cvtsi2sd %%eax, %%xmm0");
        storeOp(ctx, "%xmm0", &inst->result);
    }
    // Float to Int
    else if (srcType == IR_TYPE_FLOAT && dstType == IR_TYPE_INT) {
        loadOp(ctx, &inst->ar1, "%xmm0");
        emitInstruction(ctx, "cvttss2si %%xmm0, %%eax");
        storeOp(ctx, "a", &inst->result);
    }
    // Double to Int
    else if (srcType == IR_TYPE_DOUBLE && dstType == IR_TYPE_INT) {
        loadOp(ctx, &inst->ar1, "%xmm0");
        emitInstruction(ctx, "cvttsd2si %%xmm0, %%eax");
        storeOp(ctx, "a", &inst->result);
    }
    // Float to Double
    else if (srcType == IR_TYPE_FLOAT && dstType == IR_TYPE_DOUBLE) {
        loadOp(ctx, &inst->ar1, "%xmm0");
        emitInstruction(ctx, "cvtss2sd %%xmm0, %%xmm0");
        storeOp(ctx, "%xmm0", &inst->result);
    }
    // Double to Float
    else if (srcType == IR_TYPE_DOUBLE && dstType == IR_TYPE_FLOAT) {
        loadOp(ctx, &inst->ar1, "%xmm0");
        emitInstruction(ctx, "cvtsd2ss %%xmm0, %%xmm0");
        storeOp(ctx, "%xmm0", &inst->result);
    }
    // Bool to Int (or vice versa) - just copy
    else {
        genCopy(ctx, inst);
    }
}

void genComparison(CodeGenContext *ctx, IrInstruction *inst) {
    IrDataType type = inst->ar1.dataType;
    
    if (isFloatingPoint(type)) {
        loadOp(ctx, &inst->ar1, "%xmm0");
        loadOp(ctx, &inst->ar2, "%xmm1");
        
        if (type == IR_TYPE_FLOAT) {
            emitInstruction(ctx, "ucomiss %%xmm1, %%xmm0");
        } else {
            emitInstruction(ctx, "ucomisd %%xmm1, %%xmm0");
        }
    } else {
        loadOp(ctx, &inst->ar1, "a");
        loadOp(ctx, &inst->ar2, "c");
        emitInstruction(ctx, "cmp%s %s, %s",
                       getIntSuffix(type),
                       getIntReg("c", type),
                       getIntReg("a", type));
    }
    
    const char *setInst;
    switch (inst->op) {
        case IR_EQ: setInst = "sete"; break;
        case IR_NE: setInst = "setne"; break;
        case IR_LT: setInst = isFloatingPoint(type) ? "setb" : "setl"; break;
        case IR_LE: setInst = isFloatingPoint(type) ? "setbe" : "setle"; break;
        case IR_GT: setInst = isFloatingPoint(type) ? "seta" : "setg"; break;
        case IR_GE: setInst = isFloatingPoint(type) ? "setae" : "setge"; break;
        default: setInst = "sete"; break;
    }
    
    emitInstruction(ctx, "%s %%al", setInst);
    emitInstruction(ctx, "movzbl %%al, %%eax");
    storeOp(ctx, "a", &inst->result);
}

void genLogical(CodeGenContext *ctx, IrInstruction *inst) {
    loadOp(ctx, &inst->ar1, "a");
    loadOp(ctx, &inst->ar2, "c");
    
    switch (inst->op) {
        case IR_AND:
            emitInstruction(ctx, "andl %%ecx, %%eax");
            break;
        case IR_OR:
            emitInstruction(ctx, "orl %%ecx, %%eax");
            break;
        default:
            break;
    }
    
    storeOp(ctx, "a", &inst->result);
}

void generateInstruction(CodeGenContext *ctx, IrInstruction *inst, int *paramCount) {
    switch (inst->op) {
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
        case IR_MOD:
            genBinaryOp(ctx, inst);
            break;
        case IR_POINTER_LOAD:
            genPointerLoad(ctx, inst);
            break;
            
        case IR_POINTER_STORE:
            genPointerStore(ctx, inst);
            break;
        case IR_BIT_AND:
        case IR_BIT_OR:
        case IR_BIT_XOR:
        case IR_SHL:
        case IR_SHR:
            genBitwiseOp(ctx, inst);
            break;
            
        case IR_NEG:
        case IR_NOT:
            genUnaryOp(ctx, inst);
            break;
            
        case IR_AND:
        case IR_OR:
            genLogical(ctx, inst);
            break;
            
        case IR_EQ:
        case IR_NE:
        case IR_LT:
        case IR_LE:
        case IR_GT:
        case IR_GE:
            genComparison(ctx, inst);
            break;
        case IR_REQ_MEM:
            genReqMem(ctx, inst);
            break;
        case IR_COPY:
            genCopy(ctx, inst);
            break;
            
        case IR_LABEL:
            emitLabelNum(ctx, inst->result.value.label.labelNum);
            break;
            
        case IR_GOTO:
            genGoto(ctx, inst);
            break;
            
        case IR_IF_FALSE:
            genIfFalse(ctx, inst);
            break;
            
        case IR_RETURN:
        case IR_RETURN_VOID:
            genReturn(ctx, inst);
            break;
            
        case IR_PARAM:
            genParam(ctx, inst, (*paramCount)++);
            break;
            
        case IR_CALL:
            genCall(ctx, inst);
            *paramCount = 0;
            break;
            
        case IR_FUNC_BEGIN:
            genFuncBegin(ctx, inst);
            break;
            
        case IR_FUNC_END:
            genFuncEnd(ctx, inst);
            break;
            
        case IR_CAST:
            genCast(ctx, inst);
            break;
            
        default:
            emitComment(ctx, "Unknown instruction");
            break;
    }
}

static void generateMainWrapper(CodeGenContext *ctx) {
    sbAppend(&ctx->text, "\n    .globl main\n");
    sbAppend(&ctx->text, "    .type main, @function\n");
    sbAppend(&ctx->text, "main:\n");
    emitInstruction(ctx, "pushq %%rbp");
    emitInstruction(ctx, "movq %%rsp, %%rbp");
}

static void generateMainEpilogue(CodeGenContext *ctx) {
    emitInstruction(ctx, "movl $0, %%eax");
    emitInstruction(ctx, "movq %%rbp, %%rsp");
    emitInstruction(ctx, "popq %%rbp");
    emitInstruction(ctx, "ret");
}

char *generateAssembly(IrContext *ir) {
    if (!ir) return NULL;
    
    CodeGenContext *ctx = createCodeGenContext();
    if (!ctx) return NULL;
    
    sbAppend(&ctx->data, "    .section .rodata\n");
    
    sbAppend(&ctx->text, "    .text\n");
    
    int paramCount = 0;
    int inUserFunction = 0;
    int mainStarted = 0;
    
    IrInstruction *inst = ir->instructions;
    while (inst) {
        if (inst->op == IR_FUNC_BEGIN) {
            if (mainStarted && !inUserFunction) {
            }
            inUserFunction = 1;
            generateInstruction(ctx, inst, &paramCount);
        } 
        else if (inst->op == IR_FUNC_END) {
            generateInstruction(ctx, inst, &paramCount);
            inUserFunction = 0;
        }
        else if (!inUserFunction) {
            if (!mainStarted) {
                generateMainWrapper(ctx);
                emitInstruction(ctx, "subq $256, %%rsp");
                mainStarted = 1;
            }
            generateInstruction(ctx, inst, &paramCount);
        }
        else {
            generateInstruction(ctx, inst, &paramCount);
        }
        
        inst = inst->next;
    }
    
    if (mainStarted) {
        generateMainEpilogue(ctx);
    }
    
    StringBuffer result = sbCreate(ctx->data.len + ctx->text.len + 100);
    sbAppend(&result, ctx->data.data);
    sbAppend(&result, "\n");
    sbAppend(&result, ctx->text.data);
    
    char *assembly = result.data;
    result.data = NULL;
    
    freeCodeGenContext(ctx);
    
    return assembly;
}

int writeAssemblyToFile(const char *assembly, const char *filename) {
    if (!assembly || !filename) return 0;
    
    FILE *f = fopen(filename, "w");
    if (!f) return 0;
    
    fputs(assembly, f);
    fclose(f);
    
    return 1;
}