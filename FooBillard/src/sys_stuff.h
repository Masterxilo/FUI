#ifndef SYS_STUFF_H
#define SYS_STUFF_H
#include "myinclude.h"
typedef enum 
{
  MOUSE_LEFT_BUTTON=1,
  MOUSE_RIGHT_BUTTON=2,
  MOUSE_MIDDLE_BUTTON=3,
  MOUSE_WHEEL_UP_BUTTON=4,
  MOUSE_WHEEL_DOWN_BUTTON=5
} MouseButtonEnum ;


typedef enum 
{
  MOUSE_UP=1,
  MOUSE_DOWN=2
} MouseButtonState ;



#define KEY_MODIFIER_SHIFT 0x01
#define KEY_MODIFIER_CTRL  0x02
#define KEY_MODIFIER_ALT   0x04


enum {
  KSYM_UP=257 ,
  KSYM_DOWN ,
  KSYM_LEFT ,
  KSYM_RIGHT ,
  KSYM_PAGE_DOWN ,
  KSYM_PAGE_UP ,
  KSYM_F1 ,
  KSYM_F2 ,
  KSYM_F3 ,
  KSYM_F4 ,
  KSYM_F5 ,
  KSYM_F6 ,
  KSYM_F7 ,
  KSYM_F8 ,
  KSYM_F9 ,
  KSYM_F10 ,
  KSYM_F11 ,
  KSYM_F12 ,
  KSYM_KP_ENTER
} ;

typedef struct{
    int w, h;
} sysResolution;


int time_us(void);
double time_s(void);


// variables and functions in main.c to be called by the system
void Key( int key, int modifiers ) ;
void KeyUp( int key, int modifiers ) ;
void MouseEvent(MouseButtonEnum button,MouseButtonState  state, int x, int y,int key_modifiers) ;
void MouseMotion(int x, int y,int key_modifiers) ;
void DisplayFunc(void) ;
void ResizeWindow(int w,int h) ;
//extern int fullscreen;

// system functions 

void sys_create_display(int *argc, char **argv, int width,int height) ;
int  sys_get_fullscreen(void);
void sys_fullscreen( int fullscr, int width, int height );
void sys_toggle_fullscreen( int width, int height );
void sys_main_loop(void) ;
void sys_resize( int width, int height );
void sys_redisplay(void) ;
void sys_set_timer(int ms,void (*cb)(void)) ;
void sys_exit( int code ) ;

sysResolution * sys_list_modes(void);

void sys_remove_titlebar(void);

#endif  /* SYS_STUFF_H */
