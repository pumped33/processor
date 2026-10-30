#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>  


#include "enum_commands.h"
#include "comporator_qsort_up.cpp"
#include "read_from_file.cpp"

struct disassembler_t {
    int IP = 0;
    int len_buffer = 0;
};

int* read_from_file(file_info_t* file_info, const char* filename);
int disassembler(int* buffer, disassembler_t* disassembler_info);


int main(int argc, const char* argv[]) {

    if (argc == 2) {
        file_info_t file_info = {};
        int* code = read_from_file(&file_info, argv[1]);

        disassembler_t disassembler_info = {};
        disassembler_info.len_buffer = file_info.size_of_file_in;

        disassembler(code, &disassembler_info);
        return 0;
    }
    else {
        return 1;
    }
}

int* read_from_file(file_info_t* file_info, const char* filename) {

    open_file(file_info, filename); 
    size_of_file(file_info);

    int* buffer = (int*)calloc((size_t)(file_info -> size_of_file_in), sizeof(int));

    file_info -> size_of_file_in = fread(buffer, sizeof(int), file_info -> size_of_file_in, file_info -> ptr_file_in);
    buffer = (int*)realloc(buffer, file_info -> size_of_file_in  * sizeof(int));

    return buffer;
}

int disassembler(int* buffer, disassembler_t* disassembler_info) {

    FILE* f_disassembler_out = fopen("Disassembler_out.txt", "w");
    qsort(COMMAND_ARRAY, AMOUNT_COMMAND, sizeof(command_info_t), comporator_quick_sort);

    while(disassembler_info -> IP != disassembler_info -> len_buffer) {

        int command = (buffer[disassembler_info -> IP] & LOWER_FIVE_BITS);
        switch(command) {
        
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
                                fprintf(f_disassembler_out, "%s\n", COMMAND_ARRAY[command].name_command);
                                break;

            case PUSH_COMMAND:  fprintf(f_disassembler_out, "%s ", COMMAND_ARRAY[command].name_command);
                                if (buffer[disassembler_info -> IP] & SIXTH_BIT) {
                                    disassembler_info -> IP++;
                                    fprintf(f_disassembler_out, "%d\n", buffer[disassembler_info -> IP]);
                                break;
                                }
                                if (buffer[disassembler_info -> IP] & SEVENTH_BIT) {
                                    disassembler_info -> IP++;
                                    fprintf(f_disassembler_out, "%s\n", REGISTORS_ARRAY[buffer[disassembler_info -> IP] - 1].name_registor);
                                break;
                                }
                                if (buffer[disassembler_info -> IP] & EIGHTH_BIT) {
                                    disassembler_info -> IP++;
                                    fprintf(f_disassembler_out, "[%d]\n", buffer[disassembler_info -> IP]);
                                break;
                                }

            case POP_COMMAND:   fprintf(f_disassembler_out, "%s ", COMMAND_ARRAY[command].name_command);
                                if (buffer[disassembler_info -> IP] & SEVENTH_BIT) {
                                    disassembler_info -> IP++;
                                    fprintf(f_disassembler_out, "%s\n", REGISTORS_ARRAY[buffer[disassembler_info -> IP] - 1].name_registor);
                                break;
                                }
                                if (buffer[disassembler_info -> IP] & EIGHTH_BIT) {
                                    disassembler_info -> IP++;
                                    fprintf(f_disassembler_out, "[%d]\n", buffer[disassembler_info -> IP]);
                                break;
                                }

            case JA_COMMAND:   
            case JAE_COMMAND:  
            case JB_COMMAND:    
            case JBE_COMMAND: 
            case JE_COMMAND:      
            case JNE_COMMAND:  
            case JMP_COMMAND:   disassembler_info -> IP++;
                                fprintf(f_disassembler_out, "%s <%d>\n", COMMAND_ARRAY[command].name_command, buffer[disassembler_info -> IP]);
                                break;
                    
            case CALL_COMMAND:  disassembler_info -> IP++;
                                fprintf(f_disassembler_out, "%s <%d>\n", COMMAND_ARRAY[command].name_command, buffer[disassembler_info -> IP]);
                                break;

                        
            default:            return 1;
                                break;
        }
        disassembler_info -> IP++;
    }
    return 0;
}