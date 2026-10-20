#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <unistd.h>  
#include "stack_test.cpp"

#undef DEBUG_MODE

#define GREEN "\033[32m"
#define BLUE  "\033[34m"
#define RED   "\033[31m"
#define RESET "\033[37m"

const int NUMBER_REGISTERS             = 4;
const int CAPACITY_STACK_CALCULATIONS = 10;
const int MAX_SIZE_RAM                = 2601;
const int CAPACITY_STACK_CALL =       10;


// #define DEBUG_PROCESSOR_MODE

#ifdef DEBUG_PROCESSOR_MODE
    #define ON_DBG_PROC(...) __VA_ARGS__
#else
    #define ON_DBG_PROC(...) 
#endif

#include "enum_commands.h"

struct file_info_t {
    const char* name_file_in = "";
    int last_buffer_index = 0;
    FILE* ptr_file_in = NULL;
    commands_t command_type = ERROR_COMMAND; 
    int size_of_file_in = 0;
};

struct processor_stack_t {

    int* buffer = NULL;
    int IP = 0;
    int register_array[NUMBER_REGISTERS] = {};
    stack_info_t stack_func_memory = {};
    int last_buffer_index = 0;
    int* ram = NULL;
};

//read_file_from_assembler
int size_of_file(file_info_t* file_info);
int* open_file(file_info_t* file_info, const char* filename);


void execute_commands(int* buffer, processor_stack_t* processor_stack ON_DBG_PROC(, file_info_t* file_info));
//processor command
void hlt(stack_info_t* stack_info);
void out(stack_info_t* stack_info);
void add(stack_info_t* stack_info);
void sub(stack_info_t* stack_info);
void div(stack_info_t* stack_info);
void push(stack_info_t* stack_info, processor_stack_t* processor_stack);
void mul(stack_info_t* stack_info);
void popr(stack_info_t* stack_info, processor_stack_t* processor_stack, int register_number);
void pushr(stack_info_t* stack_info, processor_stack_t* processor_stack, int register_number);
void ja(stack_info_t* stack_info, processor_stack_t* processor_stack, int adress_to_jump);
void jae(stack_info_t* stack_info, processor_stack_t* processor_stack, int adress_to_jump);
void jb(stack_info_t* stack_info, processor_stack_t* processor_stack, int adress_to_jump);
void jbe(stack_info_t* stack_info, processor_stack_t* processor_stack, int adress_to_jump);
void je(stack_info_t* stack_info, processor_stack_t* processor_stack, int adress_to_jump);
void jne(stack_info_t* stack_info, processor_stack_t* processor_stack, int adress_to_jump);
void jmp(processor_stack_t* processor_stack, int adress_to_jump);
void in(stack_info_t* stack_info);
void call(processor_stack_t* processor_stack, int adress_jump);
void ret(processor_stack_t* processor_stack);
void squ1rt(stack_info_t* stack_info);
void power(stack_info_t* stack_info);
void pushm(stack_info_t* stack_info, processor_stack_t* processor_stack);
void popm(stack_info_t* stack_info, processor_stack_t* processor_stack);
void draw(processor_stack_t* processor_stack);
void pop(stack_info_t* stack_info, processor_stack_t* processor_stack);


void dump(processor_stack_t* processor_stack, file_info_t* file_info);


int main(int argc, const char* argv[]) {

    if (argc == 2) {
        file_info_t file_info = {};
        int* code = open_file(&file_info, argv[1]);
        processor_stack_t processor_stack = {};
        processor_stack.last_buffer_index = file_info.last_buffer_index;

        execute_commands(code, &processor_stack ON_DBG_PROC(, &file_info));
    }
    else {
        exit(1);
    }
}

int* open_file(file_info_t* file_info, const char* filename) {

    file_info -> name_file_in = filename;
    file_info -> ptr_file_in  = fopen(filename, "r");
    size_of_file(file_info);
    int* buffer = (int*)calloc((size_t)(file_info -> size_of_file_in), sizeof(int));
    int elem = 0;
    while(fscanf(file_info -> ptr_file_in,  "%d", &elem) == 1) {
        buffer[file_info -> last_buffer_index] = elem;
        file_info -> last_buffer_index++;
    }
    buffer = (int*)realloc(buffer, (file_info -> last_buffer_index + 1) * sizeof(int));

    return buffer;
}

int size_of_file(file_info_t* file_info) {

    struct stat stat_file_in;
    stat(file_info -> name_file_in, &stat_file_in);
    file_info -> size_of_file_in = stat_file_in.st_size;
    return 0;
}

void execute_commands(int* buffer, processor_stack_t* processor_stack ON_DBG_PROC(, file_info_t* file_info)) { 

    stack_info_t stack_info = {};
    stack_init(&stack_info, CAPACITY_STACK_CALCULATIONS); 

    processor_stack -> buffer = buffer;
    stack_init(&(processor_stack -> stack_func_memory), CAPACITY_STACK_CALL);
    processor_stack -> ram = (int*)calloc(MAX_SIZE_RAM, sizeof(int));


    int flag_buffer_not_end = 1;

    while (flag_buffer_not_end) {

        if (processor_stack -> last_buffer_index < processor_stack -> IP) {
            exit(3);
        }

        switch (buffer[processor_stack -> IP] & 0b00011111) {

            case PUSH_COMMAND:  push(&stack_info, processor_stack);
                                break;

            case ADD_COMMAND:   add(&stack_info);
                                break;

            case SUB_COMMAND:   sub(&stack_info);
                                break; 

            case DIV_COMMAND:   div(&stack_info);
                                break;

            case OUT_COMMAND:   out(&stack_info);
                                break;

            case HLT_COMMAND:   hlt(&stack_info);
                                flag_buffer_not_end = 0;
                                break;

            case POP_COMMAND:   pop(&stack_info, processor_stack);
                                break;

            case MUL_COMMAND:   mul(&stack_info);
                                break;

            case JMP_COMMAND:   jmp(processor_stack, buffer[processor_stack -> IP + 1]);
                                break;

            case JA_COMMAND:    ja(&stack_info, processor_stack, buffer[processor_stack -> IP + 1]);
                                break;

            case JAE_COMMAND:   jae(&stack_info, processor_stack, buffer[processor_stack -> IP + 1]);
                                break;

            case JB_COMMAND:    jb(&stack_info, processor_stack, buffer[processor_stack -> IP + 1]);
                                break;

            case JBE_COMMAND:   jbe(&stack_info, processor_stack, buffer[processor_stack -> IP + 1]);
                                break;

            case JE_COMMAND:   je(&stack_info, processor_stack, buffer[processor_stack -> IP + 1]);
                                break;

            case JNE_COMMAND:   jne(&stack_info, processor_stack, buffer[processor_stack -> IP + 1]);
                                break;
                    
            case IN_COMMAND:    in(&stack_info);   
                                break;

            case CALL_COMMAND:  call(processor_stack, processor_stack -> buffer[processor_stack -> IP + 1]);
                                break;
        
            case RET_COMMAND:   ret(processor_stack);
                                break;

            case SQRT_COMMAND:  squ1rt(&stack_info);
                                break;
                        
            case DRAW_COMMAND:  draw(processor_stack);
                                break;
            
            default:            exit(4);
        }
        processor_stack -> IP ++;

        ON_DBG_PROC(dump(processor_stack, file_info);)
    }
}

void push(stack_info_t* stack_info, processor_stack_t* processor_stack) {

    if ((processor_stack -> buffer[processor_stack -> IP] >> 5 & 1) == 1) {
        processor_stack -> IP += 1;
        stack_push(stack_info, processor_stack -> buffer[processor_stack -> IP]);
        return;
    }

    if ((processor_stack -> buffer[processor_stack -> IP] >> 6 & 1) == 1) {
        processor_stack -> IP += 1;
        pushr(stack_info, processor_stack, processor_stack -> buffer[processor_stack -> IP]);
        return;
    }

    if ((processor_stack -> buffer[processor_stack -> IP] >> 7 & 1) == 1) {
        pushm(stack_info, processor_stack);
        return;
    }
    
}
void pop(stack_info_t* stack_info, processor_stack_t* processor_stack) {

    if ((processor_stack -> buffer[processor_stack -> IP] >> 6 & 1) == 1) {
        processor_stack -> IP += 1;
        popr(stack_info, processor_stack, processor_stack -> buffer[processor_stack -> IP]);
        return;
    }

    if ((processor_stack -> buffer[processor_stack -> IP] >> 7 & 1) == 1) {
        popm(stack_info, processor_stack);
        return;
    } 
}

void add(stack_info_t* stack_info) {

    int x1 = 0;
    stack_pop(stack_info, &x1);

    int x2 = 0;
    stack_pop(stack_info, &x2);

    stack_push(stack_info, x1 + x2);
}

void sub(stack_info_t* stack_info) {

    int x1 = 0;
    stack_pop(stack_info, &x1);

    int x2 = 0;
    stack_pop(stack_info, &x2);

    stack_push(stack_info, x2 - x1);
}

void div(stack_info_t* stack_info) {

    int x1 = 0;
    stack_pop(stack_info, &x1);

    int x2 = 0;
    stack_pop(stack_info, &x2);

    if (x1 == 0){
        exit(1);
    }

    stack_push(stack_info, x2 / x1);
}

void out(stack_info_t* stack_info) {

    int x = 0;
    stack_pop(stack_info, &x);

    printf("%d\n", x);
}

void hlt(stack_info_t* stack_info) {
    // stack_destroy(stack_info);
    exit(0);
}

void popr(stack_info_t* stack_info, processor_stack_t* processor_stack, int register_number) {

    int x = 0;
    stack_pop(stack_info, &x);
    (processor_stack -> register_array)[register_number - 1] = x;
}

void pushr(stack_info_t* stack_info, processor_stack_t* processor_stack, int register_number) {

    int x =  *(processor_stack -> register_array + register_number - 1);
    stack_push(stack_info, x);
}

void in(stack_info_t* stack_info) {

    int x = 0;
    scanf("%d", &x);
    stack_push(stack_info, x);
    
}

void mul(stack_info_t* stack_info) {

    int x1 = 0;
    stack_pop(stack_info, &x1);

    int x2 = 0;
    stack_pop(stack_info, &x2);

    stack_push(stack_info, x1 * x2);
}

void jmp(processor_stack_t* processor_stack, int adress_to_jump) {

    processor_stack -> IP = adress_to_jump - 1;
}

void ja(stack_info_t* stack_info, processor_stack_t* processor_stack, int adress_to_jump) {

    int x1 = 0;
    stack_pop(stack_info, &x1);

    int x2 = 0;
    stack_pop(stack_info, &x2);

    if (x1 < x2) {
        jmp(processor_stack, adress_to_jump);    
    }
    else {
        processor_stack -> IP++;
    }
}

void jae(stack_info_t* stack_info, processor_stack_t* processor_stack, int adress_to_jump) {

    int x1 = 0;
    stack_pop(stack_info, &x1);

    int x2 = 0;
    stack_pop(stack_info, &x2);

    if (x1 <= x2) {
        jmp(processor_stack, adress_to_jump - 1);    
    }
    else {
        processor_stack -> IP++;
    }
}

void jb(stack_info_t* stack_info, processor_stack_t* processor_stack, int adress_to_jump) {

    int x1 = 0;
    stack_pop(stack_info, &x1);

    int x2 = 0;
    stack_pop(stack_info, &x2);

    if (x1 > x2) {
        jmp(processor_stack, adress_to_jump);    
    }
    else {
        processor_stack -> IP++;
    }
}

void jbe(stack_info_t* stack_info, processor_stack_t* processor_stack, int adress_to_jump) {

    int x1 = 0;
    stack_pop(stack_info, &x1);

    int x2 = 0;
    stack_pop(stack_info, &x2);

    if (x1 >= x2) {
        jmp(processor_stack, adress_to_jump);    
    }
    else {
        processor_stack -> IP++;
    }
}

void je(stack_info_t* stack_info, processor_stack_t* processor_stack, int adress_to_jump) {

    int x1 = 0;
    stack_pop(stack_info, &x1);

    int x2 = 0;
    stack_pop(stack_info, &x2);

    if (x1 == x2) {
        jmp(processor_stack, adress_to_jump);    
    }
    else {
        processor_stack -> IP++;
    }
}


void jne(stack_info_t* stack_info, processor_stack_t* processor_stack, int adress_to_jump) {

    int x1 = 0;
    stack_pop(stack_info, &x1);

    int x2 = 0;
    stack_pop(stack_info, &x2);

    if (x1 != x2) {
        jmp(processor_stack, adress_to_jump);    
    }
    else {
        processor_stack -> IP++;
    }
}

void call(processor_stack_t* processor_stack, int adress_jump) {

    stack_push(&(processor_stack -> stack_func_memory),processor_stack -> IP + 2);
    jmp(processor_stack, adress_jump);
}

void ret(processor_stack_t* processor_stack) {

    int new_IP = 0;
    stack_pop(&(processor_stack -> stack_func_memory), &new_IP);
    processor_stack -> IP = new_IP - 1;
}

void squ1rt(stack_info_t* stack_info) {
    
    int x = 0;
    stack_pop(stack_info, &x);

    if (x < 0) {
        exit(4);
    }

    stack_push(stack_info, sqrt(x));
} 

void popm(stack_info_t* stack_info, processor_stack_t* processor_stack) {

    int x = 0;
    stack_pop(stack_info, &x);

    processor_stack -> IP++;
    processor_stack -> ram[processor_stack -> register_array[processor_stack -> buffer[processor_stack -> IP] - 1]] = x;
}

void pushm(stack_info_t* stack_info, processor_stack_t* processor_stack) {

    processor_stack -> IP++;

    int x = 0;
    x = processor_stack -> ram[processor_stack -> register_array[processor_stack -> buffer[processor_stack -> IP] - 1]];
    stack_push(stack_info, x);
}

void draw(processor_stack_t* processor_stack) {

    for (int i = 0; i < sqrt(MAX_SIZE_RAM); i++) {
        for (int j = 0; j < sqrt(MAX_SIZE_RAM); j++) {
            printf("%c", processor_stack -> ram [i * (int)sqrt(MAX_SIZE_RAM) + j]);
        }
        printf("\n");
    }
}

void dump(processor_stack_t* processor_stack, file_info_t* file_info) {

    printf(GREEN "Name of exe file: %s\n", file_info -> name_file_in);

    printf("REGISTER ARRAY:\n");
    for (int i = 0; i < NUMBER_REGISTERS; i++) {
        printf("\t%CX  = %d\n", 'A' + i,  processor_stack -> register_array[i]);
    }

    printf("code array = %p\n", processor_stack -> buffer);
    if (processor_stack -> buffer[processor_stack -> IP] <= 16 && processor_stack -> buffer[processor_stack -> IP] >= 10) {
        printf("I want to jump: wait command to do: %d, current IP = %d. Adress if I jump = %d\n", processor_stack -> buffer[processor_stack -> IP], processor_stack -> IP,processor_stack -> IP + 1);
    }
    else {
        printf("wait command to do: %d, current IP = %d\n", processor_stack -> buffer[processor_stack -> IP], 
        processor_stack -> IP);
    }

    for (int i = 0; i <= file_info -> last_buffer_index; i++) {
        if (i == processor_stack -> IP) {
            printf(RED "%d " RESET, processor_stack -> buffer[i]);
        }
        else {
            printf(RESET "%d ", processor_stack -> buffer[i]);
        }
    }
    printf("\n\n");


    // for (int i = 0; i < MAX_SIZE_RAM; i++) {
    //     printf(RESET "%d ", processor_stack -> ram[i]);
    // }
    // printf("\n");

    printf(RESET "--------------------------\n");
    getchar();
    
}