
void djb_2_hash(const char* str, unsigned int len);

enum commands_t {

    HLT_COMMAND =   0,
    PUSH_COMMAND =  1,
    ADD_COMMAND =   2,
    SUB_COMMAND =   3,
    DIV_COMMAND =   4,
    OUT_COMMAND =   5,
    POPR_COMMAND =  6, 
    PUSHR_COMMAND = 7,
    MUL_COMMAND =   8,
    MARK_COMMAND =  9,
    JA_COMMAND =    10,
    JAE_COMMAND =   11,  
    JB_COMMAND =    12, 
    JBE_COMMAND =   13, 
    JE_COMMAND =    14, 
    JNE_COMMAND =   15, 
    JMP_COMMAND =   16, 
    IN_COMMAND =    17, 
    CALL_COMMAND =  18,
    RET_COMMAND =   19, 
    SQRT_COMMAND =  20,
    PUSHM_COMMAND = 21,
    POPM_COMMAND =  22,
    DRAWC_COMMAND = 23,
    POP_COMMAND =   24,
    DRAWW_COMMAND = 25,

    SUCCESS_COMMAND = 100,
    ERROR_COMMAND = -1
};

struct registor_t {
    const char* name_registor;
    int num_registor;
};

struct command_info_t {

    const char* name_command;
    commands_t command;
};

const int AMOUNT_COMMAND =  27;
const int NUMBER_REGISTER = 4;

registor_t REGISTORS_ARRAY[NUMBER_REGISTER] = {{"AX", 1}, {"BX", 2}, {"CX", 3}, {"DX", 4}};

void djb_2_hash(const char* str, unsigned int len) {
    
    unsigned long hash = 5381;
    for (int i = 0; i < len; i++) {
        hash = (hash << 5 + hash) + *(str + i);
    }
    return hash; 
    
}

command_info_t COMMAND_ARRAY[AMOUNT_COMMAND] = {
    { "HLT",     HLT_COMMAND},
    { "PUSH",    PUSH_COMMAND},
    { "ADD",     ADD_COMMAND},
    { "SUB",     SUB_COMMAND},
    { "DIV",     DIV_COMMAND},
    { "OUT",     OUT_COMMAND},
    { "POPR",    POPR_COMMAND},
    { "PUSHR",   PUSHR_COMMAND},
    { "MUL",     MUL_COMMAND},
    { ":",       MARK_COMMAND},
    { "JA",      JA_COMMAND},
    { "JAE",     JAE_COMMAND},
    { "JB",      JB_COMMAND},
    { "JBE",     JBE_COMMAND},
    { "JE",      JE_COMMAND},
    { "JNE",     JNE_COMMAND},
    { "JMP",     JMP_COMMAND},
    { "IN",      IN_COMMAND},
    { "CALL",    CALL_COMMAND},
    { "RET",     RET_COMMAND},
    { "SQRT",    SQRT_COMMAND},
    { "PUSHM",   PUSHM_COMMAND},
    { "POPM",    POPM_COMMAND},
    { "DRAWC",   DRAWC_COMMAND},
    { "POP",     POP_COMMAND},
    { "DRAWW",   DRAWW_COMMAND},
    { "ERROR",   ERROR_COMMAND} };