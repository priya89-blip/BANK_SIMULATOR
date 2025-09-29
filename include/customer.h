#ifndef CUSTOMER_H
#define CUSTOMER_H

typedef struct Customer {
    int id;
    float arrivalTime;
    float serviceStartTime;
    float serviceEndTime;
} Customer;

Customer* createCustomer(int id, float arrivalTime);

#endif
