#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>


#define GREEN "\033[32m"
#define BLUE  "\033[34m"
#define RED   "\033[31m"
#define RESET "\033[37m"



#define DEBUG_MODE
#define CANARY_MODE
#define HASH_MODE

#define STACK_VERIFY(stack_info)                                   \
do {                                                                \
    stack_verify(stack_info);                                      \
    if ((stack_info -> struct_error) != SUCCESS_FINISH_FUNCTION) { \
        stack_info -> str_code_failure = __LINE__;\
        stack_info ->func_failure = __func__; \
        stack_dump(*stack_info);                                   \
        return stack_info -> struct_error;                          \
    }                                                               \
} while(0);

#ifdef CANARY_MODE
    #define PTR_REALLOC stack_info -> data_with_canaries
#else
    #define PTR_REALLOC stack_info -> real_data
#endif

#ifdef DEBUG_MODE
#define ON_DBG(...) __VA_ARGS__
#else
#define ON_DBG(...) 
#endif

#ifdef CANARY_MODE
#define ON_CANARY(...) __VA_ARGS__
#else
#define ON_CANARY(...) 
#endif

#ifdef HASH_MODE
#define ON_HASH(...) __VA_ARGS__
#else
#define ON_HASH(...) 
#endif    


                                                        


typedef int TYPE_DATA_ELEMENTS;

const int COEF_REALLOC_INCREASE = 2;
const int COEF_REALLOC_REDUCE = 4;

const long long LEFT_CANARY =  0xEBA1CB0E04E10;
const long long RIGHT_CANARY = 0xEBA1DEDA;

const int POISON = 67676767;

enum struct_error_t {

    SUCCESS_FINISH_FUNCTION                = 0,
    INIT_ERROR_LACK_OF_MEMORY              = 1,
    PUSH_ELEMENT_ERROR_LACK_OF_MEMORY      = 2,
    POP_ELEMENT_ERROR_LACK_OF_MEMORY       = 3,

    #ifdef DEBUG_MODE
    ERROR_INCORRECT_CAPACITY               = 4, 
    ERROR_NULL_DATA                        = 5,
    ERROR_SIZE_LESS_THAN_ZERO              = 6, 
    ERROR_NULL_PTR_ON_STRUCT               = 7,
    ERROR_SIZE_MORE_THAN_CAPACITY          = 8, 
    ERROR_SIZE_VALUE_NOT_POISONED          = 9,
    ERROR_POPED_ELEMENT_IS_POISONED        = 10,
    ERROR_STACK_IS_EMPTY_TO_POP            = 15, 
    #endif 

    #ifdef CANARY_MODE
    LEFT_CANARY_DISTRUCTED                 = 11,
    RIGHT_CANARY_DISTRUCTED                = 12, 
    #endif

    #ifdef HASH_MODE 
    HASHES_STRUCT_NOT_MATCH                 = 13,
    HASHES_DATA_NOT_MATCH                   = 14,
    #endif
};

struct stack_info_t {
     
    ON_HASH (
        long long hash_struct;\
        long long hash_data;
    )

    TYPE_DATA_ELEMENTS* real_data;
    ssize_t capacity;
    ssize_t size;

    ON_CANARY (
        TYPE_DATA_ELEMENTS* data_with_canaries;\
        long long left_canary;\
        long long right_canary;\
        )
    
    ON_DBG (
        TYPE_DATA_ELEMENTS last_popped_element;\
        TYPE_DATA_ELEMENTS last_pushed_element;\
        int str_code_failure;\
        const char* func_failure;
        )


    struct_error_t struct_error;
};


struct_error_t stack_init(stack_info_t* stack_info, ssize_t capacity);
struct_error_t stack_push(stack_info_t* stack_info, TYPE_DATA_ELEMENTS push_element);
struct_error_t stack_pop(stack_info_t* stack_info, TYPE_DATA_ELEMENTS* pop_element);
struct_error_t stack_verify(stack_info_t* stack_info); 
void fill_the_array_poison(stack_info_t* stack_info);
struct_error_t check_empty_stack_for_pop(stack_info_t* stack_info);
long long hash_function_for_struct(stack_info_t* stack_info); 
long long hash_function_for_data(stack_info_t* stack_info);
void stack_destroy(stack_info_t* stack_info);
void stack_dump(stack_info_t stack_info);


struct_error_t stack_init(stack_info_t* stack_info, ssize_t capacity) {

    stack_info -> struct_error = SUCCESS_FINISH_FUNCTION;

    void* temp = realloc(PTR_REALLOC, sizeof(TYPE_DATA_ELEMENTS) * capacity ON_CANARY(+ 2 * sizeof(long long)));
    if (temp == NULL) {
        stack_info -> struct_error = INIT_ERROR_LACK_OF_MEMORY;
        ON_DBG(STACK_VERIFY(stack_info))
    }

    #ifdef CANARY_MODE
        stack_info -> data_with_canaries = (TYPE_DATA_ELEMENTS*)temp; \
        stack_info -> real_data = (TYPE_DATA_ELEMENTS*)((char*)temp + sizeof(long long));
    #else
        stack_info -> real_data = (TYPE_DATA_ELEMENTS*)(temp);
    #endif

    stack_info -> capacity = capacity;
    stack_info -> size = 0;

    ON_DBG(fill_the_array_poison(stack_info);\
           stack_info -> last_pushed_element = 0; \
           stack_info -> last_popped_element = 0; \
           stack_info -> str_code_failure = 0;\
           stack_info -> func_failure = NULL;)

    ON_CANARY(stack_info -> left_canary =   LEFT_CANARY;\
              stack_info ->  right_canary = RIGHT_CANARY;\
              *((long long*)stack_info -> data_with_canaries) = stack_info -> left_canary;\
              *((long long*)(stack_info -> real_data + capacity)) = stack_info -> right_canary;)

    ON_HASH(stack_info -> hash_data =  hash_function_for_data(stack_info);)
    ON_HASH(stack_info -> hash_struct = hash_function_for_struct(stack_info);)

    ON_DBG(STACK_VERIFY(stack_info))

    
    return SUCCESS_FINISH_FUNCTION;
}

struct_error_t stack_push(stack_info_t* stack_info, TYPE_DATA_ELEMENTS push_element) {
    
    ON_DBG(STACK_VERIFY(stack_info))

    if ((stack_info -> size) == (stack_info -> capacity)) {
        ON_DBG(printf("The size of stack is not enough. Current capacity = %zu," \
               " increase to capacity = %zu\n", \
               stack_info -> capacity, (stack_info -> capacity) * COEF_REALLOC_INCREASE);)
        
        (stack_info -> capacity) *= COEF_REALLOC_INCREASE;
        void* temp = realloc(PTR_REALLOC, sizeof(TYPE_DATA_ELEMENTS) * (stack_info -> capacity)  ON_CANARY(+ 2 * sizeof(long long)));

        if (temp == NULL) {
            stack_info -> struct_error = PUSH_ELEMENT_ERROR_LACK_OF_MEMORY;
        
            ON_DBG(STACK_VERIFY(stack_info))
  
            return(stack_info -> struct_error);
        }

        #ifdef CANARY_MODE
            stack_info -> data_with_canaries = (TYPE_DATA_ELEMENTS*)temp; \
            stack_info -> real_data = (TYPE_DATA_ELEMENTS*)((char*)temp + sizeof(long long));
        #else
            stack_info -> real_data = (TYPE_DATA_ELEMENTS*)(temp);
        #endif

        ON_DBG(fill_the_array_poison(stack_info);)


        ON_CANARY(*((long long*)(stack_info -> real_data + stack_info -> capacity)) = stack_info -> right_canary;)
    }

    (stack_info -> real_data)[stack_info -> size] = push_element;
    ON_DBG(stack_info -> last_pushed_element = (stack_info -> real_data)[stack_info -> size];)

    (stack_info -> size)++;

    ON_HASH(stack_info -> hash_data   = hash_function_for_data(stack_info);)
    ON_HASH(stack_info -> hash_struct = hash_function_for_struct(stack_info);)

    ON_DBG(STACK_VERIFY(stack_info))
    
    return SUCCESS_FINISH_FUNCTION;
}

struct_error_t stack_pop(stack_info_t* stack_info, TYPE_DATA_ELEMENTS* pop_element) { //todo define it and add in the end of function

    ON_DBG(STACK_VERIFY(stack_info))


            
    (stack_info -> size)--;

    *pop_element = (stack_info -> real_data)[stack_info -> size]; 
    ON_DBG(stack_info -> last_popped_element = (stack_info -> real_data)[stack_info -> size];)
    (stack_info -> real_data)[stack_info -> size] = POISON;
  
    if ((stack_info -> capacity) / COEF_REALLOC_REDUCE == (stack_info -> size) && (stack_info -> capacity) > 4) { 
        stack_info -> capacity /=  COEF_REALLOC_REDUCE;
        ON_DBG(printf("New capacity of data_with_canaries = %zu, size = %zu\n", \
                      stack_info -> capacity, stack_info -> size);)

        void* temp = realloc(PTR_REALLOC, sizeof(TYPE_DATA_ELEMENTS) * (stack_info -> capacity) ON_CANARY(+ 2 * sizeof(long long)));

        if (temp == NULL) {
            stack_info -> struct_error = POP_ELEMENT_ERROR_LACK_OF_MEMORY;
            ON_DBG(stack_info -> str_code_failure = __LINE__; \
                   stack_info -> func_failure = __func__; \
                   stack_verify(stack_info);)

            return(stack_info -> struct_error); 
        }

        #ifdef CANARY_MODE
        stack_info -> data_with_canaries = (TYPE_DATA_ELEMENTS*)temp; \
        stack_info -> real_data = (TYPE_DATA_ELEMENTS*)((char*)temp + sizeof(long long));
        #else
        stack_info -> real_data = (TYPE_DATA_ELEMENTS*)(temp);
        #endif

        ON_CANARY(*((long long*)(stack_info -> real_data + stack_info -> capacity)) = stack_info -> right_canary;)
    }

    ON_HASH(stack_info -> hash_data = hash_function_for_data(stack_info);)
    ON_HASH(stack_info -> hash_struct = hash_function_for_struct(stack_info);)

    ON_DBG(STACK_VERIFY(stack_info))

    return SUCCESS_FINISH_FUNCTION;
}

void stack_destroy(stack_info_t* stack_info) {

    free(stack_info -> real_data);
    
    stack_info -> real_data = NULL;
    stack_info -> capacity = 0;
    stack_info -> size = 0;

    #ifdef CANARY_MODE
    free(stack_info -> data_with_canaries);
    stack_info -> data_with_canaries = NULL;
    stack_info -> left_canary = 0;
    stack_info -> right_canary = 0;
    #endif

    #ifdef DEBUG_MODE 
    stack_info -> last_pushed_element = POISON;
    stack_info -> last_popped_element = POISON;
    #endif

    #ifdef HASH_MODE
    stack_info -> hash_struct = 0;
    stack_info -> hash_data = 0;
    #endif

}

#ifdef DEBUG_MODE
void stack_dump(stack_info_t stack_info) {
    
    printf(BLUE "Print information about stack because of failure № %d:\n"
            "failure code line %d;\n"
            "name function name failure <%s>;\n",
            stack_info.struct_error,
            stack_info.str_code_failure,
            stack_info.func_failure);
    printf(BLUE "capacity = %zd;\n", stack_info.capacity);
    printf(BLUE "size = %zd;\n",     stack_info.size);

    ON_CANARY(printf(BLUE "data_with_canaries = %p\n", stack_info.data_with_canaries);)

    printf(BLUE "real data = %p\n",  stack_info.real_data);

    ON_HASH(printf(BLUE "HASH data = %lld, predict = %lld\n", stack_info.hash_data, hash_function_for_data(&stack_info));
            printf(BLUE "HASH struct = %lld, predict = %lld\n", stack_info.hash_struct, hash_function_for_struct(&stack_info));)

    printf(BLUE "last pushed element = %d\n",  stack_info.last_pushed_element);
    printf(BLUE "last popped element = %d\n",  stack_info.last_popped_element);
    printf(BLUE "real data_with_canaries = %p\n",  stack_info.real_data);

    ON_CANARY(printf(BLUE "Left canary = %llx, Right canary = %llx\n", stack_info.left_canary, stack_info.right_canary);\
             printf(BLUE "Current left canary = %llx, Current right canary = %llx\n" RESET, \
                   *(long long*)stack_info.data_with_canaries, *(long long*)(stack_info.real_data + stack_info.capacity));\
    )
  

    for (ssize_t i = 0; i < stack_info.size; i++) {
        printf(GREEN "\t*[%zd] = %d\n" RESET, i,  stack_info.real_data[i]);
    }

    for (ssize_t i = stack_info.size; i < stack_info.capacity; i++) {
        printf(RESET "\t[%zd] = %d - POIZON\n", i,  stack_info.real_data[i]);   
    }

    printf(RESET "-------------------------------------\n");   
}

struct_error_t stack_verify(stack_info_t* stack_info) { 

    if (stack_info == NULL) {
        printf("Incorrect ptr on struct\n"
              "Code failure = %d\n", ERROR_NULL_PTR_ON_STRUCT);
                                                                                            return ERROR_NULL_PTR_ON_STRUCT;
    } 
    
    if (stack_info -> struct_error != SUCCESS_FINISH_FUNCTION)                              return (stack_info -> struct_error);

    if (stack_info -> real_data == NULL)                                                    return stack_info -> struct_error =  ERROR_NULL_DATA;


    #ifdef CANARY_MODE

        if (stack_info -> data_with_canaries == NULL)                                       return stack_info -> struct_error =  ERROR_NULL_DATA;
        if (*(long long*)(stack_info -> data_with_canaries) != stack_info -> left_canary)   return stack_info -> struct_error =  LEFT_CANARY_DISTRUCTED;
        if (*(long long*)(stack_info -> real_data + stack_info -> capacity) 
            != stack_info -> right_canary)                                                  return stack_info -> struct_error =  RIGHT_CANARY_DISTRUCTED;

    #endif

    #ifdef HASH_MODE
        if (hash_function_for_struct(stack_info) != stack_info -> hash_struct)              return stack_info -> struct_error = HASHES_STRUCT_NOT_MATCH;

        if (hash_function_for_data(stack_info) != stack_info -> hash_data)                  return stack_info -> struct_error = HASHES_DATA_NOT_MATCH;

    #endif

    if (stack_info -> capacity <= 0)                                                        return stack_info -> struct_error =  ERROR_INCORRECT_CAPACITY;
    if (stack_info -> size < 0)                                                             return stack_info -> struct_error =  ERROR_SIZE_LESS_THAN_ZERO;
                     
                     
    if ((stack_info -> size) > (stack_info -> capacity))                                    return stack_info -> struct_error = ERROR_SIZE_MORE_THAN_CAPACITY;
    if ( stack_info -> last_popped_element == POISON)                                       return stack_info -> struct_error = ERROR_POPED_ELEMENT_IS_POISONED;    
    if ((stack_info -> real_data)[stack_info -> size] != POISON &&
         stack_info -> size < stack_info -> capacity -1)                                    return stack_info -> struct_error = ERROR_SIZE_VALUE_NOT_POISONED; 

    return (stack_info -> struct_error);
}

void fill_the_array_poison(stack_info_t* stack_info) {
    for (ssize_t i = stack_info -> size; i < stack_info -> capacity; i++) {
        (stack_info -> real_data)[i] = POISON;
    }
}

struct_error_t check_empty_stack_for_pop(stack_info_t* stack_info) {
    if (stack_info -> size <= 0) {
        stack_info -> struct_error = ERROR_STACK_IS_EMPTY_TO_POP;
    }
    return (stack_info -> struct_error);
}
#endif

#ifdef HASH_MODE
long long hash_function_for_struct(stack_info_t* stack_info) {

    long long hash_struct = 0;
    for(size_t i = 0; i < sizeof(stack_info_t) - sizeof(stack_info -> hash_struct) - sizeof(stack_info -> hash_data) - sizeof(stack_info -> struct_error) - 
                      sizeof(stack_info -> str_code_failure) - sizeof(stack_info -> func_failure); i++) {
        hash_struct += ((char*)((char*)stack_info + sizeof(stack_info -> hash_struct) + sizeof(stack_info -> hash_data)))[i];
    }

    return hash_struct;
}

long long hash_function_for_data(stack_info_t* stack_info) {

    long long hash_data = 0;
    for(size_t i = 0; i < stack_info -> capacity * sizeof(TYPE_DATA_ELEMENTS); i++) {
        hash_data += ((char*)(stack_info -> real_data))[i];
    }

    return hash_data;
}
#endif 

