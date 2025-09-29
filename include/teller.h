#ifndef TELLER_H
#define TELLER_H

typedef struct Teller {
    int id;
    float totalServiceTime;
    float totalIdleTime;
} Teller;

Teller* createTeller(int id);

#endif
