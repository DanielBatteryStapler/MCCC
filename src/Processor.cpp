#include "Processor.h"

#include "Helpers.h"
#include "Asm.h"
#include "Sym.h"

#include <sstream>

namespace Processor{
	SymAST::Program generateSymASTFromFile(std::string fileName, std::set<std::string>* imported){
		
		bool createdImported = false;
		
		if(imported == nullptr){
			imported = new std::set<std::string>();
			createdImported = true;
		}
		
		std::string fileContents = loadFile(fileName);
		
		//remove comments from the code before passing it to the SymAST
		removeComments(fileContents);
		
		SymAST::Program ast = SymAST::createAST(fileContents, fileName);
		imported->insert(fileName);//add the current file to the list of imported files
		for(auto i = ast.begin(); i != ast.end(); i++){//deal with the imports
			if(i->which() == 0){//SymAST::Import
				SymAST::Import import = boost::get<SymAST::Import>(*i);
				if(imported->find(import.fileName) != imported->end()){//if this file has already been imported
					i = ast.erase(i);
					i--;
				}
				else{
					SymAST::Program importedAST = generateSymASTFromFile(import.fileName, imported);
					i = ast.erase(i);
					i = ast.insert(i, importedAST.begin(), importedAST.end());
					i = std::next(i, importedAST.size() - 1);
				}
			}
		}
		
		if(createdImported){
			delete imported;
			imported = nullptr;
		}
		return ast;
	}
	
	AsmAST::Program generateAsmASTFromFile(std::string fileName){
		std::string fileContents = loadFile(fileName);
		
		removeComments(fileContents);
		
		AsmAST::Program ast = AsmAST::createAST(fileContents, fileName);
		return ast;
	}
	
	void removeComments(std::string& fileContents){
		for(std::string::iterator i = fileContents.begin(); i != fileContents.end(); i++){
			if(*i == '/'){
				std::string::iterator start = i;
				i++;
				if(i == fileContents.end()){
					break;
				}
				if(*i == '/'){
					while(true){
						if(*i == '\n' || i == fileContents.end()){
							i = fileContents.erase(start, i);
							break;
						}
						i++;
					}
				}
			}
		}
	}
	
	void saveAsmASTToFile(const AsmAST::Program& program, std::string fileName){
		std::ofstream outFile(fileName);
		
		Asm::convertAsmASTToString(program, outFile);
		
		outFile.close();
	}
	
	void saveByteCodeToFile(const std::vector<Byte>& program, std::string fileName){
		std::ofstream outFile(fileName);
		
		outFile << "v2.0 raw\n";
		
		int columns = 0;
		
		for(auto i = program.begin(); i != program.end(); i++){
			outFile << std::hex << (int)*i;
			if(columns == 15){
				outFile << '\n';
				columns = 0;
			}
			else{
				outFile << ' ';
				columns++;
			}
		}
		
		outFile.close();
	}
	
	void linkToLibrary(std::vector<std::string> inputFiles, std::string outputFile){
		std::string fullAsmAST;
		for(auto i = inputFiles.begin(); i != inputFiles.end(); i++){
			fullAsmAST += loadFile(*i) + "\n";
			std::cout << "Linked file \"" << *i << "\n";
		}
		{
			std::string copyAST = fullAsmAST;
			removeComments(copyAST);
			AsmAST::Program ast = AsmAST::createAST(copyAST, outputFile);
			Asm::checkAsmAST(ast);
			std::cout << "File \"" << outputFile << "\" passed all checks\n";
		}
		std::ofstream output(outputFile);
		output << fullAsmAST;
		output.close();
	}
	
	void linkToExecutable(std::vector<std::string> inputFiles, std::string outputFile){
		std::string fullAsmAST;
		for(auto i = inputFiles.begin(); i != inputFiles.end(); i++){
			fullAsmAST += loadFile(*i) + "\n";
			std::cout << "Linked file \"" << *i << "\n";
		}
		{
			AsmAST::Program headerAsm = Sym::generateExecutableHeader();
			std::stringstream headerAsmString;
			Asm::convertAsmASTToString(headerAsm, headerAsmString);
			fullAsmAST = headerAsmString.str() + fullAsmAST;
			std::cout << "Added executable header\n";
		}
		{
			std::string copyAST = fullAsmAST;
			removeComments(copyAST);
			AsmAST::Program ast = AsmAST::createAST(copyAST, outputFile);
			Asm::checkAsmAST(ast);
			std::cout << "File \"" << outputFile << "\" passed all checks\n";
		}
		std::ofstream output(outputFile);
		output << fullAsmAST;
		output.close();
	}
	
	std::vector<Byte> generateByteCodeFromAsmString(std::string asmString){
		removeComments(asmString);
		AsmAST::Program asmProg = AsmAST::createAST(asmString, "NotFromAFile");
		Asm::checkAsmAST(asmProg);
		std::vector<Byte> output = Asm::createByteCodeFromAsmAST(asmProg);
		return output;
	}
	
	std::vector<Byte> generateExecutableByteCodeFromSymString(std::string symString){
		removeComments(symString);
		SymAST::Program symProg = SymAST::createAST(symString, "NotFromAFile");
		AsmAST::Program asmProg = Sym::createAsmASTFromSymAST(symProg);
		Asm::checkAsmAST(asmProg);
		AsmAST::Program exeHeader = Sym::generateExecutableHeader();
		asmProg.insert(asmProg.begin(), exeHeader.begin(), exeHeader.end());
		std::vector<Byte> output = Asm::createByteCodeFromAsmAST(asmProg);
		return output;
	}
};


















