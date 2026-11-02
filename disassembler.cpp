#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>  


#include "enum_commands.h"
#include "comporator_qsort_enum_up.cpp"
#include "read_from_file.cpp"


int* read_from_file   (file_info_t* file_info, const char* filename);
int  disassembler     (int* buffer, file_info_t* file_info);
int  my_bsearch(int current_command);


int main(int argc, const char* argv[]) {

    if (argc == 2) {
        file_info_t file_info = {};
        int* code = read_from_file(&file_info, argv[1]);

        disassembler(code, &file_info);
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

int disassembler(int* buffer, file_info_t* file_info) {

    int IP = 0;

    FILE* f_disassembler_out = fopen("disassembler_out.txt", "w");

    qsort(COMMAND_ARRAY, AMOUNT_COMMAND, sizeof(command_info_t), comporator_quick_sort_enum_up);

    // for (int i = 0; i < AMOUNT_COMMAND; i++ ) {
    //     printf("%s\n", COMMAND_ARRAY[i].name_command);
    // }
    while(IP < file_info -> size_of_file_in) {

        int command = buffer[IP] & LOWER_FIVE_BITS;
        int index_in_command_array = my_bsearch(command);

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
                                fprintf(f_disassembler_out, "%s\n", COMMAND_ARRAY[index_in_command_array].name_command);
                                break;

            case PUSH_COMMAND:  fprintf(f_disassembler_out, "%s ", COMMAND_ARRAY[index_in_command_array].name_command);
                                if (buffer[IP] & SIXTH_BIT) {
                                    IP++;
                                    fprintf(f_disassembler_out, "%d\n", buffer[IP]);
                                    break;
                                }
                                if (buffer[IP] & SEVENTH_BIT) {
                                    IP++;
                                    fprintf(f_disassembler_out, "%s\n", REGISTORS_ARRAY[buffer[IP] - 1].name_registor);
                                    break;
                                }
                                if (buffer[IP] & EIGHTH_BIT) {
                                    IP++;
                                    fprintf(f_disassembler_out, "[%d]\n", buffer[IP]);
                                    break;
                                }

            case POP_COMMAND:   fprintf(f_disassembler_out, "%s ", COMMAND_ARRAY[index_in_command_array].name_command);
                                if (buffer[IP] & SEVENTH_BIT) {
                                    IP++;
                                    fprintf(f_disassembler_out, "%s\n", REGISTORS_ARRAY[buffer[IP] - 1].name_registor);
                                    break;
                                }
                                if (buffer[IP] & EIGHTH_BIT) {
                                    IP++;
                                    fprintf(f_disassembler_out, "[%d]\n", buffer[IP]);
                                    break;
                                }

            case JA_COMMAND:   
            case JAE_COMMAND:  
            case JB_COMMAND:    
            case JBE_COMMAND: 
            case JE_COMMAND:      
            case JNE_COMMAND:  
            case JMP_COMMAND:   IP++;
                                fprintf(f_disassembler_out, "%s <%d>\n", COMMAND_ARRAY[index_in_command_array].name_command, buffer[IP]);
                                break;
                    
            case CALL_COMMAND:  IP++;
                                fprintf(f_disassembler_out, "%s <%d>\n", COMMAND_ARRAY[index_in_command_array].name_command, buffer[IP]);
                                break;

                        
            default:           
                                return 1;
        }
        
        IP++;
    }

    fclose(f_disassembler_out);
    return 0;
}

int my_bsearch(int current_command) { 

    int left = 0;
    int right = AMOUNT_COMMAND - 1;
    int mid = 0;

    while(left <= right) { //equal to bsearch()

        mid  = (left + right) / 2;
        int res = (COMMAND_ARRAY[mid].command - current_command);

        if (res == 0) {
            return mid;
        }        
        else if(res > 0) {
            right = mid - 1;
        }
        else {
            left = mid + 1;
        }  
    }
    return 0;
}

