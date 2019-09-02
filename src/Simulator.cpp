#include "Simulator.h"

#include <iostream>

void Simulator::loadAsmAST(const AsmAST::Program& program){
	rom = Asm::createByteCodeFromAsmAST(program);
	asmProgram = program;
	hasDebugData = true;
}

void Simulator::loadRom(std::vector<Byte> _rom){
    rom = _rom;
    hasDebugData = false;
}


void Simulator::enableDebugger(){
	if(hasDebugData){
		debugging = true;
	}
	else{
		throw std::runtime_error("Cannot enable Simulator debugging without debug data available");
	}
}

void Simulator::run(){
	clearData();

    std::array<Byte, 4> ins;
	bool halt = false;
	bool shouldIncrement;
	
	insPointer = 0;
	
	while(true){
        if(insPointer * 4 + 3 >= rom.size()){
            throw std::runtime_error("Simulator error, instruction pointer went past end of ROM");
        }
		ins[0] = rom.at(insPointer * 4);
		ins[1] = rom.at(insPointer * 4 + 1);
		ins[2] = rom.at(insPointer * 4 + 2);
		ins[3] = rom.at(insPointer * 4 + 3);
		shouldIncrement = true;
		
		switch(ins[0]){
		case 0:
		    halt = true;
			break;
		case 1:
			setRegister(ins[2], getRam(ins[1]));
			break;
		case 2:
			setRam(ins[1], getRegister(ins[2]));
			break;
		case 3:
			setRegister(ins[2], ins[1]);
			break;
		case 4:
			setRam(ins[1], ins[2]);
			break;
		case 5:
			setRegister(ins[2], getRegister(ins[1]));
			break;
		case 6:
			insPointer = ins[1];
			shouldIncrement = false;
			break;
		case 7:
			if(inputRegisters[0] > 0){
				insPointer = ins[1];
				shouldIncrement = false;
			}
			break;
		case 8:
			insPointer = inputRegisters[0];
			shouldIncrement = false;
			break;
		case 9:
			setRegister(ins[1], getRam(inputRegisters[0] + inputRegisters[1]));
			break;
		case 10:
			setRam(inputRegisters[0] + inputRegisters[1], getRegister(ins[1]));
			break;
		case 11:
			setRam(inputRegisters[0] + inputRegisters[1], inputRegisters[2]);
			break;
		default:
			throw std::runtime_error("Invalid Cpu Instruction: " + toHex(ins[0]));
			break;
		}
		if(halt){
            break;
		}
		
		if(debugging){
			printCpuData();
			std::cout << "Press enter to continue...";
			std::string temp;
			std::getline(std::cin, temp);//just so the cpu pauses for user input
		}
		
		if(shouldIncrement){
            insPointer++;
		}
	}
	
	if(testsEnabled){
        if(tests.size() != 0){
            throw std::runtime_error("Simulator tests failed, never pushed all of the required values into the output register");
        }
	}
}


void Simulator::clearData(){
	ram.clear();
	for(int i = 0; i < inputRegisters.size(); i++){
		inputRegisters[i] = 0;
	}
}

void Simulator::setTests(std::vector<Byte> testOutput){
    tests = testOutput;
    testsEnabled = true;
}

void Simulator::printCpuData(){
    std::cout << "Ram Dump:\n";
	int num = 0;
	for(auto i = ram.begin(); i != ram.end(); i++){
		std::cout << toHex(*i) << ' ';
		num++;
		if(num == 8){
			num = 0;
			std::cout << "\n";
		}
	}
	std::cout << "\n\n";
	
	std::cout << "Instruction Pointer: " << toHex(insPointer)
	<< " [" << toHex(rom.at(insPointer * 4 + 0)) << " " << toHex(rom.at(insPointer * 4 + 1)) << " " << toHex(rom.at(insPointer * 4 + 2)) << " " << toHex(rom.at(insPointer * 4 + 3)) << "]\n\n";
	
	std::cout << "Input Registers:\n"
	<< "    Operation Input #0    : " << toHex(inputRegisters[0]) << "\n"
	<< "    Operation Input #1____: " << toHex(inputRegisters[1]) << "\n"
	<< "    Operation Input #2    : " << toHex(inputRegisters[2]) << "\n"
	<< "    Stack Frame Ptr_______: " << toHex(inputRegisters[3]) << "\n"
	<< "    ALU Input #0          : " << toHex(inputRegisters[4]) << "\n"
	<< "    ALU Input #1__________: " << toHex(inputRegisters[5]) << "\n"
	<< "    ALU Mode Select       : " << toHex(inputRegisters[6]);
	switch(inputRegisters[6]){
	case 0:
		std::cout << " - Add\n";
		break;
	case 1:
		std::cout << " - Greater Than\n";
		break;
	case 2:
		std::cout << " - Equal To\n";
		break;
	case 3:
		std::cout << " - Less Than\n";
		break;
	default:
		std::cout << " - Invalid Mode\n";
		break;
	}
	std::cout 
	<< "    Output Monitor________: " << toHex(inputRegisters[7]) << " - " << std::to_string(inputRegisters[7]) << "\n";
	
	std::cout << "Output Registers:\n"
	<< "    Stack Frame Ptr       : " << toHex(getRegister(0)) << "\n"
	<< "    Abs. Addr of Sel. Addr: " << toHex(getRegister(1)) << "\n"
	<< "    ALU Output            : " << toHex(getRegister(2)) << "\n";
	
	if(hasDebugData){//if we have debug data we should use the data in ram + the debug data to reconstruct the stack trace
		std::cout << "\nStack Trace:\n";
		//so the way we're going to do this is use output register zero to get the current stack pointer,
		//use the debug data to print that stack information, and then use that stack to go to the previous stack and so on
		//until we get through the whole, well, stack
		
		//first, we should check to see if we're in a function or not
		AsmAST::Context context = getContextForIns(insPointer);
		if(context.stackSize){//if the stack size exists then we aren in a function
			Byte stackPointer = getRegister(0);
			Byte stackIns = insPointer;
			while(true){
				
				AsmAST::Context functionContext = getFunctionContextForIns(stackIns);
				std::cout << "[\"" << functionContext.context << "\"]\n";
				
				AsmAST::Context stackContext = getContextForIns(stackIns);
				std::cout << "    [\"" << stackContext.context << "\"";
				if(stackContext.file){
					std::cout << " \"" << stackContext.file->name << "\":" << stackContext.file->line;
				}
				std::cout << "]\n";
				for(auto i = stackContext.variables.rbegin(); i != stackContext.variables.rend(); i++){
					std::cout << "    " << toHex(stackPointer + i->address) << ": " << toHex(getRam(stackPointer + i->address)) << " = " << i->variable << " = "
					 << getRam(stackPointer + i->address) << "\n";
				}
				if(*stackContext.stackSize - stackContext.variables.size() == 3){
					std::cout << "    " << toHex(stackPointer + 2) << ": " << toHex(getRam(stackPointer + 2)) << " = Return Value Pointer\n";
				}
				
				std::cout << "    " << toHex(stackPointer + 1) << ": " << toHex(getRam(stackPointer + 1)) << " = Last Instruction\n";
				std::cout << "    " << toHex(stackPointer + 0) << ": " << toHex(getRam(stackPointer + 0)) << " = Last Stack Pointer\n";
				
				//now that we've printed that stack frame, let's go to the one before it
				stackIns = getRam(stackPointer + 1);
				stackPointer = getRam(stackPointer + 0);
				
				//if we've gone back too far, then we should break out of the loop
				if(!getContextForIns(stackIns).stackSize){
					break;
				}
			}
		}
		else{
			std::cout << "[\"" << context.context << "\"]\n";
			//if we don't have a stack to trace, just print what context we do have
		}
		
	}
}

void Simulator::setRegister(Byte addr, Byte data){
	if(addr >= inputRegisters.size()){
		throw std::runtime_error("Attempted to set invalid register: " + toHex(addr));
	}
	if(testsEnabled && addr == 7){
        if(tests.size() == 0){
            throw std::runtime_error("Simulator test failed, pushed to output monitor too many times");
        }
        if(tests[0] != data){
            throw std::runtime_error("Simulator test failed, output monitor expected \"" + std::to_string(tests[0]) + "\" but got \"" + std::to_string(data) + "\"");
        }
        tests.erase(tests.begin());//if the test is all good, then just remove it and wait for the next one
	}
	inputRegisters[addr] = data;
}

Byte Simulator::getRegister(Byte addr){
	switch(addr){
	case 0:
		return inputRegisters[3];
		break;
	case 1:
		return inputRegisters[0] + inputRegisters[1];
		break;
	case 2:
		switch(inputRegisters[6]){
		case 0:
			return inputRegisters[4] + inputRegisters[5];
			break;
		case 1:
			return Byte(inputRegisters[4] > inputRegisters[5]);
			break;
		case 2:
			return Byte(inputRegisters[4] == inputRegisters[5]);
			break;
		case 3:
			return Byte(inputRegisters[4] < inputRegisters[5]);
			break;
		default:
			throw std::runtime_error("Attempted to use alu with invalid mode: " + toHex(inputRegisters[6]));
			break;
		}
		break;
	default:
		throw std::runtime_error("Attempted to get invalid register: " + toHex(addr));
		break;
	}
}

void Simulator::setRam(Byte addr, Byte data){
	while(addr >= ram.size()){
		ram.push_back(0);
	}
	ram[addr] = data;
}

Byte Simulator::getRam(Byte addr){
	while(addr >= ram.size()){
		ram.push_back(0);
	}
	return ram[addr];
}

AsmAST::Context Simulator::getContextForIns(Byte ins){
	if(!hasDebugData){
		throw std::runtime_error("Attempted to get debug AsmAST::Context data, but the Simulator does not have any debug data");
	}
	AsmAST::Context lastContext;
	Byte currentIns = -1;//you might notice that I'm actually setting an unsigned value with a negative one here,
	//but this CPU -- and therefore the simulator -- actually must be able to subtract numbers by adding the
	//2's complement like this, so if this isn't valid then neither is the rest of this simulator
	//also, this is well defined in C++, so the compiler should have my back in making sure this works
	//and one of the automated tests actually checks to make sure this is valid, so we'll know if it doesn't work
	for(auto i = asmProgram.begin(); i != asmProgram.end(); i++){
		if(currentIns == ins){
			break;
		}
		switch(i->which()){
			case 0://AsmAST::GotoDestination
			    //do nothing
				break;
			case 1://AsmAST::Instruction
				currentIns++;
				break;
			case 2://AsmAST::Context
				lastContext = boost::get<AsmAST::Context>(*i);
				break;
            case 3://AsmAST::StartSection
                //do nothing
                break;
            case 4://AsmAST::EndSection
                //do nothing
                break;
		}
	}
	if(currentIns != ins){
		throw std::runtime_error("Could not find matching instruction inside of debug data");
	}
	return lastContext;
}

AsmAST::Context Simulator::getFunctionContextForIns(Byte ins){
	if(!hasDebugData){
		throw std::runtime_error("Attempted to get debug AsmAST::Context data, but the Simulator does not have any debug data");
	}
	AsmAST::Context lastContext;
	Byte currentIns = -1;
	for(auto i = asmProgram.begin(); i != asmProgram.end(); i++){
		if(currentIns == ins){
			break;
		}
		switch(i->which()){
			case 0://AsmAST::GotoDestination
				{
					if(boost::get<AsmAST::GotoDestination>(*i).name.rfind("__function", 0) == 0//if the goto destination starts with "__function"
					&& boost::get<AsmAST::GotoDestination>(*i).name.rfind("__functionReturn", 0) != 0){//and the goto destination doesn't start with "__functionReturn"
					
						lastContext = AsmAST::Context();//clear out the context so it can be refilled
					}
				}
				break;
			case 1://AsmAST::Instruction
				currentIns++;
				break;
			case 2://AsmAST::Context
				if(lastContext.context == ""){
					lastContext = boost::get<AsmAST::Context>(*i);
				}
				break;
            case 3://AsmAST::StartSection
                //do nothing
                break;
            case 4://AsmAST::EndSection
                //do nothing
                break;
		}
	}
	if(currentIns != ins){
		throw std::runtime_error("Could not find matching instruction inside of debug data");
	}
	return lastContext;
}









