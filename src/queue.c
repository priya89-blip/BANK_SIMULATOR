#include <stdlib.h>
#include "queue.h"

TellerQueue* createTellerQueue(){
    TellerQueue *q = (TellerQueue*)malloc(sizeof(TellerQueue));
    q->head = q->tail = NULL;
    q->length = 0;
    return q;
}

void enqueueCustomer(TellerQueue *queue, Customer *cust){
    CustomerNode *node = (CustomerNode*)malloc(sizeof(CustomerNode));
    node->customer = cust;
    node->next = NULL;
    if(queue->tail) queue->tail->next = node;
    else queue->head = node;
    queue->tail = node;
    queue->length++;
}

Customer* dequeueCustomer(TellerQueue *queue){
    if(!queue->head) return NULL;
    CustomerNode *node = queue->head;
    Customer *cust = node->customer;
    queue->head = node->next;
    if(!queue->head) queue->tail = NULL;
    free(node);
    queue->length--;
    return cust;
}
