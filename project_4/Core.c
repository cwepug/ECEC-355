#include "Core.h"

#define OPCODE(instruction)                     \
    (instruction & 0x000000000000007F)

#define RD(instruction)                         \
    ((instruction & 0x0000000000000F80) >>  7)

#define RS1(instruction)                        \
    ((instruction & 0x00000000000F8000) >> 15)

#define RS2(instruction)                        \
    ((instruction & 0x0000000001F00000) >> 20)

#define FUNCT3(instruction)                     \
    ((instruction & 0x0000000000007000) >> 12)

#define FUNCT7(instruction)                     \
    ((instruction & 0x00000000FE000000) >> 25)

IF_Stage  *core_IF;
ID_Stage  *core_ID;
EX_Stage  *core_EX;
MEM_Stage *core_MEM;
WB_Stage  *core_WB;

// Core *initCore(Instruction_Memory *i_mem)
// {
//     Core *core = (Core *)malloc(sizeof(Core));
//     core->clk = 0;
//     core->PC = 0;
//     core->instr_mem = i_mem;
//     core->tick = tickFunc;

//     // FIXME, initialize register file here.
//     //Breaks with numbers > 1 byte
//     for(int i = 0; i < 16; i++) {
//         core->data_mem[8*i] = i;
//     }

//     // FIXME, initialize data memory here.
//     core->reg_file[5] = 0;
//     core->reg_file[6] = 0;
//     core->reg_file[7] = 4;

//     return core;
// }

// FIXME, implement this function
// bool tickFunc(Core *core)
// {
//     printf("PC: %ld\n",core->PC);
//     unsigned rd  = 0;
//     unsigned rs1 = 0;
//     unsigned rs2 = 0;

//     // Steps may include
//     // (Step 1) Reading instruction from instruction memory
//     //printf("Step 1\n");
//     unsigned instruction = core->instr_mem->instructions[core->PC / 4].instruction;
    
//     // (Step 2) Instruction decode / register file read
//     //printf("Step 2\n");
//     ControlSignals *signals = (ControlSignals *)malloc(sizeof(ControlSignals));
//     Signal opcode = (instruction & 127);
//     printf("Opcode: %ld\n",opcode);
//     ControlUnit(opcode, signals);


//     rd = (instruction & 3968) >> 7;
//     printf("rd = %d\n",rd);
//     rs1 = (instruction & 1015808) >> 15;
//     printf("rs1 = %d\n",rs1);
//     rs2 = (instruction & 32505856) >> 20;
//     printf("rs2 = %d\n",rs2);

//     Signal readReg1 = core->reg_file[rs1];
//     printf("Register %d contents: %ld\n",rs1, core->reg_file[rs1]);
//     Signal readReg2 = core->reg_file[rs2];
//     printf("Register %d contents: %ld\n",rs2, core->reg_file[rs2]);
    
//     Signal immediate = ImmeGen(instruction);
//     printf("immediate: %ld\n", immediate);

//     // (Step 3) Execute / address calculation
//     //printf("Step 3\n");
//     Signal ALU_ctrl_signal;
//     Signal Funct7 = (instruction & 4261412864) >> 25;
//     Signal Funct3 = (instruction & 28672) >> 12;
//     printf("Funct3: %ld\n",Funct3);
//     Signal ALUinput2 = MUX(signals->ALUSrc, readReg2, immediate);

//     ALU_ctrl_signal = ALUControlUnit(signals->ALUOp, Funct7, Funct3);
    
//     printf("ALU Control Signal: %ld\n",ALU_ctrl_signal);
//     Signal zero;
//     Signal ALU_result;
//     ALU(readReg1, ALUinput2, ALU_ctrl_signal, &ALU_result, &zero);
//     printf("ALU RESULT: %ld\n",ALU_result);

//     Addr nextPC;
//     if (opcode = 99 && Funct3 == 1)
//     {
//         zero = !zero; // invert for bne
//     }

//     nextPC = MUX((signals->Branch & zero), 
//             (core->PC + 4), 
//             (core->PC + ShiftLeft1(immediate)));

//     // (Step 4) Memory access
//     //printf("Step 4\n");
//     Signal readMem;
//     //Probably broken.
//     if (signals->MemWrite == 1)
//     {
//         printf("Storing %ld in memory location %ld\n",readReg2, ALU_result);
//         for(int i = 0; i < 8; i++){
//             uint64_t mask = 255 << i * 8;
//             core->data_mem[ALU_result + i] = (readReg2 & mask) >> i * 8;
        
//         }
//     }
//     if (signals->MemRead == 1)
//     {
//         readMem = core->data_mem[ALU_result];
//     }

//     // (Step 5) Write back
//     //printf("Step 5\n");
//     if (signals->RegWrite == 1)
//     {
//         if (signals->MemtoReg == 1)
//         {
//             printf("Storing %ld in register %d\n",readMem, rd);
//             core->reg_file[rd] = readMem;
//         }
//         else
//         {
//             printf("Storing %ld in register %d\n",ALU_result, rd);
//             core->reg_file[rd] = ALU_result;
//         }
//     }
    

//     // (Step N) Increment PC. FIXME, is it correct to always increment PC by 4?!
//     //GET OPCODE
//     //Compare Branch signal with zero (output from ALU)
//     core->PC = nextPC;
//     printf("Next PC: %ld\n\n", nextPC);

//     ++core->clk;
//     free(signals);
//     // Are we reaching the final instruction?
    
//     if (core->PC > core->instr_mem->last->addr)
//     {
//         return false;
//     }
//     return true;
// }
Core *initCore(Instruction_Memory *i_mem)
{
    Core *core = (Core *)malloc(sizeof(Core));
    core->clk = 0;
    core->PC  = 0;
    core->instr_mem = i_mem;
    core->tick = tickFunc;

    core_IF  = (IF_Stage  *)malloc(sizeof(IF_Stage));
    core_ID  = (ID_Stage  *)malloc(sizeof(ID_Stage));
    core_EX  = (EX_Stage  *)malloc(sizeof(EX_Stage));
    core_MEM = (MEM_Stage *)malloc(sizeof(MEM_Stage));
    core_WB  = (WB_Stage  *)malloc(sizeof(WB_Stage));

	/* ==================================================================== */
    /* FETCH STAGE */
    core_IF->clk = 0;
    core_IF->PC  = 0;
    core_IF->tick = tickFunc_IF;

    /* FETCH Outputs */
    core_IF->Instruction_Out = &(core_ID->Instruction_In);
    core_IF->PC_Out = &(core_ID->PC);


	/* ==================================================================== */
    /* DECODE STAGE */
    core_ID->clk = 0;
    core_ID->PC  = 0;
    core_ID->tick = tickFunc_ID;

    /* DECODE Outputs */
    core_ID->imm_gen_64      = &(core_EX->imm_gen_64);
    core_ID->read_data_1     = &(core_EX->read_data_1);
    core_ID->read_data_2     = &(core_EX->read_data_2);
    core_ID->Instruction_Out = &(core_EX->Instruction_In);
    core_ID->rd_index_Out    = &(core_EX->rd_index_In);
    core_ID->PC_Out          = &(core_EX->PC);
    
		/* control signals */
    core_ID->ALUSrc_Out      = &(core_EX->ALUSrc);
    core_ID->MemtoReg_Out    = &(core_EX->MemtoReg);
    core_ID->RegWrite_Out    = &(core_EX->RegWrite);
    core_ID->MemRead_Out     = &(core_EX->MemRead);
    core_ID->MemWrite_Out    = &(core_EX->MemWrite);
    core_ID->Branch_Out      = &(core_EX->Branch);
    core_ID->ALUOp_Out       = &(core_EX->ALUOp);


	/* ==================================================================== */
    /* EXECUTION STAGE */
    core_EX->clk = 0;
    core_EX->PC  = 0;
    core_EX->tick = tickFunc_EX;

    /* EXECUTION Outputs */
    core_EX->ALU_result_Out  = &(core_MEM->ALU_result);
    core_EX->read_data_2_Out = &(core_MEM->read_data_2);
    core_EX->shiftleft_sum   = &(core_MEM->shiftleft_sum);
    core_EX->zero            = &(core_MEM->zero);
    core_EX->Instruction_Out = &(core_MEM->Instruction_In);
    core_EX->rd_index_Out    = &(core_MEM->rd_index_in);
    core_EX->PC_Out          = &(core_MEM->PC);

		/* control signals */
    core_EX->MemtoReg_Out    = &(core_MEM->MemtoReg);
    core_EX->RegWrite_Out    = &(core_MEM->RegWrite);
    core_EX->MemRead_Out     = &(core_MEM->MemRead);
    core_EX->MemWrite_Out    = &(core_MEM->MemWrite);
    core_EX->Branch_Out      = &(core_MEM->Branch);


	/* ==================================================================== */
    /* MEMORY STAGE */
    core_MEM->clk = 0;
    core_MEM->PC  = 0;
    core_MEM->tick = tickFunc_MEM;

    /* MEMORY Outputs */
    core_MEM->Read_data      = &(core_WB->Read_data);
    core_MEM->ALU_result_Out = &(core_WB->ALU_result);
    core_MEM->rd_index_Out   = &(core_WB->rd_index_in);
    core_MEM->PC_Out         = &(core_WB->PC);
    core_MEM->PCSrc          = &(core_IF->PCSrc);

    /* ^^^ Is this skipping a stage? 
    Maybe not because its a branch and its supposed to edit PC. */

		/* control signals */
    core_MEM->MemtoReg_Out   = &(core_WB->MemtoReg);    //
    core_MEM->RegWrite_Out   = &(core_WB->RegWrite);    //


	/* ==================================================================== */
    /* WRITE BACK STAGE */
    core_WB->clk = 0;
    core_WB->PC  = 0;
    core_WB->tick = tickFunc_WB;

    /* WRITE BACK Outputs */
    core_WB->Mux_write_data = &(core_ID->Mux_write_data);

	/* ==================================================================== */
    int i;

    // Initialize register file here.
	for(i = 0; i < 32; i++)
		core->reg_file[i] = 0;

	core->reg_file[1] =   0;  /* x1 =   0 */
	core->reg_file[2] =  10;  /* x2 =  10 */
    core->reg_file[3] = -15;  /* x3 = -15 */
    core->reg_file[4] =  20;  /* x4 =  20 */
    core->reg_file[5] =  30;  /* x5 =  30 */
    core->reg_file[6] = -35;  /* x6 = -35 */

    // Initialize data memory here.
    int64_t val1 = -63;
    int64_t val2 =  63;

    /* starting at 40th & 48th position in memory, store val1 & 2 (little-endian) */
    for(i = 0; i <= 7; i++)
    {
        core->data_mem[40+i] = (Byte) ( ((val1 & (0xFF << 8*i)) >> 8*i) );
    }

    for(i = 0; i <= 7; i++)
    {
        core->data_mem[48+i] = (Byte) ( ((val2 & (0xFF << 8*i)) >> 8*i) );
    }

    return core;
}

bool tickFunc(Core *core)
{
    bool IF, ID, EX, MEM, WB;

    WB  = tickFunc_WB (core_WB , core);
    MEM = tickFunc_MEM(core_MEM, core);
    EX  = tickFunc_EX (core_EX , core);
    ID  = tickFunc_ID (core_ID , core);
    IF  = tickFunc_IF (core_IF , core);

    ++core->clk;
    
    /* If any stage has more instructions */
    if( IF || ID || EX || MEM || WB ) {
        return true;
    }
    free(core_IF);
    free(core_ID);
    free(core_EX);
    free(core_MEM);
    free(core_WB);
    return false;
}

bool tickFunc_IF(IF_Stage *core_IF, Core *core)
{
    *(core_IF->Instruction_Out) = core->instr_mem->instructions[core_IF->PC / 4].instruction;

    *(core_IF->PC_Out) = core_IF->PC;
    core_IF->PC = MUX(core_IF->PCSrc, Add(core_IF->PC, 4), core_IF->MUX_Input_1);


    ++core_IF->clk;

    if(core_IF->PC > core->instr_mem->last->addr)
    {
        return false;
    }
    return true;
}

bool tickFunc_ID(ID_Stage *core_ID, Core *core)
{
    core_ID->rd_index = (core_ID->Instruction_In & 3968) >> 7;
    core_ID->rs1_index = (core_ID->Instruction_In & 1015808) >> 15;
    core_ID->rs2_index = (core_ID->Instruction_In & 32505856) >> 20;
    ControlUnit(OPCODE(core_ID->Instruction_In), &(core_ID->ctrl_op_sig));

    /* OUTPUTS */
    *(core_ID->read_data_1) 	= core->reg_file[core_ID->rs1_index];
    *(core_ID->read_data_2) 	= core->reg_file[core_ID->rs2_index];

    *(core_ID->imm_gen_64)      = ImmeGen((Signal) core_ID->Instruction_In);
    
    *(core_ID->Instruction_Out) = core_ID->Instruction_In;
    *(core_ID->RegWrite_Out)    = core_ID->RegWrite;
    *(core_ID->rd_index_Out)    = core_ID->rd_index;

    *(core_ID->ALUSrc_Out)   	= (core_ID->ctrl_op_sig).ALUSrc;
    *(core_ID->MemtoReg_Out)	= (core_ID->ctrl_op_sig).MemtoReg;
    *(core_ID->RegWrite_Out) 	= (core_ID->ctrl_op_sig).RegWrite;
    *(core_ID->MemRead_Out)  	= (core_ID->ctrl_op_sig).MemRead;
    *(core_ID->MemWrite_Out) 	= (core_ID->ctrl_op_sig).MemWrite;
    *(core_ID->Branch_Out)   	= (core_ID->ctrl_op_sig).Branch;
    *(core_ID->ALUOp_Out)    	= (core_ID->ctrl_op_sig).ALUOp;


    *(core_ID->PC_Out) = core_ID->PC;

    ++core_ID->clk;
    // Are we reaching the final instruction?
    if (core_ID->PC > core->instr_mem->last->addr)
    {
        return false;
    }
    return true;
}

bool tickFunc_EX(EX_Stage *core_EX, Core *core)
{
    Signal ALU_ctrl, ALU_ip_1;
    Signal ALU_result;

    ALU_ctrl = ALUControlUnit(core_EX->ALUOp, FUNCT7(core_EX->Instruction_In),
                                              FUNCT3(core_EX->Instruction_In));

    /* MUX BEFORE ALU */
    ALU_ip_1 = MUX(core_EX->ALUSrc, core_EX->read_data_2, core_EX->imm_gen_64);

    /* ALU */
    ALU(core_EX->read_data_1, ALU_ip_1, ALU_ctrl, &ALU_result, (core_EX->zero));


	/* OUTPUTS */
	*(core_EX->shiftleft_sum)   = Add(core_EX->PC, ShiftLeft1(core_EX->imm_gen_64));
	
    *(core_EX->ALU_result_Out)  = ALU_result;
    *(core_EX->read_data_2_Out) = core_EX->read_data_2;
    *(core_EX->Instruction_Out) = core_EX->Instruction_In;
    *(core_EX->rd_index_Out)    = core_EX->rd_index_In;

    *(core_EX->MemtoReg_Out) 	= core_EX->MemtoReg;
    *(core_EX->RegWrite_Out) 	= core_EX->RegWrite;
    *(core_EX->MemRead_Out)  	= core_EX->MemRead;
    *(core_EX->MemWrite_Out) 	= core_EX->MemWrite;
    *(core_EX->Branch_Out)   	= core_EX->Branch;


    *(core_EX->PC_Out) = core_EX->PC;

    ++core_EX->clk;
	// Are we reaching the final instruction?
    if (core_EX->PC > core->instr_mem->last->addr)
    {
        return false;
    }
    return true;
}

bool tickFunc_MEM(MEM_Stage *core_MEM, Core *core)
{
    int i;

    Signal data_mem_op = 0;
    Signal AND_branch_zero;

    /* DATA MEMORY COMPONENT */
    if(core_MEM->MemWrite == 1) {
        for(i = 0; i <= 7; i++)
        {
            core->data_mem[core_MEM->ALU_result+i] =
                            (Byte) ( (core_MEM->read_data_2 & (0xFF << 8*i)) >> 8*i);
        }
    }

    if(core_MEM->MemRead == 1) {
        data_mem_op = 0;
        for(i = 0; i <= 7; i++)
        {
            data_mem_op |= (core->data_mem[core_MEM->ALU_result+i] << 8*i);
        }
    }

    /* If instruction is BNE or BLT - negate ALU_op_zero for AND (branch NOT EQUAL) */
    if ( (OPCODE(core_MEM->Instruction_In) == 99) && (FUNCT3(core_MEM->Instruction_In)))
        AND_branch_zero = core_MEM->Branch && !(core_MEM->zero);
    else
        AND_branch_zero = core_MEM->Branch && core_MEM->zero;
        
        
	/* OUTPUTS */
    *(core_MEM->ALU_result_Out) = core_MEM->ALU_result;
    *(core_MEM->rd_index_Out)   = core_MEM->rd_index_in;
    *(core_MEM->PCSrc)          = AND_branch_zero;
    *(core_MEM->Read_data)      = data_mem_op;

    *(core_MEM->MemtoReg_Out)   = core_MEM->MemtoReg;
    *(core_MEM->RegWrite_Out)   = core_MEM->RegWrite;

	*(core_MEM->PC_Out)         = core_MEM->PC;

    ++core_MEM->clk;
	// Are we reaching the final instruction?
    if (core_MEM->PC > core->instr_mem->last->addr)
    {
        return false;
    }
    return true;
}

bool tickFunc_WB(WB_Stage *core_WB, Core *core)
{
    Signal write_MUX;

    write_MUX = MUX(core_WB->MemtoReg, core_WB->ALU_result, core_WB->Read_data);

    /* Write to write reg */
    if (core_WB->RegWrite == 1)
	    core->reg_file[core_WB->rd_index_in] = write_MUX;
	    
	
    ++core_WB->clk;
	// Are we reaching the final instruction?
    if (core_WB->PC > core->instr_mem->last->addr)
    {
        return false;
    }
    return true;
}


// FIXME (1). Control Unit. Refer to Figure 4.18.
void ControlUnit(Signal input,
                 ControlSignals *signals)
{
    switch(input) {
        case 51: // add, sll
            signals->ALUSrc = 0;
            signals->MemtoReg = 0;
            signals->RegWrite = 1;
            signals->MemRead = 0;
            signals->MemWrite = 0;
            signals->Branch = 0;
            signals->ALUOp = 2;
            break;
        case 3: //  ld
            signals->ALUSrc = 1;
            signals->MemtoReg = 1;
            signals->RegWrite = 1;
            signals->MemRead = 1;
            signals->MemWrite = 0;
            signals->Branch = 0;
            signals->ALUOp = 0;
            break;
        case 19: // slli / addi
            signals->ALUSrc = 1;
            signals->MemtoReg = 0;
            signals->RegWrite = 1;
            signals->MemRead =  0;
            signals->MemWrite = 0;
            signals->Branch = 0;
            signals->ALUOp = 2;
            break;
        case 99: // bne beq blt
            signals->ALUSrc = 0;
            signals->MemtoReg = 0; // don't care
            signals->RegWrite = 0;
            signals->MemRead =  0;
            signals->MemWrite = 0;
            signals->Branch = 1;
            signals->ALUOp = 1;
            break;
        case 35: // sd
            signals->ALUSrc = 1;
            signals->MemtoReg = 0; // don't care
            signals->RegWrite = 0;
            signals->MemRead = 0;
            signals->MemWrite = 1;
            signals->Branch = 0;
            signals->ALUOp = 0;
            break;
    }
}

// FIXME (2). ALU Control Unit. Refer to Figure 4.12.
Signal ALUControlUnit(Signal ALUOp,
                      Signal Funct7,
                      Signal Funct3)
{
    // ld & For sd
    if (ALUOp == 0)
    {
        return 2;
    }
    // beq, bne or blt
    else if (ALUOp == 1)
    {
        return 6;
    }
    // add and addi
    else if (ALUOp == 2 && Funct7 == 0 && Funct3 == 0)
    {
        return 2;
    }
    // slli or sll
    else if (ALUOp == 2 && Funct7 == 0 && Funct3 == 1)
    {
        return 13;
    }
    // sub
    else if (ALUOp == 2 && Funct7 == 32 && Funct3 == 0)
    {
        return 6;
    }
    // and
    else if (ALUOp == 2 && Funct7 == 0 && Funct3 == 7)
    {
        return 0;
    }
    // or
    else if (ALUOp == 2 && Funct7 == 0 && Funct3 == 6)
    {
        return 1;
    }
}

// FIXME (3). Imme. Generator
// Signal ImmeGen(Signal input)
// {
//     int16_t immediate = 0;
//     Signal opcode = (input & 127);

//     if (opcode == 3 || opcode == 19) 
//     {
//         immediate = (int16_t)((input & 4293918720) >> 20);
//         unsigned signbit = (immediate & 2048) >> 11;
//         immediate |= signbit << 12;
//         immediate |= signbit << 13;
//         immediate |= signbit << 14;
//         immediate |= signbit << 15;
//     }
//     else if (opcode == 99)
//     {
//         unsigned oneToFour = (input & 3840) >> 8;
//         unsigned fiveToTen = (input & 2113929216) >> 25;
//         unsigned eleven = (input & 128) >> 7;
//         unsigned twelve = (input & 2147483648) >> 31;

//         immediate |= oneToFour << 1;
//         immediate |= fiveToTen << 5;
//         immediate |= eleven << 10;
//         immediate |= twelve << 11;
//         immediate |= twelve << 12;
//         immediate |= twelve << 13;
//         immediate |= twelve << 14;
//         immediate |= twelve << 15;

//     }
//     else if (opcode == 35) 
//     {
//         unsigned zeroToFour = (input & 3968) >> 7;
//         unsigned fiveToEleven = (input & 4261412864) >> 25;

//         immediate |= zeroToFour;
//         immediate |= fiveToEleven << 5;     

//     }
//     return (Signal)immediate; //This is kind of evil
// }

Signal ImmeGen(Signal input)
{
    Signal imm = 0;
    /* opcode bit 6 is 1 for conditional branch, and 0 for data transfer */
    if((input & 0x0000000000000040)) {
        /* conditional branches (SB-type) */
        imm = ((input & 0x0000000000000F00) >> 7) | ((input & 0x000000007E000000) >> 20) |
              ((input & 0x0000000000000080) << 4) | ((input & 0x0000000080000000) >> 19);
    }
    else {
        /* data transfer */

        /* opcode bit 5 is 1 for store, and 0 for load */
        if((input & 0x0000000000000020)) {
            /* store (S-type) */
            imm = ((input & 0x00000000FE000000) >> 20) | ((input & 0x0000000000000F80) >> 7);
        }
        else {
            /* load (I-type) */
            imm = ((input & 0x00000000FFF00000) >> 20);
        }
    }
    /* IF MSB OF IMMEDIATE IS 1 (NEGATIVE) - THEN MAKE ALL HIGHER ORDER BITS 1 */
    if ((input & 0x0000000080000000)) {
        imm |= 0xFFFFFFFFFFFFF000;
    }
    return imm;
}

// FIXME (4). ALU
void ALU(Signal input_0,
         Signal input_1,
         Signal ALU_ctrl_signal,
         Signal *ALU_result,
         Signal *zero)
{
    // addition, ld, sd, => ADD
    if (ALU_ctrl_signal == 2)
    {
        *ALU_result = (input_0 + input_1);
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }
    // subtraction, beq, bne and blt => SUBTRACT
    if (ALU_ctrl_signal ==  6) {
        *ALU_result = (input_0 - input_1);
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }
    // and
    if (ALU_ctrl_signal == 0) {
        *ALU_result = (input_0 & input_1);
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }
    // or
    if (ALU_ctrl_signal == 1) {
        *ALU_result = (input_0 | input_1);
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }
    // slli and sll
    if (ALU_ctrl_signal == 13) {
        *ALU_result = (input_0 << input_1);
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }
}

// (4). MUX
Signal MUX(Signal sel,
           Signal input_0,
           Signal input_1)
{
    //printf("MUX\n");
    if (sel == 0) { return input_0; } else { return input_1; }
}

// (5). Add
Signal Add(Signal input_0,
           Signal input_1)
{
    return (input_0 + input_1);
}

// (6). ShiftLeft1
Signal ShiftLeft1(Signal input)
{
    //printf("Left shift 1: %ld\n", (input << 1));
    return input << 1;
}
