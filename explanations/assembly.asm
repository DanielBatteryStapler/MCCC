//Comments can be made just like they can be in the ".sym" files
//
//There are three different things that make up assembly:
//A Statement:
//    SetReg 0 5
//
//A Goto Destination:
//    subroutineA:
//
//or a Context Marker:
//    ["doThing();" "file.sym":5]
//
//Context markers are used by the debugger and compiler and do not have a stable syntax, so
//it's not the best idea to write them by hand.
//
//All white space is ignored in .asm files, besides for delimiting, and tabs are just for readability and have no
//special meaning to the assembler
//
//Whenever a statements takes in a value, denoted below using [this] syntax, it can be a few 
//different things. Using GotoOp as an example:
//    Goto 100          //a decimal constant
//    Goto 0x0b         //a hexadecimal constant
//    Goto 0b0110       //a binary constant
//    Goto #subroutineA //the instruction number of a Goto Destination
//                      //note that using a goto destination as a value will always use it
//                      //as the value of it's corresponding instruction number, this is even
//                      //valid in statements besides Goto
//
//
//Also, just some short hand:
//    IR#n = Input Register #n
//    OR#n = Output Register #n
//
//List of all valid Statements:
//    StpCpu                //shuts down the cpu and halts execution
//    RamReg [ram] [reg]    //sets IN#[reg] to the value at ram address [ram]
//    RegRam [reg] [ram]    //sets the value at ram address [ram] to the value of OR#[reg]
//    SetReg [reg] [val]    //sets IR#[reg] to [val]
//    SetRam [ram] [val]    //sets the value at ram address [ram] to [val]
//    RegReg [rgo] [rgi]    //sets IR#[rgi] to the value of OR#[rgo]
//    GotoOp [ins]          //goto instruction number [ins]
//    CnGoto [ins]          //goto instruction number [ins] if the value of IR#0 is not zero, else NOOP
//    PrGoto                //goto instruction number (value of IR#0)
//    PrRmRg [reg]          //sets IR#[reg] to the value at ram address (IR#0 + IR#1)
//    RgPrRm [reg]          //sets the value at ram address (IR#0 + IR#1) to the value of OR#[reg]
//    StPrRm                //sets the value at ram address (IR#0 + IR#1) to the value of IR#2
//
//List of all valid Input Registers:
//    00: Operation Input #0         -All of the Operation Input registers go into the Instruction Decoder
//    01: Operation Input #1         -and their behavior is defined above in the list of valid statements.
//    02: Operation Input #2         -
//    03: Stack Frame Pointer        -Store the RAM address of the current Stack Frame
//    04: ALU Input #0               -
//    05: ALU Input #1               -
//    06: ALU Mode Select            -Valid Modes: 0=Add,1=Greater Than,2=Equal To,3=Less Than 
//    07: Output Monitor             -Allows for an easy way to display data without having to
//                                   -try to look for the value in RAM
//    
//List of all valid Output Registers:
//    00: Stack Frame Pointer        -Always the current value of IR#4 
//    01: Abs. Addr. of Sel. Addr.   -Always equal to (IR#0 + IR#1)
//    02: ALU Output                 -The result of the specified ALU operation


//Here's a simple example program just to show things working in practice:
//This program is going to print all numbers 0 to 9 and then stop

SetRam 0 0      //set ram address 0 to 0, this will be the iterator

loop:
    RamReg 0 7  //print the iterator to the output monitor
    
    SetReg 6 0  //put the ALU into addition mode
    SetReg 5 1  //we're adding one to the iterator so put that there too
    RamReg 0 4  //now put in the iterator itself
    RegRam 2 0  //safe this value to ram, so now we have successfully incremented the iterator
    
    //now we need to check to see if it's equal to 10
    SetReg 6 2  //put the ALU into equal to mode
    SetReg 5 10 //we're comparing it to 10 so we need that in the ALU too
    RamReg 0 4  //we already put the iterator into the ALU,
                //but that was the old iterator and it's behind the actual one in ram
    RegReg 2 0  //the ALU should be outputting the result from the comparison so that need to go into IR#0
    CnGoto #endloop //this goto will only happen if the ALU said that the iterator was equal to 10
GotoOp #loop    //now repeat that whole thing again

endloop:
StpCpu          //if the program has gotten here then it should stop running

//To run this program first use the "assemble" action in MCCC:
//    ./MCCC assemble example.rom example.asm
//And then the resulting rom can be loaded into Logism Evolution and be ran directly there.
//To run this program directly into the debugger using the "debug" action:
//    ./MCCC debug example.asm





