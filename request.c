//
// request.c: Does the bulk of the work for the web server.
// 

#include "segel.h"
#include "request.h"
//#include "Queue.h"
#include "shared.h"
// requestError(      fd,    filename,        "404",    "Not found", "OS-HW3 Server could not find this file");
void requestError(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg,Element* element,UStas* our_args,struct timeval dispatch_time ) 
{
   char buf[MAXLINE], body[MAXBUF];

   // Create the body of the error message
   sprintf(body, "<html><title>OS-HW3 Error</title>");
   sprintf(body, "%s<body bgcolor=""fffff"">\r\n", body);
   sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
   sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
   sprintf(body, "%s<hr>OS-HW3 Web Server\r\n", body);

   // Write out the header information for this response
   sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Content-Type: text/html\r\n");
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Content-Length: %lu\r\n", strlen(body));
   // Rio_writen(fd, buf, strlen(buf));
   // printf("%s", buf);
   //addd prinet
   sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, element->arrival_time.tv_sec, element->arrival_time.tv_usec);

	sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, dispatch_time.tv_sec, dispatch_time.tv_usec);

	sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, our_args->id);

	sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, our_args->t_num);

	sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, our_args->t_static);

	sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n\r\n", buf, our_args->t_dynamic);

   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   // Write out the content
   Rio_writen(fd, body, strlen(body));
   printf("%s", body);

}


//
// Reads and discards everything up to an empty text line
//
void requestReadhdrs(rio_t *rp)
{
   char buf[MAXLINE];

   Rio_readlineb(rp, buf, MAXLINE);
   while (strcmp(buf, "\r\n")) {
      Rio_readlineb(rp, buf, MAXLINE);
   }
   return;
}

//
// Return 1 if static, 0 if dynamic content
// Calculates filename (and cgiargs, for dynamic) from uri
//
int requestParseURI(char *uri, char *filename, char *cgiargs) 
{
   char *ptr;

   if (strstr(uri, "..")) {
      sprintf(filename, "./public/home.html");
      return 1;
   }

   if (!strstr(uri, "cgi")) {
      // static
      strcpy(cgiargs, "");
      sprintf(filename, "./public/%s", uri);
      if (uri[strlen(uri)-1] == '/') {
         strcat(filename, "home.html");
      }
      return 1;
   } else {
      // dynamic
      ptr = index(uri, '?');
      if (ptr) {
         strcpy(cgiargs, ptr+1);
         *ptr = '\0';
      } else {
         strcpy(cgiargs, "");
      }
      sprintf(filename, "./public/%s", uri);
      return 0;
   }
}

//
// Fills in the filetype given the filename
//
void requestGetFiletype(char *filename, char *filetype)
{
   if (strstr(filename, ".html")) 
      strcpy(filetype, "text/html");
   else if (strstr(filename, ".gif")) 
      strcpy(filetype, "image/gif");
   else if (strstr(filename, ".jpg")) 
      strcpy(filetype, "image/jpeg");
   else 
      strcpy(filetype, "text/plain");
}

void requestServeDynamic(Element* element, char *filename, char *cgiargs,UStas* our_args, struct timeval dispatch_time)
{
   char buf[MAXLINE], *emptylist[] = {NULL};
   our_args->t_dynamic ++; // added this newly , make sure it works correctly
   // The server does only a little bit of the header.  
   // The CGI script has to finish writing out the header.
   sprintf(buf, "HTTP/1.0 200 OK\r\n");
   sprintf(buf, "%sServer: OS-HW3 Web Server\r\n", buf);
   ////////prints
    sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, element->arrival_time.tv_sec, element->arrival_time.tv_usec);
    sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, dispatch_time.tv_sec, dispatch_time.tv_usec);
    sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, our_args->id);
    sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, our_args->t_num);
    sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, our_args->t_static);
    sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n", buf, our_args->t_dynamic);

   Rio_writen(element->descriptor, buf, strlen(buf));
   int  pid = Fork();
   if ( pid == 0) {
      /* Child process */
      Setenv("QUERY_STRING", cgiargs, 1);
      /* When the CGI process writes to stdout, it will instead go to the socket */
      Dup2(element->descriptor, STDOUT_FILENO);
      Execve(filename, emptylist, environ);
   }
   WaitPid(pid, NULL, WUNTRACED);
}


void requestServeStatic(Element* element, char *filename, int filesize,UStas* our_args, struct timeval dispatch_time) 
{
   int srcfd;
   char *srcp, filetype[MAXLINE], buf[MAXBUF];
   our_args->t_static ++; // added this newly , make sure it works correctly

   //sprintf(buf, "GET %s HTTP/1.1 /r/n host: %s", filename, ); 

   requestGetFiletype(filename, filetype);

   srcfd = Open(filename, O_RDONLY, 0);

   // Rather than call read() to read the file into memory, 
   // which would require that we allocate a buffer, we memory-map the file
   srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
   Close(srcfd);

  

   // put together response
   sprintf(buf, "HTTP/1.0 200 OK\r\n");
   sprintf(buf, "%sServer: OS-HW3 Web Server\r\n", buf);
   
   sprintf(buf, "%sContent-Length: %d\r\n", buf, filesize);
   sprintf(buf, "%sContent-Type: %s\r\n", buf, filetype);///////////////
   ///// nages mn han ano etba3 (Header: ) baol kl sater
   // sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, element->arrival_time.tv_sec, element->arrival_time.tv_usec);
   // sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, dispatch_time.tv_sec, dispatch_time.tv_usec);
   // sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, our_args->id);
   // sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, our_args->t_num);
   // sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, our_args->t_static);
   // sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n", buf, our_args->t_dynamic);
   sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, element->arrival_time.tv_sec, element->arrival_time.tv_usec);

	sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, dispatch_time.tv_sec, dispatch_time.tv_usec);

	sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, our_args->id);

	sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, our_args->t_num);

	sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, our_args->t_static);

	sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n\r\n", buf, our_args->t_dynamic);

   Rio_writen(element->descriptor, buf, strlen(buf));

   //  Writes out to the client socket the memory-mapped file 
   Rio_writen(element->descriptor, srcp, filesize);
   Munmap(srcp, filesize);

}

// handle a request
void requestHandle(Element* element, UStas* our_args, struct timeval dispatch_time)
{
   int fd =element->descriptor;
   int is_static;
   struct stat sbuf;
   char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
   char filename[MAXLINE], cgiargs[MAXLINE];
   rio_t rio;
   Rio_readinitb(&rio, fd);
   Rio_readlineb(&rio, buf, MAXLINE);
   sscanf(buf, "%s %s %s", method, uri, version);
   printf("%s %s %s\n", method, uri, version);
   //printf("ZZZZZZZ\n");
   //printf("AAAAA\n");

   if (strcasecmp(method, "GET")) {
      requestError(fd, method, "501", "Not Implemented", "OS-HW3 Server does not implement this method",element,our_args,dispatch_time);
      return;
   }
   requestReadhdrs(&rio);
   // Special suffix policy – “Just a question”
    if (strstr(uri, ".skip")) {
        pthread_mutex_lock(&m);
         //printf("BBBBBBBBBBBBB\n");
        if (our_args->waiting->curr_queue_size > 0) {
            struct timeval handle_time;
            struct timeval dispatch_time;
            Element* new_element = malloc(sizeof(Element));
            new_element->arrival_time = queue_time2(our_args->waiting);
            int skip_fd = removeLast(our_args->waiting); // Remove the latest request from the waiting queue
            pthread_mutex_unlock(&m);
            //printf("AAAAAAAAAAAAAA\n");
            if (skip_fd!=-1)
            {
               new_element->descriptor = skip_fd;
               our_args->t_num++;
               gettimeofday(&handle_time, NULL);
               timersub(&handle_time, &new_element->arrival_time, &dispatch_time);
               requestHandle(new_element, our_args, dispatch_time); // Handle the skipped request
               Close(skip_fd);
            }
        } else {
            pthread_mutex_unlock(&m);
        }
        char* skip_pos = strstr(uri, ".skip");
        if (skip_pos) {
            *skip_pos = '\0'; // Truncate the string at the start of .skip
        }
    }


   is_static = requestParseURI(uri, filename, cgiargs);
   if (stat(filename, &sbuf) < 0) {
      requestError(fd, filename, "404", "Not found", "OS-HW3 Server could not find this file",element,our_args,dispatch_time);
      return;
   }

   if (is_static) {
      if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
         requestError(fd, filename, "403", "Forbidden", "OS-HW3 Server could not read this file",element,our_args,dispatch_time);
         return;
      }
      requestServeStatic(element, filename, sbuf.st_size,our_args,dispatch_time);
   } else {
      if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
         requestError(fd, filename, "403", "Forbidden", "OS-HW3 Server could not run this CGI program",element,our_args,dispatch_time);
         return;
      }
      requestServeDynamic(element, filename, cgiargs,our_args,dispatch_time);
   }
   
}


