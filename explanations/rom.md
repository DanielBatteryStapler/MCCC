# ".rom" Files
The ".rom" format holds the binary machine code that the CPU hardware can directly read and decode. Because of this, the formatting for the data is very specific. 

## Instruction Layout
The binary format is made up of individually addressed "instructions". Every time the "current instruction pointer" is incremented it goes to the next instruction, and it is not possible to address the individual bytes of an instruction. Every instruction is split into 4 16-bit bytes. The first byte denotes the type of instruction and the other 3 bytes are the parameters for that instruction.

## List of Valid Instructions
The following is a list of all valid assembly instructions and their corresponding machine code formats. It's important to pay attention to the order of the arguments in the machine code because for some instructions the arguments are actually flipped. For a more detailed explanation about the behavior of every instruction, consult the assembly syntax explanation file.
```
assembly syntax       ->  machine code format
StpCpu              ->  0   0     0     0
RamReg [ram] [reg]  ->  1   [ram] [reg] 0
RegRam [reg] [ram]  ->  2   [ram] [reg] 0
SetReg [reg] [val]  ->  3   [val] [reg] 0
SetRam [ram] [val]  ->  4   [ram] [val] 0
RegReg [rgo] [rgi]  ->  5   [rgo] [rgi] 0
GotoOp [ins]        ->  6   [ins] 0     0
CnGoto [ins]        ->  7   [ins] 0     0
PrGoto              ->  8   0     0     0
PrRmRg [reg]        ->  9   [reg] 0     0
RgPrRm [reg]        ->  10  [reg] 0     0
StPrRm              ->  11  0     0     0
```

Once every instruction has been converted to a four byte sequence all of the instructions are concatenated together to produce the full program. Consequently, program length in bytes must be a multiple of 4. In order to load this sequence into [Logism Evolution](https://github.com/reds-heig/logisim-evolution) the data can be put into Logism Evolution's own ".rom" format for importing.

## ".rom" Formatting

As a preface: The ".rom" file format is not a part of the MCCC. The MCCC is just capable of exporting programs in this format and there is no guarantee that MCCC's exporting will perfectly conform to Logism Evolution's specification for the ".rom" file format.

The easiest way to get a feel for the ".rom" format is to see a sample one. This is the ".rom" output of the assembled "example.asm" program:
```
v2.0 raw
4 0 0 0 1 0 7 0 3 0 6 0 3 1 5 0
1 0 4 0 2 0 2 0 3 2 6 0 3 a 5 0
1 0 4 0 5 2 0 0 7 c 0 0 6 1 0 0
0 0 0 0 
```
The files always starts with a `v2.0 raw` header and then consists of the sequence of 16-bit bytes in hexadecimal that make up the data seperated with space and with a line break every 16 bytes.



