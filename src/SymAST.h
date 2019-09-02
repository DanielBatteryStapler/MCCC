#ifndef SymAST_H
#define SymAST_H

#include "Definitions.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/variant.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/phoenix.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <exception>

namespace SymAST{
	using namespace boost::spirit;
	using namespace boost;
	using namespace boost::phoenix;
	
	template<typename T> using rule = qi::rule<std::string::iterator, T, ascii::space_type>;
	
	struct Import{
		std::string fileName;
	};
	
	struct VariableDeclaration{
		std::string type;
		std::string name;
	};
	
	using Value = boost::variant<std::string, Byte>;
	
	struct ReturnValue{
		Value value;
	};
	
	struct BreakStatement{
	};
	
	struct ReturnVoid{
	};
	
	struct VariableSetValue{
		std::string name;
		Value value;
	};
	
	struct FunctionCall{
		std::string name;
		std::vector<Value> parameters;
	};
	
	struct VariableSetFunction{
		std::string name;
		FunctionCall functionCall;
	};
	
	struct IfStatement;
	struct WhileStatement;
	
	using StatementVariant = boost::variant<VariableDeclaration, VariableSetValue, FunctionCall, VariableSetFunction,
			ReturnValue, ReturnVoid, BreakStatement, boost::recursive_wrapper<IfStatement>, boost::recursive_wrapper<WhileStatement> >;
	
	struct Context{
		std::string line;
		int lineNumber;
		std::string fileName;
	};
	
	struct Statement{
		StatementVariant code;
		Context context;
	};
	
	struct addAnnotationToStatement_f {
		std::string* fileContent;
		std::string fileName;
		
		addAnnotationToStatement_f(std::string* fileContent_, std::string fileName_):fileContent(fileContent_),fileName(fileName_){}
		
		void operator()(Statement& statement, std::string::iterator first, std::string::iterator last) const;
	};
	
	using Statements = std::vector<Statement>;
	
	struct IfStatement{
		Value conditional;
		Statements statements;
	};
	
	struct WhileStatement{
		Value conditional;
		Statements statements;
	};
	
	struct FunctionSignatureParameter{
		std::string type;
		std::string name;
	};
	
	struct FunctionDefinition{
		std::string returnType;
		std::string name;
		std::vector<FunctionSignatureParameter> parameters;
		Statements statements;
	};
	
	struct FunctionDeclaration{
		std::string returnType;
		std::string name;
		std::vector<FunctionSignatureParameter> parameters;
	};
	
	using Program = std::vector<boost::variant<Import, FunctionDeclaration, FunctionDefinition>>;
	
	Program createAST(std::string& fileContent, std::string fileName);
	
	struct Grammar : qi::grammar<std::string::iterator, Program(), ascii::space_type> {
		Grammar(std::string* fileContent, std::string fileName);
		
		phoenix::function<addAnnotationToStatement_f> annotateStatement_f;
		
		rule<Program()> program;
		rule<void()> reserved;
		rule<Statements()> statements;
		rule<Statement()> statement;
		rule<Import()> import;
		rule<FunctionDefinition()> functionDefinition;
		rule<FunctionDeclaration()> functionDeclaration;
		rule<VariableDeclaration()> variableDeclaration;
		rule<unsigned()> unsignedInteger;
		rule<Value()> value;
		rule<ReturnValue()> returnValue;
		rule<ReturnVoid()> returnVoid;
		rule<BreakStatement()> breakStatement;
		rule<VariableSetValue()> variableSetValue;
		rule<FunctionCall()> functionCall;
		rule<VariableSetFunction()> variableSetFunction;
		rule<IfStatement()> ifStatement;
		rule<WhileStatement()> whileStatement;
		rule<FunctionSignatureParameter()> functionSignatureParameter;
		rule<std::string()> specifierName;
		rule<std::string()> typeName;
	};
	
	struct print : public boost::static_visitor<> {
		print();
		print(int indent_);
		
		int indent;
		
		std::string createIndent() const;
		
		void operator()(Program program) const;
		void operator()(Statements statements) const;
		void operator()(Statement statement) const;
		void operator()(Import import) const;
		void operator()(FunctionDeclaration functionDeclaration) const;
		void operator()(FunctionDefinition functionDefinition) const;
		void operator()(VariableDeclaration variableDeclaration) const;
		void operator()(Value value) const;
		void operator()(ReturnValue returnValue) const;
		void operator()(ReturnVoid returnVoid) const;
		void operator()(VariableSetValue variableSetValue) const;
		void operator()(FunctionCall functionCall) const;
		void operator()(VariableSetFunction variableSetFunction) const;
		void operator()(FunctionSignatureParameter functionSignatureParameter) const;
		void operator()(IfStatement ifStatement) const;
		void operator()(WhileStatement whileStatement) const;
		void operator()(BreakStatement breakStatement) const;
	};
};


BOOST_FUSION_ADAPT_STRUCT(SymAST::ReturnVoid)
BOOST_FUSION_ADAPT_STRUCT(SymAST::BreakStatement)
BOOST_FUSION_ADAPT_STRUCT(SymAST::Statement, code)
//notice that I'm actually lying here, SymAST::Statement is not just a variant, it also has context, 
//but it doesn't need to know that. all it thinks it has to do is populate the variant, the context is handled later
BOOST_FUSION_ADAPT_STRUCT(SymAST::WhileStatement, conditional, statements)
BOOST_FUSION_ADAPT_STRUCT(SymAST::IfStatement, conditional, statements)
BOOST_FUSION_ADAPT_STRUCT(SymAST::ReturnValue, value)
BOOST_FUSION_ADAPT_STRUCT(SymAST::VariableDeclaration, type, name)
BOOST_FUSION_ADAPT_STRUCT(SymAST::FunctionSignatureParameter, type, name)
BOOST_FUSION_ADAPT_STRUCT(SymAST::Import, fileName)
BOOST_FUSION_ADAPT_STRUCT(SymAST::FunctionDeclaration, returnType, name, parameters)
BOOST_FUSION_ADAPT_STRUCT(SymAST::FunctionDefinition, returnType, name, parameters, statements)
BOOST_FUSION_ADAPT_STRUCT(SymAST::VariableSetValue, name, value)
BOOST_FUSION_ADAPT_STRUCT(SymAST::FunctionCall, name, parameters)
BOOST_FUSION_ADAPT_STRUCT(SymAST::VariableSetFunction, name, functionCall)

#endif // SymAST_H
