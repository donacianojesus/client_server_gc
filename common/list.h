#ifndef LIST_H
#define LIST_H

#include "protocol.h"

// List node structure
typedef struct list_node {
    void *data;
    struct list_node *next;
} list_node_t;

// List structure
typedef struct {
    list_node_t *head;
    list_node_t *tail;
    int size;
} list_t;

// Function declarations
list_t* list_create();
void list_destroy(list_t *list);
void list_append(list_t *list, void *data);
void list_remove(list_t *list, void *data);
void* list_find(list_t *list, void *data, int (*compare)(void*, void*));
void list_foreach(list_t *list, void (*func)(void*));
int list_size(list_t *list);
int list_is_empty(list_t *list);

// Specific list functions for users and groups
list_t* user_list_create();
void user_list_destroy(list_t *list);
user_t* user_list_find_by_username(list_t *list, const char *username);
user_t* user_list_find_by_socket(list_t *list, int socket_fd);
void user_list_remove_by_socket(list_t *list, int socket_fd);

list_t* group_list_create();
void group_list_destroy(list_t *list);
group_t* group_list_find_by_name(list_t *list, const char *group_name);

#endif // LIST_H
