#include "Parser.h"

void loadInstructions(Instruction_Memory *i_mem, const char *trace)
{
    printf("Loading trace file: %s\n", trace);

    FILE *fd = fopen(trace, "r");
    if (fd == NULL)
    {
        perror("Cannot open trace file. \n");
        exit(EXIT_FAILURE);
    }

    // Iterate all the assembly instructions
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    Addr PC = 0; // program counter points to the zeroth location initially.
    int IMEM_index = 0;
    while ((read = getline(&line, &len, fd)) != -1)
    {
        // Assign program counter
        i_mem->instructions[IMEM_index].addr = PC;

        // Extract operation
        char *raw_instr = strtok(line, " ");
        if (strcmp(raw_instr, "add") == 0 ||
            strcmp(raw_instr, "sub") == 0 ||
            strcmp(raw_instr, "sll") == 0 ||//MAKE THIS (SHOULD BE DONE)
            strcmp(raw_instr, "srl") == 0 ||
            strcmp(raw_instr, "xor") == 0 ||
            strcmp(raw_instr, "or")  == 0 ||
            strcmp(raw_instr, "and") == 0)
        {
            parseRType(raw_instr, &(i_mem->instructions[IMEM_index]));
            i_mem->last = &(i_mem->instructions[IMEM_index]);
	    } 
        else if (strcmp(raw_instr, "ld")   == 0 ||
                 strcmp(raw_instr, "addi") == 0 ||
                 strcmp(raw_instr, "slli") == 0)
        {
            parseIType(raw_instr, &(i_mem->instructions[IMEM_index]));
            i_mem->last = &(i_mem->instructions[IMEM_index]);
        }
        else if (strcmp(raw_instr, "bne")  == 0 ||
                 strcmp(raw_instr, "beq")  == 0)//MAKE THIS (SHOULD BE DONE)
        {
            parseSBType(raw_instr, &(i_mem->instructions[IMEM_index]));
            i_mem->last = &(i_mem->instructions[IMEM_index]);
        }
        else if (strcmp(raw_instr, "sd") == 0)//MAKE THIS (SHOULD BE DONE)
        {
            parseSType(raw_instr, &(i_mem->instructions[IMEM_index]));
            i_mem->last = &(i_mem->instructions[IMEM_index]);
        }
        IMEM_index++;
        PC += 4;
    }

    fclose(fd);
}

void parseRType(char *opr, Instruction *instr)
{
    instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;
    unsigned funct7 = 0;

    if (strcmp(opr, "add") == 0)
    {
        opcode = 51;
        funct3 = 0;
        funct7 = 0;
    }
    if (strcmp(opr, "sll") == 0)
    {
        opcode = 51;
        funct3 = 1;
        funct7 = 0;
    }

    char *reg = strtok(NULL, ", ");
    unsigned rd = regIndex(reg);

    reg = strtok(NULL, ", ");
    unsigned rs_1 = regIndex(reg);

    reg = strtok(NULL, ", ");
    reg[strlen(reg)-1] = '\0';
    unsigned rs_2 = regIndex(reg);

    // Contruct instruction
    instr->instruction |= opcode;
    instr->instruction |= (rd << 7);
    instr->instruction |= (funct3 << (7 + 5));
    instr->instruction |= (rs_1 << (7 + 5 + 3));
    instr->instruction |= (rs_2 << (7 + 5 + 3 + 5));
    instr->instruction |= (funct7 << (7 + 5 + 3 + 5 + 5));
}

void parseIType(char *opr, Instruction *instr)
{
    char *reg;
    instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;
    unsigned rd = 0;
    unsigned rs_1 = 0;
    int imm = 0;

    if (strcmp(opr, "ld") == 0)
    {
        opcode = 3;
        funct3 = 3;

        reg = strtok(NULL, ", ");
        rd = regIndex(reg);

        reg = strtok(NULL, "(");
        imm = atoi(reg);
        
        reg = strtok(NULL, ")");
        //reg[strlen(reg)-1] = '\0';
        rs_1 = regIndex(reg);
    } 
    else
    {
        if (strcmp(opr, "addi") == 0)
        {
            opcode = 19;
            funct3 = 0;
        }
        else if (strcmp(opr, "slli") == 0)
        {
            opcode = 19;
            funct3 = 1;
        }

        reg = strtok(NULL, ", ");
        rd = regIndex(reg);

        reg = strtok(NULL, ", ");
        rs_1 = regIndex(reg);

        reg = strtok(NULL, ", ");
        imm = atoi(reg);
    }

    // Contruct instruction
    instr->instruction |= opcode;
    instr->instruction |= (rd << 7);
    instr->instruction |= (funct3 << (7 + 5));
    instr->instruction |= (rs_1 << (7 + 5 + 3));
    instr->instruction |= (imm << (7 + 5 + 3 + 5));
}

void parseSType(char *opr, Instruction *instr)//NEW
{
    char *reg;
    instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;
    unsigned rs_2 = 0;
    unsigned rs_1 = 0;
    int imm = 0;

    if (strcmp(opr, "sd") == 0)
    {
        opcode = 35;
        funct3 = 3;

        reg = strtok(NULL, ", ");//source
        rs_2 = regIndex(reg);// Value to write (book page 142)

        reg = strtok(NULL, "(");
        imm = atoi(reg);// Immediate to offset write address 
        
        reg = strtok(NULL, ")");//base
        rs_1 = regIndex(reg);// Register containing base pointer to array/address
    } 

    // Contruct instruction
    unsigned imm_zeroToFour = (imm & 31);//first five bits if present 
    unsigned imm_fiveToEleven = (imm & 4064) >> 5;//what it says 

    instr->instruction |= opcode;
    instr->instruction |= imm_zeroToFour << 7;
    instr->instruction |= funct3 << 12;
    instr->instruction |= rs_1 << 15;//base
    instr->instruction |= rs_2 << 20;//source
    instr->instruction |= imm_fiveToEleven << 25;
}

void parseSBType(char *opr, Instruction *instr)
{
    char *reg;
    instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;
    unsigned rs_1 = 0;
    unsigned rs_2 = 0;
    int imm = 0;

    if (strcmp(opr, "bne") == 0)
    {
        opcode = 99;
        funct3 = 1;
    }
    if (strcmp(opr, "beq") == 0)
    {
        opcode = 99;
        funct3 = 0;
    }

    reg = strtok(NULL, ", ");
    rs_1 = regIndex(reg);

    reg = strtok(NULL, ", ");
    rs_2 = regIndex(reg);

    reg = strtok(NULL, ", ");
    imm = atoi(reg);

    //Construction
    unsigned imm_oneToFour = (imm & 30) >> 1;
    unsigned imm_eleven = (imm & 2048) >> 11;
    unsigned imm_fiveToTen = (imm & 2016) >> 5;
    unsigned imm_twelve = (imm & 4096) >> 12;

    instr->instruction |= opcode;
    instr->instruction |= imm_eleven << 7;
    instr->instruction |= imm_oneToFour << 8;
    instr->instruction |= funct3 << 12;
    instr->instruction |= rs_1 << 15;
    instr->instruction |= rs_2 << 20;
    instr->instruction |= imm_fiveToTen << 25;
    instr->instruction |= imm_twelve << 31;
    
}

int regIndex(char *reg)
{
    unsigned i = 0;
    for (i; i < NUM_OF_REGS; i++)
    {
        if (strcmp(REGISTER_NAME[i], reg) == 0)
        {
            break;
        }
    }

    return i;
}
