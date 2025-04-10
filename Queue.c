#include "Queue.h"


Queue* createQueue(int queue_size_q)
{
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    if(queue == NULL)
    {
        return NULL;
    }

    queue->curr_queue_size = 0;
    queue->queue_size_q = queue_size_q;
    queue->head = NULL;
    queue->tail = NULL;

    return queue;

}


void deleteQueue(Queue* queue)
{
    if(queue == NULL)
    {
        return;
    }
    while(queue->head != NULL)
    {
        Node* tmp = queue->head->next;
        deleteNode(queue->head);
        queue->head = tmp;
        queue->curr_queue_size--;
    }
    free(queue);
}

void deleteNode(Node* node){
    //free(node->data);
    //node->next = NULL;
    //node->prev = NULL;
    free(node);
}

struct timeval queue_time(Queue* queue){
    if(isEmpty(queue))
        return (struct timeval){0};
    return queue->head->data->arrival_time;
}
struct timeval queue_time2(Queue* queue) {
    if (isEmpty(queue)) {
        struct timeval zero = {0};
        return zero;
    }
    return queue->tail->data->arrival_time;
}


int enqueue(Queue* queue, int connfd, struct timeval arrival_time)
{
    Node* in_node = malloc(sizeof(Node));
    Element* elem = malloc(sizeof(Element));
    in_node->next=NULL;
    in_node->prev=NULL;
    //in_node->data = malloc(sizeof(Element));
    //fprintf(stderr, "here6");
    in_node->data = elem;
    in_node->data->descriptor = connfd;
    //fprintf(stderr, "here7");
    in_node->data->arrival_time = arrival_time;
    if(queue->curr_queue_size == 0)
    {
        queue->head = in_node;
        queue->tail = in_node;
        queue->curr_queue_size++;
        
        //fprintf(stderr, "here8");
        return 1;
    }

    else{
        // if(queue->curr_queue_size <=queue->queue_size_q) {
            queue->tail->next = in_node;
           // queue->tail->next->prev = queue->tail;
            queue->tail = in_node;
            //queue->tail->next=NULL;
            queue->curr_queue_size++;
            return 1;
        //}
    }
    return 0;

}
//remove elament from the queue 
int dequeue(Queue* queue)
{
     
    if(queue->curr_queue_size==0)
    {
        return -1;
    }
    Node* tmp = queue->head;
    int returned =queue->head->data->descriptor;
    //free(queue->head->data);
    // free(queue->head);
    if(tmp->next == NULL){
        queue->head = NULL;
        queue->tail = NULL;
    } else {
        queue->head = queue->head->next;
        queue->head->prev=NULL;
    }
    //free(tmp);
    // if(queue->curr_queue_size==1)
    // {
    //     queue->head->next->prev = NULL;
    //     Node* tmp = queue->head->next;
    //     returned = queue->head->data->descriptor;
    //     free(queue->head);
    //     queue->head = tmp;
    //     queue->curr_queue_size--;
    //     return returned;
    // }
    // returned=queue->head->data->descriptor;
    // free(queue->head);
    // queue->head = NULL;
    // queue->tail = NULL;
    queue->curr_queue_size--;

    // queue->head = queue->head->next;
    // int returned =  tmp->data->descriptor;
    ///free(tmp->data);//
    //free(tmp);
    return returned;


}
int removeLast(Queue* queue) {
    if (queue->curr_queue_size == 0) {
        return -1;
    }
    Node* tmp = queue->tail;
    int returned = queue->tail->data->descriptor;
    if (queue->tail->prev == NULL) {
        queue->head = NULL;
        queue->tail = NULL;
    } else {
        queue->tail = queue->tail->prev;
        queue->tail->next = NULL;
    }
    queue->curr_queue_size--;
    deleteNode(tmp);
    return returned;
}


int isEmpty(Queue* queue)
{
    if(queue->curr_queue_size == 0)
        return 1;
    else
        return 0;
}

// bool isFull(Queue* queue)
// {
//     if(queue->curr_queue_size == queue->queue_size_q)
//         return true;
//     else
//         return false;
// }

int indexDequeue(Queue* queue, int random_num)
{
    if(random_num == 0)
    {
        return dequeue(queue);
    }
    if(random_num > queue->curr_queue_size)
    {
        return -1;
    }
    if(queue->curr_queue_size == 0)
    {
        return -1;
    }
    Node* tmp = queue->head;
    int returned = -1;
    for(int i = 0; i < queue->curr_queue_size; i++)
    {
        if(i == random_num){
            ///its the first one
            if(tmp->prev == NULL){
                if(tmp->next != NULL){
                    tmp->next->prev = NULL;
                }

                queue->head = tmp->next;  
            }else {

                if(tmp->next != NULL) {
                    tmp->next->prev = tmp->prev;
                }
                tmp->prev->next = tmp->next;

            }

            queue->curr_queue_size--;
            returned = tmp->data->descriptor;
            //deleteNode(tmp);
            return returned;

        }

    tmp = tmp->next;
    }

    return returned;
}