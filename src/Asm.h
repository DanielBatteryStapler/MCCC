#ifndef ASM_H
#define ASM_H

#include "Definitions.h"
#include "AsmAST.h"
#include "Helpers.h"
#include <vector>

namespace Asm{
    std::vector<Byte> createByteCodeFromAsmAST(const AsmAST::Program& program);
    void printByteCode(std::vector<Byte>& byteCode);
    void convertAsmASTToString(const AsmAST::Program& program, std::ostream& output);
    
    void checkAsmAST(const AsmAST::Program& program);
};

#endif // ASM_H
