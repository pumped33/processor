int comporator_quick_sort_hash_up(const void* arg1, const void* arg2) {

    command_info_t* first_arg =  (command_info_t*) arg1;
    command_info_t* second_arg = (command_info_t*) arg2;

    if (first_arg -> hash_command < second_arg -> hash_command) {
        return -1;
    }
    return first_arg -> hash_command - second_arg -> hash_command;
}