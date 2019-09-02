#include "Sym.h"

#include <exception>
#include <ctime>

namespace Sym{
    namespace{
        struct CompFunctionSignature{
            std::string returnType;
            std::string name;
            int numberOfParameters;
        };
        
        std::string generateFunctionName(CompFunctionSignature func){
            std::string name;
            name = "__function__" + func.returnType + "__" + func.name;
            for(int i = 0; i < func.numberOfParameters; i++){
                name += "__byte"; 
            }
            return name;
        }
        
        template <typename T>
        CompFunctionSignature getFunctionSignature(T func){
            CompFunctionSignature output;
            if(func.returnType != "void" && func.returnType != "byte"){
                throw std::runtime_error("Found function \"" + func.name + "\" with invalid return type of \"" + func.returnType + "\"");
            }
            if(func.name == "getReg"){
                throw std::runtime_error("Found a function definition/declaration with an invalid name \"getReg\", this name is reserved");
            }
            if(func.name == "setReg"){
                throw std::runtime_error("Found a function definition/declaration with an invalid name \"setReg\", this name is reserved");
            }
            output.returnType = func.returnType;
            output.name = func.name;
            output.numberOfParameters = 0;
            for(auto i = func.parameters.begin(); i != func.parameters.end(); i++){
                if(i->type != "byte"){
                    throw std::runtime_error("Invalid parameter type \"" + i->type + "\" in function \"" + func.name + "\"");
                }
                output.numberOfParameters++;
            }
            if(output.name == "main"){
                if(output.returnType != "void" || output.numberOfParameters != 0){
                    throw std::runtime_error("the function \"main\" must return void and take no parameters");
                }
            }
            return output;
        }
        
        struct CompContext{
            std::vector<CompFunctionSignature> functions;
            CompFunctionSignature currentFunction;
            std::vector<std::string> variables;
            std::string breakDestination;
        };
        
        bool functionSignatureExists(const CompContext& context, CompFunctionSignature func, bool mustReturn = false){
            for(auto i = context.functions.begin(); i != context.functions.end(); i++){
                if(i->name == func.name && i->numberOfParameters == func.numberOfParameters){
                    if(mustReturn){
                        if(i->returnType == "byte"){
                            return true;
                        }
                        else{
                            return false;
                        }
                    }
                    else{
                        return true;
                    }
                }
            }
            return false;
        }
        
        CompFunctionSignature getFunctionWithSignature(const CompContext& context, CompFunctionSignature func){
            for(auto i = context.functions.begin(); i != context.functions.end(); i++){
                if(i->name == func.name && i->numberOfParameters == func.numberOfParameters){
                    return *i;
                }
            }
            throw std::runtime_error("Could not find function with Signature with name \"" + func.name + "\"");
        }
        
        void addFunctionToContext(CompContext& context, CompFunctionSignature signature){
            if(functionSignatureExists(context, signature)){
                CompFunctionSignature other = getFunctionWithSignature(context, signature);
                if(other.returnType != signature.returnType){
                    throw std::runtime_error("Attempted to declare a function that conflicts with another");
                }
            }
            else{
                context.functions.push_back(signature);
            }
        }
        
        bool variableExists(const CompContext& context, std::string variable){
            for(auto i = context.variables.begin(); i != context.variables.end(); i++){
                if(*i == variable){
                    return true;
                }
            }
            return false;
        }
        
        void addVariable(CompContext& context, std::string variable){
            if(variableExists(context, variable)){
                throw std::runtime_error("Attempted to declare variable \"" + variable + "\" in function \"" + context.currentFunction.name + "\" twice");
            }
            context.variables.push_back(variable);
        }
        
        Byte getStackVariableAddress(const CompContext& context, std::string variable){
            Byte addr = 1;//0x00 is last stack pointer, 0x01 is last instruction pointer
            if(context.currentFunction.returnType != "void"){
                addr = 2;//0x02 would be the return value pointer, if it has a return value
            }
            for(auto i = context.variables.begin(); i != context.variables.end(); i++){
                addr++;
                if(*i == variable){
                    return addr;
                }
            }
            throw std::runtime_error("Attempted to get stack variable address of \"" + variable + "\" in function \"" + context.currentFunction.name + "\" but that variable doesn't exist");
        }
        
        Byte getCurrentStackSize(const CompContext& context){
            Byte size = 2;
            if(context.currentFunction.returnType != "void"){
                size = 3;
            }
            size += context.variables.size();
            return size;
        }
        
        AsmAST::Context getAsmContext(const CompContext& compContext, const SymAST::Context& symContext){
			AsmAST::Context asmContext;
			asmContext.context = symContext.line;
			{
				AsmAST::FileContext fileContext;
				fileContext.line = symContext.lineNumber;
				fileContext.name = symContext.fileName;
				asmContext.file = fileContext;
			}
			for(auto v = compContext.variables.begin(); v != compContext.variables.end(); v++){
				AsmAST::StackContext stackContext;
				stackContext.address = getStackVariableAddress(compContext, *v);
				stackContext.variable = *v;
				asmContext.variables.push_back(stackContext);
			}
			asmContext.stackSize = getCurrentStackSize(compContext);
			return asmContext;
        }
        
        void doAsmForFunctionDefinition(CompContext context, AsmAST::Program& output, const SymAST::FunctionDefinition& definition);
        
        std::string getUniqueGotoToken(){
			static Byte number = 0;
			unsigned long nextNumber = number;
			number++;
			return std::to_string(nextNumber) + "_" + std::to_string(std::time(nullptr));//we're just gonna put time in there too so they must be unique
        }
    };
    
    AsmAST::Program createAsmASTFromSymAST(const SymAST::Program& program){
        AsmAST::Program output;
        
        CompContext context;
        
        for(auto i = program.begin(); i != program.end(); i++){
            switch(i->which()){
            case 0://SymAST::Import
                throw std::runtime_error("Found SymAST::Import when converting SymAST to AsmAST, this should never happen");
                break;
            case 1://SymAST::FunctionDeclaration
                {
                    SymAST::FunctionDeclaration declaration = boost::get<SymAST::FunctionDeclaration>(*i);
                    CompFunctionSignature signature = getFunctionSignature(declaration);
                    addFunctionToContext(context, signature);
                }
                break;
            case 2://SymAST::FunctionDefinition
                {
                    SymAST::FunctionDefinition definition = boost::get<SymAST::FunctionDefinition>(*i);
                    CompFunctionSignature signature = getFunctionSignature(definition);
                    addFunctionToContext(context, signature);
                    doAsmForFunctionDefinition(context, output, definition);
                }
                break;
            }
        }
        
        return output;
    }
    
    namespace{
        void doAsmForStatements(CompContext context, AsmAST::Program& output, const std::vector<SymAST::Statement>& statements);
        
        void doAsmForStatement(CompContext& context, AsmAST::Program& output, SymAST::VariableDeclaration stmt);
        void doAsmForStatement(CompContext& context, AsmAST::Program& output, SymAST::VariableSetValue stmt);
        void doAsmForStatement(CompContext& context, AsmAST::Program& output, SymAST::FunctionCall stmt);
        void doAsmForStatement(CompContext& context, AsmAST::Program& output, SymAST::VariableSetFunction stmt);
        void doAsmForStatement(CompContext& context, AsmAST::Program& output, SymAST::ReturnValue stmt);
        void doAsmForStatement(CompContext& context, AsmAST::Program& output, SymAST::ReturnVoid stmt);
        void doAsmForStatement(CompContext& context, AsmAST::Program& output, SymAST::BreakStatement stmt);
        void doAsmForStatement(CompContext& context, AsmAST::Program& output, SymAST::IfStatement stmt);
        void doAsmForStatement(CompContext& context, AsmAST::Program& output, SymAST::WhileStatement stmt);
        
        void doAsmForFunctionDefinition(CompContext context, AsmAST::Program& output, const SymAST::FunctionDefinition& definition){
            //every function has a stack frame:
            //the address of the current stack frame is stored in input register 0x03/output register 0x00
            //that register points towards the frame in ram that looks like(addresses are relative to the stack frame pointer):
            //0x00 = last stack pointer; what stack frame to go back to when this function is done
            //0x01 = last instruction pointer; what instruction to go back to when this function is done
            //(if the function returns something other than void)
            //  0x02 = a a pointer to where to store the return value
            //the rest of the values are where the various variables(including function arguments) are stored, so 0x03 could store a "byte a;"
            
            context.currentFunction = getFunctionSignature(definition);
            for(auto i = definition.parameters.begin(); i != definition.parameters.end(); i++){
                addVariable(context, i->name);
            }
            
            //add the goto that starts the function
            output.push_back(AsmAST::GotoDestination{generateFunctionName(context.currentFunction)});
            output.push_back(AsmAST::StartSection{});
            {//we want to add some AsmAST::Context for the debugger to have information about this function later
				AsmAST::Context asmContext;
				asmContext.context = definition.returnType + " " + definition.name + "(";
				for(int i = 0; i < definition.parameters.size(); i++){
					asmContext.context += definition.parameters[i].type + " " + definition.parameters[i].name;
					if(i + 1 != definition.parameters.size()){
						asmContext.context += ", ";
					}
				}
				asmContext.context += "){";
				for(int i = 0; i < context.variables.size(); i++){
					AsmAST::StackContext stackContext;
					stackContext.variable = context.variables[i];
					stackContext.address = getStackVariableAddress(context, context.variables[i]);
					asmContext.variables.push_back(stackContext);
				}
				asmContext.stackSize = getCurrentStackSize(context);
				output.push_back(asmContext);
            }
            output.push_back(AsmAST::RegToReg{0, 1});//prime the address register with the current stack pointer
            
            //actually handle the statements in the function
            doAsmForStatements(context, output, definition.statements);
            
            if(context.currentFunction.returnType == "void"){//if return type is void put instructions for the implicit return
                output.push_back(AsmAST::SetReg{0, 0});//set address to last stack pointer position
                output.push_back(AsmAST::ProgRamToReg{3});//write the last stack pointer to register 0x03
                output.push_back(AsmAST::SetReg{0, 1});//set address to last instruction pointer position
                output.push_back(AsmAST::ProgRamToReg{0});//set last instruction pointer into reg 0x00
                output.push_back(AsmAST::ProgGoto{});//go back to whatever function called us, with their stack pointer in reg 0x03
            }
            output.push_back(AsmAST::EndSection{});
            //if the return type isn't void, then the program must return explicitly, or else it is undefined behavior
            //which for this is to let the function bleed into the next one because there is nothing stopping it
        }
        
        void doAsmForStatements(CompContext context, AsmAST::Program& output, const std::vector<SymAST::Statement>& statements){
            
            output.push_back(AsmAST::StartSection());
            for(auto i = statements.begin(); i != statements.end(); i++){
                output.push_back(getAsmContext(context, i->context));//insert the Context into the AsmAST
                
                switch(i->code.which()){
                case 0://SymAST::VariableDeclaration
                    doAsmForStatement(context, output, boost::get<SymAST::VariableDeclaration>(i->code));
                    break;
                case 1://SymAST::VariableSetValue
                    doAsmForStatement(context, output, boost::get<SymAST::VariableSetValue>(i->code));
                    break;
                case 2://SymAST::FunctionCall
                    doAsmForStatement(context, output, boost::get<SymAST::FunctionCall>(i->code));
                    break;
                case 3://SymAST::VariableSetFunction
                    doAsmForStatement(context, output, boost::get<SymAST::VariableSetFunction>(i->code));
                    break;
                case 4://SymAST::ReturnValue
                    doAsmForStatement(context, output, boost::get<SymAST::ReturnValue>(i->code));
                    break;
                case 5://SymAST::ReturnVoid
                    doAsmForStatement(context, output, boost::get<SymAST::ReturnVoid>(i->code));
                    break;
                case 6://SymAST::BreakStatement
                    doAsmForStatement(context, output, boost::get<SymAST::BreakStatement>(i->code));
                    break;
                case 7://SymAST::IfStatement
                    doAsmForStatement(context, output, boost::get<SymAST::IfStatement>(i->code));
                    break;
                case 8://SymAST::WhileStatement
                    doAsmForStatement(context, output, boost::get<SymAST::WhileStatement>(i->code));
                    break;
                }
            }
            output.push_back(AsmAST::EndSection());
        }
        
        void doAsmForStatement(CompContext& context, AsmAST::Program& output, SymAST::VariableDeclaration stmt){
            if(stmt.type != "byte"){
                throw std::runtime_error("Variable declaration with invalid type \"" + stmt.type + "\" in function \"" + context.currentFunction.name + "\"");
            }
            addVariable(context, stmt.name);
        }
        
        void doAsmForStatement(CompContext& context, AsmAST::Program& output, SymAST::VariableSetValue stmt){
            Byte addr = getStackVariableAddress(context, stmt.name);
            
            switch(stmt.value.which()){
            case 0://std::string
                {
                    Byte addrB = getStackVariableAddress(context, boost::get<std::string>(stmt.value));
                    output.push_back(AsmAST::SetReg{0, addrB});
                    output.push_back(AsmAST::ProgRamToReg{2});
                    output.push_back(AsmAST::SetReg{0, addr});
                    output.push_back(AsmAST::SetProgRam{});
                }
                break;
            case 1://Byte
                output.push_back(AsmAST::SetReg{0, addr});
                output.push_back(AsmAST::SetReg{2, boost::get<Byte>(stmt.value)});
                output.push_back(AsmAST::SetProgRam{});
                break;
            }
        }
        
        void doAsmForFunctionCall(CompContext& context, AsmAST::Program& output, SymAST::FunctionCall stmt, bool alreadyHandledReturn){
            std::string currentCallToken = getUniqueGotoToken();
            
            CompFunctionSignature func;
            {
                CompFunctionSignature search;
                search.name = stmt.name;
                search.numberOfParameters = stmt.parameters.size();
                func = getFunctionWithSignature(context, search);
            }
            
            Byte stackSize = getCurrentStackSize(context);
            Byte parametersStart = stackSize;
            if(func.returnType != "void"){
                parametersStart += 3;//0lsp, 1lip, 2rvp, 3args...
                if(!alreadyHandledReturn){
                    //if we didn't handle the return already that means we don't care about it
                    //and we should just throw the return value away
                    
                    //what we want to do here is make the return value pointer point to itself
                    output.push_back(AsmAST::SetReg{0, stackSize + 2});
                    output.push_back(AsmAST::RegToReg{1, 2});//get the address of the rvp
                    output.push_back(AsmAST::SetProgRam{});//set the rvp to be that address
                }
            }
            else{
                parametersStart += 2;//0lsp, 1lip, 2args...
            }
            
            {//push on the stack frame header
				//push on the last stack pointer(which is currently the current one)
                output.push_back(AsmAST::RegToReg{0, 2});
                output.push_back(AsmAST::SetReg{0, stackSize + 0});//lsp has an offset of 0
                output.push_back(AsmAST::SetProgRam{});
				//push on the instruction pointer that tells the function where to resume to
                output.push_back(AsmAST::SetReg{0, stackSize + 1});//lip has an offset of 1
                output.push_back(AsmAST::SetReg{2, "__functionReturn__" + currentCallToken});
                output.push_back(AsmAST::SetProgRam{});
            }
            for(int i = 0; i < stmt.parameters.size(); i++){//push on the arguments
                switch(stmt.parameters[i].which()){
                case 0://std::string
                    {
                        Byte addr = getStackVariableAddress(context, boost::get<std::string>(stmt.parameters[i]));
                        output.push_back(AsmAST::SetReg{0, addr});
                        output.push_back(AsmAST::ProgRamToReg{2});
                        output.push_back(AsmAST::SetReg{0, parametersStart + i});
                        output.push_back(AsmAST::SetProgRam{});
                    }
                    break;
                case 1://Byte
                    output.push_back(AsmAST::SetReg{2, boost::get<Byte>(stmt.parameters[i])});
                    output.push_back(AsmAST::SetReg{0, parametersStart + i});
                    output.push_back(AsmAST::SetProgRam{});
                    break;
                }
            }
            {//set the stack frame pointer to be the function we're about to call
				output.push_back(AsmAST::SetReg{0, stackSize});
				output.push_back(AsmAST::RegToReg{1, 3});//put the new stack frame pointer into the stack frame pointer register
			}
            {//actually call the function
                output.push_back(AsmAST::GotoOp{generateFunctionName(func)});
                output.push_back(AsmAST::GotoDestination{"__functionReturn__" + currentCallToken});
                output.push_back(AsmAST::RegToReg{0, 1});
            }
            
        }
        
        void doAsmForStatement(CompContext& context, AsmAST::Program& output, SymAST::FunctionCall stmt){
            if(stmt.name == "setReg"){
                
                if(stmt.parameters.size() != 2){
                    throw std::runtime_error("attempted to call built-in function 'setReg' with invalid number of parameters in function \"" + context.currentFunction.name + "\"");
                }
                if(stmt.parameters[0].which() != 1){
                    throw std::runtime_error("attempted to call built-in function 'setReg' with an invalid parameter in function \"" + context.currentFunction.name + "\", first parameter must be a number-literal");
                }
                switch(stmt.parameters[1].which()){
                case 0://std::string
                    {
                        Byte addr = getStackVariableAddress(context, boost::get<std::string>(stmt.parameters[1]));
                        output.push_back(AsmAST::SetReg{0, addr});
                        output.push_back(AsmAST::ProgRamToReg{boost::get<Byte>(stmt.parameters[0])});
                    }
                    break;
                case 1://Byte
                    output.push_back(AsmAST::SetReg{boost::get<Byte>(stmt.parameters[0]), boost::get<Byte>(stmt.parameters[1])});
                    break;
                }
                
            }
            else{
                doAsmForFunctionCall(context, output, stmt, false);
            }
        }
        
        void doAsmForStatement(CompContext& context, AsmAST::Program& output, SymAST::VariableSetFunction stmt){
            Byte addr = getStackVariableAddress(context, stmt.name);//the address of where the return value should go
            if(stmt.functionCall.name == "getReg"){
                if(stmt.functionCall.parameters.size() != 1){
                    throw std::runtime_error("attempted to call built-in function 'getReg' with invalid number of parameters in function \"" + context.currentFunction.name + "\"");
                }
                if(stmt.functionCall.parameters[0].which() != 1){
                    throw std::runtime_error("attempted to call built-in function 'getReg' with an invalid parameter in function \"" + context.currentFunction.name + "\", first parameter must be a number-literal");
                }
                output.push_back(AsmAST::SetReg{0, addr});
                output.push_back(AsmAST::RegToReg{boost::get<Byte>(stmt.functionCall.parameters[0]), 2});
                output.push_back(AsmAST::SetProgRam{});
            }
            else{
                CompFunctionSignature func;
                func.name = stmt.functionCall.name;
                func.numberOfParameters = stmt.functionCall.parameters.size();
                if(!functionSignatureExists(context, func, true)){
                    throw std::runtime_error("Attempted to call function \"" + func.name + "\" in function \"" + context.currentFunction.name + "\" but a function with that signature does not exist");
                }
                
                Byte stackSize = getCurrentStackSize(context);
                
                //we're setting the return value pointer(rvp) to be a pointer to the variable we're setting
                output.push_back(AsmAST::SetReg{0, addr});
                output.push_back(AsmAST::RegToReg{1, 2});//make the return value pointer absolute
                output.push_back(AsmAST::SetReg{0, stackSize + 2});//currentStackFrame..., 0lsp, 1lip, 2rvp, 3arg0
                                                                    //we're setting this one~~^
                output.push_back(AsmAST::SetProgRam{});
                
                doAsmForFunctionCall(context, output, stmt.functionCall, true);
            }
        }
        
        void doAsmForStatement(CompContext& context, AsmAST::Program& output, SymAST::ReturnValue stmt){
            switch(stmt.value.which()){
            case 0://std::string
                {
                    Byte addr = getStackVariableAddress(context, boost::get<std::string>(stmt.value));
                    output.push_back(AsmAST::SetReg{0, addr});//set the address to the return value
					output.push_back(AsmAST::ProgRamToReg{2});//put the return value into the correct register
                }
                break;
            case 1://Byte
                output.push_back(AsmAST::SetReg{2, boost::get<Byte>(stmt.value)});//put return value into register
                break;
            }
            output.push_back(AsmAST::SetReg{0, 2});//set address to return pointer
            output.push_back(AsmAST::ProgRamToReg{0});//set address to where the return pointer points
            output.push_back(AsmAST::SetReg{1, 0});//don't add anything
            output.push_back(AsmAST::SetProgRam{});//set the return value to wherever it goes
            output.push_back(AsmAST::RegToReg{0, 1});//put the current stack pointer back into the add register
            output.push_back(AsmAST::SetReg{0, 0});//set address to last stack pointer position
            output.push_back(AsmAST::ProgRamToReg{3});//write the last stack pointer to 0x03
            output.push_back(AsmAST::SetReg{0, 1});//set address to last instruction pointer position
            output.push_back(AsmAST::ProgRamToReg{0});//set last instruction pointer into reg 0x00
            output.push_back(AsmAST::ProgGoto{});//go back to whatever function called us, with their stack pointer in reg 0x03
        }
        
        void doAsmForStatement(CompContext& context, AsmAST::Program& output, SymAST::ReturnVoid stmt){
            if(context.currentFunction.returnType != "void"){
                throw std::runtime_error("Attempted to return void in function \"" + context.currentFunction.name + "\", which has a return type of \"" + context.currentFunction.returnType + "\"");
            }
            output.push_back(AsmAST::SetReg{0, 0});//set address to last stack pointer position
			output.push_back(AsmAST::ProgRamToReg{3});//write the last stack pointer to 0x03
			output.push_back(AsmAST::SetReg{0, 1});//set address to last instruction pointer position
			output.push_back(AsmAST::ProgRamToReg{0});//set last instruction pointer into reg 0x00
			output.push_back(AsmAST::ProgGoto{});//go back to whatever function called us, with their stack pointer in reg 0x03
        }
        
        void doAsmForStatement(CompContext& context, AsmAST::Program& output, SymAST::BreakStatement stmt){
            if(context.breakDestination == ""){
                throw std::runtime_error("Attempted to break in function \"" + context.currentFunction.name + "\" without being inside of a while statement");
            }
            output.push_back(AsmAST::GotoOp{context.breakDestination});
        }
        
        void doAsmForStatement(CompContext& context, AsmAST::Program& output, SymAST::IfStatement stmt){
            
            switch(stmt.conditional.which()){
            case 0://std::string
                {
                    std::string currentIfToken = getUniqueGotoToken();
                    
                    Byte addr = getStackVariableAddress(context, boost::get<std::string>(stmt.conditional));
                    output.push_back(AsmAST::SetReg{0, addr});
					output.push_back(AsmAST::ProgRamToReg{0});
					output.push_back(AsmAST::ConditionalGoto{"__ifTrue__" + currentIfToken});
					output.push_back(AsmAST::GotoOp{"__ifFalse__" + currentIfToken});
					output.push_back(AsmAST::GotoDestination{"__ifTrue__" + currentIfToken});
					CompContext newContext = context;
					doAsmForStatements(newContext, output, stmt.statements);//actually put the instruction for the if statement
					output.push_back(AsmAST::GotoDestination{"__ifFalse__" + currentIfToken});
                }
                break;
            case 1://Byte
                {
                    if(boost::get<Byte>(stmt.conditional) == 0){
                        //if the if statement is always false, then just don't put the instructions into the function
                    }
                    else{
                        //if the if statement is always true, then just don't check if see if you should run the statements, just run them every time
                        CompContext newContext = context;
                        doAsmForStatements(newContext, output, stmt.statements);
                    }
                }
                break;
            }
        }
        
        void doAsmForStatement(CompContext& context, AsmAST::Program& output, SymAST::WhileStatement stmt){
            
            switch(stmt.conditional.which()){
            case 0://std::string
                {
                    std::string currentWhileToken = getUniqueGotoToken();
                    
                    Byte addr = getStackVariableAddress(context, boost::get<std::string>(stmt.conditional));
                    output.push_back(AsmAST::GotoDestination{"__whileLoop__" + currentWhileToken});
                    output.push_back(AsmAST::SetReg{0, addr});
					output.push_back(AsmAST::ProgRamToReg{0});
					output.push_back(AsmAST::ConditionalGoto{"__whileTrue__" + currentWhileToken});
					output.push_back(AsmAST::GotoOp{"__whileFalse__" + currentWhileToken});
					output.push_back(AsmAST::GotoDestination{"__whileTrue__" + currentWhileToken});
					CompContext newContext = context;
					newContext.breakDestination = "__whileFalse__" + currentWhileToken;
					doAsmForStatements(context, output, stmt.statements);//actually put the instruction for the if statement
                    output.push_back(AsmAST::GotoOp{"__whileLoop__" + currentWhileToken});
					output.push_back(AsmAST::GotoDestination{"__whileFalse__" + currentWhileToken});
                }
                break;
            case 1://Byte
                {
                    if(boost::get<Byte>(stmt.conditional) == 0){
                        //if the while statement is always false, then just don't put the instructions into the function
                    }
                    else{
                        std::string currentWhileToken = getUniqueGotoToken();
                        
                        //if the while statement is always true, then just don't check if see if you should run the statements, just run them every time
                        output.push_back(AsmAST::GotoDestination{"__whileLoop__" + currentWhileToken});
                        CompContext newContext = context;
                        newContext.breakDestination = "__whileFalse__" + currentWhileToken;
                        doAsmForStatements(newContext, output, stmt.statements);
                        output.push_back(AsmAST::GotoOp{"__whileLoop__" + currentWhileToken});
                        output.push_back(AsmAST::GotoDestination{"__whileFalse__" + currentWhileToken});
                    }
                }
                break;
            }
        }
    };
    
    AsmAST::Program generateExecutableHeader(){
        AsmAST::Program output;
        
        output.push_back(AsmAST::Context{"Executable Header"});
        output.push_back(AsmAST::SetReg{3, 0});//set the current stack pointer to where main will be placed
        output.push_back(AsmAST::SetRam{0, 0});//set the last stack pointer, which doesn't matter in this case
        output.push_back(AsmAST::SetRam{1, "__mainReturn"});//set lip to where main will return to
        CompFunctionSignature main;
        main.name = "main";
        main.numberOfParameters = 0;
        main.returnType = "void";
        output.push_back(AsmAST::GotoOp{generateFunctionName(main)});//actually go to main
        output.push_back(AsmAST::GotoDestination{"__mainReturn"});
        output.push_back(AsmAST::StopCpu{});//only main returns, just stop the CPU
        
        return output;
    }
};






















