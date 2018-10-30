#include <stdio.h>
#include <string.h>

typedef struct process_struct {
    int arrival_time;
    int priority;
    int proc_time;
} process;

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: ./dispatcher [dispatch_list]\n");
        return 1;
    }
    
    FILE *dispatch_file = fopen(argv[1], "r");
    
    if (!dispatch_file) {
        fprintf(stderr, "can't open %s as dispatch_list.\n", argv[1]);
        return 1;
    }
    
    char line_buf[255];
    while (fgets(line_buf, 255, dispatch_file)) {
        char *tok = strtok(line_buf, ",");
        while (tok) {
            printf("%s ", tok);
            tok = strtok(NULL, ",");
        }
        printf("\n");
    }
    return 0;
}
