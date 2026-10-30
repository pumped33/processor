struct file_info_t {

    int size_of_file_in    = 0;
    const char* name_file_in = "";
    FILE* ptr_file_in        = NULL;
};

void open_file(file_info_t* file_info, const char* filename) {
    
    file_info -> name_file_in = filename;
    file_info -> ptr_file_in  = fopen(filename, "r");
}

int size_of_file(file_info_t* file_info) {

    struct stat stat_file_in;
    if (stat(file_info -> name_file_in, &stat_file_in) == -1) {
        return 1;
    }
    file_info -> size_of_file_in = stat_file_in.st_size;
    return 0;
}