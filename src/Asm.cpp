#include "Asm.h"

#include <exception>

namespace Asm{
    std::vector<Byte> createByteCodeFromAsmAST(const AsmAST::Program& program){
        std::vector<Byte> output;
        
        auto addIns = [&output](Byte a, Byte b, Byte c, Byte d)->void{
                output.push_back(a);
                output.push_back(b);
                output.push_back(c);
                output.push_back(d);
            };
        
        auto getValue = [&program](AsmAST::Value value)->Byte{
                switch(value.which()){
                case 0://Byte
                    return boost::get<Byte>(value);
                    break;
                case 1://std::string
                    {
                        Byte pos = 0;
                        for(auto i = program.begin(); i != program.end(); i++){
                            switch(i->which()){
                            case 0://AsmAST::GotoDestination
                                if(boost::get<AsmAST::GotoDestination>(*i).name == boost::get<std::string>(value)){
                                    return pos;
                                }
                                break;
                            case 1://AsmAST::Instruction
                                pos++;
                                break;
                            case 2://AsmAST::Context
                                //do nothing
                                break;
                            }
                        }
                        throw std::runtime_error("Unmatched GOTO Destination reference \"" + boost::get<std::string>(value) + "\"");
                    }
                    break;
                }
            };
        
        for(auto i = program.begin(); i != program.end(); i++){
            switch(i->which()){
            case 0://AsmAST::GotoDestination
                //do nothing
                break;
            case 1://AsmAST::Instruction
                {
                    AsmAST::Instruction ins = boost::get<AsmAST::Instruction>(*i);
                    switch(ins.which()){
                    case 0://AsmAST::StopCpu
                        {
                        AsmAST::StopCpu stmt = boost::get<AsmAST::StopCpu>(ins);
                        addIns(0, 0, 0, 0);
                        }
                        break;
                    case 1://AsmAST::RamToReg
                        {
                        AsmAST::RamToReg stmt = boost::get<AsmAST::RamToReg>(ins);
                        addIns(1, getValue(stmt.ram), getValue(stmt.reg), 0);
                        }
                        break;
                    case 2://AsmAST::RegToRam
                        {
                        AsmAST::RegToRam stmt = boost::get<AsmAST::RegToRam>(ins);
                        addIns(2, getValue(stmt.ram), getValue(stmt.reg), 0);
                        }
                        break;
                    case 3://AsmAST::SetReg
                        {
                        AsmAST::SetReg stmt = boost::get<AsmAST::SetReg>(ins);
                        addIns(3, getValue(stmt.val), getValue(stmt.reg), 0);
                        }
                        break;
                    case 4://AsmAST::SetRam
                        {
                        AsmAST::SetRam stmt = boost::get<AsmAST::SetRam>(ins);
                        addIns(4, getValue(stmt.ram), getValue(stmt.val), 0);
                        }
                        break;
                    case 5://AsmAST::RegToReg
                        {
                        AsmAST::RegToReg stmt = boost::get<AsmAST::RegToReg>(ins);
                        addIns(5, getValue(stmt.from), getValue(stmt.to), 0);
                        }
                        break;
                    case 6://AsmAST::GotoOp
                        {
                        AsmAST::GotoOp stmt = boost::get<AsmAST::GotoOp>(ins);
                        addIns(6, getValue(stmt.op), 0, 0);
                        }
                        break;
                    case 7://AsmAST::ConditionalGoto
                        {
                        AsmAST::ConditionalGoto stmt = boost::get<AsmAST::ConditionalGoto>(ins);
                        addIns(7, getValue(stmt.op), 0, 0);
                        }
                        break;
                    case 8://AsmAST::ProgGoto
                        {
                        AsmAST::ProgGoto stmt = boost::get<AsmAST::ProgGoto>(ins);
                        addIns(8, 0, 0, 0);
                        }
                        break;
                    case 9://AsmAST::ProgRamToReg
                        {
                        AsmAST::ProgRamToReg stmt = boost::get<AsmAST::ProgRamToReg>(ins);
                        addIns(9, getValue(stmt.reg), 0, 0);
                        }
                        break;
                    case 10://AsmAST::RegToProgRam
                        {
                        AsmAST::RegToProgRam stmt = boost::get<AsmAST::RegToProgRam>(ins);
                        addIns(10, getValue(stmt.reg), 0, 0);
                        }
                        break;
                    case 11://AsmAST::SetProgRam
                        {
                        AsmAST::SetProgRam stmt = boost::get<AsmAST::SetProgRam>(ins);
                        addIns(11, 0, 0, 0);
                        }
                        break;
                    }
                }
                break;
            case 2://AsmAST::Context
                //do nothing
                break;
            case 3://AsmAST::StartSection
                //do nothing
                break;
            case 4://AsmAST::EndSection
                //do nothing
                break;
            }
        }
        
        return output;
    }

    void printByteCode(std::vector<Byte>& byteCode){
        for(int i = 0; i < byteCode.size(); i += 4){
            std::cout << toHex(byteCode[i]) << " " << toHex(byteCode[i + 1]) << " " << toHex(byteCode[i + 2]) << " " << toHex(byteCode[i + 3]) << "\n";
        }
    }
    
    void convertAsmASTToString(const AsmAST::Program& program, std::ostream& output){
		auto valToString = [](AsmAST::Value value)->std::string{
				switch(value.which()){
				case 0://Byte
					return std::to_string(boost::get<Byte>(value));
					break;
				case 1://std::string
					return "#" + boost::get<std::string>(value);
					break;
				}
			};
        
        int tabs = 0;
        
        auto createTab = [&tabs, &output](){
                std::string indent;
                for(int i = 0; i < tabs; i++){
                    indent += "    ";
                }
                output << indent;
            };
        
		for(auto i = program.begin(); i != program.end(); i++){
			switch(i->which()){
			case 0://AsmAST::GotoDestination
			    createTab();
				output << boost::get<AsmAST::GotoDestination>(*i).name << ":\n";
				break;
			case 1://AsmAST::Instruction
				{
					createTab();
					AsmAST::Instruction ins = boost::get<AsmAST::Instruction>(*i);
					switch(ins.which()){
					case 0://AsmAST::StopCpu
						{
						AsmAST::StopCpu stmt = boost::get<AsmAST::StopCpu>(ins);
						output << "StpCpu";
						}
						break;
					case 1://AsmAST::RamToReg
						{
						AsmAST::RamToReg stmt = boost::get<AsmAST::RamToReg>(ins);
						output << "RamReg " << valToString(stmt.ram) << " " << valToString(stmt.reg);
						}
						break;
					case 2://AsmAST::RegToRam
						{
						AsmAST::RegToRam stmt = boost::get<AsmAST::RegToRam>(ins);
						output << "RegRam " << valToString(stmt.reg) << " " << valToString(stmt.ram);
						}
						break;
					case 3://AsmAST::SetReg
						{
						AsmAST::SetReg stmt = boost::get<AsmAST::SetReg>(ins);
						output << "SetReg " << valToString(stmt.reg) << " " << valToString(stmt.val);
						}
						break;
					case 4://AsmAST::SetRam
						{
						AsmAST::SetRam stmt = boost::get<AsmAST::SetRam>(ins);
						output << "SetRam " << valToString(stmt.ram) << " " << valToString(stmt.val);
						}
						break;
					case 5://AsmAST::RegToReg
						{
						AsmAST::RegToReg stmt = boost::get<AsmAST::RegToReg>(ins);
						output << "RegReg " << valToString(stmt.from) << " " << valToString(stmt.to);
						}
						break;
					case 6://AsmAST::GotoOp
						{
						AsmAST::GotoOp stmt = boost::get<AsmAST::GotoOp>(ins);
						output << "GotoOp " << valToString(stmt.op);
						}
						break;
					case 7://AsmAST::ConditionalGoto
						{
						AsmAST::ConditionalGoto stmt = boost::get<AsmAST::ConditionalGoto>(ins);
						output << "CnGoto " << valToString(stmt.op);
						}
						break;
					case 8://AsmAST::ProgGoto
						{
						AsmAST::ProgGoto stmt = boost::get<AsmAST::ProgGoto>(ins);
						output << "PrGoto";
						}
						break;
					case 9://AsmAST::ProgRamToReg
						{
						AsmAST::ProgRamToReg stmt = boost::get<AsmAST::ProgRamToReg>(ins);
						output << "PrRmRg " << valToString(stmt.reg);
						}
						break;
					case 10://AsmAST::RegToProgRam
						{
						AsmAST::RegToProgRam stmt = boost::get<AsmAST::RegToProgRam>(ins);
						output << "RgPrRm " << valToString(stmt.reg);
						}
						break;
					case 11://AsmAST::SetProgRam
						{
						AsmAST::SetProgRam stmt = boost::get<AsmAST::SetProgRam>(ins);
						output << "StPrRm";
						}
						break;
					}
                    output << "\n";
				}
				break;
			case 2://AsmAST::Context
				{
				    createTab();
					AsmAST::Context context = boost::get<AsmAST::Context>(*i);
					output << "[\"" << context.context << "\"";
					if(context.file){
						output << " \"" << context.file->name << "\":" << context.file->line;
					}
					if(context.variables.size() > 0){
						output << " {";
						for(int v = 0; v < context.variables.size(); v++){
							output << "\"" << context.variables[v].variable << "\":" << context.variables[v].address;
							if(v + 1 != context.variables.size()){
								output << " ";
							}
						}
						output << "}";
					}
					if(context.stackSize){
						output << " " << *context.stackSize;
					}
					output << "]\n";
				}
				break;
            case 3://AsmAST::StartSection
                tabs++;
                break;
            case 4://AsmAST::EndSection
                tabs--;
                break;
			}
		}
	}
	
	void checkAsmAST(const AsmAST::Program& program){
        //this needs to make sure that all GOTOs go to a valid location and that there are no repeat GOTOs
        auto checkValue = [&program](AsmAST::Value _value)->void{
                switch(_value.which()){
				case 0://Byte
                    //nothing to check, all is good
					break;
				case 1://std::string
					{
					    std::string value = boost::get<std::string>(_value);
                        bool found = false;
                        for(auto i = program.begin(); i != program.end(); i++){
                            if(i->which() == 0){//AsmAST::GotoDestination
                                AsmAST::GotoDestination dest = boost::get<AsmAST::GotoDestination>(*i);
                                if(dest.name == value){
                                    found = true;
                                    break;
                                }
                            }
                        }
                        if(!found){
                            throw std::runtime_error("Could not find Goto Destination with name \"" + value + "\"");
                        }
					}
					break;
				}
            };
        auto checkGotoDestination = [&program](AsmAST::Program::const_iterator i){
                std::string destName = boost::get<AsmAST::GotoDestination>(*i).name;
                i++;
                for(; i != program.end(); i++){
                    if(i->which() == 0){//AsmAST::GotoDestination
                        AsmAST::GotoDestination dest = boost::get<AsmAST::GotoDestination>(*i);
                        if(dest.name == destName){
                            throw std::runtime_error("Found duplicated Goto Destination with name \"" + destName + "\"");
                        }
                    }
                }
            };
        
        for(auto i = program.begin(); i != program.end(); i++){
			switch(i->which()){
			case 0://AsmAST::GotoDestination
			    checkGotoDestination(i);
				break;
			case 1://AsmAST::Instruction
				{
					AsmAST::Instruction ins = boost::get<AsmAST::Instruction>(*i);
					switch(ins.which()){
					case 0://AsmAST::StopCpu
						{
						AsmAST::StopCpu stmt = boost::get<AsmAST::StopCpu>(ins);
						}
						break;
					case 1://AsmAST::RamToReg
						{
						AsmAST::RamToReg stmt = boost::get<AsmAST::RamToReg>(ins);
						checkValue(stmt.ram);
						checkValue(stmt.reg);
						}
						break;
					case 2://AsmAST::RegToRam
						{
						AsmAST::RegToRam stmt = boost::get<AsmAST::RegToRam>(ins);
						checkValue(stmt.reg);
						checkValue(stmt.ram);
						}
						break;
					case 3://AsmAST::SetReg
						{
						AsmAST::SetReg stmt = boost::get<AsmAST::SetReg>(ins);
						checkValue(stmt.reg);
						checkValue(stmt.val);
						}
						break;
					case 4://AsmAST::SetRam
						{
						AsmAST::SetRam stmt = boost::get<AsmAST::SetRam>(ins);
						checkValue(stmt.ram);
						checkValue(stmt.val);
						}
						break;
					case 5://AsmAST::RegToReg
						{
						AsmAST::RegToReg stmt = boost::get<AsmAST::RegToReg>(ins);
						checkValue(stmt.from);
						checkValue(stmt.to);
						}
						break;
					case 6://AsmAST::GotoOp
						{
						AsmAST::GotoOp stmt = boost::get<AsmAST::GotoOp>(ins);
						checkValue(stmt.op);
						}
						break;
					case 7://AsmAST::ConditionalGoto
						{
						AsmAST::ConditionalGoto stmt = boost::get<AsmAST::ConditionalGoto>(ins);
						checkValue(stmt.op);
						}
						break;
					case 8://AsmAST::ProgGoto
						{
						AsmAST::ProgGoto stmt = boost::get<AsmAST::ProgGoto>(ins);
						}
						break;
					case 9://AsmAST::ProgRamToReg
						{
						AsmAST::ProgRamToReg stmt = boost::get<AsmAST::ProgRamToReg>(ins);
						checkValue(stmt.reg);
						}
						break;
					case 10://AsmAST::RegToProgRam
						{
						AsmAST::RegToProgRam stmt = boost::get<AsmAST::RegToProgRam>(ins);
						checkValue(stmt.reg);
						}
						break;
					case 11://AsmAST::SetProgRam
						{
						AsmAST::SetProgRam stmt = boost::get<AsmAST::SetProgRam>(ins);
						}
						break;
					}
				}
				break;
			case 2://AsmAST::Context
				//ignore
				break;
            case 3://AsmAST::StartSection
                //ignore
                break;
            case 4://AsmAST::EndSection
                //ignore
                break;
			}
		}
	}
};

















