#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>

#include "enum_commands.h"
#include "read_from_file.cpp"
#include "comporator_qsort_hash_up.cpp"

// #define DEBUG_ASSEMBLER_MODE

#ifdef DEBUG_ASSEMBLER_MODE
    #define ON_DBG_ASS(...) __VA_ARGS__
#else
    #define ON_DBG_ASS(...) 
#endif

const int MAXLEN_REGISTER = 3;
const int MAXLEN_COMMAND  = 12;
const int MAXNUM_LABEL    = 9;
const int MAXLEN_COMMENT  = 20;

struct OP_CODE
{
    unsigned int op    :5;
    unsigned int konst :1;  
    unsigned int reg   :1;  
    unsigned int mem   :1;
};

struct command_processing_t {

    char        current_push_argument[MAXLEN_COMMAND]  = {};
    int         set_mark                               = 0;
    int         IP                                     = 0;
    int*        code_arr                               = NULL;
    char        current_register[MAXLEN_REGISTER]      = {};
    char        current_comment[MAXLEN_COMMENT]        = {};
    char        current_command[MAXLEN_COMMAND]        = {};
    int         current_label_to_jump                  = 0; 
    int         label[MAXNUM_LABEL]                    = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
    char        colon                                  = '0'; //двоеточие 
    OP_CODE     op_code                                = {};
    char*       current_ptr_code                       = NULL;
    int         len_sscanf                             = 0;

};

FILE*      write_commands_in_file           (command_processing_t* command_info);
FILE*      write_bin_commands_in_file       (command_processing_t* command_info);
int        command_processing               (command_processing_t* command_info, char* code);
commands_t check_command                    (const char* current_command);
int        check_jump_and_jump              (command_processing_t* command_info, commands_t CURRENT_COMMAND);
int        write_number_of_register         (command_processing_t* command_info, char* current_register);
void       set_command_in_array             (command_processing_t* command_info, commands_t CURRENT_COMMAND);
int        check_push_or_pop_and_do         (command_processing_t* command_info, commands_t CURRENT_COMMAND);
uint32_t   djb_2_hash                       (const char* str, unsigned int len);
int        set_mark_to_label_arr            (command_processing_t* command_info);
void       set_value_to_array_as_bin_field  (command_processing_t* command_info);
char*      read_from_file                   (file_info_t* file_info, const char* filename);
int        assembling                       (file_info_t* file_info, const char* filename);

int main(int argc, const char* argv[]) {

    if (argc == 2) {
        file_info_t file_info = {};
        assembling(&file_info, argv[1]);
        return 0;
    }
    else {
        return 1;
    }
}

char* read_from_file(file_info_t* file_info, const char* filename) {

    open_file(file_info, filename);
    size_of_file(file_info);

    char* buffer = (char*)calloc((size_t)file_info -> size_of_file_in + 1, sizeof(char)); //+1 to add /0 in the end

    int real_size_file_in = fread(buffer, sizeof(char), file_info -> size_of_file_in, file_info -> ptr_file_in );
    buffer[file_info -> size_of_file_in] = '\0';

    file_info -> size_of_file_in = real_size_file_in;
    return buffer;  
}

int assembling(file_info_t* file_info, const char* filename) {

    char* code = read_from_file(file_info, filename);
    command_processing_t command_info = {};
    command_info.code_arr = (int*)calloc(file_info -> size_of_file_in , sizeof(int));

    if (command_processing(&command_info, code)) return 1;
        command_processing(&command_info, code);

    ON_DBG_ASS(write_commands_in_file(&command_info);)
    write_bin_commands_in_file(&command_info);

    fclose(file_info -> ptr_file_in);
    free(command_info.code_arr);
    return 0;
}

int command_processing(command_processing_t* command_info, char* code) {

    command_info -> current_ptr_code = code;
    command_info -> IP = 0;

    while (sscanf(command_info -> current_ptr_code, "%s%n", command_info -> current_command, &command_info -> len_sscanf) == 1) {         
        command_info -> current_ptr_code += command_info -> len_sscanf; 

        ON_DBG_ASS(printf("%s\n", command_info -> current_command);)

        if (command_info -> current_command[0] == ';') {  //check_ comment
            while (*command_info -> current_ptr_code != '\0' && *command_info -> current_ptr_code != '\n') {
                command_info -> current_ptr_code++;
            }
            continue;
        }
    
        memset(&command_info -> op_code, 0, sizeof(command_info -> op_code)); //reset op_code

        switch(commands_t command = check_command(command_info -> current_command)) { 
            
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
                                set_command_in_array(command_info, command);
                                break;

            case PUSH_COMMAND: 
            case POP_COMMAND:
                                check_push_or_pop_and_do(command_info, command); 
                                break;
        
            case MARK_COMMAND:  set_mark_to_label_arr(command_info);
                                break;

            case JA_COMMAND:   
            case JAE_COMMAND:  
            case JB_COMMAND:    
            case JBE_COMMAND: 
            case JE_COMMAND:      
            case JNE_COMMAND:  
            case JMP_COMMAND:   
                                check_jump_and_jump(command_info, command);
                                break;
                    
            case CALL_COMMAND:  check_jump_and_jump(command_info, command);
                                break;

            default:            return 1;
        }
    }
    
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

    int left = 0;
    int right = AMOUNT_COMMAND - 1;
    int mid = 0;
    int len_command = strlen(current_command);

    qsort(COMMAND_ARRAY, AMOUNT_COMMAND, sizeof(command_info_t), comporator_quick_sort_hash_up);

    while(left <= right) { //equal to bsearch()

        mid  = (left + right) / 2;
        int res = (COMMAND_ARRAY[mid].hash_command - djb_2_hash(current_command, len_command));

        if (res == 0) {
            return COMMAND_ARRAY[mid].command;
        }        
        else if(res > 0) {
            right = mid - 1;
        }
        else {
            left = mid + 1;
        }  
    }
    return ERROR_COMMAND;
}

int check_jump_and_jump(command_processing_t* command_info, commands_t CURRENT_COMMAND) {

    command_info -> op_code.op = CURRENT_COMMAND;
    set_value_to_array_as_bin_field(command_info);
    
    if (sscanf(command_info -> current_ptr_code, " %c%d%n", &command_info -> colon, &command_info -> current_label_to_jump, &command_info -> len_sscanf) != 2) {
        return 1;
    }
    command_info -> current_ptr_code += command_info -> len_sscanf + 1; 
                                
    if (command_info -> colon == ':') { 
        command_info -> code_arr[command_info -> IP] = (command_info -> label)[command_info -> current_label_to_jump];
        command_info -> IP++;
        return 0;
    }
    return 1;

}

int write_number_of_register(command_processing_t* command_info, char* current_register) {

    for (int i = 0; i < NUMBER_REGISTER; i++) {
        if (strcmp(REGISTORS_ARRAY[i].name_registor, current_register) == 0) {
            command_info -> code_arr[command_info -> IP] =  REGISTORS_ARRAY[i].num_registor;
            command_info -> IP++;
            return 0;
        }
    }
    return 1;
}

void set_command_in_array(command_processing_t* command_info, commands_t CURRENT_COMMAND) {

    command_info -> op_code.op = CURRENT_COMMAND; 
    set_value_to_array_as_bin_field(command_info);
}

int check_push_or_pop_and_do(command_processing_t* command_info, commands_t CURRENT_COMMAND) {

    command_info -> op_code.op = CURRENT_COMMAND; 
    if (sscanf(command_info -> current_ptr_code, "%s%n", command_info -> current_push_argument, &command_info -> len_sscanf) != 1) {
        return 1;
    }
    command_info -> current_ptr_code += command_info -> len_sscanf + 1;

    char reg[MAXLEN_REGISTER] = {};
    int constanta = 0;

    if (sscanf(command_info -> current_push_argument, "%d", &constanta) == 1 && CURRENT_COMMAND == PUSH_COMMAND) { 
        command_info -> op_code.konst = 1; 
        set_value_to_array_as_bin_field(command_info);
        command_info -> code_arr[command_info -> IP] = constanta;
        command_info -> IP ++;
        return 0;
    }

    if (command_info -> current_push_argument[0] == '[') {
        if (sscanf(command_info -> current_push_argument, "[%3[A-Z]]", reg) == 1) {
            command_info -> op_code.mem = 1; 
        }
    }
    else {
        if (sscanf(command_info -> current_push_argument, "%3[A-Z]", reg) == 1) {
            command_info -> op_code.reg = 1;   
        }
        else {
            return 1;
        }
    }
    set_value_to_array_as_bin_field(command_info);

    if (write_number_of_register(command_info, reg) != 0) {
        return 1;
    };

    return 0;
}

uint32_t djb_2_hash(const char* str, unsigned int len) {
    
    uint32_t hash = 5381;

    for (unsigned int i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + *(str + i);
    }
    return hash; 
}

int set_mark_to_label_arr(command_processing_t* command_info) {

    if (sscanf(command_info -> current_command, "%c%d",  &command_info -> colon, &command_info -> set_mark) != 2) {
        return 1;
    }
    command_info -> label[command_info -> set_mark] = command_info -> IP;
    return 0;
}


void set_value_to_array_as_bin_field(command_processing_t* command_info) {

    int command_to_out = command_info -> op_code.mem   << 7 | 
                         command_info -> op_code.reg   << 6 | 
                         command_info -> op_code.konst << 5 | 
                         command_info -> op_code.op;

    command_info -> code_arr[command_info -> IP] = command_to_out;
    command_info -> IP++;
}
