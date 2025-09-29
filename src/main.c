#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "customer.h"
#include "teller.h"
#include "queue.h"
#include "event.h"

#define MIN_IDLE 1
#define MAX_IDLE 150

float simulationClock = 0;

TellerQueue **tellerQueues;
Teller **tellers;
int numTellers;
float avgServiceTime;

float totalCustomerTime = 0;
float maxCustomerTime = 0;
int totalCustomersServed = 0;

void customerArrival(Event *e);
void customerServiceComplete(Event *e);
void tellerIdle(Event *e);

extern Event *eventQueueHead;

// Return index of shortest queue
int getShortestQueue() {
    int idx = 0;
    int minLen = tellerQueues[0]->length;
    for (int i = 1; i < numTellers; i++) {
        if (tellerQueues[i]->length < minLen) {
            idx = i;
            minLen = tellerQueues[i]->length;
        }
    }
    return idx;
}

void runSimulation(int numCustomers, int queueMode, float simTime) {
    simulationClock = 0;
    totalCustomerTime = 0;
    maxCustomerTime = 0;
    totalCustomersServed = 0;
    eventQueueHead = NULL;

    // Reset tellers and queues
    for (int i = 0; i < numTellers; i++) {
        tellers[i]->totalIdleTime = 0;
        tellers[i]->totalServiceTime = 0;
        tellerQueues[i]->head = tellerQueues[i]->tail = NULL;
        tellerQueues[i]->length = 0;
    }

    // Schedule all customer arrivals
    for (int i = 0; i < numCustomers; i++) {
        float arrival = simTime * ((float)rand() / RAND_MAX);
        Customer *c = createCustomer(i, arrival);
        Event *e = (Event *)malloc(sizeof(Event));
        e->time = arrival;
        e->type = CUSTOMER_ARRIVAL;
        e->actor = c;
        e->action = customerArrival;
        e->next = NULL;
        addEvent(e);
    }

    Event *e;
    while ((e = popEvent()) != NULL) {
        simulationClock = e->time;
        executeEvent(e);
        free(e);
    }

    printf("\n--- Simulation Summary ---\n");
    printf("Queue Mode: %s\n", queueMode == 0 ? "Single Queue" : "Multiple Queues");
    printf("Total Customers Served: %d\n", totalCustomersServed);
    printf("Average Customer Time: %.2f\n", totalCustomerTime / totalCustomersServed);
    printf("Maximum Customer Time: %.2f\n", maxCustomerTime);

    float totalIdle = 0, totalService = 0;
    for (int i = 0; i < numTellers; i++) {
        totalIdle += tellers[i]->totalIdleTime;
        totalService += tellers[i]->totalServiceTime;
    }
    printf("Total Teller Service Time: %.2f\n", totalService);
    printf("Total Teller Idle Time: %.2f\n", totalIdle);
    printf("--------------------------\n\n");
}

// Event action functions
void customerArrival(Event *e) {
    Customer *c = (Customer *)e->actor;
    int idx = getShortestQueue();
    enqueueCustomer(tellerQueues[idx], c);
    printf("Customer %d arrived at %.2f, joined queue %d\n", c->id, simulationClock, idx);

    if (tellerQueues[idx]->length == 1) {
        Event *service = (Event *)malloc(sizeof(Event));
        service->time = simulationClock + 2 * avgServiceTime * ((float)rand() / RAND_MAX);
        service->type = CUSTOMER_SERVICE_COMPLETE;
        service->actor = tellers[idx];
        service->action = customerServiceComplete;
        service->next = NULL;
        addEvent(service);
    }
}

void customerServiceComplete(Event *e) {
    Teller *t = (Teller *)e->actor;
    Customer *c = dequeueCustomer(tellerQueues[t->id]);
    if (!c) return;

    c->serviceEndTime = simulationClock;
    float custTime = c->serviceEndTime - c->arrivalTime;
    totalCustomerTime += custTime;
    if (custTime > maxCustomerTime) maxCustomerTime = custTime;
    totalCustomersServed++;
    t->totalServiceTime += custTime;

    printf("Customer %d finished service at %.2f by Teller %d\n", c->id, simulationClock, t->id);
    free(c);

    if (tellerQueues[t->id]->length > 0) {
        Event *service = (Event *)malloc(sizeof(Event));
        service->time = simulationClock + 2 * avgServiceTime * ((float)rand() / RAND_MAX);
        service->type = CUSTOMER_SERVICE_COMPLETE;
        service->actor = t;
        service->action = customerServiceComplete;
        service->next = NULL;
        addEvent(service);
    } else {
        Event *idle = (Event *)malloc(sizeof(Event));
        idle->time = simulationClock + MIN_IDLE + rand() % (MAX_IDLE - MIN_IDLE + 1);
        idle->type = TELLER_IDLE_COMPLETE;
        idle->actor = t;
        idle->action = tellerIdle;
        idle->next = NULL;
        addEvent(idle);
    }
}

void tellerIdle(Event *e) {
    Teller *t = (Teller *)e->actor;

    for (int i = 0; i < numTellers; i++) {
        if (tellerQueues[i]->length > 0) {
            Event *service = (Event *)malloc(sizeof(Event));
            service->time = simulationClock + 2 * avgServiceTime * ((float)rand() / RAND_MAX);
            service->type = CUSTOMER_SERVICE_COMPLETE;
            service->actor = tellers[i];
            service->action = customerServiceComplete;
            service->next = NULL;
            addEvent(service);
            return;
        }
    }

    Event *idle = (Event *)malloc(sizeof(Event));
    idle->time = simulationClock + MIN_IDLE + rand() % (MAX_IDLE - MIN_IDLE + 1);
    idle->type = TELLER_IDLE_COMPLETE;
    idle->actor = t;
    idle->action = tellerIdle;
    idle->next = NULL;
    addEvent(idle);
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s #customers #tellers simulationTime avgServiceTime\n", argv[0]);
        return 1;
    }

    srand(time(NULL));
    int numCustomers = atoi(argv[1]);
    numTellers = atoi(argv[2]);
    float simTime = atof(argv[3]);
    avgServiceTime = atof(argv[4]);

    tellers = (Teller **)malloc(numTellers * sizeof(Teller *));
    tellerQueues = (TellerQueue **)malloc(numTellers * sizeof(TellerQueue *));
    for (int i = 0; i < numTellers; i++) {
        tellers[i] = createTeller(i);
        tellerQueues[i] = createTellerQueue();
    }

    printf("\n--- SINGLE QUEUE ---\n");
    runSimulation(numCustomers, 0, simTime);

    printf("\n--- MULTIPLE QUEUES ---\n");
    runSimulation(numCustomers, 1, simTime);

    free(tellers);
    free(tellerQueues);
    return 0;
}
