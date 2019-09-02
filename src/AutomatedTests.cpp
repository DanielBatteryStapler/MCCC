#include "AutomatedTests.h"

#include <vector>
#include <functional>
#include <iostream>
#include "Simulator.h"
#include "Processor.h"

namespace AutomatedTests{
    
    class Tests{
    public:
        std::vector<std::pair<std::string, std::function<void()>>> tests;
        
        void add(std::string name, std::function<void()> func){
            tests.push_back(std::make_pair(name, func));
        }
        
        void runTests(){
            int passedTests = 0;
            for(int i = 0; i < tests.size(); i++){
                std::string name = tests[i].first;
                std::function<void()> func = tests[i].second;
                std::cout << "\nRunning test \"" << name << "\"...\n";
                bool passed = true;
                try{
                    func();
                }
                catch(std::exception& e){
                    std::cout << "Test \"" << name << "\" failed, Error:\"" << e.what() << "\"\n";
                    passed = false;
                }
                if(passed){
                    std::cout << "Test \"" << name << "\" passed\n";
                    passedTests++;
                }
            }
            std::cout
            << "\n\n||=====================================||\n"
            << "  Summary:\n"
            << "        " << passedTests << " out of " << tests.size() << " tests passed.\n";
            if(passedTests == tests.size()){
                std::cout << "\n        All tests passed!\n";
            }
            else{
                throw std::runtime_error("Not all tests passed");
            }
        }
    };
    
    void shouldThrowException(std::function<void()> func){
        try{
            func();
        }
        catch(std::exception& e){
            return;//return with no error
        }
        throw std::runtime_error("Expected a thrown exception, but got none");
    }
    
    void runAll(){
        Tests test;
        
        test.add("testExceptionInverter", [](){
            shouldThrowException([](){shouldThrowException([](){});});
            try{
                shouldThrowException([](){});
            }
            catch(std::exception& e){
                return;
            }
            throw std::runtime_error("Expected \"shouldThrowException\" to throw an exception, but it didn't");
        });
        
        test.add("testCpuSimulatorTester", [](){
            Simulator test;
            test.loadRom(Processor::generateByteCodeFromAsmString(
            "SetReg 7 1"
            "SetReg 7 2"
            "SetReg 7 42"
            "SetReg 7 42"
            "SetReg 7 65535"
            "SetReg 4 35"
            "SetReg 5 0"
            "SetReg 6 0"
            "RegReg 2 7"
            "StpCpu"
            ));
            test.setTests({1, 2, 42, 42, 65535, 35});
            test.run();
            
            shouldThrowException([](){//wrong number
                Simulator test;
                test.loadRom(Processor::generateByteCodeFromAsmString(
                "SetReg 7 1"
                "SetReg 7 2"
                "SetReg 7 3"
                "SetReg 7 3"
                "SetReg 7 5"
                "StpCpu"
                ));
                test.setTests({1, 2, 3, 4, 5});
                test.run();
            });
            
            shouldThrowException([](){//too few
                Simulator test;
                test.loadRom(Processor::generateByteCodeFromAsmString(
                "SetReg 7 1"
                "SetReg 7 2"
                "SetReg 7 3"
                "SetReg 7 4"
                "StpCpu"
                ));
                test.setTests({1, 2, 3, 4, 5});
                test.run();
            });
            
            shouldThrowException([](){//too many
                Simulator test;
                test.loadRom(Processor::generateByteCodeFromAsmString(
                "SetReg 7 1"
                "SetReg 7 2"
                "SetReg 7 3"
                "SetReg 7 4"
                "SetReg 7 5"
                "SetReg 7 6"
                "StpCpu"
                ));
                test.setTests({1, 2, 3, 4, 5});
                test.run();
            });
        });
        
        test.add("testBasicAsm", [](){
			 Simulator test;
            test.loadRom(Processor::generateByteCodeFromAsmString(//we're just gonna throw in some different bases to make it fun
            "SetReg 7 0x1"
            "SetReg 7 0b10"
            "SetReg 7 42"
            "SetReg 7 42"
            "SetReg 7 0xFFFF"
            "SetReg 4 0x23"
            "SetReg 5 0"
            "SetReg 0b110 0"
            "RegReg 2 7"
            "StpCpu"
            ));
            test.setTests({1, 2, 42, 42, 65535, 35});
            test.run();
        });
        
        test.add("testBasicSym", [](){
            Simulator test;
            test.loadRom(Processor::generateExecutableByteCodeFromSymString(
            "void main(){"
            "   setReg(7, 1);"
            "   setReg(7, 2);"
            "   setReg(7, 42);"
            "   byte output;"
            "   output = 42;"
            "   setReg(7, output);"
            "   setReg(7, 65535);"
            "   setReg(4, 35);"
            "   setReg(5, 0);"
            "   setReg(6, 0);"
            "   output = getReg(2);"
            "   setReg(7, output);"
            "}"
            ));
            test.setTests({1, 2, 42, 42, 65535, 35});
            test.run();
        });
        
        test.add("testFunctionCall", [](){
            Simulator test;
            test.loadRom(Processor::generateExecutableByteCodeFromSymString(
            "void foo(){"
            "   setReg(7, 1);"
            "}"
            ""
            "void bar(byte input){"
            "   setReg(7, input);"
            "}"
            ""
            "void foobar(byte input){"
            "   setReg(7, input);"
            "   setReg(7, 42);"
            "}"
            ""
            "void main(){"
            "   byte input;"
            "   input = 42;"
            "   foo();"
            "   bar(35);"
            "   bar(input);"
            "   input = 65535;"
            "   foobar(45);"
            "   foobar(input);"
            "}"
            ));
            test.setTests({1, 35, 42, 45, 42, 65535, 42});
            test.run();
        });
        
        test.add("testVariableScope", [](){
            Simulator test;
            test.loadRom(Processor::generateExecutableByteCodeFromSymString(
            "void bar(byte a){"
            "   a = 4;"
            "   setReg(7, a);"
            "}"
            ""
            "void main(){"
            "   if(1){"
            "       byte a;"
            "       a = 1;"
            "       setReg(7, a);"
            "   }"
            "   while(1){"
            "       byte a;"
            "       a = 2;"
            "       setReg(7, 2);"
            "       break;"
            "   }"
            "   byte a;"
            "   a = 3;"
            "   setReg(7, a);"
            "   a = 5;"
            "   bar(a);"
            "   setReg(7, a);"
            "}"
            ));
            test.setTests({1, 2, 3, 4, 5});
            test.run();
        });
        
        test.add("testFunctionReturnValue", [](){
            Simulator test;
            test.loadRom(Processor::generateExecutableByteCodeFromSymString(
            "byte foo(){"
            "   byte a;"
            "   a = 1;"
            "   setReg(7, a);"
            "   return a;"
            "}"
            ""
            "byte foo(byte bar){"
            "   setReg(7, bar);"
            "   bar = 3;"
            "   return bar;"
            "}"
            ""
            "void main(){"
            "   byte a;"
            "   a = foo();"
            "   setReg(7, a);"
            "   a = 2;"
            "   a = foo(a);"
            "   setReg(7, a);"
            "}"
            ));
            test.setTests({1, 1, 2, 3});
            test.run();
        });
        
        test.add("decrementTest", [](){
            Simulator test;
            test.loadRom(Processor::generateExecutableByteCodeFromSymString(
            "byte decrement(byte bar){"
            "   setReg(4, bar);"
            "   setReg(5, 0xFFFF);"
            "   setReg(6, 0);"
            "   bar = getReg(2);"
            "   return bar;"
            "}"
            ""
            "void main(){"
            "   byte a;"
            "   a = 5;"
            "   a = decrement(a);"
            "   setReg(7, a);"
            "   a = decrement(a);"
            "   setReg(7, a);"
            "   a = decrement(a);"
            "   setReg(7, a);"
            "   a = decrement(a);"
            "   setReg(7, a);"
            "   a = decrement(a);"
            "   setReg(7, a);"
            "}"
            ));
            test.setTests({4, 3, 2, 1, 0});
            test.run();
        });
        
        test.add("forLoopTest", [](){
            Simulator test;
            test.loadRom(Processor::generateExecutableByteCodeFromSymString(
            "void main(){"
            "   setReg(6, 0);"
            "   setReg(5, 0xFFFF);"
            "   byte a;"
            "   a = 5;"
            "   while(a){"
            "       setReg(7, a);"
            "       "
            "       setReg(4, a);"
            "       a = getReg(2);"
            "   }"
            "   if(a){"//this line will never run because a should be zero
            "       setReg(7, 1337);"//so if this line runs, there is an error in the compiler
            "   }"
            "}"
            ));
            test.setTests({5, 4, 3, 2, 1});
            test.run();
        });
        
        test.add("factorialTest", [](){
            Simulator test;
            test.loadRom(Processor::generateExecutableByteCodeFromSymString(
            "byte add(byte a, byte b){"
            "    setReg(4, a);"
            "    setReg(5, b);"
            "    setReg(6, 0);"
            "    byte c;"
            "    c = getReg(2);"
            "    return c;"
            "}"
            ""
            "byte multiply(byte a, byte b){"
            "    byte output;"
            "    output = 0;"
            "    setReg(6, 0);"
            "    while(a){"
            "        setReg(4, output);"
            "        setReg(5, b);"
            "        output = getReg(2);"
            "        "
            "        setReg(4, a);"
            "        setReg(5, 0xFFFF);"
            "        a = getReg(2);"
            "    }"
            "    return output;"
            "}"
            ""
            "byte factorial(byte a){"
            "    if(a){"
            "        byte output;"
            "        byte last;"
            "        last = add(a, 0xFFFF);"
            "        last = factorial(last);"
            "        output = multiply(a, last);"
            "        return output;"
            "    }"
            "    return 1;"
            "}"
            ""
            "void main(){"
            "    byte answer;"
            "    answer = factorial(0);"
            "    setReg(7, answer);"
            "    answer = factorial(1);"
            "    setReg(7, answer);"
            "    answer = factorial(2);"
            "    setReg(7, answer);"
            "    answer = factorial(3);"
            "    setReg(7, answer);"
            "    answer = factorial(4);"
            "    setReg(7, answer);"
            "    answer = factorial(5);"
            "    setReg(7, answer);"
            "    answer = factorial(6);"
            "    setReg(7, answer);"
            "    answer = factorial(7);"
            "    setReg(7, answer);"
            "}"
            ));
            test.setTests({1, 1, 2, 6, 24, 120, 720, 5040});
            test.run();
        });
        
        test.runTests();
    }
};
