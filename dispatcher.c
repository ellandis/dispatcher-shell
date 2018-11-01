#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

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

struct da {
    void **arr;
    int size; // Indicates the number of elements stored in the array
    int cap; // Indicates the capacity of the array
    void (*display)(FILE *, void *);
};

typedef struct da DA;

struct cda {
    int size, cap;
    int front; // Index indicating the front of the array. No need to store the
    // back; we can compute it with the front and the size.
    void **arr;
    void (*display)(FILE *, void *);
};

typedef struct cda CDA;

void startProcess(process *);
void terminateProcess(process *);
void suspendProcess(process *);
void restartProcess(process *);
char *str_from_int(int);
DA *get_procs_with_arrival_time(CDA *, int);
process *new_proc(int, int, int);
void display_proc(FILE *, void *);



DA *newDA(void (*d)(FILE *,void *));
void insertDA(DA *items,void *value);
void *removeDA(DA *items);
void unionDA(DA *recipient,DA *donor);
void *getDA(DA *items,int index);
void *setDA(DA *items,int index,void *value);
void **extractDA(DA *items);
int sizeDA(DA *items);
void visualizeDA(FILE *fp,DA *items);
void displayDA(FILE *fp,DA *items);



CDA *newCDA(void (*d)(FILE *,void *));
void insertCDAfront(CDA *items,void *value);
void insertCDAback(CDA *items,void *value);
void *removeCDAfront(CDA *items);
void *removeCDAback(CDA *items);
void unionCDA(CDA *recipient,CDA *donor);
void *getCDA(CDA *items,int index);
void *setCDA(CDA *items,int index,void *value);
void **extractCDA(CDA *items);
int sizeCDA(CDA *items);
void visualizeCDA(FILE *,CDA *items);
void displayCDA(FILE *,CDA *items);


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

static void CDAincreaseCap(CDA *items);
static void CDAreduceCap(CDA *items);

// Function: newCDA
// Takes in a function pointer to display the objects stored in the array.
// Returns the newly allocated CDA

CDA *newCDA(void (*d)(FILE *, void *)) {
    CDA *cda = (CDA *)malloc(sizeof(CDA));
    assert(cda != 0);
    cda->size = 0;
    cda->front = 0;
    cda->cap = 1;
    cda->arr = (void **)malloc(cda->cap * sizeof(void *));
    assert(cda->arr != 0);
    cda->display = d;
    return cda;
}

// Function: insertCDAfront
// Takes in a CDA object and a void pointer.
// Inserts the void pointer at the front of the contiguous region.

void insertCDAfront(CDA *items, void *value) {
    if (items->size == items->cap) {
        CDAincreaseCap(items);
    }
    items->front--;
    if (items->front < 0) {
        items->front += items->cap;
    }
    assert(items->front < items->cap && items->front >= 0);
    items->arr[items->front] = value;
    items->size++;
}

// Function: insertCDAback
// Takes in a CDA object and a void pointer.
// Inserts the void pointer at the back of the contiguous region.

void insertCDAback(CDA *items, void *value) {
    if (items->size == items->cap) {
        CDAincreaseCap(items);
    }
    int back = (items->front + items->size) % items->cap;
    items->arr[back] = value;
    items->size++;
}

// Function: removeCDAfront
// Takes in a CDA object
// Removes and returns the void pointer at the front of the contiguous region.

void *removeCDAfront(CDA *items) {
    assert(items->size > 0);
    if (items->cap > 1 && (items->size - 1) / (float) items->cap < 0.25) {
        CDAreduceCap(items);
    }
    void *front = items->arr[items->front];
    items->front = (items->front + 1) % items->cap;
    items->size--;
    return front;
}

// Function: removeCDAback
// Takes in a CDA object
// Removes and returns the void pointer at the back of the contiguous region.

void *removeCDAback(CDA *items) {
    assert(items->size > 0);
    if (items->cap > 1 && (items->size - 1) / (float) items->cap < 0.25) {
        CDAreduceCap(items);
    }
    int b = (items->front - 1 + items->size) % items->cap;
    void *back = items->arr[b];
    items->size--;
    return back;
}

// Function: unionCDA
// Takes in recipient and donor CDAs.
// Appends all the items in the donor array to the recipient array.
// The donor array is empty after the union operation.

void unionCDA(CDA *recipient, CDA *donor) {
    for (int i = 0; i < donor->size; i++) {
        int index = donor->front + i;
        insertCDAback(recipient, donor->arr[index % donor->cap]);
    }
    while (donor->size != 0) {
        removeCDAback(donor);
    }
}

// Function: getCDA
// Takes in a CDA object and an index
// Returns the value at the specified index from the perspective of the user

void *getCDA(CDA *items, int index) {
    assert(index >= 0 && index < items->size);
    return items->arr[(items->front + index) % items->cap];
}

// Function: setCDA
// Takes in a CDA object, an index, and a void pointer.
// Changes the value at the specified index to be that of the void pointer.
// If a value is replaced, returns the replaced value. Otherwire, returns NULL

void *setCDA(CDA *items, int index, void *value) {
    assert(index >= -1 && index <= items->size);
    void *replaced = 0;
    if (index == items->size) {
        insertCDAback(items, value);
    }
    else if (index == -1) {
        insertCDAfront(items, value);
    }
    else {
        replaced = items->arr[(items->front + index) % items->cap];
        items->arr[(items->front + index) % items->cap] = value;
    }
    return replaced;
}

// Function: extractCDA
// Takes in a CDA object
// Returns the underlying C array shrunk to an exact fit.
// The CDA object gets a new array of capacity 1 and size 0.

void **extractCDA(CDA *items) {
    if (items->size == 0) {
        return 0;
    }
    void **arr = (void **)malloc(items->size * sizeof(void *));
    assert(arr != 0);
    for (int i = 0; i < items->size; i++) {
        arr[i] = items->arr[(items->front + i) % items->cap];
    }
    free(items->arr);
    items->size = 0;
    items->cap = 1;
    items->arr = (void **)malloc(items->cap * sizeof(void *));
    assert(items->arr != 0);
    return arr;
}

// Function: sizeCDA
// Takes in a CDA object
// Returns the size of the CDA

int sizeCDA(CDA *items) {
    return items->size;
}

// Function: visualizeCDA
// Takes in a file pointer and a CDA object
// Prints the contiguous region of the array enclosed with parentheses
//     and separated by commas, followed by the size of the unfilled region
//     enclosed in parentheses.

void visualizeCDA(FILE *fp, CDA *items) {
    displayCDA(fp, items);
    fprintf(fp, "(%d)", items->cap - items->size);
}

// Function: displayCDA
// Takes in a file pointer and a CDA object
// Similar to the visualizeCDA function, except it doesn't print the size of
//     the unfilled region.

void displayCDA(FILE *fp, CDA *items) {
    fprintf(fp, "(");
    for (int i = items->front; i < items->front + items->size; i++) {
        items->display(fp, items->arr[i % items->cap]);
        if (i < items->front + items->size - 1) {
            fprintf(fp, ",");
        }
    }
    fprintf(fp, ")");
}

// Static function: increaseCap
// Takes in a CDA object
// Double the size of the CDA array, and frees the old one

static void CDAincreaseCap(CDA *items) {
    int oldFront = items->front;
    int oldCap = items->cap;
    items->cap *= 2;
    void **newArr = (void **)malloc(items->cap * sizeof(void *));
    assert(newArr != 0);
    for (int i = 0; i < items->size; i++) {
        newArr[i] = items->arr[(oldFront++) % oldCap];
    }
    free(items->arr);
    items->arr = newArr;
    items->front = 0;
}

// Static function: reduceCap
// Takes in a CDA object.
// Similar to the increaseCap function, except it halves the size of the array.

static void CDAreduceCap(CDA *items) {
    int oldFront = items->front;
    int oldCap = items->cap;
    items->cap /= 2;
    void **newArr = (void **)malloc(items->cap * sizeof(void *));
    assert(newArr != 0);
    for (int i = 0; i < items->size; i++) {
        newArr[i] = items->arr[(oldFront++) % oldCap];
    }
    free(items->arr);
    items->arr = newArr;
    items->front = 0;
}

static void DAincreaseCap(DA *items);
static void DAreduceCap(DA *items);

// Function: newDA
// Takes in a callback function to display the value stored in the data structure
// User calls this function to create a new dynamic array.
// Returns the new dynamic array.

DA *newDA(void (*d)(FILE *, void *)) {
    DA *da = (DA *)malloc(sizeof(DA));
    assert(da != 0);
    da->size = 0;
    da->cap = 1;
    da->arr = (void **)malloc(da->cap * sizeof(void *));
    assert(da->arr != 0);
    da->display = d;
    return da;
}

// Function: insertDA
// Takes in a DA object and a void pointer.
// User calls this function to insert a void pointer into to the leftmost
//    unfilled slot of the dynamic array

void insertDA(DA *items, void *value) {
    if (items->size == items->cap) {
        DAincreaseCap(items);
    }
    items->arr[(items->size)++] = value;
}

// Function: removeDA
// Takes in a DA object
// User calls this function to remove the rightmost item in the filled region
//     of the dynamic array.
// Returns the value removed.

void *removeDA(DA *items) {
    assert(items->size > 0);
    void *tail = items->arr[--(items->size)];
    if (items->cap > 1 && items->size / (float) items->cap < 0.25) {
        DAreduceCap(items);
    }
    return tail;
}

// Function: unionDA
// Takes in a DA recipient and a DA donor
// User calls this function to move all items in the donor array to the
//     recipient array.
// After the union, the donor will be empty and the recipient will contain
//     have the items of the donor appended.

void unionDA(DA *recipient, DA *donor) {
    for (int i = 0; i < donor->size; i++) {
        insertDA(recipient, donor->arr[i]);
    }
    while (donor->size != 0) {
        removeDA(donor);
    }
}

// Function: getDA
// Takes in a DA object and an index
// User calls this function to get the value stored in the DA at the specified
//     index.
// Returns the specified void pointer

void *getDA(DA *items, int index) {
    assert(index >= 0);
    assert(index < items->size);
    return items->arr[index];
}

// Function: setDA
// Takes in a DA object, an index, and a void pointer
// User calls this function to change the value at the specified index.
// Returns the replaced value if a replacement was made, or a null pointer if
//     no replacement was made.

void *setDA(DA *items, int index, void *value) {
    assert(index >= 0 && index <= items->size);
    void *replaced = 0;
    if (index == items->size) {
        insertDA(items, value);
    }
    else {
        replaced = items->arr[index];
        items->arr[index] = value;
    }
    return replaced;
}

// Function: extractDA
// Takes in a DA object
// Returns the underlying C array, shrunk to an exact fit.
// DA object gets a new array of capacity 1 and size 0.

void **extractDA(DA *items) {
    if (items->size == 0) {
        return 0;
    }
    void **arr = (void **)malloc(items->size * sizeof(void *));
    assert(arr != 0);
    for (int i = 0; i < items->size; i++) {
        arr[i] = items->arr[i];
    }
    free(items->arr);
    items->size = 0;
    items->cap = 1;
    items->arr = (void **)malloc(items->cap * sizeof(void *));
    assert(items->arr != 0);
    return arr;
}

// Function: sizeDA
// Takes in a DA object
// Returns the size of the array

int sizeDA(DA *items) {
    return items->size;
}

// Function: visualizeDA
// Takes in a file pointer and a DA object
// Prints out the filled region enclosed in brackets and separated by commas,
//     followed by the size of the unfilled region enclosed in brackets.

void visualizeDA(FILE *fp, DA *items) {
    displayDA(fp, items);
    fprintf(fp, "[%d]", items->cap - items->size);
}

// Function: displayDA
// Takes in a file pointer and a DA object
// Similar to the visualizeDA method, except the bracketed size of the unfilled
//     region is not printed.

void displayDA(FILE *fp, DA *items) {
    fprintf(fp, "[");
    for (int i = 0; i < items->size; i++) {
        items->display(fp, items->arr[i]);
        if (i < items->size - 1) {
            fprintf(fp, ",");
        }
    }
    fprintf(fp, "]");
}

// Static function: increaseCap
// Takes in a DA object
// Doubles the size of the array, copies all the old elements over, and frees
//   the old array

static void DAincreaseCap(DA *items) {
    items->cap *= 2;
    void **newArr = (void **)malloc(items->cap * sizeof(void *));
    assert(newArr != 0);
    for (int i = 0; i < items->size; i++) {
        newArr[i] = items->arr[i];
    }
    free(items->arr);
    items->arr = newArr;
}

// Static function: reduceCap
// Takes in a DA object
// Similar to increaseCap, but halves the size of the array instead of doubling.

static void DAreduceCap(DA *items) {
    items->cap /= 2;
    void **newArr = (void **)malloc(items->cap * sizeof(void *));
    assert(newArr != 0);
    for (int i = 0; i < items->size; i++) {
        newArr[i] = items->arr[i];
    }
    free(items->arr);
    items->arr = newArr;
}

int testing(int a, int b) {
    return a + b;
}
