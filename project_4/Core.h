#ifndef __CORE_H__
#define __CORE_H__

#include "Instruction_Memory.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define BOOL bool

typedef uint8_t Byte;
typedef int64_t Signal;
typedef int64_t Register;

typedef struct ControlSignals
{
    Signal Branch;
    Signal MemRead;
    Signal MemtoReg;
    Signal ALUOp;
    Signal MemWrite;
    Signal ALUSrc;
    Signal RegWrite;
}ControlSignals;

struct Core;
typedef struct Core Core;
typedef struct Core
{
    Tick clk; // Keep track of core clock
    Addr PC; // Keep track of program counter

    // What else you need? Data memory? Register file?
    Instruction_Memory *instr_mem;
   
    Byte data_mem[1024]; // data memory

    Register reg_file[32]; // register file.

    bool (*tick)(Core *core);
}Core;

struct IF_Stage;
typedef struct IF_Stage IF_Stage;
typedef struct IF_Stage
{
    Tick clk; // Keep track of core clock
    Addr PC; // Keep track of program counter

    unsigned *Instruction_Out;
    Addr *PC_Out;
    Signal PCSrc;
    Signal MUX_Input_1;

    bool (*tick)(IF_Stage *core_IF, Core *core);
}IF_Stage;

struct ID_Stage;
typedef struct ID_Stage ID_Stage;
typedef struct ID_Stage
{
    Tick clk;
    Addr PC;

    unsigned rd_index;
    unsigned rs1_index;
    unsigned rs2_index;

    unsigned Instruction_In;
    Signal RegWrite;
    Signal rd_index_In;
    Signal Mux_write_data;

    unsigned *Instruction_Out;
    Signal *rd_index_Out;
    Signal *read_data_1;
    Signal *read_data_2;
    Signal *imm_gen_64;
    Signal *RegWrite_Out;
    Signal *ALUSrc_Out;
    Signal *MemtoReg_Out;
    Signal *MemRead_Out;
    Signal *MemWrite_Out;
    Signal *Branch_Out;
    Signal *ALUOp_Out;
    Addr *PC_Out;

    ControlSignals ctrl_op_sig;

    bool (*tick)(ID_Stage *core_ID, Core *core);
}ID_Stage;

struct EX_Stage;
typedef struct EX_Stage EX_Stage;
typedef struct EX_Stage
{
    Signal ALUOp;
    unsigned Instruction_In;
    Signal ALUSrc;
    Signal read_data_1;
    Signal read_data_2;
    Signal imm_gen_64;
    Addr PC;
    Signal rd_index_In;
    Signal MemtoReg;
    Signal RegWrite;
    Signal MemRead;
    Signal MemWrite;
    Signal Branch;

    Signal *shiftleft_sum;
    Signal *ALU_result_Out;
    Signal *read_data_2_Out;
    unsigned *Instruction_Out;
    Signal *rd_index_Out;
    Signal *MemtoReg_Out;
    Signal *RegWrite_Out;
    Signal *MemRead_Out;
    Signal *MemWrite_Out;
    Signal *Branch_Out;
    Signal *zero;

    Addr *PC_Out;
    
    Tick clk;

    bool (*tick)(EX_Stage *core_EX, Core *core);
}EX_Stage;

struct MEM_Stage;
typedef struct MEM_Stage MEM_Stage;
typedef struct MEM_Stage
{
    Signal MemWrite;
    Signal ALU_result;
    Signal read_data_2;
    Signal MemRead;
    unsigned Instruction_In;
    Signal Branch;
    Signal zero;
    Signal rd_index_in;
    Signal shiftleft_sum;
    Signal MemtoReg;
    Signal RegWrite;
    Addr PC;

    Signal *ALU_result_Out;
    Signal *rd_index_Out;
    Signal *PCSrc;
    Signal *Read_data;
    Signal *MemtoReg_Out;
    Signal *RegWrite_Out;
    Addr *PC_Out;

    Tick clk;

    bool (*tick)(MEM_Stage *core_MEM, Core *core);
}MEM_Stage;

struct WB_Stage;
typedef struct WB_Stage WB_Stage;
typedef struct WB_Stage
{
    Signal MemtoReg;
    Signal ALU_result;
    Signal Read_data;
    Signal *Mux_write_data;
    Signal RegWrite;
    Signal rd_index_in;
    Tick clk;
    Addr PC;

    bool (*tick)(WB_Stage *core_WB, Core *core);
}WB_Stage;

Core *initCore(Instruction_Memory *i_mem);
bool tickFunc(Core *core);
bool tickFunc_IF(IF_Stage *core_IF, Core *core);
bool tickFunc_ID(ID_Stage *core_ID, Core *core);
bool tickFunc_EX(EX_Stage *core_EX, Core *core);
bool tickFunc_MEM(MEM_Stage *core_MEM, Core *core);
bool tickFunc_WB(WB_Stage *core_WB, Core *core);


void ControlUnit(Signal input,
                 ControlSignals *signals);

// FIXME (2). ALU Control Unit.
Signal ALUControlUnit(Signal ALUOp,
                      Signal Funct7,
                      Signal Funct3);

// FIXME (3). Imme. Generator
Signal ImmeGen(Signal input);

// FIXME (4). ALU
void ALU(Signal input_0,
         Signal input_1,
         Signal ALU_ctrl_signal,
         Signal *ALU_result,
         Signal *zero);

// (4). MUX
Signal MUX(Signal sel,
           Signal input_0,
           Signal input_1);

// (5). Add
Signal Add(Signal input_0,
           Signal input_1);

// (6). ShiftLeft1
Signal ShiftLeft1(Signal input);

#endif
