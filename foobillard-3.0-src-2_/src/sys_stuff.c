/* sys_stuff.c
**
**    system functions
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

#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#include <sys/time.h>    // us time measure
#include <getopt.h>
#endif
//#include <math.h>

#ifndef USE_SDL
#include <GL/glut.h>
#else
#include "SDL.h"
//KHMan 20040422 no reason to have this here...???
//#include <SDL/SDL_syswm.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

//#ifndef _WIN32
//   #include <sys/time.h>    // us time measure
//   #include <getopt.h>
//#else
//   #include <sys/timeb.h>   // us time measure
//#endif

#include "sys_stuff.h"


/* common stuff */
int time_us()
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
    return SDL_GetTicks()*1000;
//    struct timeb t;
//    return( t.time*1000000+t.millitm*1000 );
#endif
}


double time_s()
// gives back time in seconds
{
    return (((double)time_us())*1E-6);
}


#ifndef USE_SDL    /* use glut */

static int keymodif = 0 ;
static int fullscreen = 0 ;

static void 
update_key_modifiers(void)
{
    int m;
    m=glutGetModifiers();
    keymodif=0 ;
    if (m & GLUT_ACTIVE_SHIFT)   keymodif |= KEY_MODIFIER_SHIFT ;
    if (m & GLUT_ACTIVE_CTRL)    keymodif |= KEY_MODIFIER_CTRL ;
    if (m & GLUT_ACTIVE_ALT)     keymodif |= KEY_MODIFIER_ALT ;
}

static void
handle_mouse_motion(int x, int y)
{
  MouseMotion(x,y,keymodif);
}

static void
handle_key_event(unsigned char key, int x, int y)
{
    update_key_modifiers();
    Key( ((int)key)&0xFF, keymodif ) ;
}

static void
handle_key_up_event(unsigned char key, int x, int y)
{
    update_key_modifiers();
    KeyUp( ((int)key)&0xFF, keymodif ) ;
}

static void
handle_display_event(void)
{
  DisplayFunc() ;
  glutSwapBuffers();
}

static void
handle_special_key_event(int key, int x, int y )
{
  int keysym=0 ;

  update_key_modifiers();

  switch (key) {
  case GLUT_KEY_PAGE_UP:
    keysym = KSYM_PAGE_UP ;
    break;
  case GLUT_KEY_UP:

    keysym = KSYM_UP ;
    break;
  case GLUT_KEY_PAGE_DOWN:
    keysym = KSYM_PAGE_DOWN ;
    break;
  case GLUT_KEY_DOWN:
    keysym = KSYM_DOWN ;
    break;
  case GLUT_KEY_LEFT:
    keysym = KSYM_LEFT ;
    break;
  case GLUT_KEY_RIGHT:
    keysym = KSYM_RIGHT ;
    break;
  case GLUT_KEY_F1:
    keysym = KSYM_F1 ;
     break;
  case GLUT_KEY_F2:
    keysym = KSYM_F2 ;
     break;
  case GLUT_KEY_F3:
    keysym = KSYM_F3 ;
     break;
  case GLUT_KEY_F4:
    keysym = KSYM_F4 ;
     break;
  default:
    return ;
  }
  Key(keysym,keymodif) ;
}


static void
handle_mouse_event(int button,int state, int x, int y)
{
  MouseButtonEnum b ;
  MouseButtonState s ;

  /* Process first the key modifiers */
  update_key_modifiers();

  /* then the mouse buttons */
  switch(button) {
  case GLUT_LEFT_BUTTON:
    b = MOUSE_LEFT_BUTTON;
    break ;
  case GLUT_RIGHT_BUTTON:
    b = MOUSE_RIGHT_BUTTON;
    break ;
  case GLUT_MIDDLE_BUTTON:
    b = MOUSE_MIDDLE_BUTTON;
    break ;
  case 3:
    b = MOUSE_WHEEL_UP_BUTTON;
    break ;
  case 4:
    b = MOUSE_WHEEL_DOWN_BUTTON;
    break ;
  default:
    /* Unknown button: ignore */
    return ; 
  }

  s=-1;
  if( state==GLUT_DOWN ) s = MOUSE_DOWN;
  if( state==GLUT_UP   ) s = MOUSE_UP;
//  s = (state==GLUT_DOWN)?MOUSE_DOWN:MOUSE_UP ;
  
  MouseEvent(b,s,x,y,keymodif) ; 
}


void
sys_resize( int width, int height )
{
    glutReshapeWindow(width,height);
    ResizeWindow(width,height) ;
}


static void 
handle_reshape_event( int width, int height ) 
{
    ResizeWindow(width,height) ;
}


void (*timer_cb)(void) = NULL ;

static void
timer_callback(int v) {
  if (timer_cb!=NULL) {
    timer_cb() ;
  }
}

/* ============================ */


void 
sys_create_display(int *argc, char **argv, int width,int height)
{
  

//   glutInitWindowPosition(0, 0);
   glutInitWindowSize(width,height);

   glutInit(argc, argv );

   glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );

   glutCreateWindow( argv[0] );

   //   glutFullScreen();
}


int sys_get_fullscreen(void)
{
    return fullscreen;
}

void sys_fullscreen( int fullscr, int width, int height )
{
    fullscreen=fullscr;
    if(fullscreen){
        glutFullScreen();
    } else {
        sys_resize(1024,768) ;
    }
}

void sys_toggle_fullscreen( int width, int height )
{
    if (fullscreen){
        sys_fullscreen( 0, width, height );
    } else {
        sys_fullscreen( 1, width, height );
    }
}


void 
sys_redisplay(void)
{
  glutPostRedisplay();
}


void 
sys_exit(int code)
{
  exit(code) ; 
}

void 
sys_set_timer(int ms,void (*cb)(void))
{
  timer_cb = cb ; 
  if (cb!=NULL) {
    glutTimerFunc(ms,timer_callback,0);
  }
}


sysResolution *
sys_list_modes( void )
{
    sysResolution * sysmodes;
    sysmodes = (sysResolution *) malloc(1*sizeof(sysResolution));
    sysmodes[0].w=0;  /* terminator */
    sysmodes[0].h=0;  /* terminator */
    return sysmodes;
}


void
sys_main_loop(void)
{
  
   glutReshapeFunc  ( handle_reshape_event );
   glutKeyboardFunc ( handle_key_event );
   glutKeyboardUpFunc ( handle_key_up_event );
   glutSpecialFunc  ( handle_special_key_event );
//   glutSpecialUpFunc  ( handle_special_key_up_event );
   glutDisplayFunc  ( handle_display_event );
   glutMouseFunc    ( handle_mouse_event );
   glutMotionFunc       ( handle_mouse_motion );
   glutPassiveMotionFunc( handle_mouse_motion );

   glutMainLoop();
}












#else          /* use SDL */






static int fullscreen = 0 ;
static int keymodif =0 ;

static int vidmode_bpp=0 ;

static int sdl_on = 0 ; 
static SDL_Surface * vid_surface = NULL;

void 
sys_exit( int code )
{
  if (sdl_on) {
    /*
     * Quit SDL so we can release the fullscreen
     * mode and restore the previous video settings,
     * etc.
     */
      SDL_Quit( );
  }

  /* Exit program. */
  exit( code );
}

/* ========================================= */


void 
sys_create_display(int *argc, char **argv, int width,int height)
{
  /* Information about the current video settings. */
  const SDL_VideoInfo* info = NULL;
  int vidmode_flags=0 ;

  vid_surface = SDL_GetVideoSurface();

  /* First, initialize SDL's video subsystem. */
  if( SDL_Init( SDL_INIT_VIDEO  ) < 0 ) {
    fprintf( stderr, "Video initialization failed: %s\n",
             SDL_GetError( ) );
    sys_exit(1);
  }
  atexit(SDL_Quit);

  sdl_on = 1 ; 

  /* Let's get some video information. */
  info = SDL_GetVideoInfo( );
  
  if( !info ) {
    /* This should probably never happen. */
    fprintf( stderr, "Video query failed: %s\n",
             SDL_GetError( ) );
    sys_exit(1);
  }
  
  vidmode_bpp = info->vfmt->BitsPerPixel;
  

  /*
   * Now, we want to setup our requested
   * window attributes for our OpenGL window.
   * We want *at least* 5 bits of red, green
   * and blue. We also want at least a 16-bit
   * depth buffer.
   *
   * The last thing we do is request a double
   * buffered window. '1' turns on double
   * buffering, '0' turns it off.
   *
   * Note that we do not use SDL_DOUBLEBUF in
   * the flags to SDL_SetVideoMode. That does
   * not affect the GL attribute state, only
   * the standard 2D blitting setup.
   */
  SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
  SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
  SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

  /* it could be wise to change the default key timing */
  SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL );
//  SDL_EnableKeyRepeat( 0, SDL_DEFAULT_REPEAT_INTERVAL );
  /* key repeat causaed problem when toggling fullscreen !!! */

  /*
   * We want to request that SDL provide us
   * with an OpenGL window, in a fullscreen
   * video mode.
   */
  vidmode_flags = SDL_OPENGL;
  if (fullscreen) {
    vidmode_flags |= SDL_FULLSCREEN;
  }else{
    vidmode_flags |= SDL_RESIZABLE;
  }
  
  if( SDL_SetVideoMode( width, height, vidmode_bpp, vidmode_flags ) == 0 )  {
    fprintf( stderr, "Video mode set failed: %s\n", SDL_GetError( ) );
    sys_exit( 1 );
  }
#ifdef _WIN32
  SDL_WM_SetCaption("FooBillard", "FooBillard");
#endif
}

int sys_get_fullscreen(void)
{
    return fullscreen;
}

void sys_fullscreen( int fullscr, int width, int height )
{
#ifndef _WIN32
    SDL_Surface * screen;
#endif

    fullscreen = fullscr;
#ifndef _WIN32
    screen = SDL_GetVideoSurface();

    if       ( fullscreen!=0 && (screen->flags & SDL_FULLSCREEN)==0 ){
        SDL_WM_ToggleFullScreen(screen);
    } else if( fullscreen==0 && (screen->flags & SDL_FULLSCREEN)!=0 ){
        SDL_WM_ToggleFullScreen(screen);
    }
#endif
/*    if (fullscreen) {
        vidmode_flags |= SDL_FULLSCREEN;
        vidmode_flags &= (~SDL_RESIZABLE);
    } else {
        vidmode_flags &= (~SDL_FULLSCREEN);
        vidmode_flags |= SDL_RESIZABLE;
    }
    SDL_SetVideoMode( width, height, vidmode_bpp, vidmode_flags ); */
}

void sys_toggle_fullscreen( int width, int height )
{
/*    SDL_Surface *screen = SDL_GetVideoSurface();
    SDL_WM_ToggleFullScreen(screen);
    fullscreen = (fullscreen==0)?1:0;*/
    if (fullscreen){
        sys_fullscreen( 0, width, height );
    } else {
        sys_fullscreen( 1, width, height );
    }
}


void 
sys_redisplay(void)
{
}

void (*timer_cb)(void) = NULL ;

void 
sys_set_timer(int ms,void (*cb)(void)) 
{
  timer_cb = cb ; 
}

static void 
update_key_modifiers(void)
{
  SDLMod m ;
  m=SDL_GetModState();
  keymodif=0 ;
  if (KMOD_CTRL  & m) keymodif |= KEY_MODIFIER_CTRL ;
  if (KMOD_SHIFT & m) keymodif |= KEY_MODIFIER_SHIFT ;
  if (KMOD_ALT   & m) keymodif |= KEY_MODIFIER_ALT ;
   
}

static void
handle_button_event(SDL_MouseButtonEvent *e)
{
  MouseButtonEnum b ;
  MouseButtonState s ;

  update_key_modifiers() ;

  /* then the mouse buttons */
  switch(e->button) {
  case SDL_BUTTON_LEFT:   
    b = MOUSE_LEFT_BUTTON; 
    break ;
  case SDL_BUTTON_RIGHT: 
    b = MOUSE_RIGHT_BUTTON;
    break ;
  case SDL_BUTTON_MIDDLE: 
    b = MOUSE_MIDDLE_BUTTON;
    break ;
  case 4:
    b = MOUSE_WHEEL_UP_BUTTON;
    break ;
  case 5:
    b = MOUSE_WHEEL_DOWN_BUTTON;
    break ;
  default:
    /* Unknown button: ignore */
    return ; 
  }

  s = -1;
  if(e->state==SDL_PRESSED)  s=MOUSE_DOWN;
  if(e->state==SDL_RELEASED) s=MOUSE_UP;
//  s = (e->state==SDL_PRESSED)?MOUSE_DOWN:MOUSE_UP ;
  
  MouseEvent(b,s,e->x,e->y,keymodif) ; 
}


static int translate_key(SDL_KeyboardEvent* e)
{
  int keysym=0;

  switch (e->keysym.sym) {
  case SDLK_PAGEUP:
    keysym = KSYM_PAGE_UP ;
    break;
  case SDLK_UP:
    keysym = KSYM_UP ;
    break;
  case SDLK_PAGEDOWN:
    keysym = KSYM_PAGE_DOWN ;
    break;
  case SDLK_DOWN:
    keysym = KSYM_DOWN ;
    break;
  case SDLK_LEFT:
    keysym = KSYM_LEFT ;
    break;
  case SDLK_RIGHT:
    keysym = KSYM_RIGHT ;
    break;
  case SDLK_F1:
    keysym = KSYM_F1 ;    
    break;
  case SDLK_F2:
    keysym = KSYM_F2 ;
    break;
  case SDLK_F3:
    keysym = KSYM_F3 ;
    break;
  case SDLK_F4:
    keysym = KSYM_F4 ;
    break;
  case SDLK_KP_ENTER:
    keysym = KSYM_KP_ENTER ;
    break;
  default:
    if (e->keysym.sym>0 && e->keysym.sym<=127) {
      keysym = (int) e->keysym.sym ;
    } else {
      /* ignore */
      return -1;
    }
  }
  return keysym;
}

static void
handle_key_down(SDL_KeyboardEvent* e)
{
  int keysym;

  update_key_modifiers();

  keysym = translate_key(e);

  if(keysym!=-1){
      Key(keysym, keymodif);
  }
}


static void
handle_key_up(SDL_KeyboardEvent* e)
{
  int keysym;

  update_key_modifiers();

  keysym = translate_key(e);

  if(keysym!=-1){
      KeyUp(keysym, keymodif);
  }
}


void
sys_resize( int width, int height )
{
#ifndef _WIN32
    SDL_Surface * screen;
    screen = SDL_GetVideoSurface();
    SDL_EnableKeyRepeat( 0, 0 );
    SDL_SetVideoMode( width, height, screen->format->BitsPerPixel, screen->flags );
    SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL );
#endif
    ResizeWindow(width,height) ;
}


static void
handle_reshape_event( int width, int height ) 
{
    sys_resize( width, height );
}


void
handle_motion_event(SDL_MouseMotionEvent *e) 
{
  update_key_modifiers();
  MouseMotion(e->x,e->y,keymodif);
}


static void 
process_events( void )
{
  /* Our SDL event placeholder. */
  SDL_Event event;

    /* Grab all the events off the queue. */
  while( SDL_PollEvent( &event ) ) 
    {
      switch( event.type ) {
      case SDL_KEYUP:
	handle_key_up( &event.key );
	break;
      case SDL_KEYDOWN:
	/* Handle key presses. */
	handle_key_down( &event.key );
	break;
      case SDL_QUIT:
	/* Handle quit requests (like Ctrl-c). */
	sys_exit(0);
	break;
      case SDL_MOUSEMOTION:
	handle_motion_event(&(event.motion)) ; 
	break ; 
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
	handle_button_event(&(event.button)) ; 
        break ;
      case SDL_VIDEORESIZE:
        handle_reshape_event(event.resize.w,event.resize.h);
        break;
      default:
//	printf( "EVENT: %d\n", (int) event.type ) ;
        break;
      }
    }
}


sysResolution *
sys_list_modes( void )
{
    sysResolution * sysmodes;
    SDL_Rect ** modes;
    int i, modenr;

    modes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);
    for(i=0;modes[i];i++);
    modenr=i;
    sysmodes = (sysResolution *) malloc((modenr+1)*sizeof(sysResolution));
    for(i=0;modes[i];i++){
        sysmodes[i].w = modes[i]->w;
        sysmodes[i].h = modes[i]->h;
    }
    sysmodes[i].w=0;  /* terminator */
    sysmodes[i].h=0;  /* terminator */

    return( sysmodes );
}


void
sys_main_loop(void)
{
  while(1) {
    process_events() ;
    DisplayFunc() ;
    SDL_GL_SwapBuffers( );
//    glReadBuffer(GL_FRONT);
//    glDrawBuffer(GL_FRONT);

    if (timer_cb!=NULL) {
      timer_cb();
    }
    SDL_Delay(10);
  }


}

#endif
