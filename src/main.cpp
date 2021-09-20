
#include <iostream>

#include "Processor.h"
#include "Asm.h"
#include "Sym.h"
#include "AutomatedTests.h"
#include "Simulator.h"

int main(int argc, char* argv[]) {
	
	try{
		if(argc == 2 && std::string(argv[1]) == "--help"){
			std::cout << 
			"Usage:\n"
			"	./CpuCompilerV2 --help\n"
			"	./CpuCompilerV2 compile [INFILE.sym]...\n"
			"	./CpuCompilerV2 libLink [OUTFILE.asm] [INFILE.asm]...\n"
			"	./CpuCompilerV2 exeLink [OUTFILE.asm.out] [INFILE.asm]...\n"
			"	./CpuCompilerV2 assemble [OUTFILE.rom] [INFILE.asm]\n"
			"	./CpuCompilerV2 debug [INFILE.asm.out]\n"
			"	./CpuCompilerV2 automatedTest\n";
		}
		else if(argc >= 3 && std::string(argv[1]) == "compile"){
			for(int i = 2; i < argc; i++){
				std::string file = argv[i];
				SymAST::Program symProg = Processor::generateSymASTFromFile(file);
				AsmAST::Program asmProg = Sym::createAsmASTFromSymAST(symProg);
				Processor::saveAsmASTToFile(asmProg, file + ".asm");
				std::cout << "Successfully compiled \"" << file << "\" to \"" << file << ".asm\"\n";
			}
		}
		else if(argc >= 4 && std::string(argv[1]) == "libLink"){
			std::vector<std::string> files;
			for(int i = 3; i < argc; i++){
				files.push_back(argv[i]);
			}
			Processor::linkToLibrary(files, argv[2]);
		}
		else if(argc >= 4 && std::string(argv[1]) == "exeLink"){
			std::vector<std::string> files;
			for(int i = 3; i < argc; i++){
				files.push_back(argv[i]);
			}
			Processor::linkToExecutable(files, argv[2]);
		}
		else if(argc == 4 && std::string(argv[1]) == "assemble"){
			AsmAST::Program asmProg = Processor::generateAsmASTFromFile(argv[3]);
			std::vector<Byte> byteCode = Asm::createByteCodeFromAsmAST(asmProg);
			Processor::saveByteCodeToFile(byteCode, argv[2]);
			std::cout << "Successfully assembled \"" << argv[3] << "\" to \"" << argv[2] << "\"\n";
		}
		else if(argc == 3 && std::string(argv[1]) == "debug"){
			AsmAST::Program asmProg = Processor::generateAsmASTFromFile(argv[2]);
			Simulator simulator;
			simulator.loadAsmAST(asmProg);
			simulator.enableDebugger();
			simulator.run();
			simulator.printCpuData();
		}
		else if(argc == 2 && std::string(argv[1]) == "automatedTest"){
			std::cout << "Running automated tests...\n"; 
			AutomatedTests::runAll();
		}
		else{
			throw std::runtime_error("No/Invalid action specified");
		}	
	}
	catch(std::exception& e){
		std::cout << "Error: " << e.what() << "\n";
		return 1;
	}
	
	return 0;
}
