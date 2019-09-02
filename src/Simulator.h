#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "Definitions.h"
#include "Helpers.h"
#include <vector>
#include <array>

#include "AsmAST.h"
#include "Asm.h"

class Simulator{
public:
	void loadAsmAST(const AsmAST::Program& program);
    void loadRom(std::vector<Byte> _rom);
    void enableDebugger();
    void run();
    void clearData();
    void setTests(std::vector<Byte> testOutput);
    
    void printCpuData();
    
private:
	bool hasDebugData = false;
	bool debugging = false;
	AsmAST::Program asmProgram;
	
    std::vector<Byte> rom;
    Byte insPointer;
    std::vector<Byte> ram;
    std::array<Byte, 8> inputRegisters;
    
    bool testsEnabled = false;
    std::vector<Byte> tests;
    
    void setRegister(Byte addr, Byte data);
    Byte getRegister(Byte addr);
    void setRam(Byte addr, Byte data);
    Byte getRam(Byte addr);
    
    AsmAST::Context getContextForIns(Byte ins);
    AsmAST::Context getFunctionContextForIns(Byte ins);
};

#endif // SIMULATOR_H
