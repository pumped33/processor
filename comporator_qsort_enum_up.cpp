int comporator_quick_sort_enum_up(const void* arg1, const void* arg2) {

    command_info_t* first_arg =  (command_info_t*) arg1;
    command_info_t* second_arg = (command_info_t*) arg2;

    return first_arg -> command - second_arg -> command;
}