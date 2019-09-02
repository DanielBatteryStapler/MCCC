#ifndef SYM_H
#define SYM_H

#include "SymAST.h"
#include "AsmAST.h"

namespace Sym{
    AsmAST::Program createAsmASTFromSymAST(const SymAST::Program& program);
    AsmAST::Program generateExecutableHeader();
};

#endif // SYM_H
