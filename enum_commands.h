#include <cstdint>

enum commands_t {

    HLT_COMMAND   =  0,
    PUSH_COMMAND  =  1,
    ADD_COMMAND   =  2,
    SUB_COMMAND   =  3,
    DIV_COMMAND   =  4,
    OUT_COMMAND   =  5,
    MUL_COMMAND   =  8,
    MARK_COMMAND  =  9,
    JA_COMMAND    = 10,
    JAE_COMMAND   = 11,
    JB_COMMAND    = 12,
    JBE_COMMAND   = 13,
    JE_COMMAND    = 14,
    JNE_COMMAND   = 15,
    JMP_COMMAND   = 16,
    IN_COMMAND    = 17,
    CALL_COMMAND  = 18,
    RET_COMMAND   = 19,
    SQRT_COMMAND  = 20,
    DRAWC_COMMAND = 23,
    POP_COMMAND   = 24,
    DRAWW_COMMAND = 25,

    ERROR_COMMAND = -1
};

struct registor_t {
    const char* name_registor;
    int num_registor;
};

struct command_info_t {

    const char* name_command;
    commands_t command;
    const uint32_t hash_command;
};

const int AMOUNT_COMMAND =  21;
const int NUMBER_REGISTER = 4;

registor_t REGISTORS_ARRAY[NUMBER_REGISTER] = {{"AX", 1}, {"BX", 2}, {"CX", 3}, {"DX", 4}};

command_info_t COMMAND_ARRAY[AMOUNT_COMMAND] = {
    { "PUSH",  PUSH_COMMAND,  0x7C8A6265u },
    { "DRAWC", DRAWC_COMMAND, 0x0CFB9976u },
    { "JNE",   JNE_COMMAND,   0x0B87F802u },
    { "ADD",   ADD_COMMAND,   0x0B87D06Eu },
    { "RET",   RET_COMMAND,   0x0B8818F0u },
    { "MUL",   MUL_COMMAND,   0x0B8805B3u },
    { "JB",    JB_COMMAND,    0x00597411u },
    { "CALL",  CALL_COMMAND,  0x7C82EB81u },
    { "SQRT",  SQRT_COMMAND,  0x7C8BF66Fu },
    { "IN",    IN_COMMAND,    0x005973FCu },
    { "JAE",   JAE_COMMAND,   0x0B87F655u },
    { "DIV",   DIV_COMMAND,   0x0B87DDE8u },
    { "JE",    JE_COMMAND,    0x00597414u },
    { "OUT",   OUT_COMMAND,   0x0B880E3Du },
    { "JBE",   JBE_COMMAND,   0x0B87F676u },
    { "DRAWW", DRAWW_COMMAND, 0x0CFB998Au },
    { "SUB",   SUB_COMMAND,   0x0B881F2Fu },
    { "JA",    JA_COMMAND,    0x00597410u },
    { "HLT",   HLT_COMMAND,   0x0B87EF4Du },
    { "JMP",   JMP_COMMAND,   0x0B87F7ECu },
    { "POP",   POP_COMMAND,   0x0B8811B4u },
};

enum mask_for_bin_field_t {

    SIXTH_BIT   =     0b00100000,
    SEVENTH_BIT =     0b01000000,
    EIGHTH_BIT  =     0b10000000,

    LOWER_FIVE_BITS = 0b00011111,

};