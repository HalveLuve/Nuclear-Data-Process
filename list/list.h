#include <stdio.h>
#include <stdlib.h>
#include <float.h>

typedef struct {
    float *data;
    int size;
    int capacity;
    float minn;
    float maxn;
} List;

void init_list(List *list) {
    list->size = 0;
    list->capacity = 10;
    list->data = (float *) malloc(list->capacity * sizeof(float));
    list->minn = FLT_MAX;
    list->maxn = FLT_MIN;
}

void append(List *list, float value) {
    if (list->size >= list->capacity) {
        list->capacity *= 2;
        list->data = (float *) realloc(list->data, list->capacity * sizeof(float));
    }
    list->data[list->size] = value;
    list->size++;
    if(value > list->maxn) {
        list->maxn = value;
    }
    if(value < list->minn) {
        list->minn = value;
    }
}

void print_list(List *list) {
    printf("[");
    for (int i = 0; i < list->size; i++) {
        printf("%f", list->data[i]);
        if (i != list->size - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

/*
int main() {
    List list;
    init_list(&list);
    append(&list, 1);
    append(&list, 2);
    append(&list, 3);
    print_list(&list);
    append(&list, 4);
    append(&list, 5);
    append(&list, 6);
    print_list(&list);
    return 0;
}
*/
