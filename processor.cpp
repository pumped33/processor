#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <unistd.h>  
#include "stack_LIFO.cpp"
#include "raylib.h" 

#include "enum_commands.h"
#include "read_from_file.cpp"
#include "processor.h"

#undef DEBUG_MODE

#define TERM_GREEN  "\033[32m"
#define TERM_BLUE   "\033[34m"
#define TERM_RED    "\033[31m"
#define TERM_RESET  "\033[37m"

// #define DEBUG_PROCESSOR_MODE

#ifdef DEBUG_PROCESSOR_MODE
    #define ON_DBG_PROC(...) __VA_ARGS__
#else
    #define ON_DBG_PROC(...) 
#endif      

int main(int argc, const char* argv[]) {

    if (argc == 2) {
        processor_stack_t processor_stack = {};
        execute_commands(&processor_stack, argv[1]);
        
        return 0;
    }
    return 1;
}

int* read_from_file(file_info_t* file_info, const char* filename) {

    open_file(file_info, filename);
    size_of_file(file_info);
    
    int* buffer = (int*)calloc((size_t)(file_info -> size_of_file_in), sizeof(int));
    
    file_info -> size_of_file_in = fread(buffer, sizeof(int), file_info -> size_of_file_in, file_info -> ptr_file_in);

    buffer = (int*)realloc(buffer, file_info -> size_of_file_in * sizeof(int));

    fclose(file_info -> ptr_file_in);
    return buffer;
}

int execute_commands(processor_stack_t* processor_stack, const char* filename) { 

    file_info_t file_info = {};
    processor_stack -> buffer = read_from_file(&file_info, filename);

    stack_info_t stack_info = {};
    stack_init(&stack_info, CAPACITY_STACK_CALCULATIONS); 

    stack_init(&(processor_stack -> stack_func_memory), CAPACITY_STACK_CALL);
    processor_stack -> ram = (int*)calloc(MAX_SIZE_RAM, sizeof(int));

    int flag_buffer_not_end = 1;
    while (flag_buffer_not_end) {

        switch (processor_stack -> buffer[processor_stack -> IP] & LOWER_FIVE_BITS) {

            case PUSH_COMMAND:  push(&stack_info, processor_stack);
                                break;

            case ADD_COMMAND:   add(&stack_info);
                                break;

            case SUB_COMMAND:   sub(&stack_info);
                                break; 

            case DIV_COMMAND:   if (div(&stack_info))  flag_buffer_not_end = 0;
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

            case JMP_COMMAND:   jmp(processor_stack, processor_stack -> buffer[processor_stack -> IP + 1]);
                                break;

            case JA_COMMAND:    ja(&stack_info, processor_stack, processor_stack -> buffer[processor_stack -> IP + 1]);
                                break;

            case JAE_COMMAND:   jae(&stack_info, processor_stack, processor_stack -> buffer[processor_stack -> IP + 1]);
                                break;

            case JB_COMMAND:    jb(&stack_info, processor_stack, processor_stack -> buffer[processor_stack -> IP + 1]);
                                break;

            case JBE_COMMAND:   jbe(&stack_info, processor_stack, processor_stack ->buffer[processor_stack -> IP + 1]);
                                break;

            case JE_COMMAND:    je(&stack_info, processor_stack, processor_stack -> buffer[processor_stack -> IP + 1]);
                                break;

            case JNE_COMMAND:   jne(&stack_info, processor_stack, processor_stack -> buffer[processor_stack -> IP + 1]);
                                break;
                    
            case IN_COMMAND:    in(&stack_info);   
                                break;

            case CALL_COMMAND:  call(processor_stack, processor_stack -> buffer[processor_stack -> IP + 1]);
                                break;
        
            case RET_COMMAND:   ret(processor_stack);
                                break;

            case SQRT_COMMAND:  squ1rt(&stack_info);
                                break;
                        
            case DRAWC_COMMAND: drawc(processor_stack);
                                break;

            case DRAWW_COMMAND: draww(processor_stack);
                                break;
            
            default:            return 1;
        }
        processor_stack -> IP ++;
        ON_DBG_PROC(dump(processor_stack, &file_info);)
    }
    return 0;
}


void push(stack_info_t* stack_info, processor_stack_t* processor_stack) { // todo one func

    int* src = get_adress_of_arg(stack_info, processor_stack);
    stack_push(stack_info, *src);
}

void pop(stack_info_t* stack_info, processor_stack_t* processor_stack) {

    int x = 0;
    stack_pop(stack_info, &x);

    int* dest = get_adress_of_arg(stack_info, processor_stack);
    *dest = x;
}

int* get_adress_of_arg(stack_info_t* stack_info, processor_stack_t* processor_stack) {

    if (processor_stack -> buffer[processor_stack -> IP] & SIXTH_BIT) {
        processor_stack -> IP++;
        return processor_stack -> buffer + processor_stack -> IP;
    }

    if (processor_stack -> buffer[processor_stack -> IP] & SEVENTH_BIT) {
        processor_stack -> IP++;
        return processor_stack -> register_array + processor_stack -> buffer[processor_stack -> IP] - 1;
    }

    if (processor_stack -> buffer[processor_stack -> IP] & EIGHTH_BIT) {
        processor_stack -> IP++;
        return processor_stack -> ram + processor_stack -> register_array[processor_stack -> buffer[processor_stack -> IP] - 1];
    }

    return NULL;
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

int div(stack_info_t* stack_info) {

    int x1 = 0;
    stack_pop(stack_info, &x1);

    int x2 = 0;
    stack_pop(stack_info, &x2);

    if (x1 == 0){
        return 1;
    }

    stack_push(stack_info, x2 / x1);
}

void out(stack_info_t* stack_info) {

    int x = 0;
    stack_pop(stack_info, &x);

    printf("%d\n", x);
}

void hlt(stack_info_t* stack_info) {
    stack_destroy(stack_info);
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
        jmp(processor_stack, adress_to_jump);    
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

void drawc(processor_stack_t* processor_stack) {

    for (int i = 0; i < sqrt(MAX_SIZE_RAM); i++) {
        for (int j = 0; j < sqrt(MAX_SIZE_RAM); j++) {
            printf("%c ", processor_stack -> ram [i * (int)sqrt(MAX_SIZE_RAM) + j]);
        }
        printf("\n");
    }
}

void draww(processor_stack_t* processor_stack) {
    
    const int block_of_pixels = 10; 

    InitWindow(sqrt(MAX_SIZE_RAM) * 10 + 10, sqrt(MAX_SIZE_RAM) * 10 + 10, "My picture");

    while (!WindowShouldClose()) {
        BeginDrawing();
        
        ClearBackground(BLACK);
        for (int i = 0; i < sqrt(MAX_SIZE_RAM); i++) {
            for (int j = 0; j < sqrt(MAX_SIZE_RAM); j++) {
                unsigned char color = processor_stack -> ram [i * (int)sqrt(MAX_SIZE_RAM) + j];
                Color myColor = {color, color, color, 255};
                DrawRectangle(j * block_of_pixels - block_of_pixels / 2, i * block_of_pixels - block_of_pixels / 2,
                              j * block_of_pixels + block_of_pixels / 2, i * block_of_pixels + block_of_pixels / 2, 
                              myColor);
            }
        }
        EndDrawing();
    }
    CloseWindow();
}

void dump(processor_stack_t* processor_stack, file_info_t* file_info) {

    printf(TERM_GREEN  "Name of exe file: %s\n", file_info -> name_file_in);

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

    for (int i = 0; i < file_info -> size_of_file_in ; i++) {
        if (i == processor_stack -> IP) {
            printf(TERM_RED "%2d " TERM_RESET, processor_stack -> buffer[i]);
        }
        else {
            printf(TERM_RESET "%2d ", processor_stack -> buffer[i]);
        }
    }
    printf("\n");

    for (int i = 0; i < file_info -> size_of_file_in; i++) {
        printf("%2d ", i);
    }
    printf("\n");

    printf(TERM_RESET "--------------------------\n");
    getchar();
    
}


