#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "cda.h"

#define BUF_SIZE 1024
#define TIME_QUANTUM 1

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
    
    CDA *dispatch_queue = newCDA(display_proc);
    char line_buf[BUF_SIZE];
    
    while (fgets(line_buf, BUF_SIZE, dispatch_file)) {
        int arrival_time, priority, proc_time; 
        const int got = sscanf(line_buf, "%d,%d,%d", &arrival_time, &priority, &proc_time);
        if (got != 3) {
            fprintf(stderr, "error parsing file.\n");
            return 1;
        }
        process *proc = new_proc(arrival_time, priority, proc_time);
        insertCDAfront(dispatch_queue, proc);
    }

    int curr_time = 0;

    while (sizeCDA(dispatch_queue) > 0) {
        process *curr_proc = (process *)removeCDAback(dispatch_queue);
        
        curr_time++;
        sleep(TIME_QUANTUM);
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
