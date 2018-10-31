#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "da.h"

#define BUF_SIZE 1024

typedef struct process_struct {
    int arrival_time;
    int priority;
    int proc_time;
} process;

process *new_proc(int, int, int);
void display_proc(FILE *, void *);

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
    
    DA *proc_da = newDA(display_proc);
    char line_buf[BUF_SIZE];
    
    while (fgets(line_buf, BUF_SIZE, dispatch_file)) {
        int arrival_time, priority, proc_time; 
        const int got = sscanf(line_buf, "%d,%d,%d", &arrival_time, &priority, &proc_time);
        if (got != 3) {
            fprintf(stderr, "error parsing file.\n");
            return 1;
        }
        process *proc = new_proc(arrival_time, priority, proc_time);
        insertDA(proc_da, proc);
    }

    int curr_time = 0;
    int proc_ptr = 0;
    while (proc_ptr < sizeDA(proc_da)) {
        process *curr_proc = (process *)getDA(proc_da, proc_ptr);
        if (curr_proc->arrival_time == curr_time) {
            pid_t child_pid = fork();
            if (child_pid == 0) {
                char *time_buf = malloc(sizeof(char) * sizeof(int) * 4 + 1);
                sprintf(time_buf, "%d", curr_proc->proc_time);
                char **proc_args = malloc(sizeof(char *));
                proc_args[0] = time_buf;
                execvp("./process", proc_args);
            }
        }
        sleep(1);
        curr_time++; 
    }   
    return 0;
}

process *new_proc(int arrival_time, int priority, int proc_time) {
    process *np = malloc(sizeof(process));
    np->arrival_time = arrival_time;
    np->priority = priority;
    np->proc_time = proc_time;
    return np;
}

void display_proc(FILE *fp, void *value) {
    process *proc = (process *)value;
    fprintf(fp, "{%d %d %d}", proc->arrival_time, proc->priority, proc->proc_time);
}
