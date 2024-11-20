#ifndef LIST_H
#define LIST_H

struct list {
        struct list_node *head;
};

struct list_node {
        struct list_node *next;
        struct list_node *prev;
};

#endif

