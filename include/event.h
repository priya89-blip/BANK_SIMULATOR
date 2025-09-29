#ifndef EVENT_H
#define EVENT_H

typedef enum { CUSTOMER_ARRIVAL, CUSTOMER_SERVICE_COMPLETE, TELLER_IDLE_COMPLETE } EventType;

typedef struct Event {
    float time;
    EventType type;
    void *actor;                       // Customer* or Teller*
    void (*action)(struct Event*);     // Function pointer
    struct Event *next;
} Event;

void addEvent(Event *newEvent);
Event* popEvent();
void executeEvent(Event *e);

#endif
