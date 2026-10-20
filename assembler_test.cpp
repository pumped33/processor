#include <cstdlib>
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
    commands_t command_type = ERROR_COMMAND; 
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
    int label[MAXNUM_LABEL]= {-1, -1, -1, -1, -1, -1,-1, -1}; //[0, 1 mark, 2 mark ...]
    int current_label_to_jump = 0; //[0, 1 mark, 2 mark ...]
    char colon = '0'; //двоеточие 
    OP_CODE op_code;
    
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
void set_command_in_array(command_processing_t* command_info, commands_t CURRENT_COMMAND);

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
        
            memset(&command_info.op_code, 0, sizeof(command_info.op_code));

            switch(check_command(file_info, command_info.current_command)) { //save return value and after jump cases use it 
                
                case ADD_COMMAND: 
                                set_command_in_array(&command_info, ADD_COMMAND);
                                break;

                case SUB_COMMAND: 
                                set_command_in_array(&command_info, SUB_COMMAND);
                                break;

                case DIV_COMMAND: 
                                set_command_in_array(&command_info, DIV_COMMAND);
                                break;

                case MUL_COMMAND: 
                                set_command_in_array(&command_info, MUL_COMMAND);
                                break;
                case OUT_COMMAND:  
                                set_command_in_array(&command_info, OUT_COMMAND);
                                break;

                case HLT_COMMAND: 
                                set_command_in_array(&command_info, HLT_COMMAND);
                                break;

                case PUSH_COMMAND:  {
                                    command_info.op_code.op = PUSH_COMMAND; 
                                    if (fscanf(file_info -> ptr_file_in, "%s", command_info.current_push_argument) != 1) {
                                        exit(1);
                                    }

                                    char reg[MAXLEN_REGISTER] = {};
                                    int constanta = 0;

                                    if (sscanf(command_info.current_push_argument, "%d", &constanta) == 1) { 
                                            command_info.op_code.konst = 1; 
                                            memcpy(&command_info.code_arr[command_info.IP], &command_info.op_code, sizeof(command_info.op_code));
                                            command_info.IP += 1; 
                                            command_info.code_arr[command_info.IP] = constanta;
                                    }
                                    else {
                                        if (sscanf(command_info.current_push_argument, " [%3[^]]]", reg) == 1) {
                                                command_info.op_code.mem = 1; 
                                        }
                                        else if (sscanf(command_info.current_push_argument, "%s", reg) == 1) {
                                                command_info.op_code.reg = 1; 
                                        }
                                        else {
                                            printf("aguadu\n");
                                        }
                                        memcpy(&command_info.code_arr[command_info.IP], &command_info.op_code, sizeof(command_info.op_code));
                                        command_info.IP += 1; 
                                        write_number_of_register(&command_info , file_info, reg);
                                    }

                                    command_info.IP += 1;
                                    break;

                                    }

                case POP_COMMAND:  {
                                    command_info.op_code.op = POP_COMMAND; 
                                    if (fscanf(file_info -> ptr_file_in, "%s", command_info.current_push_argument) != 1) {
                                        exit(1);
                                    }

                                    char reg[MAXLEN_REGISTER] = {};
        
                                    if (sscanf(command_info.current_push_argument, " [%3[^]]]", reg) == 1) {
                                            command_info.op_code.mem = 1; 
                                    }
                                    else if (sscanf(command_info.current_push_argument, "%s", reg) == 1) {
                                            command_info.op_code.reg = 1; 
                                    }
                                    else {
                                        printf("aguadu\n");
                                    }
                                    memcpy(&command_info.code_arr[command_info.IP], &command_info.op_code, sizeof(command_info.op_code));
                                    command_info.IP += 1;
                                    write_number_of_register(&command_info , file_info, reg);
                                

                                    command_info.IP += 1;
                                    break;

                                    }


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

                case IN_COMMAND:    set_command_in_array(&command_info, IN_COMMAND);
                                    break;

                case CALL_COMMAND:  check_jump_and_jump(&command_info, file_info, CALL_COMMAND);
                                    break;
                                    
                case RET_COMMAND:   set_command_in_array(&command_info, RET_COMMAND);
                                    break;

                case SQRT_COMMAND:  set_command_in_array(&command_info, SQRT_COMMAND);
                                    break;                         
                                
                case DRAW_COMMAND:  set_command_in_array(&command_info, DRAW_COMMAND);
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

    FILE* file_out = fopen("assembler_out_test.txt", "w");

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

    if (strcmp(current_command, "PUSH") == 0)  return file_info->command_type = PUSH_COMMAND;
    if (strcmp(current_command, "ADD")  == 0)  return file_info->command_type = ADD_COMMAND;
    if (strcmp(current_command, "SUB")  == 0)  return file_info->command_type = SUB_COMMAND;
    if (strcmp(current_command, "DIV")  == 0)  return file_info->command_type = DIV_COMMAND;
    if (strcmp(current_command, "MUL")  == 0)  return file_info->command_type = MUL_COMMAND;
    if (strcmp(current_command, "POP")  == 0)  return file_info->command_type = POP_COMMAND;
    if (strcmp(current_command, "OUT")  == 0)  return file_info->command_type = OUT_COMMAND;
    if (strcmp(current_command, "HLT")  == 0)  return file_info->command_type = HLT_COMMAND;
    if (current_command[0] == ':')                        return file_info->command_type = MARK_COMMAND;
    if (strcmp(current_command, "JA")   == 0)  return file_info->command_type = JA_COMMAND;
    if (strcmp(current_command, "JAE")  == 0)  return file_info->command_type = JAE_COMMAND;
    if (strcmp(current_command, "JB")   == 0)  return file_info->command_type = JB_COMMAND;
    if (strcmp(current_command, "JBE")  == 0)  return file_info->command_type = JBE_COMMAND;
    if (strcmp(current_command, "JE")   == 0)  return file_info->command_type = JE_COMMAND;
    if (strcmp(current_command, "JNE")  == 0)  return file_info->command_type = JNE_COMMAND;
    if (strcmp(current_command, "JMP")  == 0)  return file_info->command_type = JMP_COMMAND;
    if (strcmp(current_command, "IN")   == 0)  return file_info->command_type = IN_COMMAND;
    if (strcmp(current_command, "CALL") == 0)  return file_info->command_type = CALL_COMMAND;
    if (strcmp(current_command, "RET")  == 0)  return file_info->command_type = RET_COMMAND;
    if (strcmp(current_command, "SQRT") == 0)  return file_info->command_type = SQRT_COMMAND;
    if (strcmp(current_command, "DRAW") == 0)  return file_info->command_type = DRAW_COMMAND; //ЦИКЛ

    return file_info->command_type = ERROR_COMMAND;
}

commands_t check_jump_and_jump(command_processing_t* command_info, file_info_t* file_info, commands_t CURRENT_COMMAND) {

    command_info -> op_code.op= CURRENT_COMMAND;
    memcpy(&(command_info -> code_arr[command_info -> IP]), &(command_info -> op_code), sizeof(command_info -> op_code));
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

void write_number_of_register(command_processing_t* command_info, file_info_t* file_info, char* current_register) {

    if      (strcmp(current_register, "AX") == 0)  command_info->code_arr[command_info->IP] = 1;
    else if (strcmp(current_register, "BX") == 0)  command_info->code_arr[command_info->IP] = 2;
    else if (strcmp(current_register, "CX") == 0)  command_info->code_arr[command_info->IP] = 3;
    else if (strcmp(current_register, "DX") == 0)  command_info->code_arr[command_info->IP] = 4;
    else                                           exit(1);
}

void set_command_in_array(command_processing_t* command_info, commands_t CURRENT_COMMAND) {

    command_info -> op_code.op = CURRENT_COMMAND; 
    memcpy(&command_info -> code_arr[command_info -> IP], &(command_info -> op_code), sizeof(command_info -> op_code));
    command_info -> IP += 1;
}