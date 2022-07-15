// defining constants
#define NUM_OF_STAGES 9
#define STALL 1
#define RESUME_EXECUTION 0
#define TRUE 1
#define FALSE 0

// opcode declaration
#define OPCODE_SET 0x00
#define OPCODE_ADD1 0x10
#define OPCODE_ADD2 0x11
#define OPCODE_SUB1 0x20
#define OPCODE_SUB2 0x21
#define OPCODE_MUL1 0x30
#define OPCODE_MUL2 0x31
#define OPCODE_DIV1 0x40
#define OPCODE_DIV2 0x41
#define OPCODE_LOAD1 0x50
#define OPCODE_LOAD2 0x51
#define OPCODE_STORE1 0x60
#define OPCODE_STORE2 0x61
#define OPCODE_BZ 0x70
#define OPCODE_BGZ 0x71
#define OPCODE_BLZ 0x72
#define OPCODE_BGTZ 0x73
#define OPCODE_BLTZ 0x74
#define OPCODE_RET 0xFF

//defining structures for simulator
typedef struct INSTRUCTION
{
    char opcode_str[16];
    int opcode;
    int rx;
    int ry;
    int rz;
    int imm;
} INSTRUCTION;


typedef struct STAGE
{
    int pc;
    int result_buffer;
    int mem_addr;
    int tar_addr;
    int has_next;
    char opcode_str[1];
    int opcode;
    int rx;
    int ry;
    int rz;
    int imm;
    int rx_value;
    int ry_value;
    int rz_value;
    
} STAGE;

typedef struct SIMULATOR_STRUCT
{
    int pc;
    int clock;
     unsigned char stall;
    INSTRUCTION *mem;
    int mem_size;
    unsigned char *mem2;
    int mem2_size;
    int insn_finished;
    int regs[16];
    int reg_status[16];    
    int next_cycle;
   

    STAGE fetch;
    STAGE decode;
    STAGE inst_depn_analyze;
    STAGE ex1;
    STAGE branch;
    STAGE ex2;
    STAGE memory1;
    STAGE memory2;
    STAGE writeback;
} SIMULATOR_STRUCT;

