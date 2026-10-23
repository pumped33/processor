#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>


#include "enum_commands.h"

#define DEBUG_ASSEMBLER_MODE

#ifdef DEBUG_ASSEMBLER_MODE
    #define ON_DBG_ASS(...) __VA_ARGS__
#else
    #define ON_DBG_ASS(...) 
#endif

const int MAXLEN_REGISTER = 3;
const int MAXLEN_COMMAND  = 10;
const int MAXNUM_LABEL    = 8;
const int MAXLEN_COMMENT  = 20;

struct OP_CODE
{
    unsigned int op    :5;
    unsigned int konst :1;  
    unsigned int reg   :1;  
    unsigned int mem   :1;
};

struct file_info_t {

    off_t size_of_file_in = 0;
    const char* name_file_in = "";
    FILE* ptr_file_in = NULL;
};

struct command_processing_t {

    char current_push_argument[MAXLEN_COMMAND] = {};
    int set_mark = 0;
    int len_scanf = 0;
    int IP = 0;
    int* code_arr = NULL;
    char current_register[MAXLEN_REGISTER] = {};
    char current_comment[MAXLEN_COMMENT] = {};
    char current_command[MAXLEN_COMMAND] = {};
    int current_label_to_jump = 0; 
     int label[MAXNUM_LABEL]= {-1, -1, -1, -1, -1, -1,-1, -1};
    char colon = '0'; //двоеточие 
    OP_CODE op_code;
};

void open_file(file_info_t* file_info, const char* filename);

int size_of_file(file_info_t* file_info);
FILE* write_commands_in_file(command_processing_t* command_info);
FILE* write_bin_commands_in_file(command_processing_t* command_info);
int command_processing(file_info_t* file_info);
commands_t check_command(const char* current_command);
int check_jump_and_jump(command_processing_t* command_info, file_info_t* file_info, commands_t CURRENT_COMMAND);
int write_number_of_register(command_processing_t* command_info, char* current_register);
void set_command_in_array(command_processing_t* command_info, commands_t CURRENT_COMMAND);
int check_push_and_push(command_processing_t* command_info, file_info_t* file_info);
int check_pop_and_pop(command_processing_t* command_info, file_info_t* file_info);

int main(int argc, const char* argv[]) {

    if (argc == 2) {
        file_info_t file_info = {};
        open_file(&file_info, argv[1]);
        command_processing(&file_info);
        return 0;
    }
    else {
        return 1;
    }
}

void open_file(file_info_t* file_info, const char* filename) {
    
    file_info -> name_file_in = filename;
    file_info -> ptr_file_in  = fopen(filename, "r");
    size_of_file(file_info);
}

int size_of_file(file_info_t* file_info) {

    struct stat stat_file_in;
    if (stat(file_info -> name_file_in, &stat_file_in) == -1) {
        return 1;
    }
    file_info -> size_of_file_in = stat_file_in.st_size;
    return 0;
}

int command_processing(file_info_t* file_info) {

    command_processing_t command_info = {};
    size_of_file(file_info);

    command_info.code_arr = (int*)calloc(file_info -> size_of_file_in , sizeof(int));

    for (int i = 0; i < 2; i++) {
        command_info.IP = 0;
        while (fscanf(file_info -> ptr_file_in,  "%s%n", command_info.current_command, &command_info.len_scanf) != EOF) {
            printf("%s\n", command_info.current_command);
            if (command_info.current_command[0] == ';') { 
                fgets(command_info.current_comment, MAXLEN_COMMENT, file_info -> ptr_file_in);
                continue;
            }
        
            memset(&command_info.op_code, 0, sizeof(command_info.op_code));

            switch(commands_t command = check_command(command_info.current_command)) { //save return value and after jump cases use it 
                
                case ADD_COMMAND: 
                case SUB_COMMAND:
                case DIV_COMMAND: 
                case MUL_COMMAND:
                case OUT_COMMAND:  
                case HLT_COMMAND:  
                case IN_COMMAND:             
                case RET_COMMAND:   
                case SQRT_COMMAND:                             
                case DRAWC_COMMAND: 
                case DRAWW_COMMAND:
                                    set_command_in_array(&command_info, command);
                                    break;

                case PUSH_COMMAND:  check_push_and_push(&command_info, file_info); 
                                    break;

                case POP_COMMAND:          
                                    check_pop_and_pop(&command_info, file_info);
                                    break;


                case MARK_COMMAND: if (sscanf(command_info.current_command, "%c%d",  &command_info.colon, &command_info.set_mark) != 2) {
                                        return 1;
                                    }
                                    command_info.label[command_info.set_mark] = command_info.IP;
                                    // printf("%d\n", command_info.label[command_info.set_mark]);
                                    break;

                case JA_COMMAND:   
                case JAE_COMMAND:  
                case JB_COMMAND:    
                case JBE_COMMAND: 
                case JE_COMMAND:      
                case JNE_COMMAND:  
                case JMP_COMMAND:   
                                    check_jump_and_jump(&command_info, file_info, command);
                                    break;
                        
                case CALL_COMMAND:  check_jump_and_jump(&command_info, file_info, CALL_COMMAND);
                                    break;

                            
                default:            return 1;
            }
        }
        rewind(file_info -> ptr_file_in);
    }

    ON_DBG_ASS(write_commands_in_file(&command_info);)
    write_bin_commands_in_file(&command_info);
    fclose(file_info -> ptr_file_in);
    return 0;
}

ON_DBG_ASS (

    FILE* write_commands_in_file(command_processing_t* command_info) {
        FILE* file_out = fopen("assembler_out.txt", "w");

        for (int i = 0; i < command_info -> IP; i++) {
        fprintf(file_out, "%d ", command_info -> code_arr[i]); 
        }
        fclose(file_out);

        return file_out;
    }
)

FILE* write_bin_commands_in_file(command_processing_t* command_info) {
    
    FILE* file_out = fopen("assembler.bin", "w");
    fwrite(command_info -> code_arr, sizeof(int), command_info -> IP, file_out); 
    fclose(file_out);
    return file_out;
}

commands_t check_command(const char* current_command) {

    if (current_command[0] == ':')      return MARK_COMMAND;

    for (int i = 0; i < AMOUNT_COMMAND; i++) {
        if (strcmp(COMMAND_ARRAY[i].name_command, current_command) == 0) {
            return COMMAND_ARRAY[i].command;
        }
    }
    return ERROR_COMMAND;
}

int check_jump_and_jump(command_processing_t* command_info, file_info_t* file_info, commands_t CURRENT_COMMAND) {

    command_info -> op_code.op= CURRENT_COMMAND;
    memcpy(&(command_info -> code_arr[command_info -> IP]), &(command_info -> op_code), sizeof(command_info -> op_code));
    (command_info -> IP)++;
    
    if (fscanf(file_info -> ptr_file_in, " %c%d", &command_info -> colon, &(command_info -> current_label_to_jump)) != 2) {
        return 1;
    }
                                
    if (command_info -> colon == ':') { /*add increase label array*/
        command_info -> code_arr[command_info -> IP] = (command_info -> label)[command_info -> current_label_to_jump];
        (command_info -> IP)++;
        return 0;
    }
    return 1;

}

int write_number_of_register(command_processing_t* command_info, char* current_register) {

    for (int i = 0; i < NUMBER_REGISTER; i++) {
        if (strcmp(REGISTORS_ARRAY[i].name_registor, current_register) == 0) {
            command_info -> code_arr[command_info -> IP] = REGISTORS_ARRAY[i].num_registor;
            return 1;
        }
    }
    return 0;
}

void set_command_in_array(command_processing_t* command_info, commands_t CURRENT_COMMAND) {

    command_info -> op_code.op = CURRENT_COMMAND; 
    memcpy(&command_info -> code_arr[command_info -> IP], &(command_info -> op_code), sizeof(command_info -> op_code));
    command_info -> IP += 1;
}

int check_push_and_push(command_processing_t* command_info, file_info_t* file_info) {

    command_info -> op_code.op = PUSH_COMMAND; 
    if (fscanf(file_info -> ptr_file_in, "%s", command_info -> current_push_argument) != 1) {
        return 1;
    }

    char reg[MAXLEN_REGISTER] = {};
    int constanta = 0;

    if (sscanf(command_info -> current_push_argument, "%d", &constanta) == 1) { 
        command_info -> op_code.konst = 1; 
        memcpy(&command_info -> code_arr[command_info -> IP], &command_info -> op_code, sizeof(command_info -> op_code));
        command_info -> IP += 1; 
        command_info -> code_arr[command_info -> IP] = constanta;
    }
    else {
        if (sscanf(command_info -> current_push_argument, " [%3[^]]]", reg) == 1) {
                command_info -> op_code.mem = 1; 
        }
        else if (sscanf(command_info -> current_push_argument, "%s", reg) == 1) {
                command_info -> op_code.reg = 1; 
        }
        else {
            return 0;
        }
    memcpy(&command_info -> code_arr[command_info -> IP], &command_info -> op_code, sizeof(command_info -> op_code));
    command_info -> IP += 1; 
    write_number_of_register(command_info , reg);
    }

    command_info -> IP += 1;
    return 0;
}

int check_pop_and_pop(command_processing_t* command_info, file_info_t* file_info) {

     
    command_info -> op_code.op = POP_COMMAND; 
    if (fscanf(file_info -> ptr_file_in, "%s", command_info -> current_push_argument) != 1) {
        return 1;
    }

    char reg[MAXLEN_REGISTER] = {};

    if (sscanf(command_info -> current_push_argument, " [%3[^]]]", reg) == 1) {
            command_info -> op_code.mem = 1; 
    }
    else if (sscanf(command_info -> current_push_argument, "%s", reg) == 1) {
            command_info -> op_code.reg = 1; 
    }
    else {
        return 1;
    }
    memcpy(&command_info -> code_arr[command_info -> IP], &command_info -> op_code, sizeof(command_info -> op_code));
    command_info -> IP += 1;
    write_number_of_register(command_info , reg);

    command_info -> IP += 1;
    return 0;
}