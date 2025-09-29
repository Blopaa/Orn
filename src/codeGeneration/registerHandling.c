#include "registerHandling.h"
#include "asmTemplate.h"

// x86-64 System V ABI calling convention register assignments
static const RegisterId IntRegs[] = {
    REG_RDI, REG_RSI, REG_RDX, REG_RCX, REG_R8, REG_R9
};

static const RegisterId FloatRegs[] = {
    REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM4, REG_XMM5
};

/**
 * @brief Returns assembly register name for the given register ID and type.
 * Delegates to getFloatRegisterName() for float/double types.
 */
const char *getRegisterName(RegisterId regId, DataType type) {
    if (type == TYPE_FLOAT || type == TYPE_DOUBLE) {
        return getFloatRegisterName(regId);
    }

    static const char *registers[] = {
        ASM_REG_RAX, ASM_REG_RBX, ASM_REG_RCX, ASM_REG_RDX, ASM_REG_RSI,
        ASM_REG_RDI, ASM_REG_R8, ASM_REG_R9, ASM_REG_R10, ASM_REG_R11
    };

    if (regId >= REG_R11 + 1) return ASM_REG_RAX;
    return registers[regId];
}

/**
 * @brief Returns XMM register name for floating-point operations.
 */
const char *getFloatRegisterName(RegisterId regId) {
    static const char *xmmRegisters[] = {
        ASM_REG_XMM0, ASM_REG_XMM1, ASM_REG_XMM2, 
        ASM_REG_XMM3, ASM_REG_XMM4, ASM_REG_XMM5
    };

    int xmmIndex = regId - REG_XMM0;
    if (xmmIndex < 0 || xmmIndex > 5) return ASM_REG_XMM0;
    return xmmRegisters[xmmIndex];
}

/**
 * @brief Returns size-appropriate register name based on data type.
 * Returns 8-bit for bool, 32-bit for int/float, 64-bit for pointers/strings.
 */
const char *getRegisterNameForSize(RegisterId regId, DataType type) {
    static const char *registers64[] = {
        "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", 
        "%r8", "%r9", "%r10", "%r11"
    };
    static const char *registers32[] = {
        "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi", 
        "%r8d", "%r9d", "%r10d", "%r11d"
    };
    static const char *registers8[] = {
        "%al", "%bl", "%cl", "%dl", "%sil", "%dil", 
        "%r8b", "%r9b", "%r10b", "%r11b"
    };

    if (regId >= REG_R11 + 1) return registers64[0];

    switch (type) {
        case TYPE_INT:
        case TYPE_FLOAT: 
            return registers32[regId];
        case TYPE_BOOL: 
            return registers8[regId];
        case TYPE_STRING:
        default: 
            return registers64[regId];
    }
}

/**
 * @brief Returns register for function parameter following x86-64 System V ABI.
 * Floats/doubles use XMM0-XMM5, everything else uses RDI, RSI, RDX, RCX, R8, R9.
 * Index is tracked separately for int and float parameters.
 */
RegisterId getParameterReg(DataType type, int index) {
    if (type == TYPE_FLOAT || type == TYPE_DOUBLE) {
        if (index >= 0 && index < 6) {
            return FloatRegs[index];
        }
    } else {
        if (index >= 0 && index < 6) {
            return IntRegs[index];
        }
    }
    return REG_RAX;
}