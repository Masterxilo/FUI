/* net_socket.c
**
**    networking functions
**    Copyright (C) 2001-2004  Florian Berger
**    Email: harpin_floh@yahoo.de, florian.berger@jk.uni-linz.ac.at
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License Version 2 as
**    published by the Free Software Foundation;
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program; if not, write to the Free Software
**    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
*/
#include "myinclude.h"
#include <errno.h>       /* obligatory includes */
#include <signal.h>
#include <stdio.h>
#include <string.h>      /* memset memcpy ...   */
#include <stdlib.h>      /* exit                */
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#if defined(__WIN32__) || defined(WIN32)
#include "SDL.h"
#include <winsock2.h>
#define ECONNREFUSED WSAECONNREFUSED
#else /* UNIX */
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#endif /* WIN32 */

#ifndef _WIN32
#include <sys/time.h>    // us time measure
#else
//   #include <sys/timeb.h>   // us time measure
#endif
#include "options.h"
#include "sys_stuff.h"


#define PORTNUM options_net_portnum /* random port number, we need something */
#define MAXHOSTNAME 1000

#ifdef _WIN32
#define write(a,b,c) send((a),(b),(c),0)
#define read(a,b,c) recv((a),(b),(c),0)

int     wsa_initialized = 0;
WSADATA wsa_data;
#define timeval TIMEVAL

int ensure_WSAInitialized() {
  if (wsa_initialized)
    return 0;
  if (WSAStartup(0x101, &wsa_data))
    return -1;
  wsa_initialized = 1;
  return 0;
}
#endif

/* code to establish a socket; originally from bzs@bu-cs.bu.edu
 */
int establish(unsigned short portnum)
{
  char   myname[MAXHOSTNAME + 1];
  int    s;
  struct sockaddr_in sa;
  struct hostent *hp;

#ifdef _WIN32
  if (ensure_WSAInitialized() < 0)
    return -1;
#endif

  memset(&sa, 0, sizeof(struct sockaddr_in)); /* clear our address */
  gethostname(myname, MAXHOSTNAME);           /* who are we? */
  hp = gethostbyname(myname);                  /* get our address info */
  if (hp == NULL)                             /* we don't exist !? */
    return(-1);
  sa.sin_family = hp->h_addrtype;              /* this is our host address */
  sa.sin_port = htons(portnum);                /* this is our port number */
  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) /* create socket */
    return(-1);
  if (bind(s, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) < 0) {
    _close(s);
    return(-1);                               /* bind address to socket */
  }
  listen(s, 3);                               /* max # of queued connects */
  return(s);
}


/* wait for a connection to occur on a socket created with establish()
 */
int get_connection(int s)
{
  int t;                  /* socket of connection */

  fprintf(stderr, "get_connection\n");
  if ((t = accept(s, NULL, NULL)) < 0)   /* accept connection if there is one */
    return(-1);
  else
    return(t);
}


/* as children die we should get catch their returns or else we get
 * zombies, A Bad Thing.  fireman() catches falling children.
 */
void fireman(void)
{
#ifndef _WIN32
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
#endif
}



int host_create_socket()
{
  int s, t;

  fprintf(stderr, "host_create_socket: establishing connection\n");
  if ((s = establish(PORTNUM)) < 0) {  /* plug in the phone */
    perror("establish");
    exit(1);
  }

  fprintf(stderr, "host_create_socket: waiting for call...\n");
  while ((t = get_connection(s)) < 0){ /* get a connection */
    fprintf(stderr, "get_connection(s)=%d\n", t);
  }
  fprintf(stderr, "get_connection(s)=%d\n", t);
  fprintf(stderr, "hallo\n");

  //    fcntl(s,F_SETFL,O_NONBLOCK);
#ifndef _WIN32
  fprintf(stderr, "fcntl=%d\n", fcntl(t, F_SETFL, fcntl(t, F_GETFL, 0) | O_NONBLOCK) );
#else
  /*u_long arg=1;
  ioctlsocket(t, FIONBIO, &arg);*/
#endif

  return t;
}


int client_call_socket_once(char *hostname)
{
  struct sockaddr_in sa;
  struct hostent     *hp;
  int s, conn;

#ifdef _WIN32
  if (ensure_WSAInitialized() < 0)
    return -1;
#endif

  if ((hp = gethostbyname(hostname)) == NULL) { /* do we know the host's */
    errno = ECONNREFUSED;                       /* address? */
    return(-1);                                /* no */
  }

  memset(&sa, 0, sizeof(sa));
  memcpy((char *)&sa.sin_addr, hp->h_addr, hp->h_length); /* set address */
  sa.sin_family = hp->h_addrtype;
  sa.sin_port = htons((u_short)PORTNUM);

  if ((s = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0)   /* get socket */
    return(-1);
  fprintf(stderr, "client_call_socket_once:socket()=%d\n", s);
  if ((conn = connect(s, (struct sockaddr *)&sa, sizeof sa)) < 0) {                  /* connect */
    _close(s);
    return(-2);
  }
  fprintf(stderr, "client_call_socket_once:conect()=%d\n", conn);
  //  fcntl(s,F_SETFL,O_NONBLOCK);
#ifndef _WIN32
  fprintf(stderr, "fcntl=%d\n", fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK) );
#else
  /*u_long arg=1;
  ioctlsocket(conn, FIONBIO, &arg);*/
#endif

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
  return SDL_GetTicks() * 1000;
  //    struct timeb t;
  //    return( t.time*1000000+t.millitm*1000 );
#endif
}


int client_call_socket(char *hostname)
{
  int rval, t;
  while ((rval = client_call_socket_once(hostname)) < 0){
    t = net_sock_time_us();
    while (t + 1000000>net_sock_time_us());
  }
  fprintf(stderr, "client_call_socket:%d\n", rval);
  return rval;
}



int socket_read(int s,     /* connected socket */
  char *buf, /* pointer to the buffer */
  int n      /* number of characters (bytes) we want */
  )
{
  int bcount; /* counts bytes read */
  int br;     /* bytes read this pass */
  double t0;

  t0 = time_s();

#define TIMEOUT 5.0

  timeval tv;
  tv.tv_sec = TIMEOUT;
  tv.tv_usec = 0;

  bcount = 0;
  br = 0;
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(s, &fds);
  while (bcount<n && (tv.tv_sec>0 || tv.tv_usec > 0)) {             /* loop until full buffer */
    select(s + 1, &fds, 0, &fds, &tv);
    if ((br = read(s, buf, n - bcount)) > 0) {
      bcount += br;                /* increment byte counter */
      buf += br;                   /* move buffer ptr for next read */
    }
    else if (br<0 && bcount == 0){               /* signal an error to the caller */
      //        if( errno==EAGAIN ) fprintf(stderr,"error:EAGAIN\n");
      return 0;
    }
    int sec = t0 + TIMEOUT - time_s();
    tv.tv_sec = sec>0 ? sec : 0;
    tv.tv_usec = 0;
  }
  return(bcount);
}


int socket_write(int s,     /* connected socket */
  char *buf, /* pointer to the buffer */
  int n      /* number of characters (bytes) we want */
  )
{
  int bcount; /* counts bytes written */
  int bw;     /* bytes written this pass */

  bcount = 0;
  bw = 0;
  while (bcount<n) {             /* loop until no more data */
    if ((bw = write(s, buf, n - bcount)) > 0) {
      bcount += bw;                /* increment byte counter */
      buf += bw;                   /* move buffer ptr for next write */
    }
    else if (bw < 0)              /* signal an error to the caller */
      return(-1);
  }
  return(bcount);
}
