#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include "da.h"
#include "cda.h"

#define BUF_SIZE 1024
#define TIME_QUANTUM 1

enum proc_state { ready, waiting };

typedef struct process_struct {
    int arrival_time;
    int priority;
    int proc_time;
    enum proc_state state;
    pid_t pid;
} process;

void startProcess(process *);
void terminateProcess(process *);
void suspendProcess(process *);
void restartProcess(process *);
char *str_from_int(int);
DA *get_procs_with_arrival_time(CDA *, int);
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
        insertCDAback(dispatch_queue, proc);
    }
    
    int curr_time = 0;
    int num_procs_processed = 0; 
    
    CDA **rq = malloc(sizeof(CDA *) * 4);
    
    for (int i = 0; i < 4; i++) {
        rq[i] = newCDA(display_proc);
    }    

    process *currently_running = 0;
    int sys_running = 0;
    while (currently_running || num_procs_processed != sizeCDA(dispatch_queue)) {
        DA *curr_procs = get_procs_with_arrival_time(dispatch_queue, curr_time); 
        num_procs_processed += sizeDA(curr_procs);
        int i;
        for (i = 0; i < sizeDA(curr_procs); i++) {
            process *curr_proc = (process *)getDA(curr_procs, i);
            insertCDAback(rq[curr_proc->priority], curr_proc);
        }
        if (currently_running && currently_running->proc_time == 0) {
            terminateProcess(currently_running);
            if (sys_running) {
                sys_running = 0;
            }
            currently_running = 0;
        } 
      
        // if there are system processes waiting to be run and one is not already running 
        if (sizeCDA(rq[0]) > 0 && !sys_running) {
            // preempt
            if (currently_running) {
                suspendProcess(currently_running);
                insertCDAback(rq[currently_running->priority], currently_running);
            }
            process *sys_proc = removeCDAfront(rq[0]);
            startProcess(sys_proc);
            currently_running = sys_proc;   
            sys_running = 1;       
        }
        // if sys queue is empty but last sys process running
        else if (sys_running) {

        }
        // if no sys process is running and 1st priority queue has some things in it 
        else if (sizeCDA(rq[1]) > 0) {
            if (currently_running) {
                suspendProcess(currently_running);
                insertCDAback(rq[currently_running->priority], currently_running);
            }
            process *one_proc = removeCDAfront(rq[1]);
            if (one_proc->state == ready) {
                startProcess(one_proc);
            } else {
                restartProcess(one_proc);
            }
            currently_running = one_proc;
            sys_running = 0;
        }
        else if (sizeCDA(rq[2]) > 0) {
            if (currently_running) {
                suspendProcess(currently_running);
                insertCDAback(rq[currently_running->priority], currently_running);
            }
            process *two_proc = removeCDAfront(rq[2]);
            if (two_proc->state == ready) {
                startProcess(two_proc);
            } else {
                restartProcess(two_proc);
            }
            currently_running = two_proc;
            sys_running = 0;
        }
        else if (sizeCDA(rq[3]) > 0) {
            if (currently_running) {
                suspendProcess(currently_running);
                insertCDAback(rq[currently_running->priority], currently_running);
            }
            process *three_proc = removeCDAfront(rq[3]);
            if (three_proc->state == ready) {
                startProcess(three_proc);
            } else {
                restartProcess(three_proc);
            }
            currently_running = three_proc;
            sys_running = 0;
        }
        // decrement proc_time
        if (currently_running) {
            currently_running->proc_time--; 
        }
        curr_time++;
        free(curr_procs);
        sleep(TIME_QUANTUM);
    }
     
    return 0;
}

void startProcess(process *p) {
    pid_t child_pid = fork();
    if (child_pid == 0) {
        char **argv = malloc(sizeof(char *) * 1);
        argv[0] = "./process";
        argv[1] = "20";
        execvp(argv[0], argv);
    } else {
        p->pid = child_pid;
    }
}

void terminateProcess(process *p) {
    kill(p->pid, SIGINT);
    waitpid(p->pid, NULL, WUNTRACED);
}

void suspendProcess(process *p) {
    kill(p->pid, SIGTSTP);
    waitpid(p->pid, NULL, WUNTRACED);
    if (p->priority != 3) {
        p->priority++;
    }
    p->state = waiting;
}

void restartProcess(process *p) {
    kill(p->pid, SIGCONT);
}

char *str_from_int(int x) {
    char* buf = malloc(sizeof(char) * sizeof(int) * 4 + 1);
    if (buf) {
         sprintf(buf, "%d", x);
    }
    return buf;
}

DA *get_procs_with_arrival_time(CDA *dq, int arrival_time) {
    DA *proc_list = newDA(display_proc);
    int i;
    for (i = 0; i < sizeCDA(dq); i++) {
        process *curr_proc = (process *)getCDA(dq, i);
        if (curr_proc->arrival_time == arrival_time) {
            insertDA(proc_list, curr_proc);
        }
    }
    return proc_list;
}

process *new_proc(int arrival_time, int priority, int proc_time) {
    process *np = malloc(sizeof(process));
    np->arrival_time = arrival_time;
    np->priority = priority;
    np->proc_time = proc_time;
    np->pid = 0;
    np->state = ready;
    return np;
}

void display_proc(FILE *fp, void *value) {
    process *proc = (process *)value;
    fprintf(fp, "{%d %d %d}", proc->arrival_time, proc->priority, proc->proc_time);
}
