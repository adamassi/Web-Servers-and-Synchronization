#ifndef __REQUEST_H__
#include "Queue.h"
typedef struct Stastistics // args for the thread
{
    int id;
    int t_num;// The total number of http requests this thread has handled
    int t_static;//The total number of http requests this thread has handled
    int t_dynamic;//The total number of static requests this thread has handled
    Queue* waiting;  //waiting_requests_queue
    //Queue* running;  //running_requests_queue
    
} UStas ;//  Usage Statistics Usage Statistics

void requestHandle(Element* element, UStas* our_args, struct timeval dispatch_time);

#endif
