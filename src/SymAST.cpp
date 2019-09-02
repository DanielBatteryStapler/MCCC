#include "SymAST.h"

namespace SymAST{
	
	Grammar::Grammar(std::string* fileContent, std::string fileName):
			Grammar::base_type{program, "Program"},
			annotateStatement_f(addAnnotationToStatement_f(fileContent, fileName)) {
			
		typeName.name("TypeName");
		reserved.name("Reserved");
		specifierName.name("VariableName");
		variableDeclaration.name("VariableDeclaration");
		functionSignatureParameter.name("FunctionSignatureParameter");
		import.name("Import");
		functionDeclaration.name("FunctionDeclaration");
		functionDefinition.name("FunctionDefinition");
		statement.name("Statement");
		statements.name("Statements");
		program.name("Program");
		unsignedInteger.name("UnsignedInteger");
		value.name("Value");
		variableSetValue.name("VariableSetValue");
		functionCall.name("FunctionCall");
		variableSetFunction.name("VariableSetFunction");
		returnValue.name("ReturnValue");
		returnVoid.name("ReturnVoid");
		ifStatement.name("IfStatement");
		whileStatement.name("WhileStatement");
		breakStatement.name("BreakStatement");
		
		program = *(import | functionDeclaration | functionDefinition);
		
		import = qi::lit("import") >> '"' >> lexeme[+(ascii::char_ - '"')] >> '"' >> ';';
		
		functionDeclaration = typeName >> specifierName >> '(' >> -(functionSignatureParameter % ',') >> ')' >> qi::lit(';');
		functionDefinition = typeName >> specifierName >> '(' >> -(functionSignatureParameter % ',') >> ')' >> statements;
		functionSignatureParameter = typeName >> specifierName;
		
		statement = variableDeclaration | variableSetValue | variableSetFunction | functionCall
					| ifStatement | whileStatement | returnValue | returnVoid | breakStatement;
					
		statements = '{' >> *(statement) >> '}';
		
		variableDeclaration = typeName >> specifierName >> ';';
		variableSetValue = specifierName >> '=' >> value >> ';';
		variableSetFunction = specifierName >> '=' >> functionCall;
		returnValue = "return" >> value >> ';';
		returnVoid = "return" >> qi::lit(';');
		ifStatement = "if" >> qi::lit('(') >> value >> ')' >> statements;
		whileStatement = "while" >> qi::lit('(') >> value >> ')' >> statements;
		breakStatement = "break" >> qi::lit(';');
		
		functionCall = specifierName >> '(' >> -(value % ',') >> ')' >> ';';
		
		unsignedInteger = (qi::uint_ - (qi::lit("0b") | qi::lit("0x"))) | lexeme[qi::lit("0x") >> qi::hex] | lexeme[qi::lit("0b") >> qi::bin];
		value = unsignedInteger | specifierName;
		
		typeName = lexeme[+(ascii::alpha | ascii::char_('_'))] - reserved;
		specifierName = lexeme[+(ascii::alpha | ascii::char_('_'))] - reserved;
		
		reserved = qi::string("return") | "break" | "if" | "while";
		
		qi::on_error<qi::fail>(program, std::cout << val("Parse Error!\n"));
		qi::on_success(statement, annotateStatement_f(_val, _1, _3));
	}
	
	Program createAST(std::string& fileContent, std::string fileName){
		std::string::iterator it = fileContent.begin();
		
		Grammar g(&fileContent, fileName);
		Program program;
		if(qi::phrase_parse(it, fileContent.end(), g, ascii::space, program)) {
			if(it != fileContent.end()){
				throw std::runtime_error("SymAST: Error around line \'" + std::string(it, fileContent.end()) + "\'\n");
			}
		}
		
		return program;
	}
	
	void addAnnotationToStatement_f::operator()(Statement& statement, std::string::iterator first, std::string::iterator last) const {
		statement.context.line = std::string(first, last);
		
		std::size_t newLine = statement.context.line.find('\n');
		std::size_t bracket = std::string::npos;//statement.context.line.find('{');
		
		if(newLine != std::string::npos && bracket != std::string::npos){
			statement.context.line = statement.context.line.substr(0, std::min(newLine, bracket));
		}
		else if(newLine != std::string::npos){
			statement.context.line = statement.context.line.substr(0, newLine);
		}
		else if(newLine != std::string::npos){
			statement.context.line = statement.context.line.substr(0, bracket);
		}
		
		statement.context.fileName = fileName;
		statement.context.lineNumber = 1;
		for(std::string::iterator i = fileContent->begin(); i != first; i++){
			if(*i == '\n'){
				statement.context.lineNumber++;
			}
		}
	}
	
	print::print():indent(0){
		
	}
	
	print::print(int indent_):indent(indent_){
		
	}
	
	std::string print::createIndent() const{
		std::string output;
		for(int i = 0; i < indent; i++){
			output += "    ";
		}
		return output;
	}
	
	void print::operator()(Program program) const{
		std::cout << "Sym Program:{\n";
		for(auto i : program){
			boost::apply_visitor(print(indent + 1), i);
		}
		std::cout << "}\n";
	}
	
	void print::operator()(Statements statements) const{
		std::cout << createIndent() << "{\n";
		for(auto i : statements){
			print(indent + 1)(i);
		}
		std::cout << createIndent() << "}\n";
	}
	
	void print::operator()(Statement statement) const{
		boost::apply_visitor(*this, statement.code);
		//std::cout << createIndent() << "Context: [\"" << statement.context.line << "\" \"" << statement.context.fileName << "\":" << statement.context.lineNumber << "]\n"; 
	}
	
	void print::operator()(Import import) const{
		std::cout << createIndent() << "import \"" << import.fileName << "\";\n";
	}
	
	void print::operator()(FunctionDeclaration functionDeclaration) const{
		std::cout << createIndent() << functionDeclaration.returnType << ' ' << functionDeclaration.name << "(\n";
		for(auto i : functionDeclaration.parameters){
			print(indent + 1)(i);
		}
		std::cout << createIndent() << ");\n";
	}
	
	void print::operator()(FunctionDefinition functionDefinition) const{
		std::cout << createIndent() << functionDefinition.returnType << ' ' << functionDefinition.name << "(\n";
		for(auto i : functionDefinition.parameters){
			print(indent + 1)(i);
		}
		std::cout << createIndent() << ")\n";
		
		operator()(functionDefinition.statements);
	}

	void print::operator()(VariableDeclaration variableDeclaration) const{
		std::cout << createIndent() << variableDeclaration.type << ' ' << variableDeclaration.name << ";\n";
	}
	
	void print::operator()(Value value) const{
		switch(value.which()){
		case 0:
			std::cout << boost::get<std::string>(value);
			break;
		case 1:
			std::cout << boost::get<Byte>(value);
			break;
		}
	}
	
	void print::operator()(ReturnValue returnValue) const{
		std::cout << createIndent() << "builtIn: return ";
		operator()(returnValue.value);
		std::cout << ";\n";
	}
	
	void print::operator()(ReturnVoid returnVoid) const{
		std::cout << createIndent() << "builtIn: return;\n";
	}
	
	void print::operator()(VariableSetValue variableSetValue) const{
		std::cout << createIndent() << variableSetValue.name << " = ";
		operator()(variableSetValue.value);
		std::cout << ";\n";
	}
	
	void print::operator()(FunctionCall functionCall) const{
		std::cout << createIndent() << functionCall.name << "(";
		for(int i = 0; i < functionCall.parameters.size(); i++){
			std::cout << functionCall.parameters[i];
			if(i + 1 != functionCall.parameters.size()){
				std::cout << ", ";
			}
		}
		std::cout << ");\n";
	}
	
	void print::operator()(VariableSetFunction variableSetFunction) const{
		std::cout << createIndent() << variableSetFunction.name << " = " << variableSetFunction.functionCall.name << "(";
		for(int i = 0; i < variableSetFunction.functionCall.parameters.size(); i++){
			std::cout << variableSetFunction.functionCall.parameters[i];
			if(i + 1 != variableSetFunction.functionCall.parameters.size()){
				std::cout << ", ";
			}
		}
		std::cout << ");\n";
	}
	
	void print::operator()(FunctionSignatureParameter functionSignatureParameter) const{
		std::cout << createIndent() << functionSignatureParameter.type << " " << functionSignatureParameter.name << '\n';
	}
	
	void print::operator()(IfStatement ifStatement) const{
		std::cout << createIndent() << "if(";
		operator()(ifStatement.conditional);
		std::cout << ")\n";
		operator()(ifStatement.statements);
	}
	
	void print::operator()(WhileStatement whileStatement) const{
		std::cout << createIndent() << "while(";
		operator()(whileStatement.conditional);
		std::cout << ")\n";
		operator()(whileStatement.statements);
	}
	
	void print::operator()(BreakStatement whileStatement) const{
		std::cout << createIndent() << "builtIn: break;\n";
	}
}






