#include <stdlib.h>
#include "customer.h"

Customer* createCustomer(int id, float arrivalTime){
    Customer *c = (Customer*)malloc(sizeof(Customer));
    c->id = id;
    c->arrivalTime = arrivalTime;
    c->serviceStartTime = 0;
    c->serviceEndTime = 0;
    return c;
}
