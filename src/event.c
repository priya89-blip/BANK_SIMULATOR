#include <stdlib.h>
#include "event.h"

Event *eventQueueHead = NULL;

void addEvent(Event *newEvent){
    if(!eventQueueHead || newEvent->time < eventQueueHead->time){
        newEvent->next = eventQueueHead;
        eventQueueHead = newEvent;
        return;
    }
    Event *curr = eventQueueHead;
    while(curr->next && curr->next->time < newEvent->time) curr = curr->next;
    newEvent->next = curr->next;
    curr->next = newEvent;
}

Event* popEvent(){
    if(!eventQueueHead) return NULL;
    Event *e = eventQueueHead;
    eventQueueHead = eventQueueHead->next;
    e->next = NULL;
    return e;
}

void executeEvent(Event *e){
    if(e && e->action) e->action(e);
}
