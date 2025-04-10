#include "segel.h"
#include "request.h"
#include <time.h>
#include "shared.h"
//#include <pthread.h>
//#include "Queue.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//
// typedef struct Stastistics // args for the thread
// {
//     int id;
//     int t_num;       // The total number of http requests this thread has handled
//     int t_static;    //The total number of http requests this thread has handled
//     int t_dynamic;   //The total number of static requests this thread has handled
//     Queue* waiting;  //waiting_requests_queue
//     //Queue* running;  //running_requests_queue
    
// } UStas ;//  Usage Statistics Usage Statistics
typedef struct ParamsFromCmd
{
    int port;
    int threads;// num of threads
    int queue_size; 
    int sched_lag;
    //char sched_lag[7]; //max(len)=len(random) +\0 =7
} Params ;

// HW3: Parse the new arguments too
void getargs(Params* parms, int argc, char *argv[])
{
    if (argc < 5) {
	fprintf(stderr, "Usage: %s <port> <threads> <queue_size> <schedlag> \n", argv[0]);
	exit(1);
    }
    // if(atoi(argv[2])<0){
    //     //////////////////////////////////////////////////////////
    //     ///TODO////////////////////
    //     exit(1);
    // }
    // if(atoi(argv[3]<0)){
    //     //////////////////////////////////////////////////////////
    //     ///TODO////////////////////
    //     exit(1);
    // }
    //--------------------
    parms->sched_lag = 0;
    if (strcmp(argv[4], "block") == 0){
        parms->sched_lag = 1;
    }
    if (strcmp(argv[4], "dt") == 0){
        parms->sched_lag = 2;
    }
    if (strcmp(argv[4], "dh") == 0){
        parms->sched_lag = 3;
    }
    if (strcmp(argv[4], "bf") == 0){
        parms->sched_lag = 4;
    }
    if(strcmp(argv[4], "random") == 0){
         parms->sched_lag = 5;
    }
    //--------------------
    if (strcmp(argv[4], "block") != 0 && strcmp(argv[4], "dt") != 0 && strcmp(argv[4], "dh") != 0 && strcmp(argv[4], "bf") != 0 && strcmp(argv[4], "random") != 0) {
            // TODO
            ///////////
            exit(1);
    }
    parms->port = atoi(argv[1]); 
    parms->threads = atoi(argv[2]); //num_threads woeker
    parms->queue_size = atoi(argv[3]);
    //printf( " %s argv[3]\n", argv[3]);
    //printf( " %s argv[1]\n", argv[1]);
    //strcpy(parms->sched_lag,argv[4]);
    //printf( " parms->threads:%d \n", parms->threads);
}

pthread_cond_t worker_threads;//condition
pthread_cond_t master_thread;//condition
//pthread_cond_t no_threads;
// pthread_mutex_t m;
int num_of_req = 0; //num of running requests


// what to do the threads
void * threadFun(void* i_args){

    UStas* our_args = (UStas*) i_args;
    Element* element = malloc(sizeof(Element));
    while (1){
        //dequeue//
        pthread_mutex_lock(&m);
        while(our_args->waiting->curr_queue_size==0){
            pthread_cond_wait(&worker_threads, &m);
        }
        //Element* element;
        //struct timeval arrival=queue_time(our_args->waiting);
        //element->arrival_time = queue_time2(our_args->waiting);
        //element->arrival_time= our_args->waiting->tail->data->arrival_time;
        element->arrival_time=queue_time(our_args->waiting);
        //printf("   1Stat-Req-Dispatch:: %lu.%06lu",element->arrival_time.tv_sec, element->arrival_time.tv_usec);
        element->descriptor = dequeue(our_args->waiting);
        if (element->descriptor==-1){
            //printf("AAAAAAAAAA");
            exit(-1);
        }
        
        //printf("   2Stat-Req-Dispatch:: %lu.%06lu",element->arrival_time.tv_sec, element->arrival_time.tv_usec);

        num_of_req++;
        our_args->t_num++;
        struct timeval handle_time;
        struct timeval dispatch_time;
        pthread_mutex_unlock(&m);

        
        gettimeofday(&handle_time, NULL);
        timersub(&handle_time, &element->arrival_time, &dispatch_time);
        //printf("   Stat-Req-Dispatch:: %lu.%06lu",handle_time.tv_sec, handle_time.tv_usec);
        //printf("   Stat-Req-Dispatch:: %lu.%06lu",dispatch_time.tv_sec, dispatch_time.tv_usec);

        ///TODO
        requestHandle(element, our_args, dispatch_time);
        Close(element->descriptor);
        //free(element);
        //TODO ///
        /// ADDD  deleteNode in Queue
        pthread_mutex_lock(&m);
        num_of_req--;
        //printf("%d",num_of_req);
        pthread_cond_signal(&master_thread);
        pthread_mutex_unlock(&m);

    }
    
    return NULL;
}


int main(int argc, char *argv[])
{

    int listenfd, connfd, clientlen;
    Params* params =malloc(sizeof(Params));
    

    struct sockaddr_in clientaddr;

    getargs(params, argc, argv);

     pthread_cond_init(&master_thread, NULL); 
    pthread_cond_init(&worker_threads, NULL);
    pthread_mutex_init(&m, NULL);

    // 
    // HW3: Create some threads...
    Queue* wQueue = createQueue(params->queue_size); //waiting requests queue
    //Queue* rQueue = createQueue(params->queue_size); //running requests queue

    pthread_t* threads_arr = malloc(sizeof(pthread_t) * params->threads);
    if (!threads_arr){
        exit(1);
    }
    for (int i = 0; i < params->threads; i++){
        
        
        UStas* i_args = malloc(sizeof(UStas));
        if(i_args == NULL){
            exit(1);
        }
        i_args->id=i;
        i_args->t_static=0;
        i_args->t_dynamic=0;
        i_args->t_num=0;
        i_args->waiting=wQueue;
        //i_args->running=rQueue;
        pthread_create(&threads_arr[i],NULL,threadFun,(void *)i_args);
    }
   
    //

    listenfd = Open_listenfd(params->port);
    while (1) {

	     clientlen = sizeof(clientaddr);
	     connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
         

        // 
        // HW3: In general, don't handle the request in the main thread.
        // Save the relevant info in a buffer and have one of the worker threads 
        // do the work. 
        // 
        pthread_mutex_lock(&m);

        /////
        struct timeval arrival_time;
        gettimeofday(&arrival_time, NULL);


        ///
        // printf("num_of_req:%d",num_of_req);
        // printf("  wQueue->curr_queue_size:%d",wQueue->curr_queue_size);
        //  printf("   params->queue_size:%d",params->queue_size);
        if (wQueue->curr_queue_size+num_of_req ==  params->queue_size)
        {
            // condions 
            // block 
            // printf("AAAA");
            if (params->sched_lag==1){
                while (wQueue->curr_queue_size+num_of_req >=  params->queue_size){
                    pthread_cond_wait(&master_thread, &m);

                }
                enqueue(wQueue, connfd, arrival_time); //insert element
                pthread_cond_signal(&worker_threads);
                pthread_mutex_unlock(&m);
                continue;
            }
            // dt: drop_tail
            if (params->sched_lag==2){
                Close(connfd);
                pthread_mutex_unlock(&m);
                continue;
            }
            // dh: drop_head
            if (params->sched_lag==3){
                if (isEmpty(wQueue)){
                    Close(connfd);
                    pthread_mutex_unlock(&m);
                    continue;
                }
                else{
                    int descriptor = dequeue(wQueue);
                    Close(descriptor);
                    // enqueue(wQueue,connfd,arrival_time);
                    // close(element->descriptor);
                //    pthread_cond_signal(&worker_threads);
                //    pthread_mutex_unlock(&m);
                   
                }
                /* code */
            }
            // bf:block_flush
            if (params->sched_lag==4){
                while (wQueue->curr_queue_size+num_of_req !=0){
                    pthread_cond_wait(&master_thread, &m);
                }
                //enqueue(wQueue, connfd, arrival_time); //insert element why dont need it 7asab al update aldfo
                Close(connfd);
                //pthread_cond_signal(&worker_threads);
                pthread_mutex_unlock(&m);
                continue;

            }
            //random
            if (params->sched_lag==5){
                /* code */
                //half queue size 
                if (isEmpty( wQueue) ){
                   Close(connfd);
                   pthread_mutex_unlock(&m);
                   continue;
                }
                //else
                // if (wQueue->queue_size_q!=0) {
                    int hqueue= (int)((wQueue->curr_queue_size + 1) / 2);
                    for (int i = 0; i < hqueue; i++){
                        // if (isEmpty( wQueue) ){
                        //     break;
                        // }
                        //random num
                        int rNum=(int) (rand() % wQueue->curr_queue_size);
                        int descriptor = indexDequeue(wQueue,rNum);
                        if(descriptor==-1) exit(-1);
                        Close(descriptor);
                    }
                //     enqueue(wQueue, connfd, arrival_time); //insert element
                //     pthread_cond_signal(&worker_threads);
                //     pthread_mutex_unlock(&m);
                    
                // // }
                // continue;
            }

        }
            /* code */
            enqueue(wQueue, connfd, arrival_time); //insert element
            pthread_cond_signal(&worker_threads); // get worker theread whait waite
            pthread_mutex_unlock(&m);
	//requestHandle(connfd);
	//Close(connfd);
    }
    deleteQueue(wQueue);
    free(threads_arr);
    free(params);
    exit(0);
}


    


 
