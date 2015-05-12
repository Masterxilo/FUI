#include <errno.h>       /* obligatory includes */
#include <signal.h>
#include <stdio.h>
#include <string.h>      /* memset memcpy ...   */
#include <stdlib.h>      /* exit                */
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#ifndef _WIN32
   #include <sys/time.h>    // us time measure
#else
   #include <sys/timeb.h>   // us time measure
#endif
#include "options.h"
#include "sys_stuff.h"


#define PORTNUM options_net_portnum /* random port number, we need something */
#define MAXHOSTNAME 1000


/* code to establish a socket; originally from bzs@bu-cs.bu.edu
 */

int establish(unsigned short portnum)
{ char   myname[MAXHOSTNAME+1];
  int    s;
  struct sockaddr_in sa;
  struct hostent *hp;

  memset(&sa, 0, sizeof(struct sockaddr_in)); /* clear our address */
  gethostname(myname, MAXHOSTNAME);           /* who are we? */
  hp= gethostbyname(myname);                  /* get our address info */
  if (hp == NULL)                             /* we don't exist !? */
    return(-1);
  sa.sin_family= hp->h_addrtype;              /* this is our host address */
  sa.sin_port= htons(portnum);                /* this is our port number */
  if ((s= socket(AF_INET, SOCK_STREAM, 0)) < 0) /* create socket */
    return(-1);
  if (bind(s,(struct sockaddr *)&sa,sizeof(struct sockaddr_in)) < 0) {
    close(s);
    return(-1);                               /* bind address to socket */
  }
  listen(s, 3);                               /* max # of queued connects */
  return(s);
}


/* wait for a connection to occur on a socket created with establish()
 */
int get_connection(int s)
{ int t;                  /* socket of connection */

  fprintf(stderr,"get_connection\n");
  if ((t = accept(s,NULL,NULL)) < 0)   /* accept connection if there is one */
      return(-1);
  else
      return(t);
}


/* as children die we should get catch their returns or else we get
 * zombies, A Bad Thing.  fireman() catches falling children.
 */
void fireman(void)
{
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
}



int host_create_socket()
{
    int s, t;

    fprintf(stderr,"host_create_socket: establishing connection\n");
    if ((s= establish(PORTNUM)) < 0) {  /* plug in the phone */
        perror("establish");
        exit(1);
    }

    fprintf(stderr,"host_create_socket: waiting for call...\n");
    while( (t= get_connection(s)) < 0 ){ /* get a connection */
        fprintf(stderr,"get_connection(s)=%d\n",t);
    }
    fprintf(stderr,"get_connection(s)=%d\n",t);
    fprintf(stderr,"hallo\n");

//    fcntl(s,F_SETFL,O_NONBLOCK);
    fprintf(stderr, "fcntl=%d\n", fcntl(t, F_SETFL, fcntl(t, F_GETFL, 0) | O_NONBLOCK) );

    return t;
}


int client_call_socket_once(char *hostname)
{ struct sockaddr_in sa;
  struct hostent     *hp;
  int s, conn;

  if ((hp= gethostbyname(hostname)) == NULL) { /* do we know the host's */
    errno= ECONNREFUSED;                       /* address? */
    return(-1);                                /* no */
  }

  memset(&sa,0,sizeof(sa));
  memcpy((char *)&sa.sin_addr,hp->h_addr,hp->h_length); /* set address */
  sa.sin_family= hp->h_addrtype;
  sa.sin_port= htons((u_short)PORTNUM);

  if ((s= socket(hp->h_addrtype,SOCK_STREAM,0)) < 0)   /* get socket */
    return(-1);
  fprintf(stderr,"client_call_socket_once:socket()=%d\n",s);
  if ((conn=connect(s,(struct sockaddr *)&sa,sizeof sa)) < 0) {                  /* connect */
    close(s);
    return(-2);
  }
  fprintf(stderr,"client_call_socket_once:conect()=%d\n",conn);
//  fcntl(s,F_SETFL,O_NONBLOCK);
  fprintf(stderr, "fcntl=%d\n", fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK) );

  return(s);
}


int net_sock_time_us()
// gives back time in microseconds
{
#ifndef _WIN32
    struct timeval tv;
    struct timezone tz;
    tz.tz_minuteswest = 0;
    tz.tz_dsttime     = 0;
    gettimeofday(&tv,&tz);
    return ( tv.tv_sec*1000000+tv.tv_usec );
#else
    struct timeb t;
    return( t.time*1000000+t.millitm*1000 );
#endif
}


int client_call_socket(char *hostname)
{
    int rval, t;
    while( (rval=client_call_socket_once(hostname)) < 0 ){
        t=net_sock_time_us();
        while(t+1000000>net_sock_time_us());
    }
    fprintf(stderr,"client_call_socket:%d\n",rval);
    return rval;
}



int socket_read(int s,     /* connected socket */
              char *buf, /* pointer to the buffer */
              int n      /* number of characters (bytes) we want */
             )
{ int bcount; /* counts bytes read */
  int br;     /* bytes read this pass */
  double t0;

  t0=time_s();

#define TIMEOUT 5.0

  bcount= 0;
  br= 0;
  while( bcount<n  &&  (time_s()-t0)<=TIMEOUT ) {             /* loop until full buffer */
    if((br= read(s,buf,n-bcount)) > 0) {
      bcount += br;                /* increment byte counter */
      buf += br;                   /* move buffer ptr for next read */
    }
    else if( br<0 && bcount==0 ){               /* signal an error to the caller */
//        if( errno==EAGAIN ) fprintf(stderr,"error:EAGAIN\n");
        return 0;
    }
  }
  return(bcount);
}


int socket_write(int s,     /* connected socket */
              char *buf, /* pointer to the buffer */
              int n      /* number of characters (bytes) we want */
             )
{ int bcount; /* counts bytes read */
  int br;     /* bytes read this pass */
  double t0;

  t0=time_s();

#define TIMEOUT 5.0

  bcount= 0;
  br= 0;
  while( bcount<n  && (time_s()-t0)<=TIMEOUT ) {             /* loop until full buffer */
    if((br= write(s,buf,n-bcount)) > 0) {
      bcount += br;                /* increment byte counter */
      buf += br;                   /* move buffer ptr for next read */
    }
    else if(br < 0)              /* signal an error to the caller */
      return(-1);
  }
  return(bcount);
}
