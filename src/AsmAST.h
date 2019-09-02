#ifndef ASMAST_H
#define ASMAST_H

#include <boost/spirit/include/qi.hpp>
#include <boost/variant.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/phoenix.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <exception>

#include "Definitions.h"

namespace AsmAST{
	
	// StpCpu 
	// RamReg RAM REG
	// RegRam REG RAM
	// SetReg REG VAL
	// SetRam RAM VAL
	// RegReg REG REG
	// GotoOp OP
	// CnGoto OP    //uses the value of register 0x00 to see if it should goto that position
	// PrGoto       //goto the value of register 0x00, allows for arbitrary goto commands
	// PrRmRg REG   //set register using a pointer to ram(reg 0x00 + reg 0x01)
	// RgPrRm REG   //set ram using a pointer(reg 0x00 + reg 0x01) and a register
	// StPrRm       //set ram using a pointer(reg 0x00 + reg 0x01) and a value(reg 0x02)
	// ["context"]
	// ["context" "file":0]
	// ["context" "file":0 {"var":2}]
	
	using namespace boost::spirit;
	using namespace boost;
	using namespace boost::phoenix;
	
	template<typename T> using rule = qi::rule<std::string::iterator, T, ascii::space_type>;
	
	using Value = boost::variant<Byte, std::string>;
	
	struct GotoDestination{
		std::string name;
	};
	
	struct StopCpu{
	};
	
	struct RamToReg{
		Value ram;
		Value reg;
	};
	
	struct RegToRam{
		Value reg;
		Value ram;
	};
	
	struct SetReg{
		Value reg;
		Value val;
	};
	
	struct SetRam{
		Value ram;
		Value val;
	};
	
	struct RegToReg{
		Value from;
		Value to;
	};
	
	struct GotoOp{
		Value op;
	};
	
	struct ConditionalGoto{
		Value op;
	};
	
	struct ProgGoto{
	};
	
	struct ProgRamToReg{
		Value reg;
	};
	
	struct RegToProgRam{
		Value reg;
	};
	
	struct SetProgRam{
	};
	
	using Instruction = boost::variant<StopCpu, RamToReg, RegToRam, SetReg, SetRam, RegToReg,
							GotoOp, ConditionalGoto, ProgGoto, ProgRamToReg, RegToProgRam, SetProgRam>;
	
	struct StackContext{
		std::string variable;
		Byte address;
	};
	
	struct FileContext{
		std::string name;
		int line;
	};
	
	struct Context{
		std::string context;
		boost::optional<FileContext> file;
		std::vector<StackContext> variables;
		boost::optional<Byte> stackSize;
	};
	
	struct StartSection{
	};
	
	struct EndSection{
	};
	
	using Program = std::vector<boost::variant<GotoDestination, Instruction, Context, StartSection, EndSection> >;
	
	Program createAST(std::string& fileContent, std::string fileName);
	
	struct Grammar : qi::grammar<std::string::iterator, Program(), ascii::space_type> {
		Grammar();
		
		rule<Program()> program;
		rule<void()> reserved;
		rule<unsigned()> unsignedInteger;
		rule<Value()> value;
		rule<GotoDestination()> gotoDestination;
		rule<Context()> context;
		rule<FileContext()> fileContext;
		rule<StackContext()> stackContext;
		rule<Instruction()> instruction;
		
		rule<StopCpu()> stopCpu;
		rule<RamToReg()> ramToReg;
		rule<RegToRam()> regToRam;
		rule<SetReg()> setReg;
		rule<SetRam()> setRam;
		rule<RegToReg()> regToReg;
		rule<GotoOp()> gotoOp;
		rule<ConditionalGoto()> conditionalGoto;
		rule<ProgGoto()> progGoto;
		rule<ProgRamToReg()> progRamToReg;
		rule<RegToProgRam()> regToProgRam;
		rule<SetProgRam()> setProgRam;
		
		rule<std::string()> specifierName;
		rule<std::string()> quotedString;
	};
	
	struct print : public boost::static_visitor<> {
		print();
		
		std::string createIndent() const;
		
		void operator()(Program program) const;
		void operator()(StartSection startSection) const;
		void operator()(EndSection endSection) const;
		void operator()(Value value) const;
		void operator()(GotoDestination destination) const;
		void operator()(Context context) const;
		void operator()(Instruction instruction) const;
		
		void operator()(StopCpu ins) const;
		void operator()(RamToReg ins) const;
		void operator()(RegToRam ins) const;
		void operator()(SetReg ins) const;
		void operator()(SetRam ins) const;
		void operator()(RegToReg ins) const;
		void operator()(GotoOp ins) const;
		void operator()(ConditionalGoto ins) const;
		void operator()(ProgGoto ins) const;
		void operator()(ProgRamToReg ins) const;
		void operator()(RegToProgRam ins) const;
		void operator()(SetProgRam ins) const;
	};
};

BOOST_FUSION_ADAPT_STRUCT(AsmAST::StopCpu)
BOOST_FUSION_ADAPT_STRUCT(AsmAST::RamToReg, ram, reg)
BOOST_FUSION_ADAPT_STRUCT(AsmAST::RegToRam, reg, ram)
BOOST_FUSION_ADAPT_STRUCT(AsmAST::SetReg, reg, val)
BOOST_FUSION_ADAPT_STRUCT(AsmAST::SetRam, ram, val)
BOOST_FUSION_ADAPT_STRUCT(AsmAST::RegToReg, from, to)
BOOST_FUSION_ADAPT_STRUCT(AsmAST::GotoOp, op)
BOOST_FUSION_ADAPT_STRUCT(AsmAST::ConditionalGoto, op)
BOOST_FUSION_ADAPT_STRUCT(AsmAST::ProgGoto)
BOOST_FUSION_ADAPT_STRUCT(AsmAST::ProgRamToReg, reg)
BOOST_FUSION_ADAPT_STRUCT(AsmAST::RegToProgRam, reg)
BOOST_FUSION_ADAPT_STRUCT(AsmAST::SetProgRam)

BOOST_FUSION_ADAPT_STRUCT(AsmAST::StackContext, variable, address)
BOOST_FUSION_ADAPT_STRUCT(AsmAST::FileContext, name, line)
BOOST_FUSION_ADAPT_STRUCT(AsmAST::Context, context, file, variables, stackSize)
BOOST_FUSION_ADAPT_STRUCT(AsmAST::GotoDestination, name)

#endif // ASMAST_H
