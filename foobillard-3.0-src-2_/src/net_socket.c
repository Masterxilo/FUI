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

int     wsa_initialized=0;
WSADATA wsa_data;
#define timeval TIMEVAL

int ensure_WSAInitialized() {
    _NOT_IMPLEMENTED
}
#endif

/* code to establish a socket; originally from bzs@bu-cs.bu.edu
 */
int establish(unsigned short portnum)
{
    _NOT_IMPLEMENTED
}


/* wait for a connection to occur on a socket created with establish()
 */
int get_connection(int s)
{
    _NOT_IMPLEMENTED
}


/* as children die we should get catch their returns or else we get
 * zombies, A Bad Thing.  fireman() catches falling children.
 */
void fireman(void)
{
    _NOT_IMPLEMENTED
}



int host_create_socket()
{
    _NOT_IMPLEMENTED
}


int client_call_socket_once(char *hostname)
{
    _NOT_IMPLEMENTED
}


int net_sock_time_us()
// gives back time in microseconds
{
    _NOT_IMPLEMENTED
}


int client_call_socket(char *hostname)
{
    _NOT_IMPLEMENTED
}



int socket_read(int s,     /* connected socket */
              char *buf, /* pointer to the buffer */
              int n      /* number of characters (bytes) we want */
             )
{
    _NOT_IMPLEMENTED
}


int socket_write(int s,     /* connected socket */
              char *buf, /* pointer to the buffer */
              int n      /* number of characters (bytes) we want */
             )
{
    _NOT_IMPLEMENTED
}
