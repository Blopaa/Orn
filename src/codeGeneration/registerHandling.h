#ifndef REGISTERHANDLING_H
#define REGISTERHANDLIGN_H

#include "codeGeneration.h"

const char *getRegisterName(RegisterId regId, DataType type);
const char *getFloatRegisterName(RegisterId regId);
const char *getRegisterNameForSize(RegisterId regId, DataType type);
RegisterId getParameterReg(DataType type, int index);

#endif