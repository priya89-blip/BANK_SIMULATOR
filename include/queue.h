#ifndef QUEUE_H
#define QUEUE_H

#include "customer.h"

typedef struct CustomerNode {
    Customer *customer;
    struct CustomerNode *next;
} CustomerNode;

typedef struct TellerQueue {
    CustomerNode *head;
    CustomerNode *tail;
    int length;
} TellerQueue;

TellerQueue* createTellerQueue();
void enqueueCustomer(TellerQueue *queue, Customer *cust);
Customer* dequeueCustomer(TellerQueue *queue);

#endif
