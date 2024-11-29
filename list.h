#ifndef LIST_H
#define LIST_H

struct list {
        struct list_node *head;
};

struct list_node {
        struct list_node *next;
        struct list_node *prev;
};

void list_push(struct list_node **head, void *ptr) 
{
        if (ptr == NULL) 
                return;

        if (*head == NULL) {
                (*head) = ptr;
                (*head)->next = NULL;
                return;
        }

        struct list_node *new = ptr;

        new->next = (*head);
        (*head) = new;
}

void list_pop(struct list_node **head)
{
        if ((*head) == NULL) 
                return;

        if ((*head)->next != NULL) 
                (*head) = (*head)->next;
        else 
                (*head) = NULL; 
}

const unsigned list_delete(struct list_node **head, void *node)
{
        struct list_node* ptr = (*head);
        struct list_node* prev = NULL; 	
        
        if ((*head) != NULL && (*head) == node) {
                (*head) = ptr->next;
                return 1;
        }

        while (ptr != NULL && ptr != node) {
                prev = ptr;
                ptr = prev->next; 
        }

        if (ptr == NULL) 
                return 0;

        prev->next = ptr->next;
        
        return 1;
}

#endif

