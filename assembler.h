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