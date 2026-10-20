#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <unistd.h>  
#include "stack_test.cpp"

#include "enum_commands.h"

const int MAXLEN_REGISTER = 3;
const int MAXLEN_COMMAND  = 10;
const int MAXLEN_LABEL    = 8;
const int MAXLEN_COMMENT  = 20;

#define MAXLEN_COMMENT_STR "20"

struct file_info_t {

    off_t size_of_file_in = 0;
    const char* name_file_in = "";
    FILE* ptr_file_in = NULL;
    commands_t command_type = ERROR_COMMAND; 
};

struct command_processing_t {

    int current_int_argument  = 0;
    int set_mark = 0;
    int len_scanf = 0;
    int IP = 0;
    int* code_arr = NULL;
    char current_register[MAXLEN_REGISTER] = {};
    char current_comment[MAXLEN_COMMENT] = {};
    char current_command[MAXLEN_COMMAND] = {};
    char current_arg_ram[MAXLEN_REGISTER]  = {};
    int label[MAXLEN_LABEL]= {-1, -1, -1, -1, -1, -1,-1, -1}; //[0, 1 mark, 2 mark ...]
    int current_label_to_jump = 0; //[0, 1 mark, 2 mark ...]
    char colon = '0'; //двоеточие 
    
};

void open_file(file_info_t* file_info, const char* filename);

void size_of_file(file_info_t* file_info);
int hash_str(const char* str);
commands_t check_command(file_info_t* file_info, const char* current_command);
void command_processing(file_info_t* file_info);
commands_t check_jump_and_jump(command_processing_t* command_info, file_info_t* file_info, commands_t CURRENT_COMMAND);
void check_ram(command_processing_t* command_info, file_info_t* file_info, commands_t CURRENT_COMMAND);
void write_number_of_register(command_processing_t* command_info ,file_info_t* file_info, char* current_register);
FILE* write_commands_in_file(command_processing_t* command_info);
FILE* write_bin_commands_in_file(command_processing_t* command_info);

int main(int argc, const char* argv[]) {

    if (argc == 2) {
        file_info_t file_info = {};
        open_file(&file_info, argv[1]);
        command_processing(&file_info);
    }
    else {
        exit(1);
    }
}

void open_file(file_info_t* file_info, const char* filename) {
    
    file_info -> name_file_in = filename;
    file_info -> ptr_file_in  = fopen(filename, "r");
    size_of_file(file_info);
}

void size_of_file(file_info_t* file_info) {

    struct stat stat_file_in;
    if (stat(file_info -> name_file_in, &stat_file_in) == -1) {
        exit(-1);
    }
    file_info -> size_of_file_in = stat_file_in.st_size;
}

void command_processing(file_info_t* file_info) {

    command_processing_t command_info = {};

    size_of_file(file_info);

    command_info.code_arr = (int*)calloc(file_info -> size_of_file_in , sizeof(int));

    for (int i = 0; i < 2; i++) {
        command_info.IP = 0;
        while (fscanf(file_info -> ptr_file_in,  "%s%n", command_info.current_command, &command_info.len_scanf) != EOF) {
            printf("%s\n", command_info.current_command);

            if (command_info.current_command[0] == ';') { 
                fgets(command_info.current_comment, MAXLEN_COMMENT, file_info -> ptr_file_in);
                // printf("%s, %s\n", command_info.current_comment, command_info.current_command);
                continue;
            }

            switch(check_command(file_info, command_info.current_command)) {
                
                case ADD_COMMAND: 
                                command_info.code_arr[command_info.IP] = ADD_COMMAND; 
                                command_info.IP += 1;
                                break;

                case SUB_COMMAND: 
                                command_info.code_arr[command_info.IP] = SUB_COMMAND; 
                                command_info.IP += 1;
                                break;

                case DIV_COMMAND: 
                                command_info.code_arr[command_info.IP] = DIV_COMMAND; 
                                command_info.IP += 1;
                                break;
                case MUL_COMMAND: 
                                command_info.code_arr[command_info.IP] = MUL_COMMAND; 
                                command_info.IP += 1;
                                break;
                            
                case OUT_COMMAND:  
                                command_info.code_arr[command_info.IP] = OUT_COMMAND; 
                                command_info.IP += 1;
                                break;

                case HLT_COMMAND: 
                                command_info.code_arr[command_info.IP] = HLT_COMMAND; 
                                command_info.IP += 1;
                                break;

                case PUSH_COMMAND:  if (fscanf(file_info -> ptr_file_in, "%d", &command_info.current_int_argument) != 1) {
                                        exit(1);
                                    }
                                    command_info.code_arr[command_info.IP] = PUSH_COMMAND; 
                                    command_info.IP += 1;
                                    command_info.code_arr[command_info.IP] = command_info.current_int_argument;
                                    command_info.IP += 1;
                                    break;

                case POPR_COMMAND: 
                                    if (fscanf(file_info -> ptr_file_in, "%s", command_info.current_register) != 1) {
                                        exit(1);
                                    }
                                    command_info.code_arr[command_info.IP] = POPR_COMMAND; 
                                    command_info.IP += 1;
                                    write_number_of_register(&command_info, file_info, command_info.current_register);
                                    command_info.IP += 1;
                                    break;
                                    

                case PUSHR_COMMAND:
                                    if (fscanf(file_info -> ptr_file_in, "%s", command_info.current_register) != 1) {
                                        exit(1);
                                    }
                                    command_info.code_arr[command_info.IP] = PUSHR_COMMAND; 
                                    command_info.IP += 1;
                                    write_number_of_register(&command_info, file_info, command_info.current_register);
                                    command_info.IP += 1;
                                    break;

                case MARK_COMMAND: if (sscanf(command_info.current_command, "%c%d",  &command_info.colon, &command_info.set_mark) != 2) {
                                        exit(1);
                                    }
                                    command_info.label[command_info.set_mark] = command_info.IP;
                                    // printf("%d\n", command_info.label[command_info.set_mark]);
                                    break;

                case JA_COMMAND:    check_jump_and_jump(&command_info, file_info, JA_COMMAND);
                                    // printf("%d\n", command_info.code_arr[command_info.IP -1]);
                                    break;

                case JAE_COMMAND:   check_jump_and_jump(&command_info, file_info, JAE_COMMAND);
                                    break;

                case JB_COMMAND:    check_jump_and_jump(&command_info, file_info, JB_COMMAND);
                                    break;
                        
                case JBE_COMMAND:   check_jump_and_jump(&command_info, file_info, JBE_COMMAND);
                                    break;

                case JE_COMMAND:    check_jump_and_jump(&command_info, file_info, JE_COMMAND);
                                    break;
                            
                case JNE_COMMAND:   check_jump_and_jump(&command_info, file_info, JNE_COMMAND);
                                    break;
                
                case JMP_COMMAND:   check_jump_and_jump(&command_info, file_info, JMP_COMMAND);
                                    // printf("%d\n", command_info.code_arr[command_info.IP -1]);
                                    break;

                case IN_COMMAND:    command_info.code_arr[command_info.IP] = IN_COMMAND; 
                                    command_info.IP += 1;
                                    break;

                case CALL_COMMAND:  check_jump_and_jump(&command_info, file_info, CALL_COMMAND);
                                    break;
                                    
                case RET_COMMAND:   command_info.code_arr[command_info.IP] = RET_COMMAND; 
                                    command_info.IP += 1;
                                    break;
                            
                case SQRT_COMMAND:  command_info.code_arr[command_info.IP] = SQRT_COMMAND; 
                                    command_info.IP += 1;
                                    break;
                            
                case PUSHM_COMMAND: check_ram(&command_info, file_info, PUSHM_COMMAND);
                                    break;
                
                case POPM_COMMAND:  check_ram(&command_info, file_info, POPM_COMMAND);
                                    break;                              
                                
                case DRAW_COMMAND:  command_info.code_arr[command_info.IP] = DRAW_COMMAND; 
                                    command_info.IP += 1;
                                    break;
                            
                default:            exit(1);
            }
        }
        rewind(file_info -> ptr_file_in);
    }

    write_commands_in_file(&command_info);
    write_bin_commands_in_file(&command_info);
}

FILE* write_commands_in_file(command_processing_t* command_info) {

    FILE* file_out = fopen("assembler.txt", "w");

    for (int i = 0; i < command_info -> IP; i++) {
        fprintf(file_out, "%d ", command_info -> code_arr[i]); 
    }
   
    return file_out;
}

FILE* write_bin_commands_in_file(command_processing_t* command_info) {
    FILE* file_out = fopen("assembler.bin", "w");

        fwrite(command_info -> code_arr, sizeof(int), command_info -> IP -1, file_out); 
   
    return file_out;
}

commands_t check_command(file_info_t* file_info, const char* current_command) {

    if (strlen(current_command) > MAXLEN_COMMAND)
        return file_info->command_type = ERROR_COMMAND;

    if (strcmp(current_command, "PUSH")  == 0)  return file_info->command_type = PUSH_COMMAND;
    if (strcmp(current_command, "ADD")   == 0)  return file_info->command_type = ADD_COMMAND;
    if (strcmp(current_command, "SUB")   == 0)  return file_info->command_type = SUB_COMMAND;
    if (strcmp(current_command, "DIV")   == 0)  return file_info->command_type = DIV_COMMAND;
    if (strcmp(current_command, "MUL")   == 0)  return file_info->command_type = MUL_COMMAND;
    if (strcmp(current_command, "POPR")  == 0)  return file_info->command_type = POPR_COMMAND;
    if (strcmp(current_command, "PUSHR") == 0)  return file_info->command_type = PUSHR_COMMAND;

    if (strcmp(current_command, "OUT")   == 0)  return file_info->command_type = OUT_COMMAND;
    if (strcmp(current_command, "HLT")   == 0)  return file_info->command_type = HLT_COMMAND;
    if (current_command[0] == ':')                         return file_info->command_type = MARK_COMMAND;

    if (strcmp(current_command, "JA")    == 0)  return file_info->command_type = JA_COMMAND;
    if (strcmp(current_command, "JAE")   == 0)  return file_info->command_type = JAE_COMMAND;
    if (strcmp(current_command, "JB")    == 0)  return file_info->command_type = JB_COMMAND;
    if (strcmp(current_command, "JBE")   == 0)  return file_info->command_type = JBE_COMMAND;
    if (strcmp(current_command, "JE")    == 0)  return file_info->command_type = JE_COMMAND;
    if (strcmp(current_command, "JNE")   == 0)  return file_info->command_type = JNE_COMMAND;
    if (strcmp(current_command, "JMP")   == 0)  return file_info->command_type = JMP_COMMAND;

    if (strcmp(current_command, "IN")    == 0)  return file_info->command_type = IN_COMMAND;
    if (strcmp(current_command, "CALL")  == 0)  return file_info->command_type = CALL_COMMAND;
    if (strcmp(current_command, "RET")   == 0)  return file_info->command_type = RET_COMMAND;
    if (strcmp(current_command, "SQRT")  == 0)  return file_info->command_type = SQRT_COMMAND;
    if (strcmp(current_command, "PUSHM") == 0)  return file_info->command_type = PUSHM_COMMAND;
    if (strcmp(current_command, "POPM")  == 0)  return file_info->command_type = POPM_COMMAND;
    if (strcmp(current_command, "DRAW")  == 0)  return file_info->command_type = DRAW_COMMAND;

    return file_info->command_type = ERROR_COMMAND;
}

int hash_str(const char* str) {

    int hash = 0;
    for (int i = 0; i < strlen(str); i++) {
        hash += str[i]; 
    }

    return hash;
}

commands_t check_jump_and_jump(command_processing_t* command_info, file_info_t* file_info, commands_t CURRENT_COMMAND) {

    command_info -> code_arr[command_info -> IP] = CURRENT_COMMAND;
    (command_info -> IP)++;
    
    if (fscanf(file_info -> ptr_file_in, " %c%d", &command_info -> colon, &(command_info -> current_label_to_jump)) != 2) {
        exit(1);
    }
                                
    if (command_info -> colon == ':') { /*add increase label array*/
        command_info -> code_arr[command_info -> IP] = (command_info -> label)[command_info -> current_label_to_jump];
        (command_info -> IP)++;
        return CURRENT_COMMAND;
    }
    exit(1);

}

void check_ram(command_processing_t* command_info, file_info_t* file_info, commands_t CURRENT_COMMAND) {
    if (fscanf(file_info -> ptr_file_in, " [%4[^]]]", command_info -> current_arg_ram) != 1) {
        exit(3);
    }
     printf("%s\n", command_info -> current_arg_ram);

    command_info -> code_arr[command_info -> IP] = CURRENT_COMMAND; 
    command_info -> IP += 1;
    write_number_of_register(command_info, file_info, command_info -> current_arg_ram);
    command_info -> IP += 1;
}

void write_number_of_register(command_processing_t* command_info, file_info_t* file_info, char* current_register) {

    if      (strcmp(current_register, "AX") == 0)  command_info->code_arr[command_info->IP] = 1;
    else if (strcmp(current_register, "BX") == 0)  command_info->code_arr[command_info->IP] = 2;
    else if (strcmp(current_register, "CX") == 0)  command_info->code_arr[command_info->IP] = 3;
    else if (strcmp(current_register, "DX") == 0)  command_info->code_arr[command_info->IP] = 4;
    else                                                      exit(1);
}