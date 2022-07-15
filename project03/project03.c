#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "project03.h"

int data_hazard_count = 0;
int control_hazard_count = 0;
int execution_cycles = 0;
int completed_instructions = 0;
int count = 0;

static int last_stage = RESUME_EXECUTION;
static int last_decode_stage = RESUME_EXECUTION;
static int halting = FALSE;

const char *StageName[NUM_OF_STAGES] = {
    "IF",
    "ID",
    "IA",
    "EX1",
    "BR",
    "EX2",
    "MEM1",
    "MEM2",
    "WB"};

/*
 * Fetch Stage of Pipeline
 */
static void fetch(SIMULATOR_STRUCT *sim)
{
    INSTRUCTION *curr_inst;
    if (sim->stall)
    {
        last_stage = sim->stall;
    }
    else if (last_stage == STALL)
    {
        last_stage = RESUME_EXECUTION;
    }
    else
    {
        if (sim->next_cycle == TRUE)
        {
            sim->next_cycle = FALSE;
            return;
        }

        sim->fetch.pc = sim->pc;
        curr_inst = &sim->mem[sim->pc];
        strcpy(sim->fetch.opcode_str, curr_inst->opcode_str);
        sim->fetch.opcode = curr_inst->opcode;
        sim->fetch.rx = curr_inst->rx;
        sim->fetch.ry = curr_inst->ry;
        sim->fetch.rz = curr_inst->rz;
        sim->fetch.imm = curr_inst->imm;
        sim->pc++;

        sim->decode = sim->fetch;

        if (sim->fetch.opcode == OPCODE_RET)
        {
            sim->fetch.has_next = FALSE;
        }
    }
}

/*
 * Decode Stage of Pipeline
 */
static void decode(SIMULATOR_STRUCT *sim)
{
    if (sim->decode.has_next)
    {
        if (sim->stall == STALL)
        {
            last_decode_stage = STALL;
        }
        else if (last_decode_stage == STALL)
        {
            last_decode_stage = RESUME_EXECUTION;
        }
        else
        {
            sim->inst_depn_analyze = sim->decode;
            sim->decode.has_next = FALSE;
        }
    }
}

/*
 * Execute Stage of Pipeline
 */
static void inst_depn_analyze(SIMULATOR_STRUCT *sim)
{
    if (sim->inst_depn_analyze.has_next)
    {
        switch (sim->inst_depn_analyze.opcode)
        {
        case OPCODE_ADD1:
        case OPCODE_SUB1:
        case OPCODE_MUL1:
        case OPCODE_DIV1:
        {
            if (sim->reg_status[sim->inst_depn_analyze.rx] == 0 || sim->reg_status[sim->inst_depn_analyze.ry] == 0 || sim->reg_status[sim->inst_depn_analyze.rz] == 0)
            {
                data_hazard_count++;
                sim->stall = STALL;
            }
            else
            {
                sim->inst_depn_analyze.ry_value = sim->regs[sim->inst_depn_analyze.ry];
                sim->inst_depn_analyze.rz_value = sim->regs[sim->inst_depn_analyze.rz];
                sim->reg_status[sim->inst_depn_analyze.rx] = 0;

                if (sim->stall == STALL)
                {
                    count++;
                    sim->stall = RESUME_EXECUTION;
                }
            }
            break;
        }

        case OPCODE_ADD2:
        case OPCODE_SUB2:
        case OPCODE_MUL2:
        case OPCODE_DIV2:
        case OPCODE_LOAD2:
        {
            if (sim->reg_status[sim->inst_depn_analyze.rx] == 0 || sim->reg_status[sim->inst_depn_analyze.ry] == 0)
            {
                data_hazard_count++;
                sim->stall = STALL;
            }
            else
            {
                sim->inst_depn_analyze.ry_value = sim->regs[sim->inst_depn_analyze.ry];
                sim->reg_status[sim->inst_depn_analyze.rx] = 0;

                if (sim->stall == STALL)
                {
                    count++;
                    sim->stall = RESUME_EXECUTION;
                }
            }
            break;
        }

        case OPCODE_SET:
        case OPCODE_LOAD1:
        {
            if (sim->reg_status[sim->inst_depn_analyze.rx] == 0)
            {
                data_hazard_count++;
                sim->stall = STALL;
            }
            else
            {
                sim->reg_status[sim->inst_depn_analyze.rx] = 0;

                if (sim->stall == STALL)
                {
                    count++;
                    sim->stall = RESUME_EXECUTION;
                }
            }
            break;
        }

        case OPCODE_BZ:
        case OPCODE_BGZ:
        case OPCODE_BLZ:
        case OPCODE_BGTZ:
        case OPCODE_BLTZ:
        case OPCODE_STORE1:
        {
            if (sim->reg_status[sim->inst_depn_analyze.rx] == 0)
            {
                data_hazard_count++;
                sim->stall = STALL;
            }
            else
            {
                sim->inst_depn_analyze.rx_value = sim->regs[sim->inst_depn_analyze.rx];

                if (sim->stall == STALL)
                {
                    count++;
                    sim->stall = RESUME_EXECUTION;
                }
            }
            break;
        }

        case OPCODE_STORE2:
        {
            if (sim->reg_status[sim->inst_depn_analyze.rx] == 0 || sim->reg_status[sim->inst_depn_analyze.ry] == 0)
            {
                data_hazard_count++;
                sim->stall = STALL;
            }
            else
            {
                sim->inst_depn_analyze.rx_value = sim->regs[sim->inst_depn_analyze.rx];
                sim->inst_depn_analyze.ry_value = sim->regs[sim->inst_depn_analyze.ry];

                if (sim->stall == STALL)
                {
                    count++;
                    sim->stall = RESUME_EXECUTION;
                }
            }
            break;
        }
        }

        if (!sim->stall)
        {
            sim->ex1 = sim->inst_depn_analyze;
            sim->inst_depn_analyze.has_next = FALSE;
        }
    }
}

/*
 * Execute Stage of Pipeline
 */
static void ex1(SIMULATOR_STRUCT *sim)
{
    if (sim->ex1.has_next)
    {
        switch (sim->ex1.opcode)
        {
        case OPCODE_ADD1:
        {
            sim->ex1.result_buffer = sim->ex1.ry_value + sim->ex1.rz_value;
            break;
        }

        case OPCODE_ADD2:
        {
            sim->ex1.result_buffer = sim->ex1.ry_value + sim->ex1.imm;
            break;
        }

        case OPCODE_SUB1:
        {
            sim->ex1.result_buffer = sim->ex1.ry_value - sim->ex1.rz_value;
            break;
        }

        case OPCODE_SUB2:
        {
            sim->ex1.result_buffer = sim->ex1.ry_value - sim->ex1.imm;
            break;
        }

        case OPCODE_LOAD1:
        case OPCODE_STORE1:
        {
            sim->ex1.mem_addr = sim->ex1.imm;
            break;
        }

        case OPCODE_LOAD2:
        case OPCODE_STORE2:
        {
            sim->ex1.mem_addr = sim->ex1.ry_value;
            break;
        }

        case OPCODE_BZ:
        case OPCODE_BGZ:
        case OPCODE_BGTZ:
        case OPCODE_BLZ:
        case OPCODE_BLTZ:
        {
            sim->ex1.tar_addr = sim->ex1.imm / 4;
            break;
        }
        }

        sim->branch = sim->ex1;
        sim->ex1.has_next = FALSE;
    }
}

/*
 * Execute Stage of Pipeline
 */
static void ex2(SIMULATOR_STRUCT *sim)
{
    if (sim->ex2.has_next)
    {
        switch (sim->ex2.opcode)
        {
        case OPCODE_MUL1:
        {
            sim->ex2.result_buffer = sim->ex2.ry_value * sim->ex2.rz_value;
            break;
        }

        case OPCODE_MUL2:
        {
            sim->ex2.result_buffer = sim->ex2.ry_value * sim->ex2.imm;
            break;
        }

        case OPCODE_DIV1:
        {
            sim->ex2.result_buffer = sim->ex2.ry_value / sim->ex2.rz_value;
            break;
        }

        case OPCODE_DIV2:
        {
            sim->ex2.result_buffer = sim->ex2.ry_value / sim->ex2.imm;
            break;
        }

        case OPCODE_SET:
        {
            sim->ex2.result_buffer = sim->ex2.imm;
            break;
        }
        }
        sim->memory1 = sim->ex2;
        sim->ex2.has_next = FALSE;
    }
}

/*
 * Execute Stage of Pipeline
 */
static void branch(SIMULATOR_STRUCT *sim)
{
    if (sim->branch.has_next)
    {
        switch (sim->branch.opcode)
        {
        case OPCODE_BZ:
        {
            if (sim->branch.rx_value == 0)
            {
                sim->pc = sim->branch.tar_addr;
                sim->next_cycle = TRUE;
                sim->decode.has_next = FALSE;
                sim->inst_depn_analyze.has_next = FALSE;
                sim->ex1.has_next = FALSE;
                sim->fetch.has_next = TRUE;
                halting = FALSE;
                control_hazard_count++;
            }
            break;
        }

        case OPCODE_BGZ:
        {
            if (sim->branch.rx_value >= 0)
            {
                sim->pc = sim->branch.tar_addr;
                sim->next_cycle = TRUE;
                sim->decode.has_next = FALSE;
                sim->inst_depn_analyze.has_next = FALSE;
                sim->ex1.has_next = FALSE;
                sim->fetch.has_next = TRUE;

                halting = FALSE;

                control_hazard_count++;
            }
            break;
        }

        case OPCODE_BGTZ:
        {
            if (sim->branch.rx_value > 0)
            {
                sim->pc = sim->branch.tar_addr;
                sim->next_cycle = TRUE;
                sim->decode.has_next = FALSE;
                sim->inst_depn_analyze.has_next = FALSE;
                sim->ex1.has_next = FALSE;
                sim->fetch.has_next = TRUE;
                halting = FALSE;
                control_hazard_count++;
            }
            break;
        }

        case OPCODE_BLZ:
        {
            if (sim->branch.rx_value <= 0)
            {
                sim->pc = sim->branch.tar_addr;
                sim->fetch.has_next = TRUE;
                sim->next_cycle = TRUE;
                sim->decode.has_next = FALSE;
                sim->inst_depn_analyze.has_next = FALSE;
                sim->ex1.has_next = FALSE;
                halting = FALSE;
                control_hazard_count++;
            }
            break;
        }

        case OPCODE_BLTZ:
        {
            if (sim->branch.rx_value < 0)
            {
                sim->pc = sim->branch.tar_addr;
                sim->decode.has_next = FALSE;
                sim->inst_depn_analyze.has_next = FALSE;
                sim->ex1.has_next = FALSE;
                sim->fetch.has_next = TRUE;
                halting = FALSE;
                control_hazard_count++;
            }
            break;
        }
        }
        sim->ex2 = sim->branch;
        sim->branch.has_next = FALSE;
    }
}

/*
 * Memory Stage of Pipeline
 */
static void mem1(SIMULATOR_STRUCT *sim)
{
    if (sim->memory1.has_next)
    {
        switch (sim->memory1.opcode)
        {
        case OPCODE_LOAD1:
        case OPCODE_LOAD2:
        case OPCODE_STORE1:
        case OPCODE_STORE2:
            break;
        }
        sim->memory2 = sim->memory1;
        sim->memory1.has_next = FALSE;
    }
}

/*
 * Memory Stage of Pipeline
 */
static void mem2(SIMULATOR_STRUCT *sim)
{
    if (sim->memory2.has_next)
    {
        switch (sim->memory2.opcode)
        {
        case OPCODE_LOAD1:
        case OPCODE_LOAD2:
            sim->memory2.result_buffer = 0;
            sim->memory2.result_buffer = sim->memory2.result_buffer | sim->mem2[sim->memory2.mem_addr];
            sim->memory2.result_buffer = (sim->memory2.result_buffer << 8) | sim->mem2[sim->memory2.mem_addr + 1];
            sim->memory2.result_buffer = (sim->memory2.result_buffer << 8) | sim->mem2[sim->memory2.mem_addr + 2];
            sim->memory2.result_buffer = (sim->memory2.result_buffer << 8) | sim->mem2[sim->memory2.mem_addr + 3];
            break;

        case OPCODE_STORE1:
        case OPCODE_STORE2:
        {
            sim->mem2[sim->memory2.mem_addr] = sim->memory2.rx_value;
            break;
        }
        }

        sim->writeback = sim->memory2;
        sim->memory2.has_next = FALSE;
    }
}

/*
 * Writeback Stage of Pipeline
 */
static int writeback(SIMULATOR_STRUCT *sim)
{
    if (sim->writeback.has_next)
    {
        switch (sim->writeback.opcode)
        {
        case OPCODE_SET:
        case OPCODE_ADD1:
        case OPCODE_SUB1:
        case OPCODE_MUL1:
        case OPCODE_DIV1:
        case OPCODE_ADD2:
        case OPCODE_SUB2:
        case OPCODE_MUL2:
        case OPCODE_DIV2:
        case OPCODE_LOAD1:
        case OPCODE_LOAD2:
            sim->regs[sim->writeback.rx] = sim->writeback.result_buffer;
            sim->reg_status[sim->writeback.rx] = TRUE;
            break;
        }
        sim->insn_finished++;
        sim->writeback.has_next = FALSE;

        if (sim->writeback.opcode == OPCODE_RET)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*
* reading memory file
*/
void read_memory_map(SIMULATOR_STRUCT *sim)
{
    const char *file = "memory_map";
    struct stat file2;
    stat(file, &file2);
    sim->mem2_size = file2.st_size;
    sim->mem2 = (unsigned char *)calloc(file2.st_size, sizeof(unsigned char));
    FILE *fp = fopen(file, "rb");
    fread(sim->mem2, file2.st_size, 1, fp);
    fclose(fp);
}

/*
* reading input file
*/
void read_file_as_binary(SIMULATOR_STRUCT *sim, const char *file)
{
    struct stat file2;
    stat(file, &file2);
    unsigned char *mem_alloc = (unsigned char *)calloc(file2.st_size, sizeof(unsigned char));
    sim->mem_size = file2.st_size / 4;
    sim->mem = (INSTRUCTION *)calloc(sim->mem_size, sizeof(INSTRUCTION));
    FILE *fp = fopen(file, "rb");

    fread(mem_alloc, file2.st_size, 1, fp);
    fclose(fp);

    int file_init = 0;
    int curr_index = 0;
    while (file_init < file2.st_size)
    {
        sim->mem[curr_index].opcode = (unsigned char)mem_alloc[file_init + 3];
        sim->mem[curr_index].rx = (unsigned char)mem_alloc[file_init + 2];
        sim->mem[curr_index].ry = (unsigned char)mem_alloc[file_init + 1];
        sim->mem[curr_index].rz = (unsigned char)mem_alloc[file_init];
        sim->mem[curr_index].imm = (unsigned char)mem_alloc[file_init];

        switch (sim->mem[curr_index].opcode)
        {
        case OPCODE_ADD1:
            strcpy(sim->mem[curr_index].opcode_str, "ADD1");
            break;
        case OPCODE_SUB1:
            strcpy(sim->mem[curr_index].opcode_str, "SUB1");
            break;
        case OPCODE_MUL1:
            strcpy(sim->mem[curr_index].opcode_str, "MUL1");
            break;
        case OPCODE_DIV1:
            strcpy(sim->mem[curr_index].opcode_str, "DIV1");
            break;
        case OPCODE_ADD2:
            strcpy(sim->mem[curr_index].opcode_str, "ADD2");
            break;
        case OPCODE_SUB2:
            strcpy(sim->mem[curr_index].opcode_str, "SUB2");
            break;
        case OPCODE_MUL2:
            strcpy(sim->mem[curr_index].opcode_str, "MUL2");
            break;
        case OPCODE_DIV2:
            strcpy(sim->mem[curr_index].opcode_str, "DIV2");
            break;
        case OPCODE_SET:
            strcpy(sim->mem[curr_index].opcode_str, "SET");
            break;
        case OPCODE_BZ:
            strcpy(sim->mem[curr_index].opcode_str, "BEZ");
            break;
        case OPCODE_BGZ:
            strcpy(sim->mem[curr_index].opcode_str, "BGEZ");
            break;
        case OPCODE_BLZ:
            strcpy(sim->mem[curr_index].opcode_str, "BLEZ");
            break;
        case OPCODE_BGTZ:
            strcpy(sim->mem[curr_index].opcode_str, "BGTZ");
            break;
        case OPCODE_BLTZ:
            strcpy(sim->mem[curr_index].opcode_str, "BLTZ");
            break;
        case OPCODE_LOAD1:
            strcpy(sim->mem[curr_index].opcode_str, "LOAD1");
            break;
        case OPCODE_STORE1:
            strcpy(sim->mem[curr_index].opcode_str, "STORE1");
            break;
        case OPCODE_LOAD2:
            strcpy(sim->mem[curr_index].opcode_str, "LOAD2");
            break;
        case OPCODE_STORE2:
            strcpy(sim->mem[curr_index].opcode_str, "STORE2");
            break;
        case OPCODE_RET:
            strcpy(sim->mem[curr_index].opcode_str, "RET");
            break;
        }

        curr_index++;
        file_init += 4;
    }
}

/*
 * This function creates and initialize the simulator.
 */
SIMULATOR_STRUCT *simulator_init(const char *filename)
{
    SIMULATOR_STRUCT *sim;
    sim = (SIMULATOR_STRUCT *)calloc(1, sizeof(SIMULATOR_STRUCT));
    sim->pc = 0;
    sim->clock = 0;
    memset(sim->regs, 0, sizeof(int) * 16);
    memset(sim->reg_status, 1, sizeof(int) * 16);
    sim->stall = 0;

    read_memory_map(sim);
    read_file_as_binary(sim, filename);

    sim->fetch.has_next = TRUE;
    return sim;
}

/*
 * Function contains the simulation loop
 */
void start_simulator(SIMULATOR_STRUCT *sim)
{
    while (TRUE)
    {
        if (writeback(sim))
        {
            break;
        }

        mem2(sim);
        mem1(sim);
        ex2(sim);
        branch(sim);
        ex1(sim);
        inst_depn_analyze(sim);
        decode(sim);
        fetch(sim);
        sim->clock++;
    }
}

int main(int argc, char const *argv[])
{
    SIMULATOR_STRUCT *sim;

    if (argc < 2)
    {
        fprintf(stderr, "No input file provided\n", argv[0]);
        return -1;
    }
    sim = simulator_init(argv[1]);
    int regs[16];
    for (int i = 0; i < 16; i++)
    {
        regs[i] = 0;
    }

    // starting the simulator.
    start_simulator(sim);

    printf("================================\n");
    printf("--------------------------------\n");
    for (int reg = 0; reg < 16; reg++)
    {
        printf("REG[%2d]   |   Value=%d  \n", reg, sim->regs[reg]);
        printf("--------------------------------\n");
    }
    printf("================================\n\n");

    // data_hazard_count = 0;
    // control_hazard_count = 0;
    execution_cycles = sim->clock + count;
    completed_instructions = sim->insn_finished;
    data_hazard_count += count;

    printf("Stalled cycles due to data hazard: %d \n", data_hazard_count);
    printf("Stalled cycles due to control hazard: %d \n", control_hazard_count);
    printf("\n");
    printf("Total stalls: %d \n", data_hazard_count + control_hazard_count);
    printf("Total execution cycles: %d\n", execution_cycles);
    printf("Total instruction simulated: %d\n", completed_instructions);
    printf("IPC: %f\n", ((double)completed_instructions / execution_cycles));
    return 0;
}
