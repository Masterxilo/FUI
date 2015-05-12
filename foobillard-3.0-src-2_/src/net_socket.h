#ifndef NET_SOCKET_H
#define NET_SOCKET_H


int host_create_socket();


int client_call_socket(char *hostname);



int socket_read(int s,     /* connected socket */
              char *buf, /* pointer to the buffer */
              int n      /* number of characters (bytes) we want */
             );


int socket_write(int s,     /* connected socket */
              char *buf, /* pointer to the buffer */
              int n      /* number of characters (bytes) we want */
             );


#endif
