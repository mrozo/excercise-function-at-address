#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>

#define MAP_TEXT_SECTION_NAME "text"
#define MAP_TEXT_SUBSECTION_NAME "text"
#define MAP_MAX_FUNCTION_NAME_LENGTH 1024
#define MAP_INVALID_ADDRESS -1

#ifdef DEBUG
#   define TRACE(line) fprintf(stdout, "%s:%d line=%s\n", __FUNCTION__, __LINE__, line)
#else
#   define TRACE(line)
#endif

#define ERROR_EXIT(msg, ...) fprintf(stderr, msg "\n", ## __VA_ARGS__); exit(EXIT_FAILURE)

static inline char *map_skip_spaces(char *line){
    while(true){
        if(*line!=' ' || *line == '\0'){
            return line;
        }
        line++;
    }
}


static inline bool map_is_whitespace(char c){
    return c==' ' || c=='\t';
}

static inline char *map_skip_whitespaces(char *line){
    while(true){
        if(!map_is_whitespace(*line) || *line == '\0'){
            return line;
        }
        line++;
    }
}

static inline bool map_line_is_section_declaration(char *line, char *section){
    return line[0] == '.' && strstr(line+1, section) == line+1;
}


static inline bool map_line_is_subsection_declaration(char *line, char *sub_section){
    return map_line_is_section_declaration(map_skip_spaces(line), sub_section);
}

void map_rewind_to_section_contents(FILE *map, char *section){   
    char *line = NULL;
    bool section_found = false;
    ssize_t line_len = 0;
    while(!section_found && -1 != getline(&line, &line_len, map)){
        TRACE(line);
        if(map_line_is_section_declaration(line, section)){
            section_found = true;
        }
        free(line);
        line=NULL;
    }
}

void map_rewind_to_subsection_contents(FILE *map, char *section){   
    char *line = NULL;
    bool sub_section_found = false;
    ssize_t line_len = 0;
    while(!sub_section_found && -1 != getline(&line, &line_len, map)){
        TRACE(line);
        if(map_line_is_subsection_declaration(line, section)){
            sub_section_found = true;
        }
        free(line);
        line=NULL;
    }
}

static inline char *map_skip_token(char *line){
    while(!map_is_whitespace(*line) && *line != '\0'){
        line++;
    }
    return line;
}

void map_parse_function_info(char *line, ssize_t *address, char **fname){
    *fname = NULL;
    line = map_skip_whitespaces(line);
    if(1 == sscanf(line, "%li", address)){
        line = map_skip_token(line);
        line = map_skip_whitespaces(line);
        *fname = line;
    }
}

void map_find_function_by_address(FILE *map, ssize_t address, char *function_name){
    char *line = NULL;
    bool function_found = false;
    ssize_t line_len = 0;
    while(!function_found && -1 != getline(&line, &line_len, map)){
        TRACE(line);
        ssize_t found_address = -1;
        char *fname = NULL;
        map_parse_function_info(line, &found_address, &fname);
        if(fname && address == found_address){
            memcpy(function_name, fname, strlen(fname));
            function_found = true;
        }
        free(line);
        line=NULL;
    }
}

static void map_print_help(void){
    printf("add2func: find function name in *.map file by address\n"
           "Usage:\n"
           "add2func --address=<address> --map=<map file>\n"
           "add2func --help\n"
           "Arguments:\n"
           "--address=<address> - address of to be searched for\n"
           "--map=<map file> - path to the map file\n"
           "--help - display this help\n");

}

int map_parse_input(int argc, char *argv[], ssize_t *address_arg, FILE **map_file){
    int c;
    int digit_optind = 0;
    *address_arg = MAP_INVALID_ADDRESS;
    *map_file = NULL;
    
    while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"address", required_argument, NULL,  'a' },
            {"map",     required_argument, NULL,  'm' },
            {"help",    optional_argument, NULL,  'h' }
            };

        c = getopt_long(argc, argv, "a:m:h", long_options, &option_index);
        if (c == -1){
            break;
        }
        
        switch (c) {
        
        case 'a':
            if(optarg){
                if(1 != sscanf(optarg, "%li", address_arg)){
                    ERROR_EXIT("failed to parse address value: %s", optarg);
                }
            } else {
                ERROR_EXIT("missing address value");
            }
            break;
            
        case 'm':
            if(optarg){
                *map_file = fopen(optarg, "r");
                if(*map_file==NULL){
                    ERROR_EXIT("failed to open map file");
                }
            } else {
                ERROR_EXIT("missing map file path");
            }
            break;
            
        case 'h':
            map_print_help();
            exit(EXIT_SUCCESS);
            break;

        case '?':
            fprintf(stderr, "unrecognized parameter\n");
            map_print_help();
            break;

        default:
            printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    if (optind < argc) {
        fprintf(stderr, "non-option ARGV-elements: ");
        while (optind < argc){
            fprintf(stderr, "%s ", argv[optind++]);
        }
        printf("\n");
    }
}


int main(int argc, char *argv[]){
    
    FILE *map = NULL;
    ssize_t address=MAP_INVALID_ADDRESS;

    map_parse_input(argc, argv, &address, &map);

    if(address == MAP_INVALID_ADDRESS || map == NULL){
        ERROR_EXIT("missing required argument(s)");
    }
    
    char *line;
    map_rewind_to_section_contents(map, MAP_TEXT_SECTION_NAME);
    char function_name[MAP_MAX_FUNCTION_NAME_LENGTH] = "";
    while(feof(map) == 0){
        function_name[0] = '\0';
        map_rewind_to_subsection_contents(map, MAP_TEXT_SUBSECTION_NAME);
        map_find_function_by_address(map, address, function_name);
        if(strlen(function_name)){
            break;
        }
    }
    fclose(map);
    
    if(strlen(function_name)){
        printf("%s\n", function_name);
    } else {
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}
