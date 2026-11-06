const int NUMBER_REGISTERS            = 4;
const int CAPACITY_STACK_CALCULATIONS = 10;
const int MAX_SIZE_RAM                = 2601;
const int CAPACITY_STACK_CALL =         10;

struct processor_stack_t {

    int*         buffer =                           NULL;
    int          IP =                               0;
    int          register_array[NUMBER_REGISTERS] = {};
    stack_info_t stack_func_memory =                {};
    int          last_buffer_index =                0;
    int*         ram =                              NULL;
};

//read_file_from_assembler
int  size_of_file       (file_info_t* file_info);
int* read_from_file     (file_info_t* file_info, const char* filename);
void open_file          (file_info_t* file_info);


int execute_commands    (processor_stack_t* processor_stack, const char* filename);
//processor command
void hlt                (stack_info_t* stack_info);
void out                (stack_info_t* stack_info);
void add                (stack_info_t* stack_info);
void sub                (stack_info_t* stack_info);
int  div                (stack_info_t* stack_info);
void push               (stack_info_t* stack_info, processor_stack_t* processor_stack);
void mul                (stack_info_t* stack_info);
void popr               (stack_info_t* stack_info, processor_stack_t* processor_stack, int register_number);
void pushr              (stack_info_t* stack_info, processor_stack_t* processor_stack, int register_number);
void ja                 (stack_info_t* stack_info, processor_stack_t* processor_stack, int adress_to_jump);
void jae                (stack_info_t* stack_info, processor_stack_t* processor_stack, int adress_to_jump);
void jb                 (stack_info_t* stack_info, processor_stack_t* processor_stack, int adress_to_jump);
void jbe                (stack_info_t* stack_info, processor_stack_t* processor_stack, int adress_to_jump);
void je                 (stack_info_t* stack_info, processor_stack_t* processor_stack, int adress_to_jump);
void jne                (stack_info_t* stack_info, processor_stack_t* processor_stack, int adress_to_jump);
void jmp                (processor_stack_t* processor_stack, int adress_to_jump);
void in                 (stack_info_t* stack_info);
void call               (processor_stack_t* processor_stack, int adress_jump);
void ret                (processor_stack_t* processor_stack);
void squ1rt             (stack_info_t* stack_info);
void power              (stack_info_t* stack_info);
void pushm              (stack_info_t* stack_info, processor_stack_t* processor_stack);
void popm               (stack_info_t* stack_info, processor_stack_t* processor_stack);
void drawc              (processor_stack_t* processor_stack);
void draww              (processor_stack_t* processor_stack);
void pop                (stack_info_t* stack_info, processor_stack_t* processor_stack);
int* get_adress_of_arg  (stack_info_t* stack_info, processor_stack_t* processor_stack);
// dump
void dump               (processor_stack_t* processor_stack, file_info_t* file_info);