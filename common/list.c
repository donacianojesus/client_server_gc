#include "list.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Generic list functions
list_t* list_create() {
    list_t *list = malloc(sizeof(list_t));
    if (list) {
        list->head = NULL;
        list->tail = NULL;
        list->size = 0;
    }
    return list;
}

void list_destroy(list_t *list) {
    if (!list) return;
    
    list_node_t *current = list->head;
    while (current) {
        list_node_t *next = current->next;
        free(current);
        current = next;
    }
    free(list);
}

void list_append(list_t *list, void *data) {
    if (!list) return;
    
    list_node_t *node = malloc(sizeof(list_node_t));
    if (!node) return;
    
    node->data = data;
    node->next = NULL;
    
    if (list->tail) {
        list->tail->next = node;
    } else {
        list->head = node;
    }
    list->tail = node;
    list->size++;
}

void list_remove(list_t *list, void *data) {
    if (!list || !list->head) return;
    
    list_node_t *current = list->head;
    list_node_t *prev = NULL;
    
    while (current) {
        if (current->data == data) {
            if (prev) {
                prev->next = current->next;
            } else {
                list->head = current->next;
            }
            
            if (current == list->tail) {
                list->tail = prev;
            }
            
            free(current);
            list->size--;
            return;
        }
        prev = current;
        current = current->next;
    }
}

void* list_find(list_t *list, void *data, int (*compare)(void*, void*)) {
    if (!list || !compare) return NULL;
    
    list_node_t *current = list->head;
    while (current) {
        if (compare(current->data, data) == 0) {
            return current->data;
        }
        current = current->next;
    }
    return NULL;
}

void list_foreach(list_t *list, void (*func)(void*)) {
    if (!list || !func) return;
    
    list_node_t *current = list->head;
    while (current) {
        func(current->data);
        current = current->next;
    }
}

int list_size(list_t *list) {
    return list ? list->size : 0;
}

int list_is_empty(list_t *list) {
    return list ? (list->size == 0) : 1;
}

// User list specific functions
list_t* user_list_create() {
    return list_create();
}

void user_list_destroy(list_t *list) {
    if (!list) return;
    
    list_node_t *current = list->head;
    while (current) {
        list_node_t *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
    free(list);
}

static int compare_username(void *a, void *b) {
    user_t *user = (user_t*)a;
    char *username = (char*)b;
    return strcmp(user->username, username);
}

static int compare_socket(void *a, void *b) {
    user_t *user = (user_t*)a;
    int socket_fd = *(int*)b;
    return user->socket_fd - socket_fd;
}

user_t* user_list_find_by_username(list_t *list, const char *username) {
    return (user_t*)list_find(list, (void*)username, compare_username);
}

user_t* user_list_find_by_socket(list_t *list, int socket_fd) {
    return (user_t*)list_find(list, &socket_fd, compare_socket);
}

void user_list_remove_by_socket(list_t *list, int socket_fd) {
    if (!list || !list->head) return;
    
    list_node_t *current = list->head;
    list_node_t *prev = NULL;
    
    while (current) {
        user_t *user = (user_t*)current->data;
        if (user->socket_fd == socket_fd) {
            if (prev) {
                prev->next = current->next;
            } else {
                list->head = current->next;
            }
            
            if (current == list->tail) {
                list->tail = prev;
            }
            
            free(user);
            free(current);
            list->size--;
            return;
        }
        prev = current;
        current = current->next;
    }
}

// Group list specific functions
list_t* group_list_create() {
    return list_create();
}

void group_list_destroy(list_t *list) {
    if (!list) return;
    
    list_node_t *current = list->head;
    while (current) {
        list_node_t *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
    free(list);
}

static int compare_group_name(void *a, void *b) {
    group_t *group = (group_t*)a;
    char *name = (char*)b;
    return strcmp(group->name, name);
}

group_t* group_list_find_by_name(list_t *list, const char *group_name) {
    return (group_t*)list_find(list, (void*)group_name, compare_group_name);
}
