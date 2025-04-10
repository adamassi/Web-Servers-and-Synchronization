#ifndef WEBSERVER_FILES_QUEUE_H
#define WEBSERVER_FILES_QUEUE_H
#include "segel.h"
#include <stdbool.h>


////this is the data in the queue node
typedef struct element_t{
    int descriptor;
    struct timeval arrival_time;
}Element;

////this is the node in the linked list that is a queue
typedef struct node_t{
    Element* data;
    struct node_t* prev;
    struct node_t* next;
}Node;

typedef struct queue_t{
    Node* head;
    Node* tail;
    int curr_queue_size;
    int queue_size_q;
}Queue;


Queue* createQueue(int queue_size_q);

void deleteQueue(Queue* queue);
void deleteNode(Node* node);
struct timeval queue_time(Queue* queue);
struct timeval queue_time2(Queue* queue);
//insert element
int enqueue(Queue* queue,int connfd, struct timeval arrival_time);
int dequeue(Queue* queue);
int removeLast(Queue* queue);
int isEmpty(Queue* queue);
// bool isFull(Queue* queue);

int indexDequeue(Queue* queue, int random_num);


#endif //WEBSERVER_FILES_QUEUE_H
