#include <stdlib.h>
#include "teller.h"

Teller* createTeller(int id){
    Teller *t = (Teller*)malloc(sizeof(Teller));
    t->id = id;
    t->totalServiceTime = 0;
    t->totalIdleTime = 0;
    return t;
}
