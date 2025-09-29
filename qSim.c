#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_CUSTOMERS 1000
#define MAX_TELLERS 10

typedef enum { ARRIVAL, DEPARTURE } EventType;

typedef struct Event {
    int customerID;
    int tellerID;
    double time;
    EventType type;
    struct Event* next;
} Event;

typedef struct Customer {
    int id;
    double arrivalTime;
    double serviceTime;
    double startService;
    double finishTime;
} Customer;

typedef struct TellerQueue {
    int tellerID;
    int size;
    int customers[MAX_CUSTOMERS];
} TellerQueue;

// Global variables
Event* eventQueue = NULL;
Customer customers[MAX_CUSTOMERS];
TellerQueue tellers[MAX_TELLERS];
int numCustomers, numTellers;
double simulationTime, avgServiceTime;

// Statistics
double totalTimeSpent = 0.0;
double maxTimeSpent = 0.0;
double totalTellerServiceTime[MAX_TELLERS] = {0};
double totalTellerIdleTime[MAX_TELLERS] = {0};
double clockTime = 0.0;

// Function prototypes
void addEvent(Event* e);
Event* popEvent();
void scheduleArrival(int id);
void scheduleDeparture(int custID, int tellerID, double serviceTime);
double randTime(double max);
int findShortestQueue();
void runSimulationSingleQueue();
void runSimulationMultipleQueues();

int main(int argc, char* argv[]) {
    if(argc != 5) {
        printf("Usage: %s #customers #tellers simulationTime avgServiceTime\n", argv[0]);
        return 1;
    }

    numCustomers = atoi(argv[1]);
    numTellers = atoi(argv[2]);
    simulationTime = atof(argv[3]);
    avgServiceTime = atof(argv[4]);

    srand(time(NULL));

    printf("\n--- SINGLE QUEUE SIMULATION ---\n");
    runSimulationSingleQueue();

    printf("\n--- MULTIPLE QUEUE SIMULATION ---\n");
    runSimulationMultipleQueues();

    return 0;
}

// Add event to event queue sorted by time
void addEvent(Event* e) {
    if(eventQueue == NULL || e->time < eventQueue->time) {
        e->next = eventQueue;
        eventQueue = e;
    } else {
        Event* temp = eventQueue;
        while(temp->next && temp->next->time <= e->time)
            temp = temp->next;
        e->next = temp->next;
        temp->next = e;
    }
}

// Pop event from queue
Event* popEvent() {
    if(!eventQueue) return NULL;
    Event* e = eventQueue;
    eventQueue = eventQueue->next;
    return e;
}

// Random time generator
double randTime(double max) {
    return max * ((double)rand() / RAND_MAX);
}

// Schedule customer arrival
void scheduleArrival(int id) {
    double t = randTime(simulationTime);
    customers[id].id = id;
    customers[id].arrivalTime = t;

    Event* e = malloc(sizeof(Event));
    e->customerID = id;
    e->type = ARRIVAL;
    e->time = t;
    e->next = NULL;
    addEvent(e);
}

// Schedule departure
void scheduleDeparture(int custID, int tellerID, double serviceTime) {
    Event* e = malloc(sizeof(Event));
    e->customerID = custID;
    e->tellerID = tellerID;
    e->type = DEPARTURE;
    e->time = clockTime + serviceTime;
    customers[custID].finishTime = e->time;
    addEvent(e);
}

// Find the shortest teller queue
int findShortestQueue() {
    int minSize = tellers[0].size;
    int idx = 0;
    for(int i = 1; i < numTellers; i++) {
        if(tellers[i].size < minSize) {
            minSize = tellers[i].size;
            idx = i;
        }
    }
    return idx;
}

// SINGLE QUEUE simulation
void runSimulationSingleQueue() {
    // Reset
    eventQueue = NULL;
    totalTimeSpent = maxTimeSpent = 0;
    for(int i=0;i<numTellers;i++){ totalTellerServiceTime[i]=totalTellerIdleTime[i]=0; tellers[i].size=0; }

    // Schedule all arrivals
    for(int i = 0; i < numCustomers; i++)
        scheduleArrival(i);

    // Single common queue
    int commonQueue[MAX_CUSTOMERS], front=0, rear=0;
    int tellerFree[MAX_TELLERS];
    for(int i=0;i<numTellers;i++) tellerFree[i]=1; // all free

    // Simulation loop
    Event* e;
    while((e = popEvent()) != NULL) {
        clockTime = e->time;
        int cid = e->customerID;
        if(e->type == ARRIVAL) {
            // Add to common queue
            commonQueue[rear++] = cid;
            printf("Customer %d arrived at %.2f, added to common queue\n", cid, clockTime);

            // Check if any free teller
            for(int t=0;t<numTellers;t++) {
                if(tellerFree[t] && front<rear) {
                    int serveCID = commonQueue[front++];
                    tellerFree[t] = 0;
                    double sTime = 2 * avgServiceTime * ((double)rand()/RAND_MAX);
                    customers[serveCID].startService = clockTime;
                    totalTellerServiceTime[t] += sTime;
                    scheduleDeparture(serveCID, t, sTime);
                }
            }
        } else if(e->type == DEPARTURE) {
            int t = e->tellerID;
            tellerFree[t] = 1;
            double spent = customers[cid].finishTime - customers[cid].arrivalTime;
            totalTimeSpent += spent;
            if(spent > maxTimeSpent) maxTimeSpent = spent;
            printf("Customer %d finished at %.2f by Teller %d (time spent %.2f)\n", cid, clockTime, t, spent);

            // Serve next in queue if any
            if(front < rear) {
                int serveCID = commonQueue[front++];
                tellerFree[t] = 0;
                double sTime = 2 * avgServiceTime * ((double)rand()/RAND_MAX);
                customers[serveCID].startService = clockTime;
                totalTellerServiceTime[t] += sTime;
                scheduleDeparture(serveCID, t, sTime);
            }
        }
        free(e);
    }

    double avg = totalTimeSpent/numCustomers;
    printf("\nSingle queue summary:\n");
    printf("Average time in bank: %.2f\n", avg);
    printf("Max time in bank: %.2f\n", maxTimeSpent);
    for(int i=0;i<numTellers;i++)
        printf("Teller %d service time: %.2f\n", i, totalTellerServiceTime[i]);
} 

// MULTIPLE QUEUE simulation
void runSimulationMultipleQueues() {
    // Reset
    eventQueue = NULL;
    totalTimeSpent = maxTimeSpent = 0;
    for(int i=0;i<numTellers;i++){ totalTellerServiceTime[i]=0; tellers[i].size=0; }

    // Schedule all arrivals
    for(int i = 0; i < numCustomers; i++)
        scheduleArrival(i);

    // Simulation loop
    Event* e;
    while((e = popEvent()) != NULL) {
        clockTime = e->time;
        int cid = e->customerID;
        if(e->type == ARRIVAL) {
            int q = findShortestQueue();
            tellers[q].customers[tellers[q].size++] = cid;
            printf("Customer %d arrived at %.2f, joined queue %d\n", cid, clockTime, q);

            // If teller idle, serve immediately
            if(tellers[q].size == 1) {
                double sTime = 2 * avgServiceTime * ((double)rand()/RAND_MAX);
                customers[cid].startService = clockTime;
                totalTellerServiceTime[q] += sTime;
                scheduleDeparture(cid, q, sTime);
            }
        } else if(e->type == DEPARTURE) {
            int t = e->tellerID;
            double spent = customers[cid].finishTime - customers[cid].arrivalTime;
            totalTimeSpent += spent;
            if(spent > maxTimeSpent) maxTimeSpent = spent;
            printf("Customer %d finished at %.2f by Teller %d (time spent %.2f)\n", cid, clockTime, t, spent);

            // Remove customer from teller queue
            for(int i=0;i<tellers[t].size-1;i++)
                tellers[t].customers[i] = tellers[t].customers[i+1];
            tellers[t].size--;

            // Serve next in teller queue
            if(tellers[t].size > 0) {
                int nextCID = tellers[t].customers[0];
                double sTime = 2 * avgServiceTime * ((double)rand()/RAND_MAX);
                customers[nextCID].startService = clockTime;
                totalTellerServiceTime[t] += sTime;
                scheduleDeparture(nextCID, t, sTime);
            }
        }
        free(e);
    }

    double avg = totalTimeSpent/numCustomers;
    printf("\nMultiple queue summary:\n");
    printf("Average time in bank: %.2f\n", avg);
    printf("Max time in bank: %.2f\n", maxTimeSpent);
    for(int i=0;i<numTellers;i++)
        printf("Teller %d service time: %.2f\n", i, totalTellerServiceTime[i]);
}
 