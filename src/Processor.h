#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "SymAST.h"
#include "AsmAST.h"

#include <set>

namespace Processor{
	SymAST::Program generateSymASTFromFile(std::string fileName, std::set<std::string>* imported = nullptr);
	AsmAST::Program generateAsmASTFromFile(std::string fileName);
	void removeComments(std::string& fileContents);
	
	void saveAsmASTToFile(const AsmAST::Program& program, std::string fileName);
	void saveByteCodeToFile(const std::vector<Byte>& program, std::string fileName);
	
	void linkToLibrary(std::vector<std::string> inputFiles, std::string outputFile);
	void linkToExecutable(std::vector<std::string> inputFiles, std::string outputFile);
	
	std::vector<Byte> generateByteCodeFromAsmString(std::string asmString);
	std::vector<Byte> generateExecutableByteCodeFromSymString(std::string symString);
};

#endif // PROCESSOR_H
