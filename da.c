#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "da.h"

struct da {
    void **arr;
    int size; // Indicates the number of elements stored in the array
    int cap; // Indicates the capacity of the array
    void (*display)(FILE *, void *);
};

static void increaseCap(DA *items);
static void reduceCap(DA *items);

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
        increaseCap(items);
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
        reduceCap(items);
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

static void increaseCap(DA *items) { 
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

static void reduceCap(DA *items) { 
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
