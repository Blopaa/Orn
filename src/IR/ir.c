#include "./ir.h"
#include <stdlib.h>
#include <stdio.h>
#include "../semantic/symbolTable.h"
#include "../semantic/typeChecker.h"
#include "./irHelpers.h"

IrContext *createIrContext(){
    IrContext *ctx = malloc(sizeof(IrContext));
    if(!ctx) return NULL;

    ctx->instructions = NULL;
    ctx->lastInstruction = NULL;
    ctx->instructionCount = 0;
    ctx->nextTempNum = 1;
    ctx->nextLabelNum = 1;
    ctx->pendingJumps = NULL;
    return ctx;
}

void freeIrContext(IrContext *ctx) {
    if (!ctx) return;
    
    IrInstruction *inst = ctx->instructions;
    while (inst) {
        IrInstruction *next = inst->next;
        free(inst);
        inst = next;
    }
    
    free(ctx);
}

IrOperand createTemp(IrContext *ctx, IrDataType type){
    return (IrOperand){
        .type =   OPERAND_TEMP,
        .dataType = type,
        .value.temp.tempNum = ctx->nextTempNum++
    };
}

IrOperand createVar(const char *name, size_t len, IrDataType type){
    return (IrOperand){
        .type = OPERAND_VAR,
        .dataType = type,
        .value.var.name = name,
        .value.var.nameLen = len
    };
}

IrOperand createConst(IrDataType type) {
    IrOperand op = {0};
    op.type = OPERAND_CONSTANT;
    op.dataType = type;
    return op;
}

IrOperand createIntConst(int val){
    IrOperand op = createConst(IR_TYPE_INT);
    op.value.constant.intVal = val;
    return op;
}

IrOperand createFloatConst(float val){
    IrOperand op = createConst(IR_TYPE_FLOAT);
    op.value.constant.floatVal = val;
    return op;
}

IrOperand createDoubleConst(double val){
    IrOperand op = createConst(IR_TYPE_DOUBLE);
    op.value.constant.doubleVal = val;
    return op;
}

IrOperand createBoolConst(int val){
    IrOperand op = createConst(IR_TYPE_BOOL);
    op.value.constant.intVal = val ? 1 : 0;
    return op;
}

IrOperand createStringConst(const char* val, size_t len){
    IrOperand op = createConst(IR_TYPE_STRING);
    op.value.constant.str.stringVal = val;
    op.value.constant.str.len = len;
    return op;
}

IrOperand createLabel(int label){
    return (IrOperand){
        .type = OPERAND_LABEL,
        .dataType = IR_TYPE_VOID,
        .value.label.labelNum = label
    };
}

IrOperand createFn(const char *start, size_t len){
    return (IrOperand) {
        .type = OPERAND_FUNCTION,
        .value.fn.name = start,
        .value.fn.nameLen = len
    };
}

IrOperand createNone(){
    return (IrOperand){
        .type = OPERAND_NONE,
        .dataType = IR_TYPE_VOID
    };
}

void appendInstruction(IrContext *ctx, IrInstruction *inst){
    if(!ctx->instructions){
        ctx->instructions = inst;
        ctx->lastInstruction = inst;
    } else {
        ctx->lastInstruction->next = inst;
        inst->prev = ctx->lastInstruction;
        ctx->lastInstruction = inst;
    }
    ctx->instructionCount++;
}

IrInstruction *emitBinary(IrContext *ctx, IrOpCode op, IrOperand res, IrOperand ar1, IrOperand ar2){
    IrInstruction *inst = malloc(sizeof(IrInstruction));
    if(!inst) return NULL;

    inst->op = op;
    inst->result = res;
    inst->ar1 = ar1;
    inst->ar2 = ar2;
    inst->next = NULL;
    inst->prev = NULL;

    appendInstruction(ctx, inst);
    return inst;
}

IrInstruction *emitUnary(IrContext *ctx, IrOpCode op, IrOperand res, IrOperand ar1){
    IrOperand none = createNone();
    return emitBinary(ctx, op, res, ar1, none);
}

IrInstruction *emitCopy(IrContext *ctx, IrOperand res, IrOperand ar1){
    return emitUnary(ctx, IR_COPY, res, ar1);
}

IrInstruction *emitLabel(IrContext *ctx, int lab){
    IrOperand label = createLabel(lab);
    IrOperand none = createNone();
    return emitBinary(ctx, IR_LABEL, label, none, none);
}

IrInstruction *emitGoto(IrContext *ctx, int lab){
    IrOperand label = createLabel(lab);
    IrOperand none = createNone();
    return emitBinary(ctx, IR_GOTO, none, label, none);
}

IrInstruction *emitIfFalse(IrContext *ctx, IrOperand cond, int lab) {
    IrOperand label = createLabel(lab);
    IrOperand none = createNone();
    return emitBinary(ctx, IR_IF_FALSE, none, cond, label);
}

IrInstruction *emitReturn(IrContext *ctx, IrOperand ret) {
    IrOpCode op = (ret.type == OPERAND_NONE) ? IR_RETURN_VOID : IR_RETURN;
    IrOperand none = createNone();
    return emitBinary(ctx, op, none, ret, none);
}

IrInstruction *emitCall(IrContext *ctx, IrOperand res, const char *fnName, 
                        size_t nameLen, int params) {
    IrOperand func = (IrOperand) {
        .type = OPERAND_FUNCTION,
        .value.fn.name = fnName,
        .value.fn.nameLen = nameLen
    };
    
    IrOperand paramCount = createIntConst(params);
    
    return emitBinary(ctx, IR_CALL, res, func, paramCount);
}

IrDataType symbolTypeToIrType(DataType type) {
    switch (type) {
        case TYPE_INT: return IR_TYPE_INT;
        case TYPE_FLOAT: return IR_TYPE_FLOAT;
        case TYPE_DOUBLE: return IR_TYPE_DOUBLE;
        case TYPE_BOOL: return IR_TYPE_BOOL;
        case TYPE_STRING: return IR_TYPE_STRING;
        case TYPE_VOID: return IR_TYPE_VOID;
        default: return IR_TYPE_INT;
    }
}

IrDataType nodeTypeToIrType(NodeTypes nodeType) {
    switch (nodeType) {
        case INT_LIT:
        case REF_INT:
        case INT_VARIABLE_DEFINITION:
            return IR_TYPE_INT;

        case FLOAT_LIT:
        case REF_FLOAT:
        case FLOAT_VARIABLE_DEFINITION:
            return IR_TYPE_FLOAT;

        case DOUBLE_LIT:
        case REF_DOUBLE:
        case DOUBLE_VARIABLE_DEFINITION:
            return IR_TYPE_DOUBLE;

        case BOOL_LIT:
        case REF_BOOL:
        case BOOL_VARIABLE_DEFINITION:
            return IR_TYPE_BOOL;

        case STRING_LIT:
        case REF_STRING:
        case STRING_VARIABLE_DEFINITION:
            return IR_TYPE_STRING;

        case REF_VOID:
            return IR_TYPE_VOID;

        case REF_CUSTOM:
        case STRUCT_VARIABLE_DEFINITION:
            return IR_TYPE_POINTER;

        default:
            return IR_TYPE_INT;
    }
}

IrOpCode astOpToIrOp(NodeTypes nodeType) {
    switch (nodeType) {
        case ADD_OP: return IR_ADD;
        case SUB_OP: return IR_SUB;
        case MUL_OP: return IR_MUL;
        case DIV_OP: return IR_DIV;
        case MOD_OP: return IR_MOD;
        case BITWISE_AND: return IR_BIT_AND;
        case BITWISE_OR: return IR_BIT_OR;
        case BITWISE_XOR: return IR_BIT_XOR;
        case BITWISE_LSHIFT: return IR_SHL;
        case BITWISE_RSHIFT: return IR_SHR;
        case EQUAL_OP: return IR_EQ;
        case NOT_EQUAL_OP: return IR_NE;
        case LESS_THAN_OP: return IR_LT;
        case LESS_EQUAL_OP: return IR_LE;
        case GREATER_THAN_OP: return IR_GT;
        case GREATER_EQUAL_OP: return IR_GE;
        case LOGIC_AND: return IR_AND;
        case LOGIC_OR: return IR_OR;
        case LOGIC_NOT: return IR_NOT;
        case UNARY_MINUS_OP: return IR_NEG;
        default: return IR_NOP;
    }
}

IrOperand generateExpressionIr(IrContext *ctx, ASTNode node, TypeCheckContext typeCtx){
    if(!node) return createNone();
    switch (node->nodeType)
    {
    case INT_LIT: return createIntConst(parseInt(node->start, node->length));
    
    case FLOAT_LIT: return createFloatConst(parseFloat(node->start, node->length));
    
    case DOUBLE_LIT: return createDoubleConst(parseFloat(node->start, node->length));
    
    case BOOL_LIT: return createBoolConst(matchLit(node->start, node->length, "true") ? 1 : 0);

    case STRING_LIT: return createStringConst(node->start, node->length);

    case VARIABLE: {
        Symbol sym = lookupSymbol(typeCtx->current, node->start, node->length);
        if(!sym && typeCtx->currentFunction){
            FunctionParameter param = typeCtx->currentFunction->parameters;
            while (param) {
                if(bufferEqual(node->start, node->length, param->nameStart, param->nameLength)){
                    IrDataType type = symbolTypeToIrType(param->type);
                    return createVar(node->start, node->length, type);
                }
                param = param->next;
            }
        }
        if(!sym) return createNone();

        IrDataType type = symbolTypeToIrType(sym->type);
        return createVar(node->start, node->length, type);
    }

    case ADD_OP:
    case SUB_OP:
    case MUL_OP:
    case DIV_OP:
    case MOD_OP:
    case BITWISE_AND:
    case BITWISE_OR:
    case BITWISE_XOR:
    case BITWISE_LSHIFT:
    case BITWISE_RSHIFT:
    case EQUAL_OP:
    case NOT_EQUAL_OP:
    case LESS_THAN_OP:
    case LESS_EQUAL_OP:
    case GREATER_THAN_OP:
    case GREATER_EQUAL_OP:
    case LOGIC_AND:
    case LOGIC_OR: {
        ASTNode left = node->children;
        ASTNode right = left ? left->brothers : NULL;
        if (!left || !right) return createNone();

        IrOperand leftOp = generateExpressionIr(ctx, left, typeCtx);
        IrOperand rightOp = generateExpressionIr(ctx, right, typeCtx);

        IrDataType resultType = symbolTypeToIrType(getOperationResultType(
            getDataTypeFromNode(left->nodeType), getDataTypeFromNode(right->nodeType), node->nodeType));

        IrOperand res = createTemp(ctx, resultType);
        IrOpCode op = astOpToIrOp(node->nodeType);
        emitBinary(ctx, op, res, leftOp, rightOp);
        return res;
    }

    case UNARY_MINUS_OP:
    case LOGIC_NOT: {
        ASTNode operand = node->children;
        IrOperand operandOp = generateExpressionIr(ctx, operand, typeCtx);
        IrOperand res = createTemp(ctx, operandOp.dataType);

        IrOpCode irOp = astOpToIrOp(node->nodeType);
        emitUnary(ctx, irOp, res, operandOp);
        return res;
    }

    case PRE_INCREMENT:
    case PRE_DECREMENT: {
        printf("inside");
        ASTNode operand = node->children;
        IrOperand var = generateExpressionIr(ctx, operand, typeCtx);
        
        IrOperand one;
        if (var.dataType == IR_TYPE_FLOAT) {
            one = createFloatConst(1.0f);
        } else if (var.dataType == IR_TYPE_DOUBLE) {
            one = createDoubleConst(1.0);
        } else {
            one = createIntConst(1);
        }
        
        IrOperand temp = createTemp(ctx, var.dataType);
        
        IrOpCode op = (node->nodeType == PRE_INCREMENT) ? IR_ADD : IR_SUB;
        emitBinary(ctx, op, temp, var, one);
        
        emitCopy(ctx, var, temp);
        return var;
    }

    case POST_INCREMENT:
    case POST_DECREMENT: {
        ASTNode operand = node->children;
        IrOperand var = generateExpressionIr(ctx, operand, typeCtx);
        IrOperand oldValue = createTemp(ctx, var.dataType);
        emitCopy(ctx, oldValue, var);
        
        IrOperand one;
        if (var.dataType == IR_TYPE_FLOAT) {
            one = createFloatConst(1.0f);
        } else if (var.dataType == IR_TYPE_DOUBLE) {
            one = createDoubleConst(1.0);
        } else {
            one = createIntConst(1);
        }
        
        IrOperand newValue = createTemp(ctx, var.dataType);
        
        IrOpCode op = (node->nodeType == POST_INCREMENT) ? IR_ADD : IR_SUB;
        emitBinary(ctx, op, newValue, var, one);
        
        emitCopy(ctx, var, newValue);
        
        return oldValue;
    }

    case FUNCTION_CALL: {
        int paramCount = 0;
        ASTNode argList = node->children;
        if (argList && argList->nodeType == ARGUMENT_LIST) {
            ASTNode arg = argList->children;
            while (arg) {
                IrOperand argOp = generateExpressionIr(ctx, arg, typeCtx);
                IrOperand none = createNone();
                emitBinary(ctx, IR_PARAM, none, argOp, none);
                paramCount++;
                arg = arg->brothers;
            }
        }
        
        Symbol funcSymbol = lookupSymbol(typeCtx->current, node->start, node->length);
        IrDataType retType = funcSymbol ? symbolTypeToIrType(funcSymbol->type) : IR_TYPE_VOID;
        
        IrOperand result = (retType == IR_TYPE_VOID) ? createNone() : createTemp(ctx, retType);
        emitCall(ctx, result, node->start, node->length, paramCount);
        
        return result;
    }

    case ASSIGNMENT:
    case COMPOUND_ADD_ASSIGN:
    case COMPOUND_SUB_ASSIGN:
    case COMPOUND_MUL_ASSIGN:
    case COMPOUND_DIV_ASSIGN:
    case COMPOUND_AND_ASSIGN:
    case COMPOUND_OR_ASSIGN:
    case COMPOUND_XOR_ASSIGN:
    case COMPOUND_LSHIFT_ASSIGN:
    case COMPOUND_RSHIFT_ASSIGN: {
        ASTNode left = node->children;
        ASTNode right = left ? left->brothers : NULL;
        if (!left || !right) return createNone();

        IrOperand rightOp = generateExpressionIr(ctx, right, typeCtx);
        IrOperand leftOp = generateExpressionIr(ctx, left, typeCtx);

        if (node->nodeType != ASSIGNMENT) {
            IrDataType resultType = leftOp.dataType;
            IrOperand temp = createTemp(ctx, resultType);

            IrOpCode op;
            switch (node->nodeType) {
            case COMPOUND_ADD_ASSIGN:
                op = IR_ADD;
                break;
            case COMPOUND_SUB_ASSIGN:
                op = IR_SUB;
                break;
            case COMPOUND_MUL_ASSIGN:
                op = IR_MUL;
                break;
            case COMPOUND_DIV_ASSIGN:
                op = IR_DIV;
                break;
            case COMPOUND_AND_ASSIGN:
                op = IR_BIT_AND;
                break;
            case COMPOUND_OR_ASSIGN:
                op = IR_BIT_OR;
                break;
            case COMPOUND_XOR_ASSIGN:
                op = IR_BIT_XOR;
                break;
            case COMPOUND_LSHIFT_ASSIGN:
                op = IR_SHL;
                break;
            case COMPOUND_RSHIFT_ASSIGN:
                op = IR_SHR;
                break;
            default:
                op = IR_NOP;
                break;
            }

            emitBinary(ctx, op, temp, leftOp, rightOp);

            emitCopy(ctx, leftOp, temp);

            return leftOp;
        }

        emitCopy(ctx, leftOp, rightOp);
        return leftOp;
    }

    
    case CAST_EXPRESSION: {
        ASTNode sourceExpr = node->children;
        ASTNode targetType = sourceExpr->brothers;

        IrOperand source = generateExpressionIr(ctx, sourceExpr, typeCtx);
        IrDataType target = nodeTypeToIrType(targetType->nodeType);

        IrOperand res = createTemp(ctx, target);
        emitUnary(ctx, IR_CAST, res, source);
        return res;
    }

    default: return createNone();
    }
}


void generateStatementIr(IrContext *ctx, ASTNode node, TypeCheckContext typeCtx){
    switch(node->nodeType){
        case PROGRAM:
        case BLOCK_STATEMENT:
        case BLOCK_EXPRESSION: {
            ASTNode child = node->children;
            while(child){
                generateStatementIr(ctx, child, typeCtx);
                child = child->brothers;
            }
            break;
        }
        case LET_DEC:
        case CONST_DEC:
            if (node->children) {
                generateStatementIr(ctx, node->children, typeCtx);
            }
            break;
        case INT_VARIABLE_DEFINITION:
        case FLOAT_VARIABLE_DEFINITION:
        case DOUBLE_VARIABLE_DEFINITION:
        case BOOL_VARIABLE_DEFINITION:
        case STRING_VARIABLE_DEFINITION: {
            if(node->children){
                IrOperand val = generateExpressionIr(ctx, node->children, typeCtx);
                IrDataType type  = nodeTypeToIrType(node->nodeType);

                IrOperand var = createVar(node->start, node->length, type);
                emitCopy(ctx, var, val);
            }
            break;
        }

        case IF_CONDITIONAL: {
            ASTNode cond = node->children;
            ASTNode trueBranchWrap = cond->brothers;
            ASTNode elseBranchWrap = trueBranchWrap->brothers;

            int elseLab = ctx->nextLabelNum++;
            int endLab = ctx->nextLabelNum++;

            IrOperand condOp = generateExpressionIr(ctx, cond, typeCtx);

            if(elseBranchWrap){
                emitIfFalse(ctx, condOp, elseLab);
            }else {
                emitIfFalse(ctx, condOp, endLab);
            }

            generateStatementIr(ctx, trueBranchWrap->children, typeCtx);
            if(elseBranchWrap){
                emitGoto(ctx, endLab);
                emitLabel(ctx, elseLab);
                generateStatementIr(ctx, elseBranchWrap->children, typeCtx);
            }
            emitLabel(ctx, endLab);
            break;
        }

        case LOOP_STATEMENT: {
            ASTNode cond = node->children;
            ASTNode body = cond->brothers;

            int startLab = ctx->nextLabelNum++;
            int endLab = ctx->nextLabelNum++;

            emitLabel(ctx, startLab);

            IrOperand condOp = generateExpressionIr(ctx, cond, typeCtx);
            emitIfFalse(ctx, condOp, endLab);
            generateStatementIr(ctx, body, typeCtx);
            emitGoto(ctx, startLab);
            emitLabel(ctx, endLab);

            break;
        }

         case RETURN_STATEMENT: {
            if (node->children) {
                IrOperand retVal = generateExpressionIr(ctx, node->children, typeCtx);
                emitReturn(ctx, retVal);
            } else {
                IrOperand none = createNone();
                emitReturn(ctx, none);
            }
            break;
        }

        case FUNCTION_DEFINITION: {
            ASTNode paramList = node->children;
            ASTNode returnType = paramList->brothers;
            ASTNode body = returnType->brothers;
            
            Symbol fnSymbol = lookupSymbol(typeCtx->current, node->start, node->length);
            Symbol oldFunction = typeCtx->currentFunction;
            typeCtx->currentFunction = fnSymbol;
            
            IrOperand funcName = createFn(node->start, node->length);
            IrOperand none = createNone();
            emitBinary(ctx, IR_FUNC_BEGIN, funcName, none, none);

            if (fnSymbol && fnSymbol->parameters) {
                FunctionParameter param = fnSymbol->parameters;
                int paramIndex = 0;
                while (param) {
                    IrDataType irType = symbolTypeToIrType(param->type);
                    IrOperand paramVar = createVar(param->nameStart, param->nameLength, irType);
                    IrOperand indexOp = createIntConst(paramIndex);

                    emitBinary(ctx, IR_COPY, paramVar, paramVar, indexOp);

                    param = param->next;
                    paramIndex++;
                }
            }

            generateStatementIr(ctx, body, typeCtx);
            
            emitBinary(ctx, IR_FUNC_END, funcName, none, none);
            typeCtx->currentFunction = oldFunction;
            break;
        }

        default: generateExpressionIr(ctx, node, typeCtx);
    }
}

IrContext *generateIr(ASTNode ast, TypeCheckContext typeCtx){
    IrContext *ctx = createIrContext();
    if(!ctx)  return NULL;

    generateStatementIr(ctx, ast, typeCtx);

    return ctx;
}

// printing stuff

static const char *opCodeToString(IrOpCode op) {
    switch (op) {
        case IR_ADD: return "ADD";
        case IR_SUB: return "SUB";
        case IR_MUL: return "MUL";
        case IR_DIV: return "DIV";
        case IR_MOD: return "MOD";
        case IR_NEG: return "NEG";
        case IR_BIT_AND: return "BIT_AND";
        case IR_BIT_OR: return "BIT_OR";
        case IR_BIT_XOR: return "BIT_XOR";
        case IR_BIT_NOT: return "BIT_NOT";
        case IR_SHL: return "SHL";
        case IR_SHR: return "SHR";
        case IR_AND: return "AND";
        case IR_OR: return "OR";
        case IR_NOT: return "NOT";
        case IR_EQ: return "EQ";
        case IR_NE: return "NE";
        case IR_LT: return "LT";
        case IR_LE: return "LE";
        case IR_GT: return "GT";
        case IR_GE: return "GE";
        case IR_COPY: return "COPY";
        case IR_LOAD: return "LOAD";
        case IR_STORE: return "STORE";
        case IR_ADDR: return "ADDR";
        case IR_LABEL: return "LABEL";
        case IR_GOTO: return "GOTO";
        case IR_IF_TRUE: return "IF_TRUE";
        case IR_IF_FALSE: return "IF_FALSE";
        case IR_PARAM: return "PARAM";
        case IR_CALL: return "CALL";
        case IR_RETURN: return "RETURN";
        case IR_RETURN_VOID: return "RETURN_VOID";
        case IR_NOP: return "NOP";
        case IR_FUNC_BEGIN: return "FUNC_BEGIN";
        case IR_FUNC_END: return "FUNC_END";
        case IR_CAST: return "CAST";
        default: return "UNKNOWN";
    }
}

static void printOperand(IrOperand op) {
    switch (op.type) {
        case OPERAND_TEMP:
            printf("t%d", op.value.temp.tempNum);
            break;
        case OPERAND_VAR:
            printf("%.*s", (int)op.value.var.nameLen, op.value.var.name);
            break;
        case OPERAND_CONSTANT:
            if (op.dataType == IR_TYPE_INT || op.dataType == IR_TYPE_BOOL) {
                printf("%d", op.value.constant.intVal);
            } else if (op.dataType == IR_TYPE_STRING) {
                printf("%.*s", (int)op.value.constant.str.len, op.value.constant.str.stringVal);
            } else if(op.dataType == IR_TYPE_FLOAT){
                printf("%g", op.value.constant.floatVal);
            }else {
                printf("%f", op.value.constant.doubleVal);
            }
            break;
        case OPERAND_LABEL:
            printf("L%d", op.value.label.labelNum);
            break;
        case OPERAND_FUNCTION:
            printf("%.*s", (int)op.value.fn.nameLen, op.value.fn.name);
            break;
        case OPERAND_NONE:
            printf("-");
            break;
    }
}

void printInstruction(IrInstruction *inst) {
    if (!inst) return;

    printf("%-12s ", opCodeToString(inst->op));
    switch (inst->op) {
        case IR_LABEL:
            printf("L%d:", inst->result.value.label.labelNum);
            break;

        case IR_GOTO:
            printf("goto L%d", inst->ar1.value.label.labelNum);
            break;

        case IR_IF_TRUE:
            printf("if ");
            printOperand(inst->ar1);
            printf(" goto L%d", inst->ar2.value.label.labelNum);
            break;

        case IR_IF_FALSE:
            printf("!");
            printOperand(inst->ar1);
            printf(" goto L%d", inst->ar2.value.label.labelNum);
            break;

        case IR_COPY:
            printOperand(inst->result);
            printf(" = ");
            printOperand(inst->ar1);
            break;

        case IR_RETURN:
            printf("return ");
            printOperand(inst->ar1);
            break;

        case IR_RETURN_VOID:
            printf("return");
            break;

        case IR_FUNC_BEGIN:
            printOperand(inst->result);
            printf(":");
            break;

        case IR_FUNC_END:
            printf("end ");
            printOperand(inst->result);
            break;

        case IR_PARAM:
            printf("param ");
            printOperand(inst->ar1);
            break;

        case IR_CALL:
            if (inst->result.type != OPERAND_NONE) {
                printOperand(inst->result);
                printf(" = ");
            }
            printf("call ");
            printOperand(inst->ar1); 
            printf(", ");
            printOperand(inst->ar2);
            break;

        default:
            if (inst->result.type != OPERAND_NONE) {
                printOperand(inst->result);
                printf(" = ");
            }
            
            if (inst->ar1.type != OPERAND_NONE) {
                printOperand(inst->ar1);
            }
            
            if (inst->ar2.type != OPERAND_NONE) {
                printf(", ");
                printOperand(inst->ar2);
            }
            break;
    }
}

void printIR(IrContext *ctx) {
    if (!ctx) return;
    printf("Total instructions: %d\n", ctx->instructionCount);
    printf("Temporaries used: t1 - t%d\n", ctx->nextTempNum - 1);
    printf("Labels used: L1 - L%d\n\n", ctx->nextLabelNum - 1);

    IrInstruction *inst = ctx->instructions;
    int count = 0;

    while (inst) {
        printf("%4d: ", count++);
        printInstruction(inst);
        printf("\n");
        inst = inst->next;
    }
}
