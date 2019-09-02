#include "AsmAST.h"


namespace AsmAST{
	
	Grammar::Grammar():
			Grammar::base_type{program, "Program"}{
			
		program.name("Program");
		reserved.name("Reserved");
		unsignedInteger.name("UnsignedInteger");
		value.name("Value");
		gotoDestination.name("GotoDestination");
		context.name("Context");
		fileContext.name("FileContext");
		stackContext.name("StackContext");
		instruction.name("Instruction");
		
		stopCpu.name("StopCpu");
		ramToReg.name("RamToReg");
		regToRam.name("RegToRam");
		setReg.name("SetReg");
		setRam.name("SetRam");
		regToReg.name("RegToReg");
		gotoOp.name("GotoOp");
		conditionalGoto.name("ConditionalGoto");
		progGoto.name("ProgGoto");
		progRamToReg.name("ProgRamToReg");
		regToProgRam.name("RegToProgRam");
		setProgRam.name("SetProgRam");
		
		specifierName.name("SpecifierName");
		quotedString.name("QuotedString");
		
		program = *(gotoDestination | instruction | context);
		
		unsignedInteger = (qi::uint_ - (qi::lit("0b") | qi::lit("0x"))) | lexeme[qi::lit("0x") >> qi::hex] | lexeme[qi::lit("0b") >> qi::bin];
		value = unsignedInteger | ('#' >> specifierName);
		
		gotoDestination = specifierName >> ':';
		
		context = '[' >> quotedString >> -fileContext >> -('{' >> (*stackContext) >> '}') >> -unsignedInteger >> ']';
		fileContext = quotedString >> ':' >> ushort_;
		stackContext = quotedString >> ':' >> unsignedInteger;
		
		instruction = stopCpu | ramToReg | regToRam | setReg | setRam | regToReg |
			gotoOp | conditionalGoto | progGoto | progRamToReg | regToProgRam | setProgRam;
		
		stopCpu = eps >> lit("StpCpu");
		ramToReg = lit("RamReg") >> value >> value;
		regToRam = lit("RegRam") >> value >> value;
		setReg = lit("SetReg") >> value >> value;
		setRam = lit("SetRam") >> value >> value;
		regToReg = lit("RegReg") >> value >> value;
		gotoOp = lit("GotoOp") >> value;
		conditionalGoto = lit("CnGoto") >> value;
		progGoto = eps >> lit("PrGoto");
		progRamToReg = lit("PrRmRg") >> value;
		regToProgRam = lit("RgPrRm") >> value;
		setProgRam = eps >> lit("StPrRm");
		
		specifierName = lexeme[+(ascii::alpha | ascii::digit | ascii::char_('_'))] - reserved;
		quotedString = '"' >> lexeme[*(ascii::char_ - '"')] >> '"';
		
		reserved = qi::lit("StpCpu") | qi::lit("RamReg") | qi::lit("RegRam") | qi::lit("SetReg") | qi::lit("SetRam") | qi::lit("RegReg") | 
			qi::lit("GotoOp") | qi::lit("CnGoto") | qi::lit("PrGoto") | qi::lit("PrRmRg") | lit("RgPrRm") | lit("StPrRm");
		
		qi::on_error<qi::fail>(program, std::cout << val("Parse Error!\n"));
	}
	
	Program createAST(std::string& fileContent, std::string fileName){
		std::string::iterator it = fileContent.begin();
		
		Grammar g;
		Program program;
		if(qi::phrase_parse(it, fileContent.end(), g, ascii::space, program)) {
			if(it != fileContent.end()){
				throw std::runtime_error("AsmAST: Error around line \'" + std::string(it, fileContent.end()) + "\'\n");
			}
		}
		
		return program;
	}
	
	
	
	print::print(){
		
	}
		
	void print::operator()(Program program) const{
		std::cout << "Asm Program:{\n";
		for(auto i : program){
			boost::apply_visitor(print(), i);
		}
		std::cout << "}\n";
	}
	
	
	void print::operator()(StartSection startSection) const{
		std::cout << "AsmAST::StartSection\n";
	}
	
	void print::operator()(EndSection endSection) const{
		std::cout << "AsmAST::EndSection\n";
	}
	
	void print::operator()(Value value) const{
		switch(value.which()){
		case 0:
			std::cout << boost::get<Byte>(value);
			break;
		case 1:
			std::cout << "#" << boost::get<std::string>(value);
			break;
		}
	}
	
	void print::operator()(GotoDestination destination) const{
		std::cout << destination.name << ":\n";
	}
	
	void print::operator()(Context context) const{
		std::cout << "[\"" << context.context << "\"";
		if(context.file){
			std::cout << " \"" << context.file->name << "\":" << context.file->line;
		}
		std::cout << "]\n";
	}
	
	void print::operator()(Instruction instruction) const{
		boost::apply_visitor(print(), instruction);
		std::cout << "\n";
	}
	
	void print::operator()(StopCpu ins) const{
		std::cout << "StpCpu";
	}
	
	
	void print::operator()(RamToReg ins) const{
		std::cout << "RamReg ";
		operator()(ins.ram);
		std::cout << " ";
		operator()(ins.reg);
	}
	
	void print::operator()(RegToRam ins) const{
		std::cout << "RegRam ";
		operator()(ins.reg);
		std::cout << " ";
		operator()(ins.ram);
	}
	
	void print::operator()(SetReg ins) const{
		std::cout << "SetReg ";
		operator()(ins.reg);
		std::cout << " ";
		operator()(ins.val);
	}
	
	void print::operator()(SetRam ins) const{
		std::cout << "SetRam ";
		operator()(ins.ram);
		std::cout << " ";
		operator()(ins.val);
	}
	
	void print::operator()(RegToReg ins) const{
		std::cout << "RegReg ";
		operator()(ins.from);
		std::cout << " ";
		operator()(ins.to);
	}
	
	void print::operator()(GotoOp ins) const{
		std::cout << "GotoOp ";
		operator()(ins.op);
	}
	
	void print::operator()(ConditionalGoto ins) const{
		std::cout << "CnGoto ";
		operator()(ins.op);
	}
	
	void print::operator()(ProgGoto ins) const{
		std::cout << "PrGoto";
	}
	
	void print::operator()(ProgRamToReg ins) const{
		std::cout << "PrRmRg ";
		operator()(ins.reg);
	}
	
	void print::operator()(RegToProgRam ins) const{
		std::cout << "RgPrRm ";
		operator()(ins.reg);
	}
	
	void print::operator()(SetProgRam ins) const{
		std::cout << "StPrRm";
	}
}









