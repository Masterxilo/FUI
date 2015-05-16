/* billard3d.c
**
**    drawing all with OpenGL
**    Copyright (C) 2001-2004  Florian Berger
**    Email: harpin_floh@yahoo.de, florian.berger@jk.uni-linz.ac.at
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License Version 2 as
**    published by the Free Software Ffoundation;
**f
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
#include <string.h>
#include <math.h>
#include <unistd.h>
extern  int wantFullscreen;
#if defined(__WIN32__) || defined(WIN32)
/* replaces missing endian.h on Win32 */
#define	__LITTLE_ENDIAN	1234
#define	__BIG_ENDIAN	4321
#ifdef USE_SDL /* grab endian from SDL */
#include "SDL_byteorder.h"
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define __BYTE_ORDER __BIG_ENDIAN
#else
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif
#else /* assume IA32!!! */
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif
#else /* UNIX */
#include <endian.h>
#endif /* WIN32 */

#ifndef USE_SDL
#include <GL/glut.h>
#else
#include <GL/glu.h>
#include <GL/gl.h>
#endif

//#include <sys/timeb.h>  /* my_time */
#ifndef _WIN32
#include <sys/time.h>    // us time measure
#include <getopt.h>
#else
//#include <GL/glext.h>
#include <GL/glext.h>
#include "SDL.h"
//   #include <sys/timeb.h>   // us time measure
#endif

#ifdef _WIN32
#include <shlobj.h>
/* Win*Path - returns convenient paths (WIN32 only) */
char *WinExePath(void);
char *WinDataPath(void);
char *WinRCPath(void);
/* Mingw's libgw32c has getopt_long_only() function here */
#include <getopt.h>
#endif

#include "billard.h"
#include "ball.h"
#include "table.h"
#include "queue.h"
#include "png_loader.h"
#include "aiplayer.h"
#include "options.h"
#include "player.h"
#include "evaluate_move.h"
#include "helpscreen.h"

#ifdef options_use_freetype
#include "font.h"
#include "textobj.h"
#endif

#ifdef XMESA
#include <GL/xmesa.h>
#endif


#include "sys_stuff.h"

#include "net_socket.h"

#include "sound_stuff.h"


#include "menu.h"
#include "gamemenu.h"



#define LIT 1
#define TEXTURED 2
#define REFLECT 3
#define ANIMATE 10
#define POINT_FILTER 20
#define LINEAR_FILTER 21
#define QUIT 100

#define CUE_BALL_IND (player[act_player].cue_ball)
#define CUE_BALL_POS (balls.ball[CUE_BALL_IND].r)
#define CUE_BALL_XYPOS (vec_xyz(CUE_BALL_POS.x,CUE_BALL_POS.y,0.0))

#define CUEBALL_MAXSPEED 7.0

//#define strcpy_whtspace_2_uscore(dst,src) {int _i_; for(_i_=0;(dst[_i_]=src[_i_])!=0;_i_++) if(dst[_i_]==' ') dst[_i_]='_';}
//#define strcpy_uscore_2_whtspace(dst,src) {int _i_; for(_i_=0;(dst[_i_]=src[_i_])!=0;_i_++) if(dst[_i_]=='_') dst[_i_]=' ';}
#define strcpy_uscore_2_whtspace(d,s) {int i; for(i=0;(d[i]=(s[i]!='_'?s[i]:' '))!=0;i++);}
#define strcpy_whtspace_2_uscore(d,s) {int i; for(i=0;(d[i]=(s[i]!=' '?s[i]:'_'))!=0;i++);}

/* control-flags */
int control__updated = 0;  /* just activated */
int control__active = 0;   /* one conrol is active */
/* the controls */
int control__english = 0;
int control__cue_butt_updown = 0;
int control__mouse_shoot = 0;
int control__place_cue_ball = 0;

//#define WIDTH   1280
//#define HEIGHT  1024
//#define WIDTH   1024
//#define HEIGHT   768

int win_width = 800;
int win_height = 600;
#ifdef _WIN32
/* remembers resolution change for Win32 */
int win_width_change = 0;
int win_height_change = 0;
#endif

//int win_width = 1024;
//int win_height = 768;

#define WIDTH    win_width
#define HEIGHT   win_height

//#define TIME_INTERPOLATE

double scr_dpi = 80.0;

BallsType balls;
BallsType bakballs;
BordersType walls;


static int frametime_ms_min = 10;
static int frametime_ms_max = 200;
static int frametime_ms = 40;


static int fullscreen = 0;  /* this is not updated during runtime - its only for startup */

static GLuint table_obj = 0;
static GLboolean Animate = GL_TRUE;

GLfloat Xrot = -70.0, Yrot = 0.0, Zrot = 0.0;
static GLfloat Xque = -83.0, Zque = 0.0;
static GLfloat Xrot_offs = 0.0, Yrot_offs = 0.0, Zrot_offs = 0.0;
static GLfloat scale = 1.0;



int b1_hold = 0;
int start_x, start_y;
int mouse_moved_after_b1_dn = 0;

int b2_hold = 0;
int scaling_start, scaling_start2;
int b2_b1_hold = 0;


GLfloat cam_dist_aim = 2.5;
GLfloat cam_dist;
GLfloat cam_FOV = 40.0;


VMvect  free_view_pos_aim;
VMvect  free_view_pos;
#define FREE_VIEW  ((!queue_view) && options_free_view_on)


static int  vline_on = 1;
//static int  key_modifiers;
int  queue_view = 1;
static int  old_queue_view = 1;

static double queue_anim = 0.0;
static GLfloat queue_offs = 0.06;
#define queue_point_x  (player[act_player].cue_x)
#define queue_point_y  (player[act_player].cue_y)
#define queue_strength (player[act_player].strength)

int  balls_moving = 0;

/* reflection map */
static int    spheretexw, spheretexh;
static char * spheretexdata;
static GLuint spheretexbind;
static char * lightspheretexdata;
static GLuint lightspheretexbind;
static GLuint reftexbind;
static GLuint placecueballtexbind;
static GLuint blendetexbind;
static GLuint lightflaretexbind;

static GLuint halfsymboltexbind;
static GLuint fullsymboltexbind;
static GLuint fullhalfsymboltexbind;

static GLuint fblogotexbind;

static int  show_fps = 0;

static int  helpscreen_on = 0;

/* cubemap reflection stuff */
//#define MAX_BALLS 22
//static int   cuberef_res=128;
#define cuberef_res options_cuberef_res
//static int   cubereftexbind=0;
static int   cuberef_allballs_texbind_nr = 0;
static int * cuberef_allballs_texbind = 0;



VMvect comp_dir;

static int  human_human_mode = 0;
int  act_player = 0;   /* 0 or 1 */
static char * player_names[] = {"Human Player", "AI Player", "Human Player 2", "AI Player 2"};
static char * half_full_names[] = {"any", "full", "half"};
static int  b1_b2_hold = 0;

/*
static struct Player{
int is_AI;
int half_full;
int queue_view;
double Zque;
double Xque;
double cue_x;
double cue_y;
double strength;
char *name;
} player[2];
*/
struct Player player[2];

VMvect lightpos[10];
int    lightnr = 3;

enum gameType gametype = GAME_8BALL;


static int  g_socket = 0;
static int  g_network_play = 0;
static int  g_is_host = 0;



//static int  g_lookballnr;


static menuType  * g_act_menu;
static menuType  * g_main_menu;
static menuType  * g_options_menu;
static int menu_on = 0;


int   g_shot_due = 1;  /* a shot to be due at the beginning */
float g_motion_ratio = 1.0;  /* a shot to be due at the beginning */

typedef enum
{
  NET_STATE_REQ,
  NET_STATE_ACK,
  NET_STATE_DATA
} NetState;

NetState g_net_state = NET_STATE_REQ;

typedef struct
{
  double Xrot;
  double Zrot;
  double white_x;
  double white_y;
  double cue_x;
  double cue_y;
  double strength;
  int    shoot;
} NetData;
NetData g_net_data;


//#define Xque (player[act_player].X_que)
//#define Zque (player[act_player].Z_que)
//#define queue_view (player[act_player].queueview)


#ifdef USE_SOUND
static TSound ball_ball_snd;
static TSound ball_wall_snd;
static TSound ball_cue_snd;
#define SOUND_NULLOFFS 10000
#endif

/*#define TIME_INTERPOLATE*/

#ifdef TIME_INTERPOLATE
int g_frametime_laststep;
int g_frametime_fromlast;
BallsType g_lastballs;
BallsType g_drawballs;
#endif


#define ROSTER_MAX_NUM 128
struct PlayerRoster{
  int             nr;       /* number of players */
  struct Player   player[ROSTER_MAX_NUM];   /* players */
} human_player_roster;


#define TOURNAMENT_ROUND_NUM 4
static struct TournamentState_ {

  int round_num;

  int game_ind;
  int round_ind;

  int wait_for_next_match; /* show status meanwhile */
  int wait_for_next_round; /* show status meanwhile */
  int overall_winner;
  int tournament_over;
  double ai_fast_motion;

  struct {
    int roster_player1;
    int roster_player2;
    int winner;
  } game[TOURNAMENT_ROUND_NUM/*rounds*/][1 << (TOURNAMENT_ROUND_NUM - 1)/*games*/];

  struct PlayerRoster roster;

} tournament_state;

static textObj * winner_name_text_obj;
static textObj * winner_text_obj;

//#ifndef _WIN32

enum optionType
{
  OPT_PLAYER1,
  OPT_PLAYER2,
  OPT_NAME1,
  OPT_NAME2,
  OPT_HELP,
  OPT_8BALL,
  OPT_9BALL,
  OPT_CARAMBOL,
  OPT_SNOOKER,
  OPT_TABLECOL,
  OPT_EDGECOL,
  OPT_FRAMECOL,
  OPT_CHROMEBLUE,
  OPT_GOLDGREEN,
  OPT_GOLDRED,
  OPT_BLACKWHITE,
  OPT_BLACKBEIGE,
  OPT_TABLESIZE,
  OPT_LENSFLARE,
  OPT_NOLENSFLARE,
  OPT_POSLIGHT,
  OPT_DIRLIGHT,
  OPT_AI1ERR,
  OPT_AI2ERR,
  OPT_BALLDETAIL,
  OPT_RGSTEREO,
  OPT_RGAIM,
  /*    OPT_NETGAME,
      OPT_HOST,*/
      OPT_HOSTADDR,
      OPT_PORTNUM,
      OPT_GEOMETRY,
      OPT_FULLSCREEN,
      OPT_FREEMOVE,
      OPT_CUBEREF,
      OPT_CUBERES,
      OPT_BUMPREF,
      OPT_BUMPWOOD,
      OPT_BALLTRACE,
      OPT_GAMEMODE,
      OPT_BALL_FRESNEL,
      OPT_AVATAR,
      OPT_TOURFAST,
      OPT_CLOTHTEX,
      OPT_DUMMY
};


static char * appname_str = "foobillard";


static struct option long_options[] =
{
  {"player1", required_argument, (int *)"arg=ai|human set player1 ai/human", OPT_PLAYER1},
  {"player2", required_argument, (int *)"arg=ai|human set player2 ai/human", OPT_PLAYER2},
  {"p1", required_argument, (int *)"arg=ai|human set player1 ai/human", OPT_PLAYER1},
  {"p2", required_argument, (int *)"arg=ai|human set player2 ai/human", OPT_PLAYER2},
  {"name1", required_argument, (int *)"set name of player1", OPT_NAME1},
  {"name2", required_argument, (int *)"set name of player2", OPT_NAME2},
  {"8ball", no_argument, (int *)"8ball pool game", OPT_8BALL},
  {"9ball", no_argument, (int *)"9ball pool game", OPT_9BALL},
  {"carambol", no_argument, (int *)"carambol billard game", OPT_CARAMBOL},
  {"snooker", no_argument, (int *)"snooker billard game", OPT_SNOOKER},
  {"tablecolor", required_argument, (int *)"table color in C-style hex notation <0xrrggbb>", OPT_TABLECOL},
  {"edgecolor", required_argument, (int *)"edge color in C-style hex notation <0xrrggbb>", OPT_EDGECOL},
  {"framecolor", required_argument, (int *)"frame color in C-style hex notation <0xrrggbb>", OPT_FRAMECOL},
  {"chromeblue", no_argument, (int *)"blue table with chrome edges", OPT_CHROMEBLUE},
  {"goldgreen", no_argument, (int *)"green table with gold edges", OPT_GOLDGREEN},
  {"goldred", no_argument, (int *)"red table with gold edges", OPT_GOLDRED},
  {"blackwhite", no_argument, (int *)"black table with white frame", OPT_BLACKWHITE},
  {"blackbeige", no_argument, (int *)"beige table with balck metal", OPT_BLACKBEIGE},
  {"tablesize", required_argument, (int *)"table size (length) in foot (default=7.0)", OPT_TABLESIZE},
  {"lensflare", no_argument, (int *)"turn on lensflare", OPT_LENSFLARE},
  {"nolensflare", no_argument, (int *)"turn off lensflare", OPT_NOLENSFLARE},
  {"poslight", no_argument, (int *)"use positional light", OPT_POSLIGHT},
  {"dirlight", no_argument, (int *)"use directional light", OPT_DIRLIGHT},
  {"ai1err", required_argument, (int *)"to err is artificial (player1 error 0..1)", OPT_AI1ERR},
  {"ai2err", required_argument, (int *)"to err is artificial (player2 error 0..1)", OPT_AI2ERR},
  {"balldetail", required_argument, (int *)"set ball detail l[ow] m[edium] h[igh] or v[eryhigh]", OPT_BALLDETAIL},
  {"rgstereo", no_argument, (int *)"start in stereo mode (red-green(cyan))", OPT_RGSTEREO},
  {"rgaim", required_argument, (int *)"arg=left|right|middle for aiming eye position", OPT_RGAIM},
  //    {"netgame",      no_argument,       (int *)"host a networkgame (player2=client)",             OPT_NETGAME},
  //    {"host",         required_argument, (int *)"arg=IP play network game with IP as host",        OPT_HOST},
  {"hostaddr", required_argument, (int *)"arg=IP-address for TCP/IP connection", OPT_HOSTADDR},
  {"portnum", required_argument, (int *)"arg=port# for TCP/IP connection", OPT_PORTNUM},
  {"geometry", required_argument, (int *)"<width>x<height> window geometry", OPT_GEOMETRY},
  {"fullscreen", no_argument, (int *)"play in fullscreen mode", OPT_FULLSCREEN},
  {"freemove", required_argument, (int *)"arg=on|off free move in external view mode", OPT_FREEMOVE},
  {"cuberef", required_argument, (int *)"arg=on|off rendered cubemap reflections", OPT_CUBEREF},
  {"cuberes", required_argument, (int *)"arg=<texture size for cuberef> (must be power of 2)", OPT_CUBERES},
  {"bumpref", required_argument, (int *)"arg=on|off bumpmap reflections of edges", OPT_BUMPREF},
  {"bumpwood", required_argument, (int *)"arg=on|off bumpmap of wood covers", OPT_BUMPWOOD},
  {"balltraces", required_argument, (int *)"arg=on|off trace lines of balls", OPT_BALLTRACE},
  {"gamemode", required_argument, (int *)"arg=match|training|tournament", OPT_GAMEMODE},
  {"fresnel", required_argument, (int *)"arg=on|off fresnel ball reflections", OPT_BALL_FRESNEL},
  {"avatar", required_argument, (int *)"arg=on|off enable/disable avatar", OPT_AVATAR},
  {"tourfast", required_argument, (int *)"arg=1.0..10.0 AI fast motion ratio for tournament", OPT_TOURFAST},
  {"clothtex", required_argument, (int *)"arg=on|off for table detail map", OPT_CLOTHTEX},
  {"help", no_argument, (int *)"this help", OPT_HELP},
  {NULL, 0, NULL, 0}
};


void set_gametype(int gtype);



void process_option(enum optionType act_option)
{
  switch (act_option){
  case OPT_PLAYER1:
    human_player_roster.player[0].is_AI = (optarg[0] == 'a') ? 1 : 0;
    human_player_roster.player[0].queue_view = (optarg[0] == 'a') ? 0 : 1;
    // FIXME
    queue_view = human_player_roster.player[0].queue_view;
    break;
  case OPT_PLAYER2:
    human_player_roster.player[1].is_AI = (optarg[0] == 'a') ? 1 : 0;
    human_player_roster.player[1].queue_view = (optarg[0] == 'a') ? 0 : 1;
    break;
  case OPT_NAME1:       strcpy_uscore_2_whtspace(human_player_roster.player[0].name, optarg); break;
  case OPT_NAME2:       strcpy_uscore_2_whtspace(human_player_roster.player[1].name, optarg); break;
  case OPT_8BALL:       set_gametype(GAME_8BALL);   break;
  case OPT_9BALL:       set_gametype(GAME_9BALL);   break;
  case OPT_CARAMBOL:    set_gametype(GAME_CARAMBOL);   break;
  case OPT_SNOOKER:     set_gametype(GAME_SNOOKER);   break;
  case OPT_TABLECOL:    sscanf(optarg, "%x", &options_table_color);   break;
  case OPT_EDGECOL:     sscanf(optarg, "%x", &options_diamond_color); break;
  case OPT_FRAMECOL:    sscanf(optarg, "%x", &options_frame_color);   break;
  case OPT_CHROMEBLUE:  options_diamond_color = options_diamond_color_chrome; options_table_color = options_table_color_blue;   options_frame_tex_var = 1; break;
  case OPT_GOLDGREEN:   options_diamond_color = options_diamond_color_gold;   options_table_color = options_table_color_green;  options_frame_tex_var = 1; break;
  case OPT_GOLDRED:     options_diamond_color = options_diamond_color_gold;   options_table_color = options_table_color_red;    options_frame_tex_var = 1; break;
  case OPT_BLACKBEIGE:  options_diamond_color = options_diamond_color_black;  options_table_color = options_table_color_beige;  options_frame_tex_var = 1; break;
  case OPT_BLACKWHITE:  options_diamond_color = options_diamond_color_black;  options_table_color = options_table_color_black;  options_frame_tex_var = 0;   options_frame_color = options_frame_color_white;  break;
  case OPT_TABLESIZE:   sscanf(optarg, "%lf", &options_table_size); options_table_size *= 0.3048; break;
  case OPT_LENSFLARE:   options_lensflare = 1; break;
  case OPT_NOLENSFLARE: options_lensflare = 0; break;
  case OPT_POSLIGHT:    options_positional_light = 1; break;
  case OPT_DIRLIGHT:    options_positional_light = 0; break;
  case OPT_AI1ERR:      sscanf(optarg, "%lf", &(human_player_roster.player[0].err)); break;
  case OPT_AI2ERR:      sscanf(optarg, "%lf", &(human_player_roster.player[1].err)); break;
  case OPT_BALLDETAIL:  switch (optarg[0]){
  case 'l':
    options_max_ball_detail = options_max_ball_detail_LOW;
    options_ball_detail_nearmax = options_ball_detail_nearmax_LOW;
    options_ball_detail_farmin = options_ball_detail_farmin_LOW;
    break;
  case 'm':
    options_max_ball_detail = options_max_ball_detail_MED;
    options_ball_detail_nearmax = options_ball_detail_nearmax_MED;
    options_ball_detail_farmin = options_ball_detail_farmin_MED;
    break;
  case 'h':
    options_max_ball_detail = options_max_ball_detail_HIGH;
    options_ball_detail_nearmax = options_ball_detail_nearmax_HIGH;
    options_ball_detail_farmin = options_ball_detail_farmin_HIGH;
    break;
  case 'v':
    options_max_ball_detail = options_max_ball_detail_VERYHIGH;
    options_ball_detail_nearmax = options_ball_detail_nearmax_VERYHIGH;
    options_ball_detail_farmin = options_ball_detail_farmin_VERYHIGH;
    break;
  }
                        break;
  case OPT_RGSTEREO:    options_rgstereo_on = 1; break;
  case OPT_RGAIM:       if (optarg[0] == 'l') options_rgaim = 1;
    if (optarg[0] == 'r') options_rgaim = 2;
    if (optarg[0] == 'm') options_rgaim = 0;
    break;
    /*       case OPT_NETGAME:     player[1].is_net=1;    player[0].is_net=0;
                                 g_is_host=1;
                                 g_network_play=1;
                                 break;
                                 case OPT_HOST:        player[1].is_net=0;    player[0].is_net=1;
                                 g_is_host=0;
                                 g_network_play=1;
                                 options_net_hostname = optarg;
                                 break;*/
  case OPT_HOSTADDR:    strcpy(options_net_hostname, optarg);
    break;
  case OPT_PORTNUM:     sscanf(optarg, "%d", &options_net_portnum);
    break;
  case OPT_GEOMETRY:    sscanf(optarg, "%dx%d", &win_width, &win_height); break;
  case OPT_FULLSCREEN:  fullscreen = 1; break;
  case OPT_FREEMOVE:    switch (optarg[1]){
  case 'f': /* off */
    options_free_view_on = 0; break;
  case 'n': /* on  */
    options_free_view_on = 1; break;
  }
                        break;
  case OPT_CUBEREF:     switch (optarg[1]){
  case 'f': /* off */
    options_cuberef = 0; break;
  case 'n': /* on  */
    options_cuberef = 1; break;
  }
                        break;
  case OPT_CUBERES:     sscanf(optarg, "%d", &options_cuberef_res);
    break;
  case OPT_BUMPREF:     switch (optarg[1]){
  case 'f': /* off */
    options_bumpref = 0; break;
  case 'n': /* on  */
    options_bumpref = 1; break;
  }
                        break;
  case OPT_BUMPWOOD:    switch (optarg[1]){
  case 'f': /* off */
    options_bumpwood = 0; break;
  case 'n': /* on  */
    options_bumpwood = 1; break;
  }
                        break;
  case OPT_BALLTRACE:   switch (optarg[1]){
  case 'f': /* off */
    options_balltrace = 0; break;
  case 'n': /* on  */
    options_balltrace = 1; break;
  }
                        break;
  case OPT_GAMEMODE:    if (_strnicmp("match", optarg, 5) == 0){
                          options_gamemode = options_gamemode_match;
  }
                        else if (_strnicmp("train", optarg, 5) == 0){
                          options_gamemode = options_gamemode_training;
                        }
                        else if (_strnicmp("tourn", optarg, 5) == 0){
                          options_gamemode = options_gamemode_tournament;
                        }
                        break;
  case OPT_BALL_FRESNEL:switch (optarg[1]){
  case 'f': /* off */
    options_ball_fresnel_refl = 0; break;
  case 'n': /* on  */
    options_ball_fresnel_refl = 1; break;
  }
                        break;
  case OPT_AVATAR:      switch (optarg[1]){
  case 'f': /* off */
    options_avatar_on = 0; break;
  case 'n': /* on  */
    options_avatar_on = 1; break;
  }
                        break;
  case OPT_TOURFAST:    sscanf(optarg, "%lf", &options_tourfast); break;
  case OPT_CLOTHTEX:    switch (optarg[1]){
  case 'f': /* off */
    options_cloth_tex = 0; break;
  case 'n': /* on  */
    options_cloth_tex = 1; break;
  }
                        break;
  case OPT_HELP:        exit(1);   break;
  case OPT_DUMMY:       break;
  }
}


void print_help(struct option * opt, char *appname, FILE * io)
{
  int i;

  fprintf(io, "usage: %s [--option [<arg>]]\n", appname);
  fprintf(io, "  options:\n");
  for (i = 0; opt[i].name != 0; i++){
    fprintf(io, "--%s %s\n", opt[i].name, opt[i].has_arg ? "<arg>" : "");
    fprintf(io, "     %s\n", (char *)(opt[i].flag));
    opt[i].flag = NULL;
  }
  printf("the color <0xrrggbb> means one byte for each red, green, blue\n");
  //  printf("the transparency specification is optional e.g. <0xrrggbb>\n");
  fprintf(io, "\n");
}


int load_config(char *** confv, int * confc, char ** argv, int argc)
{
  FILE * f;
  int c, i;
  char * str;
  char allstr[64000];
  char filename[512];

  *confc = 1;
  str = allstr;

#ifndef _WIN32
  sprintf(filename,"%s/.foobillardrc",getenv("HOME"));
#else
  if (getenv("HOME")) {
    sprintf(filename, "%s/.foobillardrc", getenv("HOME"));
  }
  else {
    strcpy(filename, WinRCPath());
  }
#endif

  if ((f = fopen(filename, "rb")) != NULL){

    do{
      str[0] = '-'; str[1] = '-';
      for (i = 2; (c = fgetc(f)) != '\n' && c != EOF; i++){
        if (c != ' ' && c != 0x13 && c != 0x0A) str[i] = c;
        else {
          str[i] = 0;
          (*confc)++;
          //                    while((c=fgetc(f))==' ' && c!=EOF);
        }
      }
      str[i] = 0;
      if (str[2] != 0){
        (*confc)++;
        /*           fprintf(stderr,"confstring:<%s> confc=%d\n",str,*confc);*/
        str += i + 1;
      }
    } while (c != EOF);

    *confv = malloc((argc + *confc)*sizeof(char *));
    str = allstr;
    /*    fprintf(stderr,"allstr:<%s>\n",allstr);*/
    (*confv)[0] = argv[0];
    for (i = 1; i < *confc; i++){
      (*confv)[i] = str;
      /*        fprintf(stderr,"confstring2:<%s>\n",(*confv)[i]);*/
      if (i != (*confc) - 1){ for (; (*str) != 0; str++); str++; }
    }
    for (i = 1; i < argc; i++){
      (*confv)[*confc + i - 1] = argv[i];
    }
    (*confc) += argc - 1;
  }
  else {
    (*confv) = argv;
    *confc = argc;
    /*      fprintf(stderr,"no rc file found\n");*/
  }

  /*    printf("number of args = %d\n",*confc);
      for(i=0;i<*confc;i++){
      printf("arg %d = %s\n",i,(*confv)[i]);
      }*/

  return (f != NULL);
}


void write_rc(FILE * f, int opt, char * arg)
{
  int i;
  for (i = 0; i < OPT_DUMMY && long_options[i].val != opt; i++);

  if (arg != NULL){
    //        int j;
    char argstr[256];
    //        for(j=0;(argstr[j]=arg[j])!=0;j++) if(argstr[j]==' ') argstr[j]=='_';
    strcpy_whtspace_2_uscore(argstr, arg);
    fprintf(f, "%s=%s\n", long_options[i].name, argstr);
  }
  else {
    fprintf(f, "%s\n", long_options[i].name);
  }
}


void save_config()
{
  int opt;
  FILE * f;
  char filename[512];
  char str[256];

#ifndef _WIN32
  sprintf(filename,"%s/.foobillardrc",getenv("HOME"));
#else
  if (getenv("HOME")) {
    sprintf(filename, "%s/.foobillardrc", getenv("HOME"));
  }
  else {
    strcpy(filename, WinRCPath());
  }
#endif
  if ((f = fopen(filename, "wb")) == NULL){
    fprintf(stderr, "can't write to %s - check rights\n", filename);
    return;
  }

  for (opt = 0; opt < OPT_DUMMY; opt++){
    DPRINTF("save_config: writing option %d\n", opt);
    switch (opt){
    case OPT_PLAYER1:     write_rc(f, opt, (human_player_roster.player[0].is_AI) ? "ai" : "human"); break;
    case OPT_PLAYER2:     write_rc(f, opt, (human_player_roster.player[1].is_AI) ? "ai" : "human"); break;
    case OPT_NAME1:       write_rc(f, opt, human_player_roster.player[0].name); break;
    case OPT_NAME2:       write_rc(f, opt, human_player_roster.player[1].name); break;
    case OPT_8BALL:       if (gametype == GAME_8BALL)    write_rc(f, opt, NULL);  break;
    case OPT_9BALL:       if (gametype == GAME_9BALL)    write_rc(f, opt, NULL);  break;
    case OPT_CARAMBOL:    if (gametype == GAME_CARAMBOL) write_rc(f, opt, NULL);  break;
    case OPT_SNOOKER:     if (gametype == GAME_SNOOKER)  write_rc(f, opt, NULL);  break;
      /*       case OPT_TABLECOL:    sprintf(str,"0x%06X",&options_table_color);   write_rc(OPT_TABCOL,str);  break;
             case OPT_EDGECOL:     sprintf(str,"0x%06X",&options_diamond_color); write_rc(OPT_EDGECOL,str);  break;
             case OPT_FRAMECOL:    sprintf(str,"0x%06X",&options_frame_color);   write_rc(OPT_FRAMECOL,str);  break;*/
    case OPT_CHROMEBLUE:  if (options_diamond_color == options_diamond_color_chrome && options_table_color == options_table_color_blue  && options_frame_tex_var == 1) write_rc(f, opt, NULL); break;
    case OPT_GOLDGREEN:   if (options_diamond_color == options_diamond_color_gold   && options_table_color == options_table_color_green && options_frame_tex_var == 1) write_rc(f, opt, NULL); break;
    case OPT_GOLDRED:     if (options_diamond_color == options_diamond_color_gold   && options_table_color == options_table_color_red   && options_frame_tex_var == 1) write_rc(f, opt, NULL); break;
    case OPT_BLACKBEIGE:  if (options_diamond_color == options_diamond_color_black  && options_table_color == options_table_color_beige && options_frame_tex_var == 1) write_rc(f, opt, NULL); break;
    case OPT_BLACKWHITE:  if (options_diamond_color == options_diamond_color_black  && options_table_color == options_table_color_black && options_frame_tex_var == 0 && options_frame_color == options_frame_color_white) write_rc(f, opt, NULL);   break;
    case OPT_TABLESIZE:   sprintf(str, "%f", options_table_size / 0.3048); write_rc(f, opt, str); break;
    case OPT_LENSFLARE:   if (options_lensflare) write_rc(f, opt, NULL); break;
      /*       case OPT_POSLIGHT:    options_positional_light=1; break;
             case OPT_DIRLIGHT:    options_positional_light=0; break;*/
    case OPT_AI1ERR:      sprintf(str, "%f", human_player_roster.player[0].err); write_rc(f, opt, str); break;
    case OPT_AI2ERR:      sprintf(str, "%f", human_player_roster.player[1].err); write_rc(f, opt, str); break;
    case OPT_BALLDETAIL:
      if (options_max_ball_detail == options_max_ball_detail_LOW){
        write_rc(f, opt, "l"); break;
      }
      else if (options_max_ball_detail == options_max_ball_detail_MED){
        write_rc(f, opt, "m"); break;
      }
      else if (options_max_ball_detail == options_max_ball_detail_HIGH){
        write_rc(f, opt, "h"); break;
      }
      else if (options_max_ball_detail == options_max_ball_detail_VERYHIGH){
        write_rc(f, opt, "v"); break;
      }
      break;
    case OPT_RGSTEREO:    if (options_rgstereo_on) write_rc(f, opt, NULL); break;
    case OPT_RGAIM:
      if (options_rgaim == 1) write_rc(f, opt, "l");
      if (options_rgaim == 2) write_rc(f, opt, "r");
      if (options_rgaim == 0) write_rc(f, opt, "m");
      break;
    case OPT_HOSTADDR:    write_rc(f, opt, options_net_hostname); break;
    case OPT_PORTNUM:     sprintf(str, "%d", options_net_portnum); write_rc(f, opt, str); break;
    case OPT_GEOMETRY:
#ifndef _WIN32
      sprintf(str,"%dx%d",win_width,win_height);
#else
      if ((win_width_change != 0) && (win_height_change != 0)) {
        sprintf(str, "%dx%d", win_width_change, win_height_change);
      }
      else {
        sprintf(str, "%dx%d", win_width, win_height);
      }
#endif
      write_rc(f, opt, str); break;
    case OPT_FULLSCREEN:  if (sys_get_fullscreen()) write_rc(f, opt, NULL); break;
    case OPT_FREEMOVE:    write_rc(f, opt, options_free_view_on ? "on" : "off"); break;
    case OPT_CUBEREF:     write_rc(f, opt, options_cuberef ? "on" : "off"); break;
    case OPT_CUBERES:     sprintf(str, "%d", options_cuberef_res); write_rc(f, opt, str); break;
    case OPT_BUMPREF:     write_rc(f, opt, options_bumpref ? "on" : "off"); break;
    case OPT_BUMPWOOD:    write_rc(f, opt, options_bumpwood ? "on" : "off"); break;
    case OPT_BALLTRACE:   write_rc(f, opt, options_balltrace ? "on" : "off"); break;
    case OPT_GAMEMODE:    switch (options_gamemode){
    case options_gamemode_match:
      write_rc(f, opt, "match"); break;
    case options_gamemode_training:
      write_rc(f, opt, "training"); break;
    case options_gamemode_tournament:
      write_rc(f, opt, "tournament"); break;
    }
                          break;
    case OPT_BALL_FRESNEL:write_rc(f, opt, options_ball_fresnel_refl ? "on" : "off"); break;
    case OPT_AVATAR:      write_rc(f, opt, options_avatar_on ? "on" : "off"); break;
    case OPT_TOURFAST:    sprintf(str, "%f", options_tourfast); write_rc(f, opt, str); break;
    case OPT_CLOTHTEX:    write_rc(f, opt, options_cloth_tex ? "on" : "off"); break;
      /*      case OPT_HELP:        exit(1);   break;
            case OPT_DUMMY:       break;*/
    }
  }

  fclose(f);

}


//#endif  // not WIN32



/*
int my_time(void)
{struct timeb ts;
ftime(&ts);
return(ts.time*1000+ts.millitm);
}*/

void set_gametype(int gtype)
{
  gametype = gtype;
  if (gametype == GAME_8BALL){
    setfunc_evaluate_last_move(evaluate_last_move_8ball);
    setfunc_create_scene(create_8ball_scene);
    setfunc_create_walls(create_6hole_walls);
    setfunc_ai_get_stroke_dir(ai_get_stroke_dir_8ball);
    player[0].cue_ball = 0;   player[1].cue_ball = 0;
    player[act_player].place_cue_ball = 1;
    human_player_roster.player[0].cue_ball = 0;   human_player_roster.player[1].cue_ball = 0;
    human_player_roster.player[act_player].place_cue_ball = 1;
  }
  else if (gametype == GAME_9BALL){
    setfunc_evaluate_last_move(evaluate_last_move_9ball);
    setfunc_create_scene(create_9ball_scene);
    setfunc_create_walls(create_6hole_walls);
    setfunc_ai_get_stroke_dir(ai_get_stroke_dir_9ball);
    player[0].cue_ball = 0;   player[1].cue_ball = 0;
    player[act_player].place_cue_ball = 1;
    human_player_roster.player[0].cue_ball = 0;   human_player_roster.player[1].cue_ball = 0;
    human_player_roster.player[act_player].place_cue_ball = 1;
  }
  else if (gametype == GAME_CARAMBOL){
    setfunc_evaluate_last_move(evaluate_last_move_carambol);
    setfunc_create_scene(create_carambol_scene);
    setfunc_create_walls(create_0hole_walls);
    setfunc_ai_get_stroke_dir(ai_get_stroke_dir_carambol);
    player[0].cue_ball = 0;   player[1].cue_ball = 1;
    player[act_player].place_cue_ball = 0;
    human_player_roster.player[0].cue_ball = 0;   human_player_roster.player[1].cue_ball = 1;
    human_player_roster.player[act_player].place_cue_ball = 0;
  }
  else if (gametype == GAME_SNOOKER){
    setfunc_evaluate_last_move(evaluate_last_move_snooker);
    setfunc_create_scene(create_snooker_scene);
    setfunc_create_walls(create_6hole_walls);
    setfunc_ai_get_stroke_dir(ai_get_stroke_dir_snooker);
    /*        options_table_size = 9.0*2.54*12.0/100.0;
            if( table_obj!=0 )  table_obj=create_table( spheretexbind, &walls );*/
    player[0].cue_ball = 0;   player[1].cue_ball = 0;
    player[act_player].place_cue_ball = 1;
    human_player_roster.player[0].cue_ball = 0;   human_player_roster.player[1].cue_ball = 0;
    human_player_roster.player[act_player].place_cue_ball = 1;
  }
}



double angle_pm180(double ang)
{
  while (ang > 180.0) ang -= 360.0;
  while (ang < -180.0) ang += 360.0;
  return ang;
}



void toggle_queue_view()
{
  queue_view = (queue_view == 0) ? 1 : 0;
  if (queue_view){
    Xrot_offs = angle_pm180(Xrot - Xque);  Xrot = Xque;
    Zrot_offs = angle_pm180(Zrot - Zque);  Zrot = Zque;
  }
  else {
    double th = Xrot / 180.0*M_PI;
    double ph = Zrot / 180.0*M_PI;
    free_view_pos_aim = vec_scale(vec_xyz(sin(th)*sin(ph), sin(th)*cos(ph), cos(th)), cam_dist);
    free_view_pos_aim = vec_add(free_view_pos_aim, CUE_BALL_XYPOS);
    free_view_pos = free_view_pos_aim;
  }
}

void birdview()
{
  if (options_free_view_on == 0) options_free_view_on = 1;
  if (queue_view) toggle_queue_view();

  if ((!queue_view) && options_free_view_on){
    double Xoffs = 0.0 - Xrot;
    double Zoffs = -90.0 - Zrot;
    free_view_pos_aim = vec_xyz(0, 0, 3.5*options_table_size / 2.1336);
    free_view_pos = free_view_pos_aim;
    Xrot += Xoffs;
    Zrot += Zoffs;
    Xrot_offs -= Xoffs;
    Zrot_offs -= Zoffs;
    Xrot = 0.0;
    Zrot = -90.0;
    Xrot_offs = 0.0;
    Zrot_offs = 0.0;
  }
  old_queue_view = queue_view;
}


void player_copy(struct Player * player, struct Player src)
{
  player->is_AI = src.is_AI;
  player->is_net = src.is_net;
  player->half_full = src.half_full;
  printf("player_copy1\n");
  strcpy(player->name, src.name);
  printf("player_copy2\n");
  player->Xque = src.Xque;
  player->Zque = src.Zque;
  player->cue_x = src.cue_x;
  player->cue_y = src.cue_y;
  player->strength = src.strength;
  player->queue_view = src.queue_view;
  player->place_cue_ball = src.place_cue_ball;
  player->winner = src.winner;
  player->err = src.err;
  printf("player_copy3\n");
  printf("%s\n", player->name);
  if (player->text) textObj_setText(player->text, player->name);
  printf("player_copy3.5\n");
  if (player->score_text) textObj_setText(player->score_text, "0");
  printf("player_copy4\n");
  player->snooker_on_red = src.snooker_on_red;
  player->score = src.score;
  player->cue_ball = src.cue_ball;
}


void init_player(struct Player * player, int ai)
{
  player->is_AI = ai;
  player->is_net = 0;
  player->half_full = BALL_ANY;
  strcpy(player->name, ai ? "AI-Player" : "Human");
  player->Xque = -83.0;
  player->Zque = 0.0;
  player->cue_x = 0.0;
  player->cue_y = 0.0;
  player->strength = 0.7;
  player->queue_view = ai ? 0 : 1;
  player->place_cue_ball = 0;
  player->winner = 0;
  player->err = 0;
  player->text = 0;
  player->score_text = 0;
  player->snooker_on_red = 1;
  player->score = 0;
  player->cue_ball = 0;
  //    player[0].free_view_pos=vec_xyz(0,-1.0,1.5);
  //    player[1].free_view_pos=vec_xyz(0,-1.0,1.5);
}

void init_players()
{
  init_player(&player[0], 0);
  init_player(&player[1], human_human_mode ? 0 : 1);
  /*    player[0].is_AI=0;
      player[1].is_AI=human_human_mode?0:1;
      player[0].is_net=0;
      player[1].is_net=0;
      player[0].half_full=BALL_ANY;
      player[1].half_full=BALL_ANY;*/
  strcpy(player[0].name, player_names[0]);
  strcpy(player[1].name, player_names[human_human_mode ? 2 : 1]);
  /*    player[0].Xque=-83.0;
      player[1].Xque=-83.0;
      player[0].Zque=0.0;
      player[1].Zque=0.0;
      player[0].cue_x=0.0;
      player[1].cue_x=0.0;
      player[0].cue_y=0.0;
      player[1].cue_y=0.0;
      player[0].strength=0.7;
      player[1].strength=0.7;
      player[0].queue_view=1;
      player[1].queue_view=human_human_mode?1:0;
      player[0].place_cue_ball=0;
      player[1].place_cue_ball=0;
      player[0].winner=0;
      player[1].winner=0;
      player[0].err=0;
      player[1].err=0;
      player[0].text = 0;
      player[1].text = 0;
      player[0].score_text = 0;
      player[1].score_text = 0;
      player[0].snooker_on_red=1;
      player[1].snooker_on_red=1;
      player[0].score=0;
      player[1].score=0;
      player[0].cue_ball=0;
      player[1].cue_ball=0;*/
  //    player[0].free_view_pos=vec_xyz(0,-1.0,1.5);
  //    player[1].free_view_pos=vec_xyz(0,-1.0,1.5);
}

void init_ai_player_roster(struct PlayerRoster * roster)
{
  int i;

  for (i = 0; i < roster->nr; i++){

    init_player(&(roster->player[i]), 1);

    if (i == roster->nr - 1) { /* human player */
      roster->player[i] = human_player_roster.player[0];
    }
    else if (i == 0){
      strcpy(roster->player[i].name, "billardo bill");
      roster->player[i].err = 0.0;
    }
    else if (i == 1) {
      strcpy(roster->player[i].name, "suzy cue");
      roster->player[i].err = 0.02;
    }
    else if (i == 2) {
      strcpy(roster->player[i].name, "pooledo pete");
      roster->player[i].err = 0.05;
    }
    else if (i == 3) {
      strcpy(roster->player[i].name, "billie ball");
      roster->player[i].err = 0.1;
    }
    else if (i == 4) {
      strcpy(roster->player[i].name, "snookie");
      roster->player[i].err = 0.2;
    }
    else if (i == 5) {
      strcpy(roster->player[i].name, "diamond dan");
      roster->player[i].err = 0.4;
    }
    else if (i == 6) {
      strcpy(roster->player[i].name, "tom tuxedo");
      roster->player[i].err = 0.6;
    }
    else if (i == 7) {
      strcpy(roster->player[i].name, "sally silver");
      roster->player[i].err = 0.7;
    }
    else if (i == 8) {
      strcpy(roster->player[i].name, "wicked wendy");
      roster->player[i].err = 0.8;
    }
    else if (i == 9) {
      strcpy(roster->player[i].name, "bald ben");
      roster->player[i].err = 0.9;
    }
    else if (i == 10) {
      strcpy(roster->player[i].name, "badino buck");
      roster->player[i].err = 0.10;
    }
    else if (i == 11) {
      strcpy(roster->player[i].name, "worse will");
      roster->player[i].err = 0.11;
    }
    else if (i == 12) {
      strcpy(roster->player[i].name, "rita rookie");
      roster->player[i].err = 0.12;
    }
    else if (i == 13) {
      strcpy(roster->player[i].name, "don dumb");
      roster->player[i].err = 0.15;
    }
    else if (i == 14) {
      strcpy(roster->player[i].name, "dana dummy");
      roster->player[i].err = 0.19;
    }
    else {
      char str[256];
      sprintf(str, "dumb for %d", i - 13);
      strcpy(roster->player[i].name, str);
      roster->player[i].err = 0.1*(double)(i - 13);
    }
    roster->player[i].text = textObj_new(roster->player[i].name, options_roster_fontname, 28);
  }
}

void init_human_player_roster(struct PlayerRoster * roster)
{
  int i;

  roster->nr = 2;

  for (i = 0; i < roster->nr; i++){

    init_player(&(roster->player[i]), 0);

    {
      char str[256];
      sprintf(str, "human player %d", i + 1);
      strcpy(roster->player[i].name, str);
      roster->player[i].err = (double)i / 10.0;
    }
    roster->player[i].text = 0;
  }
}

void create_human_player_roster_text(struct PlayerRoster * roster)
{
  int i;

  for (i = 0; i < roster->nr; i++){
    if (roster->player[i].text == 0){
      roster->player[i].text = textObj_new(roster->player[i].name, options_roster_fontname, 28);
    }
    else {
      textObj_setText(roster->player[i].text, roster->player[i].name);
    }
  }
}

void init_tournament_state(struct TournamentState_ * ts)
{
  static int init_me = 1;
  int i, j, k, dummy, game;
  int players[100];

  ts->round_num = TOURNAMENT_ROUND_NUM;
  ts->game_ind = 0;
  ts->round_ind = 0;
  ts->wait_for_next_match = 1;
  ts->tournament_over = 0;
  ts->overall_winner = -1;
  ts->ai_fast_motion = options_tourfast;
  for (i = 0; i < (1 << ts->round_num); i++) players[i] = i;
  /* mix players for tournament */
  for (k = 0; k < 1000; k++){
    i = k % (1 << ts->round_num);
    j = rand() % (1 << ts->round_num);
    dummy = players[i];
    players[i] = players[j];
    players[j] = dummy;
  }
  /* set up pairings */
  for (game = 0; game < (1 << (ts->round_num - 1)); game++){
    ts->game[0][game].roster_player1 = players[(game * 2)];
    ts->game[0][game].roster_player2 = players[(game * 2) + 1];
    ts->game[0][game].winner = -1;
  }
  ts->roster.nr = 1 << ts->round_num;

  if (init_me){
    DPRINTF("init_tournament_state: initializing player roster\n");
    init_ai_player_roster(&(ts->roster));
    init_me = 0;
  }
}

void restart_game_common();

void tournament_state_setup_next_round(struct TournamentState_ * ts)
{
  int i;
  int players[100];

  printf("tournament_state_setup_next_round\n");

  (ts->round_ind)++;
  if (ts->round_ind == ts->round_num){ /* tournament over ? */
    ts->tournament_over = 1;
    ts->round_ind = ts->round_num - 1;
    if (ts->game[ts->round_ind][0].winner == 0){
      ts->overall_winner = ts->game[ts->round_ind][0].roster_player1;
    }
    else if (ts->game[ts->round_ind][0].winner == 1){
      ts->overall_winner = ts->game[ts->round_ind][0].roster_player2;
    }
    else {
      fprintf(stderr, "error: nobody won the tournament !?\n");
      exit(1);
    }
  }
  else {
    for (i = 0; i < (1 << (ts->round_num - ts->round_ind)); i++){
      if (ts->game[ts->round_ind - 1][i].winner == 0){
        players[i] = ts->game[ts->round_ind - 1][i].roster_player1;
      }
      else if (ts->game[ts->round_ind - 1][i].winner == 1){
        players[i] = ts->game[ts->round_ind - 1][i].roster_player2;
      }
      else {
        fprintf(stderr, "error: sbdy didnt win one of the last matches !?\n");
        exit(1);
      }
    }
    printf("Pairings:\n");
    for (i = 0; i < (1 << (ts->round_num - ts->round_ind - 1)); i++){
      ts->game[ts->round_ind][i].roster_player1 = players[2 * i];
      ts->game[ts->round_ind][i].roster_player2 = players[2 * i + 1];
      ts->game[ts->round_ind][i].winner = -1;
      printf("%d vs. %d\n",
        ts->game[ts->round_ind][i].roster_player1,
        ts->game[ts->round_ind][i].roster_player2
        );
      printf("%s vs. %s\n",
        ts->roster.player[ts->game[ts->round_ind][i].roster_player1].name,
        ts->roster.player[ts->game[ts->round_ind][i].roster_player2].name
        );
    }
  }
}

void tournament_evaluate_last_match(struct TournamentState_ * ts)
{
  printf("tournament_evaluate_last_match 1\n");
  if (player[0].winner){
    ts->game[ts->round_ind][ts->game_ind].winner = 0;
  }
  else if (player[1].winner){
    ts->game[ts->round_ind][ts->game_ind].winner = 1;
  }
  else {
    ts->game[ts->round_ind][ts->game_ind].winner = -1;
  }
  printf("tournament_evaluate_last_match 2\n");
  ts->game_ind++;
  printf("tournament_evaluate_last_match 3\n");
  if (ts->game_ind >= (1 << (ts->round_num - ts->round_ind - 1))){
    printf("tournament_evaluate_last_match 4\n");
    tournament_state_setup_next_round(ts);
    ts->game_ind = 0;
    /*        if(ts->round_ind>=ts->round_num){
                ts->tournament_over=1;
                if( ts->game[ts->round_num-1][0].winner == 0 ){
                ts->overall_winner = ts->game[ts->round_num-1][0].roster_player1;
                } else {
                ts->overall_winner = ts->game[ts->round_num-1][0].roster_player2;
                }
                }*/
  }
  printf("tournament_evaluate_last_match 5\n");
}

void tournament_state_setup_next_match(struct TournamentState_ * ts)
{
  printf("tournament_state_setup_next_match 1\n");
  printf("ts->game[ts->round_ind][ts->game_ind].roster_player1=%d\n", ts->game[ts->round_ind][ts->game_ind].roster_player1);
  player_copy(&(player[0]), ts->roster.player[ts->game[ts->round_ind][ts->game_ind].roster_player1]);
  printf("tournament_state_setup_next_match 2\n");
  printf("ts->game[ts->round_ind][ts->game_ind].roster_player2=%d\n", ts->game[ts->round_ind][ts->game_ind].roster_player2);
  player_copy(&(player[1]), ts->roster.player[ts->game[ts->round_ind][ts->game_ind].roster_player2]);
  printf("tournament_state_setup_next_match 3\n");
  player[0].winner = 0;
  player[1].winner = 0;
  printf("tournament_state_setup_next_match 4\n");
  if (player[0].is_AI && player[1].is_AI){
    g_motion_ratio = ts->ai_fast_motion;
    printf("ts->ai_fast_motion=%f\n", g_motion_ratio);
    printf("tournament_state_setup_next_match 4-1\n");
  }
  else {
    g_motion_ratio = 1.0;
    printf("g_motion_ratio=%f\n", g_motion_ratio);
    printf("tournament_state_setup_next_match 4-2\n");
  }
  printf("tournament_state_setup_next_match 5\n");
  restart_game_common();
  printf("tournament_state_setup_next_match 6\n");
  act_player = 0;
  printf("tournament_state_setup_next_match 7\n");
  queue_view = player[act_player].queue_view;
  printf("tournament_state_setup_next_match 8\n");
}

void create_players_text()
{
  player[0].text = textObj_new(player[0].name, options_player_fontname, 28);
  player[1].text = textObj_new(player[1].name, options_player_fontname, 28);
  //    player[0].text = textObj3D_new(player[0].name, options_player_fontname, 28, 1.0, 3);
  //    player[1].text = textObj3D_new(player[1].name, options_player_fontname, 28, 1.0, 3);
  player[0].score_text = textObj_new("0", options_score_fontname, 20);
  player[1].score_text = textObj_new("0", options_score_fontname, 20);
}


void copy_balls(BallsType * balls1, BallsType * balls2)
{
  int i;
  if (balls2->nr != balls1->nr){
    balls2->nr = balls1->nr;
    free(balls2->ball);
    balls2->ball = (BallType *)malloc(balls2->nr*sizeof(BallType));
  }
  for (i = 0; i < balls1->nr; i++){
    balls2->ball[i] = balls1->ball[i];
  }
  balls2->gametype = balls1->gametype;
}


void queue_shot()
{
  VMvect dir, nx, ny, hitpoint;

  if (!balls_moving){
    int other_player;

    /* backup actual ball setup */
    copy_balls(&balls, &bakballs);

    other_player = (act_player == 1) ? 0 : 1;
    if (player[other_player].is_net){
      socket_write(g_socket, (char *)&(Zque), sizeof(Zque));
      socket_write(g_socket, (char *)&(Xque), sizeof(Xque));
    }
    //        player[act_player].place_cue_ball=0;

    dir = vec_xyz(sin(Zque*M_PI / 180.0)*sin(Xque*M_PI / 180.0),
      cos(Zque*M_PI / 180.0)*sin(Xque*M_PI / 180.0),
      cos(Xque*M_PI / 180.0));
    nx = vec_unit(vec_cross(vec_ez(), dir));  /* parallel to table */
    ny = vec_unit(vec_cross(nx, dir));        /* orthogonal to dir and nx */
    hitpoint = vec_add(vec_scale(nx, queue_point_x), vec_scale(ny, queue_point_y));
    //        fprintf(stderr,"queue_shot: Zque=%f\n",Zque);
    //        Zque=-137.020020;
    balls.ball[CUE_BALL_IND].v = vec_scale(dir, -CUEBALL_MAXSPEED*queue_strength);
#if options_jump_shots
#else
    balls.ball[CUE_BALL_IND].v.z = 0.0;
#endif
    /*        balls.ball[CUE_BALL_IND].v.x =  -CUEBALL_MAXSPEED*queue_strength*sin(Xque*M_PI/180.0)*sin(Zque*M_PI/180.0);
            balls.ball[CUE_BALL_IND].v.y =  -CUEBALL_MAXSPEED*queue_strength*sin(Xque*M_PI/180.0)*cos(Zque*M_PI/180.0);*/
    //        balls.ball[0].w.x = -2.0/balls.ball[0].d/2.0*balls.ball[0].v.y;
    //        balls.ball[0].w.y = +2.0/balls.ball[0].d/2.0*balls.ball[0].v.x;
    //        balls.ball[0].w.z = -2.0/balls.ball[0].d/2.0;
    if (vec_abssq(hitpoint) == 0.0){
      balls.ball[CUE_BALL_IND].w = vec_xyz(0.0, 0.0, 0.0);
    }
    else {
      /* w = roll speed if hit 1/3of radius above center */
      //            balls.ball[CUE_BALL_IND].w = vec_scale(vec_cross(dir,hitpoint),4.0*3.0*CUEBALL_MAXSPEED*queue_strength/balls.ball[CUE_BALL_IND].d/balls.ball[CUE_BALL_IND].d);
      /* hmm, this one works better */
      balls.ball[CUE_BALL_IND].w = vec_scale(vec_cross(dir, hitpoint), 2.0*3.0*CUEBALL_MAXSPEED*queue_strength / balls.ball[CUE_BALL_IND].d / balls.ball[CUE_BALL_IND].d);
    }

#ifdef USE_SOUND
    PlaySound_fb(&ball_cue_snd,options_snd_volume*queue_strength/2.0);
#endif

    /* clear recorded ballpaths */
    {int i;
    for (i = 0; i < balls.nr; i++){
      BM_clearpath(&balls.ball[i]);
    }
    }

    /* reset offset parameters */
    queue_point_x = 0.0;
    queue_point_y = 0.0;

  }
}



void shoot(int ani);


void do_computer_move(int doit)
{
  VMvect dir;

  DPRINTF("do_computermove: begin ai_get_strike_dir\n");

  ai_set_err(player[act_player].err);
  dir = ai_get_stroke_dir(&balls, &walls, &player[act_player]);

  DPRINTF("do_computermove: end ai_get_strike_dir\n");

  Zque = atan2(dir.x, dir.y) / M_PI*180.0;
  //    Xque = atan2(sqrt(dir.x*dir.x+dir.y*dir.y),dir.z)/M_PI*180.0;
  if (doit){
    shoot(!queue_view);
    /*        if(!queue_view){
                queue_anim=30.0;
                } else {
                queue_shot();
                }*/
  }
  comp_dir = dir;
}

int do_net_move(void)
{
  int nbytes;

  DPRINTF("do_net_move: start\n");

  nbytes = socket_read(g_socket, (char *)&(Zque), sizeof(Zque));
  if (nbytes == 0) return 0;
  nbytes = socket_read(g_socket, (char *)&(Xque), sizeof(Xque));
  if (nbytes == 0) return 0;
  DPRINTF("do_net_move: %d bytes read\n", nbytes);
  DPRINTF("Zque = %f\n", Zque);

  if (!queue_view){
    queue_anim = 30.0;
  }
  else {
    queue_shot();
  }
  DPRINTF("do_net_move: end\n");

  return 1;
}


double queue_offs_func1(double t)
{
  return(1.0 - cos(t*2.0*M_PI));
}


double queue_offs_func2(double t)
{
  return(sin(t*M_PI));
}


double queue_offs_func(double t)
{
  double tx6, rval, dt1, dt2, dt3, dt4, dt5, dt6, t1, t2, t3, t4, t5, t6;
  rval = 0.0;
  dt1 = 1.0;
  dt2 = 0.4;
  dt3 = 1.0;
  dt4 = 0.4;
  dt5 = 1.0;
  dt6 = 0.7;
  t1 = dt1;
  t2 = t1 + dt2;
  t3 = t2 + dt3;
  t4 = t3 + dt4;
  t5 = t4 + dt5;
  t6 = t5 + dt6;
  tx6 = t*t6;
  if (tx6 >= 0.0 && tx6 < t1){
    rval = 1.0*queue_offs_func1(tx6);
  }
  else if (tx6 >= t1 && tx6 < t2){
    rval = 0.0;
  }
  else if (tx6 >= t2 && tx6 < t3){
    rval = 1.0*queue_offs_func1((tx6 - t2) / dt3);
  }
  else if (tx6 >= t3 && tx6 < t4){
    rval = 0.0;
  }
  else if (tx6 >= t4 && tx6 < t5){
    rval = 1.3*queue_offs_func1((tx6 - t4) / dt5);
  }
  else if (tx6 >= t5 && tx6 < t6){
    rval = 6.0*queue_offs_func2((tx6 - t5) / dt6*1.06);
  }
  return 0.5*(0.7 + rval);
  //    return( 1.0+t*t*sin(t*(10.0+M_PI)) );
}


int net_get_send_req()
{
  char c;
  if (socket_read(g_socket, &c, 1) == 1 && c == 'R'){
    return 1;
  }
  return 0;
}


int net_get_ack()
{
  char c;
  if (socket_read(g_socket, &c, 1) == 1 && c == 'A'){
    return 1;
  }
  return 0;
}


int net_get_data()
{
  if (socket_read(g_socket, (char *)&g_net_data, sizeof(NetData)) == sizeof(NetData)){
    Xque = g_net_data.Xrot;
    Zque = g_net_data.Zrot;
    player[act_player].cue_x = g_net_data.cue_x;
    player[act_player].cue_y = g_net_data.cue_y;
    player[act_player].strength = g_net_data.strength;
    balls.ball[CUE_BALL_IND].r.x = g_net_data.white_x;
    balls.ball[CUE_BALL_IND].r.y = g_net_data.white_y;
    return 1;
  }
  return 0;
}


void net_send_send_req()
{
  char c = 'R';
  socket_write(g_socket, &c, 1);
}


void net_send_ack()
{
  char c = 'A';
  socket_write(g_socket, &c, 1);
}


void net_send_data()
{
  g_net_data.Xrot = Xque;
  g_net_data.Zrot = Zque;
  g_net_data.cue_x = player[act_player].cue_x;
  g_net_data.cue_y = player[act_player].cue_y;
  g_net_data.strength = player[act_player].strength;
  g_net_data.white_x = balls.ball[CUE_BALL_IND].r.x;
  g_net_data.white_y = balls.ball[CUE_BALL_IND].r.y;
  socket_write(g_socket, (char *)&g_net_data, sizeof(NetData));
}


void shoot(int ani)
{
  int other_player;

  other_player = (act_player == 0) ? 1 : 0;
  if (player[other_player].is_net){
    DPRINTF("shoot: handling unresolved net request\n");
    switch (g_net_state){
    case NET_STATE_ACK:
      while (!net_get_ack());
    case NET_STATE_DATA:
      g_net_data.shoot = 0;
      net_send_data();
    case NET_STATE_REQ: break;
    }
    fprintf(stderr, "shoot: sending new data with shot\n");
    net_send_send_req();
    while (!net_get_ack());
    g_net_data.shoot = 1;
    net_send_data();
    g_net_data.shoot = 0;
    g_net_state = NET_STATE_REQ;
  }

  if (ani){
    DPRINTF("shoot: ani-shot\n");
    queue_anim = 30;
  }
  else {
    DPRINTF("shoot: direct shot\n");
    queue_shot();
  }
}


int
in_cue_ball_region(VMvect pos)
{
  int in_region = 1;

  switch (gametype){
  case GAME_8BALL:
  case GAME_9BALL:
    if (pos.x > (TABLE_W - BALL_D) / 2.0)  in_region = 0;
    if (pos.x < -(TABLE_W - BALL_D) / 2.0)  in_region = 0;
    if (pos.y > -TABLE_L / 4.0)  in_region = 0;
    if (pos.y < -(TABLE_L - BALL_D) / 2.0)  in_region = 0;
    break;
  case GAME_CARAMBOL:
    break;
  case GAME_SNOOKER:
#define TABLE_SCALE (TABLE_L/(3.571042))
    if (pos.y > -TABLE_L / 2.0 + TABLE_SCALE*0.737) in_region = 0;
    if (vec_abs(vec_diff(pos, vec_xyz(0, -TABLE_L / 2.0 + TABLE_SCALE*0.737, 0))) > TABLE_SCALE*0.292) in_region = 0;
#undef TABLE_SCALE
    break;
  }

  return(in_region);
}

int
in_table_region(VMvect pos)
{
  int in_region = 1;

  if (pos.x > (TABLE_W - BALL_D) / 2.0)  in_region = 0;
  if (pos.x < -(TABLE_W - BALL_D) / 2.0)  in_region = 0;
  if (pos.y >(TABLE_L - BALL_D) / 2.0)  in_region = 0;
  if (pos.y < -(TABLE_L - BALL_D) / 2.0)  in_region = 0;

  return(in_region);
}


static void ball_free_place(int ind, BallsType * pballs)
{
  int i, exitloop;
  double x, y, x0, y0, r, phi;
  x = pballs->ball[ind].r.x; y = pballs->ball[ind].r.y;
  x0 = x; y0 = y;
  phi = 0.0;
  do{
    exitloop = 1;
    r = floor(phi / 2.0 / M_PI)*0.01;
    x = x0 + r*cos(phi);
    y = y0 + r*sin(phi);
    DPRINTF("phi=%f\n", phi);
    DPRINTF("ind=%d, CUE_BALL_IND=%d\n", ind, CUE_BALL_IND);
    if ((ind == CUE_BALL_IND && in_cue_ball_region(vec_xyz(x, y, 0)) && player[act_player].place_cue_ball) ||
      in_table_region(vec_xyz(x, y, 0))
      )
    {
    }
    else exitloop = 0;
    DPRINTF("1:exitloop=%d\n", exitloop);
    for (i = 0; i < pballs->nr; i++) if (i != ind && pballs->ball[i].in_game){
      if (vec_abs(vec_diff(vec_xyz(x, y, 0), pballs->ball[i].r)) <
        (pballs->ball[ind].d + pballs->ball[i].d) / 2.0)
      {
        exitloop = 0; break;
      }
    }
    DPRINTF("2:exitloop=%d\n", exitloop);
    phi += 0.01;
  } while (!exitloop);
  pballs->ball[ind].r.x = x;
  pballs->ball[ind].r.y = y;
}

static void all_balls_free_place(BallsType * pballs)
{
  int i;
  for (i = 0; i < pballs->nr; i++) if (pballs->ball[i].in_game){
    ball_free_place(i, pballs);
  }
}


//static void Idle_timer( int value )
static void Idle_timer(void)
{
  int i;
  //    static int balls_moving=0;
  static int balls_were_moving = 0;
  static int first_time = 1;
  int t_act;
  static int dt;
  static int t_prev = -1;
  static int frametime_rest = 0;
  //    static int frametime_fromlast=0;
  //    static int framestep=0;
  static int dt_rest = 0;
  static int count = 0;
  static double dt_s_rest = 0.0;
  //    static int waiting_for_net_data=0;
  int other_player;
  double fact;

  count++;
  t_act = time_us();
  if (t_prev == -1)  t_prev = t_act;
  dt += t_act - t_prev;
  dt_s_rest += (double)(t_act - t_prev) / 1000000.0;
  t_prev = t_act;
  if (count == 1){
    count = 0;
    frametime_ms = (int)((double)dt / 1000.0);
    if (frametime_ms<1) frametime_ms = 1;
    //        if( frametime_ms<frametime_ms_min ) frametime_ms=frametime_ms_min;
    if (frametime_ms>frametime_ms_max) frametime_ms = frametime_ms_max;
    dt = 0;
    dt_rest += frametime_ms;
    //        frametime_ms=frametime_ms_min;
  }

  //    glutTimerFunc( frametime_ms*0.8,       /* -5 for assuring framerate increase */
  //                   Idle_timer, value );  /* assure a framerate of max ... fps */

  //    glutTimerFunc( frametime_ms_min, Idle_timer, value );
  sys_set_timer(frametime_ms_min, Idle_timer);

#ifdef USE_SOUND
#ifndef USE_SDL
  mixaudio();
#endif
#endif

  //    fprintf(stderr,"dt=%d\n",dt);
  fact = pow(0.85, (double)frametime_ms / 50.0);
  //    fact=0.85;
  Xrot_offs *= fact;
  Zrot_offs *= fact;
  fact = pow(0.94, (double)frametime_ms / 50.0);
  //    fact=0.94;
  cam_dist = (cam_dist*fact) + (cam_dist_aim + vec_abs(balls.ball[CUE_BALL_IND].v)*0.4)*(1.0 - fact);

  free_view_pos = vec_add(vec_scale(free_view_pos, fact), vec_scale(free_view_pos_aim, 1.0 - fact));


  if (Animate) {
    //        int bhit=0;
    //        int whit=0;
    double bhitstrength = 0.0;
    double whitstrength = 0.0;
    double toffs = 0.0;

    //      Xrot += DXrot;
    //      Yrot += DYrot;

#if 0
    first_time=1; /* to get into loop when balls not moving */
    for(i=0;
      (balls_moving || first_time ) &&
      (
      i<(frametime_ms+frametime_rest)/(int)(10.0/g_motion_ratio) /* assure constant time flow */
      );
    i++)
#else
    while (dt_s_rest > 0.0) /* assure constant time flow */
#endif
    {
      first_time = 0; /* to get into loop when balls not moving */
      //       for(i=0;i<(frametime_ms+frametime_rest)/3;i++){   /* assure constant time flow */
      //       for(i=0;i<(frametime_ms+5)/10;i++){   /* assure constant time flow */
      //           balls_moving = proceed_dt( &balls, &walls, 0.01 );
#ifdef TIME_INTERPOLATE
      copy_balls(&balls,&g_lastballs);
#endif
#define TIMESTEP 0.0075
      dt_s_rest -= TIMESTEP / 0.75 / g_motion_ratio;
      //           printf("g_motion_ratio=%f\n",g_motion_ratio);
      balls_moving = proceed_dt(&balls, &walls, TIMESTEP);
      if (balls_moving) balls_were_moving = 1;
      /* 0.0075 instead of 0.01 - finetuning */
      /*           bhitstrength+=BM_get_balls_hit_strength_last();
                 whitstrength+=BM_get_walls_hit_strength_last();*/
      //           fprintf(stderr,"whitstrength=%f\n",whitstrength);
      //           fprintf(stderr,"bhitstrength=%f\n",bhitstrength);
#ifdef USE_SOUND
      {
        int index;
        index=0;
        do{
          BM_get_balls_hit_strength_last_index( index++ ,&bhitstrength, &toffs );
          bhitstrength = 1.75 * (0.3 * bhitstrength / CUEBALL_MAXSPEED +
            0.7 * bhitstrength*bhitstrength / CUEBALL_MAXSPEED / CUEBALL_MAXSPEED);
          if(bhitstrength!=0.0){
            if( toffs>TIMESTEP || toffs<0.0 ){
              exit(0);
            }else{
              //                       printf("toffs/TIMESTEP=%f\n",toffs/TIMESTEP);
            }
            PlaySound_offs(&ball_ball_snd,options_snd_volume*((bhitstrength>1.0)?1.0:bhitstrength), SOUND_NULLOFFS-(TIMESTEP-toffs)*22050);
          }
        } while(bhitstrength!=0.0);
        index=0;
        do{
          BM_get_walls_hit_strength_last_index( index++ ,&whitstrength, &toffs );
          whitstrength = 0.4 * (0.3 * whitstrength / CUEBALL_MAXSPEED +
            0.7 * whitstrength*whitstrength / CUEBALL_MAXSPEED / CUEBALL_MAXSPEED);
          if(whitstrength!=0.0){
            PlaySound_offs(&ball_wall_snd,options_snd_volume*((whitstrength>1.0)?1.0:whitstrength), SOUND_NULLOFFS-(TIMESTEP-toffs)*22050);
            //                  PlaySound_fb(&ball_wall_snd,(whitstrength*0.125>1.0)?1.0:whitstrength*0.125);
          }
        } while(whitstrength!=0.0);
      }
#endif
      if (!balls_moving) break;
    }
    if (dt_s_rest > 0.0) dt_s_rest = 0.0; /* to move on if last move was completely in last simulation step */
#ifdef TIME_INTERPOLATE
    if((frametime_ms+frametime_rest)/10>0)
      g_frametime_laststep = (frametime_ms+frametime_rest)/10*10;
    g_frametime_fromlast = frametime_rest;
#endif
    frametime_rest = (frametime_ms + frametime_rest) % 10;

    if (options_balltrace){
      for (i = 0; i < balls.nr; i++) if (balls.ball[i].in_game){
        BM_add2path(&balls.ball[i]);
      }
    }

    /*#ifdef USE_SDL
           if(bhitstrength!=0.0){
           PlaySound_fb(&ball_ball_snd,(bhitstrength*0.25>1.0)?1.0:bhitstrength*0.25);
           }
           if(whitstrength!=0.0){
           PlaySound_fb(&ball_wall_snd,(whitstrength*0.125>1.0)?1.0:whitstrength*0.125);
           }
           #endif*/
    /*       for(;dt_rest>=10;dt_rest-=10){
               balls_moving = proceed_dt( &balls, &walls, 0.0075 );
               }*/
    if ((!balls_moving) && balls_were_moving){
      int i;
      int old_queue_view;

      /* allways a shot to be due when balls just stopped moving */
      g_shot_due = 1;
      balls_were_moving = 0;

      //           evaluate_last_move();
      old_queue_view = queue_view;
      if (options_gamemode != options_gamemode_training){
        evaluate_last_move(player, &act_player, &balls, &queue_view, &Xque);
        if (!tournament_state.wait_for_next_match &&
          options_gamemode == options_gamemode_tournament &&
          (player[0].winner || player[1].winner))
        {
          tournament_evaluate_last_match(&tournament_state);
          tournament_state.wait_for_next_match = 1;
        }
      }
      else {
        int old_cueball_ind;
        player[act_player].place_cue_ball = 1;
        /* find a ball still in game */
        old_cueball_ind = CUE_BALL_IND;
        while (!balls.ball[CUE_BALL_IND].in_game){
          CUE_BALL_IND++;
          if (CUE_BALL_IND == balls.nr) CUE_BALL_IND = 0;
          if (CUE_BALL_IND == old_cueball_ind) break;
        }



      }
      if (old_queue_view == 1 && queue_view == 0) /* this is sloppy and ugly */
      { /* set free_view_pos to actual view */
        double th = Xrot / 180.0*M_PI;
        double ph = Zrot / 180.0*M_PI;
        free_view_pos_aim = vec_scale(vec_xyz(sin(th)*sin(ph), sin(th)*cos(ph), cos(th)), cam_dist);
        free_view_pos_aim = vec_add(free_view_pos_aim, CUE_BALL_XYPOS);
        free_view_pos = free_view_pos_aim;
      }

      /* no balls should overlap */
      all_balls_free_place(&balls);

      /* score text */
      for (i = 0; i < 2; i++){
        char str[256];
        switch (gametype){
        case GAME_8BALL: strcpy(str, "0"); break;
        case GAME_9BALL:
        {
                         int j;
                         int minballnr = 15;
                         for (j = 0; j < balls.nr; j++){
                           if (balls.ball[j].nr < minballnr && balls.ball[j].nr != 0 && balls.ball[j].in_game)
                             minballnr = balls.ball[j].nr;
                         }
                         player[i].next_9ball = minballnr;
                         sprintf(str, "next:%d", minballnr);
        }
          break;
        case GAME_CARAMBOL:
          sprintf(str, "%d", player[i].score);
          break;
        case GAME_SNOOKER:
          sprintf(str, "%c%03d  %s", (player[i].score < 0) ? '-' : '+', abs(player[i].score), player[i].snooker_on_red ? "red" : "col");
          break;
        }
        textObj_setText(player[i].score_text, str);
      }
    }
    if (g_shot_due &&
      !(options_gamemode == options_gamemode_tournament &&
      (tournament_state.wait_for_next_match || tournament_state.tournament_over)
      )
      /*          g_act_menu==0 &&
                !helpscreen_on*/
                )
    {
      //           first_time=0;
      g_shot_due = 0;
      if (player[act_player].is_AI && !(player[act_player].winner || player[(act_player + 1) % 2].winner)){
        do_computer_move(1);
      }
      if ((!player[act_player].is_AI) && player[act_player].is_net){
        fprintf(stderr, "waiting for net move----------------------------------------------\n");
        //               do_net_move();
      }
    }
    other_player = (act_player == 0) ? 1 : 0;
    if (!balls_moving && queue_anim == 0.0){
#if 1
      if (player[act_player].is_net){
        switch (g_net_state){
        case NET_STATE_REQ:
          //                   fprintf(stderr,"NET_STATE_REQ\n");
          if (net_get_send_req())  g_net_state = NET_STATE_ACK;
          break;
        case NET_STATE_ACK:
          //                   fprintf(stderr,"NET_STATE_ACK\n");
          net_send_ack();           g_net_state = NET_STATE_DATA;
          break;
        case NET_STATE_DATA:
          //                   fprintf(stderr,"NET_STATE_DATA\n");
          if (net_get_data())      g_net_state = NET_STATE_REQ;
          if (g_net_data.shoot){
            g_net_data.shoot = 0;
            shoot(!queue_view);
          }
          break;
        }
      }
      else if (player[other_player].is_net){
        switch (g_net_state){
        case NET_STATE_REQ:
          //                   fprintf(stderr,"NET_STATE_REQ\n");
          net_send_send_req();      g_net_state = NET_STATE_ACK;
          break;
        case NET_STATE_ACK:
          //                   fprintf(stderr,"NET_STATE_ACK\n");
          if (net_get_ack())       g_net_state = NET_STATE_DATA;
          break;
        case NET_STATE_DATA:
          //                   fprintf(stderr,"NET_STATE_DATA\n");
          g_net_data.shoot = 0;
          net_send_data();          g_net_state = NET_STATE_REQ;
          break;
        }
      }//    glutPostRedisplay();
#endif
    }
    //       balls_moving = 1;
    if (queue_anim > 0.0){
      //           fprintf(stderr,"frametime_ms=%f",frametime_ms);
      //           queue_anim-=(double)frametime_ms*frametime_ms/40.0/40.0;
      queue_anim -= (double)frametime_ms / 120.0*g_motion_ratio;
      //           queue_anim-=40.0/(double)frametime_ms;
      if (queue_anim < 0.0) queue_anim = 0.0;
      //           queue_offs=0.16+( 0.16*(30.0-queue_anim)*(30.0-queue_anim)/30.0/30.0*sin((30.0-queue_anim)/3.0) );
      queue_offs = 0.16*queue_offs_func((30.0 - queue_anim) / 30.0);
      if (queue_anim == 0.0){
        queue_shot();
        queue_offs = 0.06;
      }
    }
    //       glutPostRedisplay();
    sys_redisplay();
  }
  else {
    //       glutPostRedisplay();
    sys_redisplay();
  }
}




/*static void Display_timer( int value )
{
glutTimerFunc( frametime_ms_max, Display_timer, value );
glutPostRedisplay();
}*/



double strength01(double value)
{
  if (value > 1.0) value = 1.0;
  if (value < 0.0) value = 0.0;
  return value;
}

// ==
int MouseEventEnabled = 0;
// == 
void
MouseEvent(MouseButtonEnum button, MouseButtonState  state, int x, int y, int key_modifiers)
//MouseEvent(int button, int state, int x, int y)
{
    // ==
    if (!MouseEventEnabled) return;
    // ==

  //    key_modifiers=glutGetModifiers();

  if (g_act_menu != 0) {

    //        fprintf(stderr,"x,y=%d,%d\n",x,y);
    menu_select_by_coord(g_act_menu, x - win_width / 2, -y + win_height / 2);
    if (button == MOUSE_LEFT_BUTTON && state == MOUSE_DOWN) menu_choose(&g_act_menu);

    /*    } else if (options_gamemode==options_gamemode_tournament && tournament_state.wait_for_next_match) {
            if (button == MOUSE_MIDDLE_BUTTON && state == MOUSE_UP) {
            tournament_state_setup_next_match(&tournament_state);
            tournament_state.wait_for_next_match=0;
            }*/
  }
  else {

    if (button == MOUSE_LEFT_BUTTON) {
      if (state == MOUSE_DOWN) {
        if (b2_hold){
          b2_b1_hold = 1;
        }
        else {
          b1_hold = 1;
          start_x = x;
          start_y = y;
        }
      }
      if (state == MOUSE_UP) {
        b1_hold = 0;
        b2_b1_hold = 0;
      }
    }
    if (button == MOUSE_RIGHT_BUTTON){
      if (state == MOUSE_DOWN) {
        mouse_moved_after_b1_dn = 0;
        if (b1_hold){
          b1_b2_hold = 1;
        }
        else {
          b2_hold = 1;
          scaling_start = y;
          scaling_start2 = x;
        }
      }
      if (state == MOUSE_UP) {
        if (b1_b2_hold && !mouse_moved_after_b1_dn) toggle_queue_view();
        b1_b2_hold = 0;
        b2_hold = 0;
      }
    }
    if (button == MOUSE_MIDDLE_BUTTON) {
      if (state == MOUSE_UP) {
        /* this has to be the same as enter,space key !!! - maybe put it in a function some day  */
        if (options_gamemode == options_gamemode_tournament && tournament_state.wait_for_next_match) {
          tournament_state_setup_next_match(&tournament_state);
          tournament_state.wait_for_next_match = 0;
        }
        else if ((!player[act_player].is_net) && (!player[act_player].is_AI)){
          g_shot_due = 0;
          shoot(!queue_view);
        }
      }
    }
    if (button == MOUSE_WHEEL_UP_BUTTON) {
      if (!player[act_player].is_AI && !balls_moving)
        queue_strength = strength01(queue_strength + 0.01);
    }
    if (button == MOUSE_WHEEL_DOWN_BUTTON) {
      if (!player[act_player].is_AI && !balls_moving)
        queue_strength = strength01(queue_strength - 0.01);
    }

  }
  //    fprintf(stderr,"button=%d\n", button);
  //    glutPostRedisplay();
  sys_redisplay();
}


void
ball_displace_clip(VMvect * cue_pos, VMvect offs)
{
    printf("ball_displace_clip %f %f %f\n", offs.x, offs.y, offs.z);

  VMvect newpos;

  newpos = vec_add(*cue_pos, offs);

  if (options_gamemode == options_gamemode_training){

    if (newpos.x > (TABLE_W - BALL_D) / 2.0) newpos.x = (TABLE_W - BALL_D) / 2.0;
    if (newpos.x < -(TABLE_W - BALL_D) / 2.0) newpos.x = -(TABLE_W - BALL_D) / 2.0;
    if (newpos.y >(TABLE_L - BALL_D) / 2.0) newpos.y = (TABLE_L - BALL_D) / 2.0;
    if (newpos.y < -(TABLE_L - BALL_D) / 2.0) newpos.y = -(TABLE_L - BALL_D) / 2.0;

  }
  else {

    switch (gametype){
    case GAME_8BALL:
    case GAME_9BALL:
      if (newpos.x >(TABLE_W - BALL_D) / 2.0) newpos.x = (TABLE_W - BALL_D) / 2.0;
      if (newpos.x < -(TABLE_W - BALL_D) / 2.0) newpos.x = -(TABLE_W - BALL_D) / 2.0;
      if (newpos.y > -TABLE_L / 4.0) newpos.y = -TABLE_L / 4.0;
      if (newpos.y < -(TABLE_L - BALL_D) / 2.0) newpos.y = -(TABLE_L - BALL_D) / 2.0;
      break;
    case GAME_CARAMBOL:
      break;
    case GAME_SNOOKER:
#define TABLE_SCALE (TABLE_L/(3.571042))
      if (newpos.y > -TABLE_L / 2.0 + TABLE_SCALE*0.737) newpos.y = -TABLE_L / 2.0 + TABLE_SCALE*0.737;
      newpos = vec_diff(newpos, vec_xyz(0, -TABLE_L / 2.0 + TABLE_SCALE*0.737, 0));
      if (vec_abs(newpos) > TABLE_SCALE*0.292) newpos = vec_scale(vec_unit(newpos), TABLE_SCALE*0.292);
      newpos = vec_add(newpos, vec_xyz(0, -TABLE_L / 2 + TABLE_SCALE*0.737, 0));
#undef TABLE_SCALE
      break;
    }
  }

  *cue_pos = newpos;
}

extern int doingTouchGesture;
void
MouseMotion(int x, int y, int key_modifiers)
{
  static double acc;
  int place_cue_ball = 0;
  //    int other_player;

  if (control__updated) {
    printf("updated\n");
    start_x = x;
    start_y = y;
    scaling_start = y;
    scaling_start2 = x;
    control__updated = 0;
  }

  mouse_moved_after_b1_dn = 1;

  if (g_act_menu != 0){
    //        fprintf(stderr,"x,y=%d,%d\n",x,y);
    menu_select_by_coord(g_act_menu, x - win_width / 2, -y + win_height / 2);
  }
  else if (!doingTouchGesture) {

    acc = 1.0;

    if (control__active){
      if (control__place_cue_ball){
        if (player[act_player].place_cue_ball && !balls_moving  && !player[act_player].is_AI && !player[act_player].is_net){
          VMvect xv, yv, whitepos;
          double dx, dy;
          int i, move_ok;

          dx = (double)(x - start_x);
          dx = dx*0.0001 + fabs(dx)*dx*0.0002;
          dy = (double)(y - start_y);
          dy = dy*0.0001 + fabs(dy)*dy*0.0002;
          xv = vec_xyz(+dx*cos(Zrot / 180.0*M_PI), -dx*sin(Zrot / 180.0*M_PI), 0.0);
          yv = vec_xyz(-dy*sin(Zrot / 180.0*M_PI), -dy*cos(Zrot / 180.0*M_PI), 0.0);
          whitepos = balls.ball[CUE_BALL_IND].r;
          ball_displace_clip(&(balls.ball[CUE_BALL_IND].r), vec_add(xv, yv));
          move_ok = 1;
          for (i = 0; i<balls.nr; i++){
            if (i != CUE_BALL_IND){
              move_ok = move_ok &&
                (vec_abs(vec_diff(balls.ball[CUE_BALL_IND].r, balls.ball[i].r))>(balls.ball[CUE_BALL_IND].d + balls.ball[i].d) / 2.0 ||
                (!balls.ball[i].in_game));
            }
          }
          if (!move_ok) balls.ball[CUE_BALL_IND].r = whitepos;
        }
      }
      else if (control__mouse_shoot){
        if ((!queue_view) && (!balls_moving) &&
          !player[act_player].is_AI && !player[act_player].is_net) {  /* dynamic cue shot */
          queue_offs += (double)(y - start_y)*0.002;
          //        if( queue_offs < balls.ball[0].d/2.0 ) shoot();
          queue_strength = strength01(-0.02*(double)(y - start_y));
          if (queue_offs < balls.ball[CUE_BALL_IND].d / 2.0){
            queue_offs = 0.04;
            //                queue_shot();
            shoot(0);
          }
          start_x = x;
          start_y = y;
        }
      }
      else if (control__english){
        if (!player[act_player].is_AI && !player[act_player].is_net){
          double queue_point_abs;
          queue_point_y += (y - scaling_start)*0.0005;
          queue_point_x += (x - scaling_start2)*0.0005;
          queue_point_abs = sqrt(queue_point_y*queue_point_y + queue_point_x*queue_point_x);
          if (queue_point_abs > (BALL_D - QUEUE_D2) / 2.0){
            queue_point_x = queue_point_x / queue_point_abs*(BALL_D - QUEUE_D2) / 2.0;
            queue_point_y = queue_point_y / queue_point_abs*(BALL_D - QUEUE_D2) / 2.0;
          }
          scaling_start = y;
          scaling_start2 = x;
        }
      }
      else if (control__cue_butt_updown){
        double Xoffs;
        Xoffs = (double)(y - start_y)*0.02*acc;
        Xoffs += (double)(y - start_y)*fabs(y - start_y)*0.01*acc;
        if (Xque + Xoffs < -90.0) Xoffs = -90.0 - Xque;
        if (Xque + Xoffs >   0.0) Xoffs = 0.0 - Xque;
        //            Xrot += Xoffs;
        //            Xrot_offs -= Xoffs;
        Xque += Xoffs;
        /*            if( queue_view ){
                        Xque=Xrot;
                        }*/
      }
      start_x = x;
      start_y = y;
      scaling_start = y;
      scaling_start2 = x;

    }
    else if (b1_hold){
      if ((key_modifiers & KEY_MODIFIER_SHIFT) || b1_b2_hold)
        place_cue_ball = player[act_player].place_cue_ball;
      if ((key_modifiers & KEY_MODIFIER_CTRL) && (!queue_view) && (!balls_moving) &&
        !player[act_player].is_AI && !player[act_player].is_net) {  /* dynamic cue shot */
        queue_offs += (double)(y - start_y)*0.002;
        //        if( queue_offs < balls.ball[0].d/2.0 ) shoot();
        queue_strength = strength01(-0.02*(double)(y - start_y));
        if (queue_offs < balls.ball[CUE_BALL_IND].d / 2.0){
          queue_offs = 0.04;
          //                queue_shot();
          shoot(0);
        }
        start_x = x;
        start_y = y;
      }
      else if (!place_cue_ball && !(key_modifiers & KEY_MODIFIER_CTRL)){
        double Xoffs, Zoffs;
        /*            Xrot +=  (double)(y-start_y)*acc;
                    Zrot +=  (double)(x-start_x)*acc;
                    if( Xrot < -90.0  ) Xrot=-90.0;
                    if( Xrot >   0.0  ) Xrot= 0.0;*/
        Xoffs = (double)(y - start_y)*0.02*acc;
        Zoffs = (double)(x - start_x)*0.02*acc;
        Xoffs += (double)(y - start_y)*fabs(y - start_y)*0.01*acc;
        Zoffs += (double)(x - start_x)*fabs(x - start_x)*0.01*acc;
        if (Xrot + Xoffs < -90.0) Xoffs = -90.0 - Xrot;
        if (Xrot + Xoffs >   0.0) Xoffs = 0.0 - Xrot;
        Xrot += Xoffs;
        Zrot += Zoffs;
        Xrot_offs -= Xoffs;
        Zrot_offs -= Zoffs;
        if (queue_view){
          Xque = Xrot;
          Zque = Zrot;
        }
      }
      else if (place_cue_ball && !balls_moving  && !player[act_player].is_AI && !player[act_player].is_net){
        // update position of cueball
          printf("place cueball\n");
          VMvect xv, yv, whitepos;
        double dx, dy;
        int i, move_ok;
        //            dx=(double)(x-start_x)*acc*0.005;
        //            dy=(double)(y-start_y)*acc*0.005;
        dx = (double)(x - start_x);
        dx = dx*0.0001 + fabs(dx)*dx*0.0002;
        dy = (double)(y - start_y);
        dy = dy*0.0001 + fabs(dy)*dy*0.0002;
        //            yv=vec_xyz(dx*cos(Zrot),-dx*sin(Zrot),0.0);
        xv = vec_xyz(+dx*cos(Zrot / 180.0*M_PI), -dx*sin(Zrot / 180.0*M_PI), 0.0);
        yv = vec_xyz(-dy*sin(Zrot / 180.0*M_PI), -dy*cos(Zrot / 180.0*M_PI), 0.0);
        whitepos = balls.ball[CUE_BALL_IND].r;
        ball_displace_clip(&(balls.ball[CUE_BALL_IND].r), vec_add(xv, yv));
        /*
        balls.ball[CUE_BALL_IND].r = vec_add( balls.ball[CUE_BALL_IND].r, xv );
        balls.ball[CUE_BALL_IND].r = vec_add( balls.ball[CUE_BALL_IND].r, yv );
        if( balls.ball[CUE_BALL_IND].r.x >  (TABLE_W-BALL_D)/2.0 ) balls.ball[CUE_BALL_IND].r.x= (TABLE_W-BALL_D)/2.0 ;
        if( balls.ball[CUE_BALL_IND].r.x < -(TABLE_W-BALL_D)/2.0 ) balls.ball[CUE_BALL_IND].r.x=-(TABLE_W-BALL_D)/2.0 ;
        if( balls.ball[CUE_BALL_IND].r.y > -TABLE_L/4.0          ) balls.ball[CUE_BALL_IND].r.y= -TABLE_L/4.0 ;
        if( balls.ball[CUE_BALL_IND].r.y < -(TABLE_L-BALL_D)/2.0 ) balls.ball[CUE_BALL_IND].r.y=-(TABLE_L-BALL_D)/2.0 ;
        */
        move_ok = 1;
        for (i = 0; i<balls.nr; i++){
          if (i != CUE_BALL_IND){
            move_ok = move_ok &&
              (vec_abs(vec_diff(balls.ball[CUE_BALL_IND].r, balls.ball[i].r))>(balls.ball[CUE_BALL_IND].d + balls.ball[i].d) / 2.0 ||
              (!balls.ball[i].in_game));
          }
        }
        if (!move_ok) balls.ball[CUE_BALL_IND].r = whitepos;
      }
      start_x = x;
      start_y = y;
    }
    else if (b2_hold){
      if (key_modifiers & KEY_MODIFIER_CTRL){
        double old_FOV;
        old_FOV = cam_FOV;
        cam_FOV += (y - scaling_start)*0.05;
        if (cam_FOV<10.0) cam_FOV = 10.0;
        if (cam_FOV>110.0) cam_FOV = 110.0;
        cam_dist_aim = cam_dist_aim*tan(old_FOV*M_PI / 360.0) / tan(cam_FOV*M_PI / 360.0);
        cam_dist = cam_dist_aim;
        scaling_start = y;
      }
      else if (((key_modifiers & KEY_MODIFIER_SHIFT) || b2_b1_hold) && /*!queue_view &&*/
        !player[act_player].is_AI && !player[act_player].is_net){
        double queue_point_abs;
        queue_point_y += (y - scaling_start)*0.0005;
        queue_point_x += (x - scaling_start2)*0.0005;
        queue_point_abs = sqrt(queue_point_y*queue_point_y + queue_point_x*queue_point_x);
        if (queue_point_abs > (BALL_D - QUEUE_D2) / 2.0){
          queue_point_x = queue_point_x / queue_point_abs*(BALL_D - QUEUE_D2) / 2.0;
          queue_point_y = queue_point_y / queue_point_abs*(BALL_D - QUEUE_D2) / 2.0;
        }
        //            fprintf(stderr,"queue_point_x: %f\n",queue_point_x);
        //            fprintf(stderr,"queue_point_y: %f\n",queue_point_y);
        //            if( cam_dist_aim<0.0 ) cam_dist_aim=0.0;
        scaling_start = y;
        scaling_start2 = x;
      }
      else {
        if (!FREE_VIEW){
          cam_dist_aim += (y - scaling_start)*0.005;
          if (cam_dist_aim < 0.0) cam_dist_aim = 0.0;
          scaling_start = y;
        }
        else {
          double th, ph;
          VMvect dvec;
          th = Xrot / 180.0*M_PI;
          ph = Zrot / 180.0*M_PI;
          dvec = vec_xyz(sin(th)*sin(ph), sin(th)*cos(ph), cos(th));
          dvec = vec_scale(dvec, (y - scaling_start)*0.005);
          free_view_pos_aim = vec_add(free_view_pos_aim, dvec);
          if (free_view_pos_aim.z < 0.1) free_view_pos_aim.z = 0.1;
          scaling_start = y;
        }
      }
    }

  }
  sys_redisplay();
}


void myRect2D(double x1, double y1, double x2, double y2)
{
  glBegin(GL_QUADS);
  glVertex3f(x1, y1, -0.5);
  glVertex3f(x1, y2, -0.5);
  glVertex3f(x2, y2, -0.5);
  glVertex3f(x2, y1, -0.5);
  glEnd();
}


#ifdef TIME_INTERPOLATE
void interpolate_balls( BallsType * balls1, BallsType * balls2, BallsType * newballs, double ratio )
{
  int i,j;
  //    fprintf(stderr,"ratio=%f",ratio);
  for(i=0;i<balls2->nr;i++){
    newballs->ball[i]=balls2->ball[i];
    newballs->ball[i].r = vec_add(
      vec_scale(balls1->ball[i].r,1.0-ratio),
      vec_scale(balls2->ball[i].r,ratio)
      );
    for(j=0;j<3;j++)
      newballs->ball[i].b[j] = vec_unit(vec_add(
      vec_scale(balls1->ball[i].b[j],1.0-ratio),
      vec_scale(balls2->ball[i].b[j],ratio)
      )
      );
  }
}
#endif


#if 0
void cull_balls( BallsType balls, myvec cam_pos, int win_width, int win_height, float cam_FOV, float th, float ph)
{
  int ballnr;
  double d, ang, ang1, ang2;
  double cam_FOV2;
  VMvect dvec, ballvec, right, up, cam_pos_;

  cam_FOV2=2.0*180.0/M_PI*atan(tan(cam_FOV*M_PI/180.0/2.0)/win_width*win_height);

  //    th=(Xrot+Xrot_offs)/180.0*M_PI;
  //    ph=(Zrot+Zrot_offs)/180.0*M_PI;
  th=th/180.0*M_PI;
  ph=ph/180.0*M_PI;
  dvec  = vec_xyz(-sin(th)*sin(ph),-sin(th)*cos(ph),-cos(th));
  for(ballnr=0;ballnr<balls.ball[i];ballnr++){
    cam_pos_ = vec_diff( cam_pos, vec_scale(dvec,balls.ball[ballnr].d/2.0/sin(cam_FOV*M_PI/180.0/2.0)) );
    right = vec_unit(vec_xyz(dvec.y,-dvec.x,0));
    up    = vec_cross(right,dvec);
    ballvec = vec_diff(balls.ball[ballnr].r,cam_pos_);
    ang1  = atan2( vec_mul(ballvec, right), vec_mul(ballvec, dvec) );
    ang2  = atan2( vec_mul(ballvec, up),    vec_mul(ballvec, dvec) );
    d = vec_mul(vec_diff(balls.ball[ballnr].r,cam_pos_),dvec);
    ang = d/vec_abs(ballvec);
    ang = (fabs(ang)<1.0)?acos(ang):0.0;
    if(
      fabs(ang1) < cam_FOV*M_PI/180.0/2.0 &&
      fabs(ang2) < cam_FOV2*M_PI/180.0/2.0
      )
    {
      balls.ball[ballnr].in_fov=1;
    } else {
      balls.ball[ballnr].in_fov=1;
    }
  }
}
#endif


void draw_3D_winner_text()
{
  static double ang = 0.0;
  static double tprev = 0.0;
  double t;
  double dt;

  t = time_us();
  dt = (t - tprev) / 1000000.0;
  tprev = t;

  textObj_setText(winner_name_text_obj, player[player[0].winner ? 0 : 1].name);
  //           glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
  glColor3f(1.0, 1.0, 1.0);
  glBindTexture(GL_TEXTURE_2D, spheretexbind);
  glPushMatrix();
  glRotatef(ang += 60.0*dt, 0, 0, 1);
  glTranslatef(0, 0, 0.35);
  glRotatef(90, 1, 0, 0);
  textObj_draw_centered(winner_text_obj);
  glTranslatef(0, 0.3, 0);
  glRotatef(-ang*2.0, 0, 1, 0);
  textObj_draw_centered(winner_name_text_obj);
  glPopMatrix();
  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);
  glEnable(GL_LIGHTING);
  //           glEnable(GL_TEXTURE_2D);
}


void DisplayFunc_cubemap(int ballnr, int side, VMvect cam_pos, int cube_res)
{
  GLfloat light_position[] = {0.0, 0.0, 0.7, 1.0};
  //   GLfloat light_diff[]     = { 0.3, 0.3, 0.3, 1.0 };
  //   GLfloat light_amb[]      = { 0.2, 0.2, 0.2, 1.0 };
  GLfloat light_diff[] = {0.2, 0.2, 0.2, 1.0};
  GLfloat light_amb[] = {0.05, 0.05, 0.05, 1.0};

  int i;

  float mv_matr[16];


  glDisable(GL_LIGHT1);

#ifdef TIME_INTERPOLATE
  interpolate_balls( &g_lastballs, &balls, &g_drawballs, (double)g_frametime_fromlast/(double)g_frametime_laststep );
#endif

  if (options_positional_light){
    /* always use directional light for reflections for better performance */
    light_position[3] = 0.0;
  }
  else {
    light_position[3] = 0.0;
  }


  glMatrixMode(GL_PROJECTION);
  {
    double znear, zfar;
    znear = 0.01;
    zfar = 3.0;
    //       glFrustum( left, right, bottom, top, zNear, zFar );
    /*       {
               float m[16];
               glLoadIdentity();
               glFrustum( -0.1, 0.2,  -0.1,0.2,  0.1,1.0 );
               glGetFloatv(GL_PROJECTION_MATRIX,m);
               printf("\nmatrix=\n %f %f %f %f\n %f %f %f %f\n %f %f %f %f\n %f %f %f %f\n",
               m[0],m[4],m[8],m[12],
               m[1],m[5],m[9],m[13],
               m[2],m[6],m[10],m[14],
               m[3],m[7],m[11],m[15]
               );
               }*/
    glLoadIdentity();
    glFrustum(-znear, +znear, -znear, +znear, znear, zfar);
  }
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  for (i = 0; i < 16; i++) mv_matr[i] = 0.0;
  mv_matr[15] = 1.0;

#define NEG (1.0)
  switch (side){
  case GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB:  /* (s,t)=(-z,-y) */
    //                                   -y*ey                   -z*ex
    mv_matr[0] = 0.0;  mv_matr[4] = 0.0*NEG;  mv_matr[8] = -1.0;
    mv_matr[1] = 0.0;  mv_matr[5] = -1.0*NEG;  mv_matr[9] = 0.0;
    mv_matr[2] = -1.0;  mv_matr[6] = 0.0*NEG;  mv_matr[10] = 0.0;
    break;
  case GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB:  /* (s,t)=(+z,-y) */
    //                                   -y*ey                   +z*ex
    mv_matr[0] = 0.0;  mv_matr[4] = 0.0*NEG;  mv_matr[8] = 1.0;
    mv_matr[1] = 0.0;  mv_matr[5] = -1.0*NEG;  mv_matr[9] = 0.0;
    mv_matr[2] = 1.0;  mv_matr[6] = 0.0*NEG;  mv_matr[10] = 0.0;
    break;
  case GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB:  /* (s,t)=(+x,+z) */
    //                +x*ex                                  +z*ey
    mv_matr[0] = 1.0;  mv_matr[4] = 0.0;  mv_matr[8] = 0.0*NEG;
    mv_matr[1] = 0.0;  mv_matr[5] = 0.0;  mv_matr[9] = 1.0*NEG;
    mv_matr[2] = 0.0;  mv_matr[6] = -1.0;  mv_matr[10] = 0.0*NEG;
    break;
  case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB:  /* (s,t)=(+x,-z) */
    //                +x*ex                                  -z*ey
    mv_matr[0] = 1.0;  mv_matr[4] = 0.0;  mv_matr[8] = 0.0*NEG;
    mv_matr[1] = 0.0;  mv_matr[5] = 0.0;  mv_matr[9] = -1.0*NEG;
    mv_matr[2] = 0.0;  mv_matr[6] = 1.0;  mv_matr[10] = 0.0*NEG;
    break;
  case GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB:  /* (s,t)=(+x,-y) */
    //                +x*ex              -y*ey
    mv_matr[0] = 1.0;  mv_matr[4] = 0.0*NEG;  mv_matr[8] = 0.0;
    mv_matr[1] = 0.0;  mv_matr[5] = -1.0*NEG;  mv_matr[9] = 0.0;
    mv_matr[2] = 0.0;  mv_matr[6] = 0.0*NEG;  mv_matr[10] = -1.0;
    break;
  case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB:  /* (s,t)=(-x,-y) */
    //                -x*ex              -y*ey
    mv_matr[0] = -1.0;  mv_matr[4] = 0.0*NEG;  mv_matr[8] = 0.0;
    mv_matr[1] = 0.0;  mv_matr[5] = -1.0*NEG;  mv_matr[9] = 0.0;
    mv_matr[2] = 0.0;  mv_matr[6] = 0.0*NEG;  mv_matr[10] = 1.0;
    break;
  }
#undef NEG

  glLoadMatrixf(mv_matr);
  glTranslatef(-balls.ball[ballnr].r.x,
    -balls.ball[ballnr].r.y,
    -balls.ball[ballnr].r.z);
  cam_pos = vec_scale(vec_unit(vec_diff(cam_pos, balls.ball[ballnr].r)), BALL_D / 2.5);
  glTranslatef(-cam_pos.x, -cam_pos.y, -cam_pos.z);

  glPushMatrix();


  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diff);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_amb);
  //   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);


  /* draw table */
  glCallList(table_obj);

  /* some whole-ball-culling might be of use here */


  /* draw balls with reflections and shadows */
#ifdef TIME_INTERPOLATE
  draw_balls(g_drawballs,balls.ball[ballnr].r,90.0,cube_res,spheretexbind,lightpos,lightnr, (int *)0);
#else
  draw_balls(balls, balls.ball[ballnr].r, 90.0, cube_res, spheretexbind, lightpos, lightnr, (int *)0);
#endif

  if (!balls_moving){  /* draw queue */
    draw_queue(balls.ball[CUE_BALL_IND].r, Xque, Zque, queue_offs,
      queue_point_x, queue_point_y,
      spheretexbind, lightpos, lightnr);
  }

  if (options_place_cue_ball_tex && player[act_player].place_cue_ball && !balls_moving){
    int i;
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glBlendFunc(GL_ONE, GL_ONE);
    glColor3f(0.25, 0.25, 0.25);
    glBindTexture(GL_TEXTURE_2D, placecueballtexbind);
#define SH_SZ 0.087
    i = CUE_BALL_IND;
    glBegin(GL_QUADS);
    glNormal3f(0.0, 0.0, 1.0);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(balls.ball[i].r.x - SH_SZ, balls.ball[i].r.y + SH_SZ, balls.ball[i].r.z - balls.ball[i].d / 2.02);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(balls.ball[i].r.x + SH_SZ, balls.ball[i].r.y + SH_SZ, balls.ball[i].r.z - balls.ball[i].d / 2.02);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(balls.ball[i].r.x + SH_SZ, balls.ball[i].r.y - SH_SZ, balls.ball[i].r.z - balls.ball[i].d / 2.02);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(balls.ball[i].r.x - SH_SZ, balls.ball[i].r.y - SH_SZ, balls.ball[i].r.z - balls.ball[i].d / 2.02);
    glEnd();
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
#undef SH_SZ
    glEnable(GL_LIGHTING);
  }

  if ((player[0].winner || player[1].winner)){
    if (options_3D_winnertext){
      draw_3D_winner_text();
    }
  }

  /* light rects */
  {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0, 1.0, 1.0, 1.0);

    glBegin(GL_QUADS);
    glNormal3f(0.0, 0.0, -1.0);

    glVertex3f(0.18, 0.15, 1.0);
    glVertex3f(0.18, 0.76, 1.0);
    glVertex3f(-0.18, 0.76, 1.0);
    glVertex3f(-0.18, 0.15, 1.0);

    glVertex3f(-0.18, -0.15, 1.0);
    glVertex3f(-0.18, -0.76, 1.0);
    glVertex3f(0.18, -0.76, 1.0);
    glVertex3f(0.18, -0.15, 1.0);

    glEnd();

    glColor4f(0.6, 0.6, 0.6, 1.0);
    glBegin(GL_QUADS);

    glVertex3f(0.20, 0.13, 1.001);
    glVertex3f(0.20, 0.78, 1.001);
    glVertex3f(-0.20, 0.78, 1.001);
    glVertex3f(-0.20, 0.13, 1.001);

    glVertex3f(-0.20, -0.13, 1.001);
    glVertex3f(-0.20, -0.78, 1.001);
    glVertex3f(0.20, -0.78, 1.001);
    glVertex3f(0.20, -0.13, 1.001);

    glEnd();

    glColor4f(0.3, 0.3, 0.3, 1.0);
    glBegin(GL_QUADS);
    glVertex3f(0.28, -0.86, 1.002);
    glVertex3f(0.28, 0.86, 1.002);
    glVertex3f(-0.28, 0.86, 1.002);
    glVertex3f(-0.28, -0.86, 1.002);
    glEnd();

    glColor4f(0.15, 0.2, 0.15, 1.0);
    glBegin(GL_QUADS);
    glVertex3f(0.38, -0.96, 1.004);
    glVertex3f(0.38, 0.96, 1.004);
    glVertex3f(-0.38, 0.96, 1.004);
    glVertex3f(-0.38, -0.96, 1.004);
    glEnd();

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
  }

  glPopMatrix();

  //   glutSwapBuffers();
}


void create_cuberef_map(int ballnr, int texbind, VMvect cam_pos)
{
  int i, w, h, target, level;
  double d, ang, ang1, ang2;
  double th, ph, cam_FOV2;
  VMvect dvec, ballvec, right, up, cam_pos_;

  glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, texbind);

  /* calc necessary detail level */
  //    level=ballnr%5;
  //    level=vec_abs(vec_diff(balls.ball[ballnr].r,cam_pos))/2.5*5.0;

  cam_FOV2 = 2.0*180.0 / M_PI*atan(tan(cam_FOV*M_PI / 180.0 / 2.0) / win_width*win_height);

  th = (Xrot + Xrot_offs) / 180.0*M_PI;
  ph = (Zrot + Zrot_offs) / 180.0*M_PI;
  dvec = vec_xyz(-sin(th)*sin(ph), -sin(th)*cos(ph), -cos(th));
  cam_pos_ = vec_diff(cam_pos, vec_scale(dvec, balls.ball[ballnr].d / 2.0 / sin(cam_FOV*M_PI / 180.0 / 2.0)));
  right = vec_unit(vec_xyz(dvec.y, -dvec.x, 0));
  up = vec_cross(right, dvec);
  ballvec = vec_diff(balls.ball[ballnr].r, cam_pos_);
  ang1 = atan2(vec_mul(ballvec, right), vec_mul(ballvec, dvec));
  ang2 = atan2(vec_mul(ballvec, up), vec_mul(ballvec, dvec));
  d = vec_mul(vec_diff(balls.ball[ballnr].r, cam_pos_), dvec);
  ang = d / vec_abs(ballvec);
  ang = (fabs(ang) < 1.0) ? acos(ang) : 0.0;

  /*    if(ballnr==0)
          printf("ang2=%lf\n",ang2);*/
  //    if( ang < M_PI/4.0 ){
  if (
    fabs(ang1) < cam_FOV*M_PI / 180.0 / 2.0 &&
    fabs(ang2) < cam_FOV2*M_PI / 180.0 / 2.0
    ){

    level = log(d / 0.2) / log(2.0) - 1.0;
    if (level < 0) level = 0;
    /*        if(ballnr==0)
                printf("level==%d\n",level);*/

    w = cuberef_res >> level;
    h = w;

    /*        glScissor( 0, 0, 3*cuberef_res, 2*cuberef_res );
            glViewport( 0, 0, 3*cuberef_res, 2*cuberef_res );
            glEnable( GL_SCISSOR_TEST );
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glDisable( GL_SCISSOR_TEST );*/
    for (i = 0; i < 6; i++){
      //        printf("   creating cubemap #%d\n",i);
      glViewport((i % 3)*cuberef_res/*+9*/, (i / 3)*cuberef_res, w, h);
      switch (i){
      case 0: target = GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB;
        DisplayFunc_cubemap(ballnr, GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, cam_pos, w);
        break;
      case 1: target = GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB;
        DisplayFunc_cubemap(ballnr, GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, cam_pos, w);
        break;
      case 2: target = GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB;
        DisplayFunc_cubemap(ballnr, GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, cam_pos, w);
        break;
      case 3: target = GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB;
        DisplayFunc_cubemap(ballnr, GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, cam_pos, w);
        break;
      case 4: target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB;
        DisplayFunc_cubemap(ballnr, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, cam_pos, w);
        break;
      case 5: target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB;
        DisplayFunc_cubemap(ballnr, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, cam_pos, w);
        break;
      }
      glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_BASE_LEVEL, level);
      glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAX_LEVEL, level);
      glCopyTexSubImage2D(target, level,
        0, 0,
        (i % 3)*cuberef_res, (i / 3)*cuberef_res/*+1*/,
        w, h);
    }
  }
}

void create_cuberef_maps(VMvect cam_pos)
{
  int i;
  for (i = 0; i < balls.nr; i++) if (balls.ball[i].in_game){
    //        printf("creating cubemaps for ball #%d\n",i);
    glColorMask(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    create_cuberef_map(i, cuberef_allballs_texbind[i], cam_pos);
  }
}


void Display_tournament_tree(struct TournamentState_ * ts)
{
  int i, j;
  static textObj * title = 0;
  static textObj * bottom = 0;

  DPRINTF("Display_tournament_tree: 1");
  if (title == 0){
    title = textObj_new("Tournament", options_menu_fontname, 32);
  }
  if (bottom == 0){
    bottom = textObj_new("<fire> to continue", options_menu_fontname, 16);
  }

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glPushMatrix();
  glLoadIdentity();
  glScalef(0.8, 0.8, 1.0);

  glColor4f(0.4, 0.4, 0.4, 0.7);
  glDisable(GL_TEXTURE_2D);
  /* top line */
  glBegin(GL_QUAD_STRIP);
  glVertex3f(-1, 0.98, 0);
  glVertex3f(1, 0.98, 0);
  glVertex3f(-1, 0.82, 0);
  glVertex3f(1, 0.82, 0);
  glEnd();
  /* bottom line */
  glBegin(GL_QUAD_STRIP);
  glVertex3f(-1, -0.82, 0);
  glVertex3f(1, -0.82, 0);
  glVertex3f(-1, -0.98, 0);
  glVertex3f(1, -0.98, 0);
  glEnd();

  glColor4f(0.6, 0.6, 0.6, 0.85);
  glBindTexture(GL_TEXTURE_2D, fblogotexbind);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUAD_STRIP);
  glTexCoord2f(-0.3 - 0.15, 0.06 - 0.15);
  glVertex3f(-1, 0.8, 0);
  glTexCoord2f(1.3 - 0.15, 0.06 - 0.15);
  glVertex3f(1, 0.8, 0);
  glTexCoord2f(-0.3 + 0.15, 0.94 + 0.15);
  glVertex3f(-1, -0.8, 0);
  glTexCoord2f(1.3 + 0.15, 0.94 + 0.15);
  glVertex3f(1, -0.8, 0);
  glEnd();

  glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
  //    glDisable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);

  glPushMatrix();
  glTranslatef(0, 0.9, 0);
  glPushMatrix();
  glScalef(0.004, 0.005, 1.0);
  textObj_draw_bound(title, HBOUND_CENTER, VBOUND_CENTER);
  glPopMatrix();
  glTranslatef(0, -1.8, 0);
  glPushMatrix();
  glScalef(0.004, 0.004, 1.0);
  textObj_draw_bound(bottom, HBOUND_CENTER, VBOUND_CENTER);
  glPopMatrix();
  glPopMatrix();

  glTranslatef(-1.0, 0.8, 0);
  glTranslatef(0.5*2.0 / ts->round_num, 0, 0);

  DPRINTF("Display_tournament_tree: 2");
  for (i = 0; i <= ts->round_ind; i++){
    glPushMatrix();
    glTranslatef(0, -0.5*1.6 / (float)(1 << (ts->round_num - i)), 0);
    for (j = 0; j < (1 << (ts->round_num - i - 1)); j++){
      DPRINTF("Display_tournament_tree: drawing player %s\n", ts->roster.player[ts->game[i][j].roster_player1].text->str);
      if (ts->roster.player[ts->game[i][j].roster_player1].text){
        glPushMatrix();
        glScalef(0.003, 0.003, 1.0);
        if (ts->game[i][j].winner == 0) glColor3f(0.0, 1.0, 1.0);
        else if (ts->game[i][j].winner == 1) glColor3f(0.5, 0.5, 0.5);
        else                              glColor3f(1.0, 1.0, 1.0);
        textObj_draw_bound(ts->roster.player[ts->game[i][j].roster_player1].text, HBOUND_CENTER, VBOUND_CENTER);
        glPopMatrix();
      }
      glTranslatef(0, -1.6 / (float)(1 << (ts->round_num - i)), 0);
      DPRINTF("Display_tournament_tree: drawing player %s\n", ts->roster.player[ts->game[i][j].roster_player2].text->str);
      if (ts->roster.player[ts->game[i][j].roster_player2].text){
        glPushMatrix();
        glScalef(0.003, 0.003, 1.0);
        if (ts->game[i][j].winner == 1) glColor3f(0.0, 1.0, 1.0);
        else if (ts->game[i][j].winner == 0) glColor3f(0.5, 0.5, 0.5);
        else                              glColor3f(1.0, 1.0, 1.0);
        textObj_draw_bound(ts->roster.player[ts->game[i][j].roster_player2].text, HBOUND_CENTER, VBOUND_CENTER);
        glPopMatrix();
      }
      glTranslatef(0, -1.6 / (float)(1 << (ts->round_num - i)), 0);
    }
    glPopMatrix();
    glTranslatef(2.0 / ts->round_num, 0, 0);
  }

  glPopMatrix(); /* glScalef(0.8,0.8,1.0); */

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
}


void draw_text(int x, int y, const char* s, int height) {
	// === custom text
	textObj* text = textObj_new(s, options_ball_fontname, height);
	glPushMatrix();

	// Screen goes from -1 to 1 in both directions, (0,0) is the center of the screen
	glTranslatef(x, y, 0);
	glScalef(1, -1, 0); // invert because the screen is upside down
	textObj_draw(text);

	glPopMatrix();
	textObj_delete(text);
}

void draw_rect(int x, int y, int w, int h)
{
	glPushMatrix();  //Make sure our transformations don't affect any other transformations in other code
	glTranslatef(x, y, 0);  //Translate rectangle to its assigned x and y position
	//Put other transformations here
	glBegin(GL_LINE_LOOP);   //We want to draw a quad, i.e. shape with four sides
	glVertex2f(0, 0);            //Draw the four corners of the rectangle
	glVertex2f(0, h);
	glVertex2f(w, h);
	glVertex2f(w, 0);
	glEnd();
	glPopMatrix();
}

void draw_circle(float x, float y, float radius)
{
	glPushMatrix();  //Make sure our transformations don't affect any other transformations in other code
	glTranslatef(x, y, 0);  //Translate rectangle to its assigned x and y position

	glBegin(GL_LINE_LOOP);

	for (int i = 0; i < 360; i++)
	{
		float degInRad = i*(M_PI / 180);
		glVertex2f(cos(degInRad)*radius, sin(degInRad)*radius);
	}

	glEnd();

	glPopMatrix();
}

// ==
extern GLdouble modelview[16];
extern GLdouble projection[16];
extern GLint viewport[4];
extern int touchmode;
// ==

void DisplayFunc(void)
{
    // == custom
    textObj* text;
    // ===
  //   int i;
  double th, ph;
  GLfloat light_position[] = {0.0, 0.0, 0.7, 1.0};
  GLfloat light0_position[] = {0.0, 0.7, 0.7, 1.0};
  GLfloat light0_diff[] = {0.6, 0.6, 0.6, 1.0};
  GLfloat light0_amb[] = {0.35, 0.35, 0.35, 1.0};
  GLfloat light1_position[] = {0.0, -0.7, 0.7, 1.0};
  GLfloat light1_diff[] = {0.6, 0.6, 0.6, 1.0};
  GLfloat light1_amb[] = {0.35, 0.35, 0.35, 1.0};
  //   GLfloat light_diff_r[]   = { 1.0, 0.0, 0.0, 1.0 };
  //   GLfloat light_amb_r[]    = { 0.5, 0.0, 0.0, 1.0 };
  //   GLfloat light_diff_g[]   = { 0.0, 0.7, 0.0, 1.0 };
  //   GLfloat light_amb_g[]    = { 0.0, 0.5*0.7, 0.0, 1.0 };
  myvec cam_pos;
  static GLfloat real_dist = 0.0;
  static double fps;
  int i;
  //   int act_buffer;
  VMmatrix4 mv_matr;
  VMmatrix4 prj_matr;

  static GLfloat rg_eye_dist = 0.05;
  //   static GLfloat rg_scrw=0.225;
  //   static GLfloat rg_scrh=0.165;


  if (old_queue_view == 1 && queue_view == 0) /* this is sloppy and ugly */
  { /* set free_view_pos to actual view */
    double th = Xrot / 180.0*M_PI;
    double ph = Zrot / 180.0*M_PI;
    free_view_pos_aim = vec_scale(vec_xyz(sin(th)*sin(ph), sin(th)*cos(ph), cos(th)), cam_dist);
    free_view_pos_aim = vec_add(free_view_pos_aim, CUE_BALL_XYPOS);
    free_view_pos = free_view_pos_aim;
  }
  old_queue_view = queue_view;


  th = (Xrot + Xrot_offs) / 180.0*M_PI;
  ph = (Zrot + Zrot_offs) / 180.0*M_PI;
  if (!FREE_VIEW){
    cam_pos = vec_scale(vec_xyz(sin(th)*sin(ph), sin(th)*cos(ph), cos(th)),
      real_dist);
    cam_pos = vec_add(cam_pos, balls.ball[CUE_BALL_IND].r);
  }
  else {
    cam_pos = free_view_pos;
  }


  if (options_cuberef){
    create_cuberef_maps(cam_pos);
  }

  glViewport(0, 0, win_width, win_height);



#ifdef TIME_INTERPOLATE
  interpolate_balls( &g_lastballs, &balls, &g_drawballs, (double)g_frametime_fromlast/(double)g_frametime_laststep );
#endif

  if (options_positional_light){
    light_position[3] = 1.0;
    light0_position[3] = 1.0;
    light1_position[3] = 1.0;
  }
  else {
    light_position[3] = 0.0;
    light0_position[3] = 0.0;
    light1_position[3] = 0.0;
  }


  fps = 1000.0 / frametime_ms;

  //   glScalef(100.0,100.0,100.0);

  //   my_glBox(cam_pos.x+0.01, cam_pos.y+0.01, cam_pos.z+0.01,
  //            cam_pos.x-0.01, cam_pos.y-0.01, cam_pos.z-0.01);



  if (!FREE_VIEW){
    glFogf(GL_FOG_START, (cam_dist / 2.0 > cam_dist - 1.0) ? cam_dist / 2.0 : cam_dist - 1.0);
    glFogf(GL_FOG_END, cam_dist + 6.0);
  }
  else{
    double cam_dist0;
    cam_dist0 = vec_abs(cam_pos);
    glFogf(GL_FOG_START, (cam_dist0 / 2.0 > cam_dist0 - 1.0) ? cam_dist0 / 2.0 : cam_dist0 - 1.0);
    glFogf(GL_FOG_END, cam_dist0 + 6.0);
  }

  real_dist = cam_dist;

  //   glGetIntegerv(GL_DRAW_BUFFER, &act_buffer);
  if (options_rgstereo_on){
    //       glClearAccum(0.0, 0.0, 0.0, 0.0);
    //       glClear(GL_ACCUM_BUFFER_BIT);
    glColorMask(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
  }
  for (i = 0; i <= options_rgstereo_on; i++){

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (options_rgstereo_on){
      double znear, zfar;
      double eye_offs, zeye;
      znear = 0.03;
      zfar = 10.0;
      zeye = (double)win_width / 2.0 / scr_dpi*0.025 / tan(cam_FOV*M_PI / 360.0);
      eye_offs = rg_eye_dist / 2.0*znear / zeye;
      glLoadIdentity();
      if (i == 0){
        double eye_offs0;
        if (options_rgaim == 0) eye_offs0 = -eye_offs;
        if (options_rgaim == 1) eye_offs0 = -2.0*eye_offs;
        if (options_rgaim == 2) eye_offs0 = 0.0;
        glColorMask(1, 0, 0, 1);
        glMatrixMode(GL_PROJECTION);
        glFrustum(-znear*tan(cam_FOV*M_PI / 360.0) + eye_offs0, znear*tan(cam_FOV*M_PI / 360.0) + eye_offs0,
          -znear*tan(cam_FOV*M_PI / 360.0)*(double)win_height / (double)win_width,
          +znear*tan(cam_FOV*M_PI / 360.0)*(double)win_height / (double)win_width, znear, zfar);
        glMatrixMode(GL_MODELVIEW);
        glTranslatef(eye_offs0 / znear*zeye, 0.0, 0.0);

      }
      else if (i == 1){
        double eye_offs1;
        if (options_rgaim == 0) eye_offs1 = +eye_offs;
        if (options_rgaim == 1) eye_offs1 = 0.0;
        if (options_rgaim == 2) eye_offs1 = +2.0*eye_offs;
        glColorMask(0, 1, 1, 1);
        glMatrixMode(GL_PROJECTION);
        glFrustum(-znear*tan(cam_FOV*M_PI / 360.0) + eye_offs1, znear*tan(cam_FOV*M_PI / 360.0) + eye_offs1,
          -znear*tan(cam_FOV*M_PI / 360.0)*(double)win_height / (double)win_width,
          +znear*tan(cam_FOV*M_PI / 360.0)*(double)win_height / (double)win_width, znear, zfar);
        glMatrixMode(GL_MODELVIEW);
        glTranslatef(eye_offs1 / znear*zeye, 0.0, 0.0);
      }
    }
    else{
      double znear, zfar;
      znear = 0.03;
      zfar = 10.0;
      //       glFrustum( left, right, bottom, top, zNear, zFar );
      glMatrixMode(GL_PROJECTION);
      glFrustum(-znear*tan(cam_FOV*M_PI / 360.0), znear*tan(cam_FOV*M_PI / 360.0),
        -znear*tan(cam_FOV*M_PI / 360.0)*(double)win_height / (double)win_width,
        +znear*tan(cam_FOV*M_PI / 360.0)*(double)win_height / (double)win_width, znear, zfar);
      /*       {
                 GLfloat m[16];
                 glGetFloatv(GL_PROJECTION_MATRIX,m);
                 printf("\nmatrix=\n %f %f %f %f\n %f %f %f %f\n %f %f %f %f\n %f %f %f %f\n",
                 m[0],m[4],m[8],m[12],
                 m[1],m[5],m[9],m[13],
                 m[2],m[6],m[10],m[14],
                 m[3],m[7],m[11],m[15]
                 );
                 }*/
      glMatrixMode(GL_MODELVIEW);
    }

    glMatrixMode(GL_MODELVIEW);

    /*   if       (i==0 && options_rgstereo_on){
           glTranslatef( -rg_eye_dist/2.0, 0.0, 0.0 );
           }else if (i==1 && options_rgstereo_on){
           glTranslatef( rg_eye_dist/2.0, 0.0, 0.0 );
           }else{
           }*/


    if (FREE_VIEW){
      glRotatef(Xrot + Xrot_offs, 1.0, 0.0, 0.0);
      glRotatef(Yrot + Yrot_offs, 0.0, 1.0, 0.0);
      glRotatef(Zrot + Zrot_offs, 0.0, 0.0, 1.0);
      glTranslatef(-free_view_pos.x, -free_view_pos.y, -free_view_pos.z);
      //       glTranslatef( 0.0, 0.0, -1.0 );
    }

    if (!FREE_VIEW){
      glTranslatef(0.0, 0.0, -real_dist);
    }

    glPushMatrix();

    if (!FREE_VIEW){
      glRotatef(Xrot + Xrot_offs, 1.0, 0.0, 0.0);
      glRotatef(Yrot + Yrot_offs, 0.0, 1.0, 0.0);
      glRotatef(Zrot + Zrot_offs, 0.0, 0.0, 1.0);
      /*   if(g_lookballnr<balls.nr){
             BallType * ball;
             #ifdef TIME_INTERPOLATE
             ball=BM_get_ball_by_nr( g_lookballnr, &g_drawballs );
             #else
             ball=BM_get_ball_by_nr( g_lookballnr, &balls );
             #endif
             glTranslatef( -ball->r.x, -ball->r.y, -ball->r.z );*/
      glTranslatef(-balls.ball[CUE_BALL_IND].r.x,
        -balls.ball[CUE_BALL_IND].r.y,
        -balls.ball[CUE_BALL_IND].r.z);
      /*   }else{
         }*/
    }


    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diff);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_amb);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diff);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_amb);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);

    if (i == 1 && options_rgstereo_on) {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    else if (i == 0 && options_rgstereo_on) {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    else {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

	// ==
	// Store matrices
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);
	// ==

    /* draw table */
    glCallList(table_obj);

    /*   {
           float col_text[4]={1.0,1.0,1.0,1.0};
           glDisable(GL_TEXTURE_2D);
           glPushMatrix();
           glRotatef(90.0,1,0,0);
           glMaterialfv(GL_FRONT, GL_DIFFUSE, col_text);
           glCallList(g_test_list);
           glPopMatrix();
           glEnable(GL_TEXTURE_2D);
           }*/


    /* draw balls with reflections and shadows */
#ifdef TIME_INTERPOLATE
    if(options_ball_reflections_blended){
      if( options_calc_ball_reflections ){
        draw_balls(g_drawballs,cam_pos,cam_FOV,win_width,reftexbind,lightpos,lightnr, (int *)0);
      }else if( options_cuberef ){
        draw_balls(g_drawballs,cam_pos,cam_FOV,win_width,spheretexbind,lightpos,lightnr, cuberef_allballs_texbind);
      }else{
        draw_balls(g_drawballs,cam_pos,cam_FOV,win_width,spheretexbind,lightpos,lightnr, (int *)0);
      }
    } else {
      draw_balls(g_drawballs,cam_pos,cam_FOV,win_width,lightspheretexbind,lightpos,lightnr, (int *)0);
    }
#else
    if (options_ball_reflections_blended){
      if (options_calc_ball_reflections){
        draw_balls(balls, cam_pos, cam_FOV, win_width, reftexbind, lightpos, lightnr, (int *)0);
      }
      else if (options_cuberef){
        draw_balls(balls, cam_pos, cam_FOV, win_width, spheretexbind, lightpos, lightnr, cuberef_allballs_texbind);
      }
      else{
        draw_balls(balls, cam_pos, cam_FOV, win_width, spheretexbind, lightpos, lightnr, (int *)0);
      }
    }
    else {
      draw_balls(balls, cam_pos, cam_FOV, win_width, lightspheretexbind, lightpos, lightnr, (int *)0);
    }
#endif

    if (!queue_view && !balls_moving){  /* draw queue */
      /*       double dx,dy,dz,th,ph;
             ph=Zque/180.0*M_PI;
             th=Xque/180.0*M_PI;
             dy=+1.0*sin(th)*cos(ph);
             dx=+1.0*sin(th)*sin(ph);
             dz=+1.0*cos(th);
             glBegin( GL_LINES );
             glVertex3f(balls.ball[0].r.x,balls.ball[0].r.y,balls.ball[0].r.z);
             glVertex3f(balls.ball[0].r.x+dx,balls.ball[0].r.y+dy,balls.ball[0].r.z+dz);
             glEnd();*/
      /*       glBegin( GL_LINES );
             glVertex3f(balls.ball[0].r.x,balls.ball[0].r.y,balls.ball[0].r.z);
             glVertex3f(balls.ball[0].r.x+comp_dir.x,balls.ball[0].r.y+comp_dir.y,balls.ball[0].r.z+comp_dir.z);
             glEnd();*/
		if (!touchmode)
      draw_queue(balls.ball[CUE_BALL_IND].r, Xque, Zque, queue_offs,
        queue_point_x, queue_point_y,
        spheretexbind, lightpos, lightnr);
    }

    if (options_place_cue_ball_tex && player[act_player].place_cue_ball && !balls_moving){
      int i;
      //        glMaterialfv(GL_FRONT, GL_DIFFUSE, col_shad);
      glDepthMask(GL_FALSE);
      glEnable(GL_BLEND);
      //        glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      //        glBlendFunc (GL_ONE, GL_SRC_ALPHA);
      //        glColor4f(0.5,0.5,0.5,1.0);
      //        glBlendFunc (GL_SRC_ALPHA, GL_ONE);
      glDisable(GL_LIGHTING);
      glBlendFunc(GL_ONE, GL_ONE);
      glColor3f(0.5, 0.5, 0.5);
      //        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      // glBlendFunc ( GL_ONE, GL_SRC_ALPHA );
      //        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBindTexture(GL_TEXTURE_2D, placecueballtexbind);
      //        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#define SH_SZ 0.087
      i = CUE_BALL_IND;
      glBegin(GL_QUADS);
      //            glBegin(GL_POLYGON);
      glNormal3f(0.0, 0.0, 1.0);
      glTexCoord2f(0.0, 1.0);
      glVertex3f(balls.ball[i].r.x - SH_SZ, balls.ball[i].r.y + SH_SZ, balls.ball[i].r.z - balls.ball[i].d / 2.02);
      glTexCoord2f(1.0, 1.0);
      glVertex3f(balls.ball[i].r.x + SH_SZ, balls.ball[i].r.y + SH_SZ, balls.ball[i].r.z - balls.ball[i].d / 2.02);
      glTexCoord2f(1.0, 0.0);
      glVertex3f(balls.ball[i].r.x + SH_SZ, balls.ball[i].r.y - SH_SZ, balls.ball[i].r.z - balls.ball[i].d / 2.02);
      glTexCoord2f(0.0, 0.0);
      glVertex3f(balls.ball[i].r.x - SH_SZ, balls.ball[i].r.y - SH_SZ, balls.ball[i].r.z - balls.ball[i].d / 2.02);
      glEnd();
      glDisable(GL_BLEND);
      glDepthMask(GL_TRUE);
#undef SH_SZ
      glEnable(GL_LIGHTING);
    }

    if (options_balltrace){
      int i;
      for (i = 0; i < balls.nr; i++){
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        draw_ballpath(&balls.ball[i]);
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
      }
    }


    if ((player[0].winner || player[1].winner)){
      if (options_3D_winnertext){
        draw_3D_winner_text();
      }
    }

    /*       if( vline_on && queue_view && !balls_moving ){
               glLineStipple( 1, 0xF0F0 );
               glEnable(GL_LINE_STIPPLE);

               glBegin( GL_LINES );
               glVertex3f(balls.ball[CUE_BALL_IND].r.x,balls.ball[CUE_BALL_IND].r.y,balls.ball[CUE_BALL_IND].r.z);
               glVertex3f(balls.ball[CUE_BALL_IND].r.x,balls.ball[CUE_BALL_IND].r.y,balls.ball[CUE_BALL_IND].r.z+1.0);
               glEnd();

               glDisable(GL_LINE_STIPPLE);
               }*/

    if (options_lensflare){
      VMvect dpos, dpos1, actpos, centpos, right, up;
      int i, j;

      glDepthMask(GL_FALSE);
      glEnable(GL_BLEND);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

      /*       glRasterPos3f(lightpos[0].x,lightpos[0].y,lightpos[0].z);
             fprintf(stderr,"lightpos =<%f,%f,%f>\n",lightpos[0].x,lightpos[0].y,lightpos[0].z);
             glGetFloatv(GL_CURRENT_RASTER_POSITION,pos);
             fprintf(stderr,"rasterpos=<%f,%f,%f>\n",pos[0],pos[1],pos[2]);*/

      for (i = 0; i < 1/*lightnr*/; i++){
        int k;
        //           glBlendFunc ( GL_ONE, GL_SRC_COLOR );
        glBlendFunc(GL_ONE, GL_ONE);

        glBindTexture(GL_TEXTURE_2D, blendetexbind);
        glGetFloatv(GL_MODELVIEW_MATRIX, mv_matr.m);
        glGetFloatv(GL_PROJECTION_MATRIX, prj_matr.m);
        //           actpos  = matr4_rdot( matr4_mul(prj_matr,mv_matr), lightpos[i] );
        //           centpos = vec_add(balls.ball[0].r,vec_scale(vec_diff(cam_pos,balls.ball[0].r),0.5));
        //           centpos = matr_rdot( matr_mul(prj_matr,mv_matr,4), centpos, 4 );

        dpos1 = matr4_rdot(mv_matr, vec_xyz(0, 0, 0.77));
        //           dpos1    = matr4_rdot( mv_matr, lightpos[i] );
        centpos = vec_xyz(0, 0, -0.5);
        dpos = vec_unit(vec_diff(dpos1, centpos));

        /*           glMatrixMode(GL_PROJECTION_MATRIX);
                   glPushMatrix();
                   glLoadIdentity();*/
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glDisable(GL_LIGHTING);
        //           glDisable(GL_TEXTURE_2D);
        //           glDisable(GL_DEPTH_TEST);

        for (k = 0; k < 3; k++)
        for (j = -1; j < 20; j++){
          VMfloat zact;

          if (!options_rgstereo_on){
            glColor3f(1.0*(double)(k % 3 != 1),
              1.0*(double)(k % 3 != 2),
              1.0*(double)(k % 3 != 0));
          }
          else {
            glColor3f(0.5 + 0.25*(double)(k % 3),
              0.5 + 0.25*(double)(k % 3),
              0.5 + 0.25*(double)(k % 3));
          }

          if (j == -1 && k == 0){
            glColor3f(1.0, 1.0, 1.0);
            glBindTexture(GL_TEXTURE_2D, lightflaretexbind);
            zact = dpos1.z;
            actpos = dpos1;
            right = vec_xyz(0.02 / 0.4*(0.5 - zact), 0, 0);
            up = vec_xyz(0, 0.02 / 0.4*(0.5 - zact), 0);
          }
          else if (j >= 0 && j < 10){
            glBindTexture(GL_TEXTURE_2D, blendetexbind);
            zact = 0.32 - 0.25*exp((j - 3) + k*1.4345);
            actpos = vec_add(centpos, vec_scale(dpos, zact / dpos.z));
            right = vec_xyz(0.008*(1.0 - k*0.23) / 0.4*(0.5 - zact), 0, 0);
            up = vec_xyz(0, 0.008*(1.0 - k*0.23) / 0.4*(0.5 - zact), 0);
          }
          else {
            glBindTexture(GL_TEXTURE_2D, blendetexbind);
            zact = 0.282 - 0.127*exp((j - 3 - 10) + k*1.2453);
            actpos = vec_add(centpos, vec_scale(dpos, zact / dpos.z));
            right = vec_xyz(0.003*(1.0 - k*0.23) / 0.4*(0.5 - zact), 0, 0);
            up = vec_xyz(0, 0.003*(1.0 - k*0.23) / 0.4*(0.5 - zact), 0);
          }

          glBegin(GL_QUADS);
          glTexCoord2f(0.0, 0.0);
          glVertex3f(actpos.x + up.x - right.x, actpos.y + up.y - right.y, actpos.z + up.z - right.z);
          glTexCoord2f(1.0, 0.0);
          glVertex3f(actpos.x + up.x + right.x, actpos.y + up.y + right.y, actpos.z + up.z + right.z);
          glTexCoord2f(1.0, 1.0);
          glVertex3f(actpos.x - up.x + right.x, actpos.y - up.y + right.y, actpos.z - up.z + right.z);
          glTexCoord2f(0.0, 1.0);
          glVertex3f(actpos.x - up.x - right.x, actpos.y - up.y - right.y, actpos.z - up.z - right.z);
          glEnd();
        }

        //           glEnable(GL_TEXTURE_2D);
        glPushMatrix();
        glTranslatef(0, 0, -0.5);
        glScalef(0.0005, 0.0005, 1.0);
        //           glCallList( create_string_quad( "Hallo-fagd",24 ) );
        glPopMatrix();

        glPopMatrix();
        /*           glMatrixMode(GL_PROJECTION);
                   glPopMatrix();*/
        glMatrixMode(GL_MODELVIEW);
      }
      glDisable(GL_BLEND);
      glDepthMask(GL_TRUE);
    }

    if (1){   /* HUD stuff */
      /*       char str[200];
             if (gametype==GAME_8BALL){
             sprintf(str,"%s - %s",player[act_player].name,half_full_names[player[act_player].half_full]);
             }else if (gametype==GAME_9BALL){
             int i;
             int minballnr=15;
             for(i=0;i<balls.nr;i++){
             if(balls.ball[i].nr<minballnr && balls.ball[i].nr!=0 && balls.ball[i].in_game)
             minballnr=balls.ball[i].nr;
             }
             sprintf(str,"%s next:%d",player[act_player].name,minballnr);
             }*/


      glDisable(GL_DEPTH_TEST);
      if (vline_on && queue_view && !balls_moving &&
        ((2 - options_rgaim) == i || options_rgaim == 0 || !options_rgstereo_on)){
        VMvect bx, by, bz;
        VMvect p, p1, p2;
        bz = vec_unit(vec_diff(cam_pos, balls.ball[CUE_BALL_IND].r));
        bx = vec_unit(vec_xyz(-bz.y, bz.x, 0));
        by = vec_cross(bz, bx);
        p = vec_add(vec_scale(bx, queue_point_x), vec_scale(by, -queue_point_y));
        p = vec_add(p, balls.ball[CUE_BALL_IND].r);
        glLineStipple(1, 0x3333);
        glEnable(GL_LINE_STIPPLE);

        glBegin(GL_LINES);
        p1 = vec_add(p, vec_scale(bx, -0.01));
        p2 = vec_add(p, vec_scale(bx, +0.01));
        glVertex3f(p.x, p.y, p.z);
        glVertex3f(p1.x, p1.y, p1.z);
        glVertex3f(p.x, p.y, p.z);
        glVertex3f(p2.x, p2.y, p2.z);
        p1 = vec_add(p, vec_scale(by, -0.01));
        p2 = vec_add(p, vec_scale(by, +0.01));
        glVertex3f(p.x, p.y, p.z);
        glVertex3f(p1.x, p1.y, p1.z);
        glVertex3f(p.x, p.y, p.z);
        glVertex3f(p2.x, p2.y, p2.z);
        glEnd();

        glDisable(GL_LINE_STIPPLE);
      }

      glMatrixMode(GL_TEXTURE);
      glPushMatrix();
      glLoadIdentity();

      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();

      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();

      //       glColor3f(0.5,1.0,0.7);
      //       glColor3f(1.0,1.0,1.0);
      glColor3f(1.0, 1.0, 1.0);
      glDisable(GL_LIGHTING);
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_TEXTURE_GEN_S);
      glDisable(GL_TEXTURE_GEN_T);

      glEnable(GL_TEXTURE_2D);
      glEnable(GL_BLEND);
      glBlendFunc(GL_ONE, GL_ONE);
      /* act player */
      glPushMatrix();
      glTranslatef(-0.94, -0.94, -1.0);
      glScalef(2.0 / win_width, 2.0 / win_height, 1.0);
      if (player[act_player].text != 0){
        textObj_draw(player[act_player].text);
      }
      glTranslatef(0, 30, 0);
      if (gametype == GAME_8BALL){
        switch (player[act_player].half_full){
        case BALL_HALF: glBindTexture(GL_TEXTURE_2D, halfsymboltexbind); break;
        case BALL_FULL: glBindTexture(GL_TEXTURE_2D, fullsymboltexbind); break;
        case BALL_ANY:  glBindTexture(GL_TEXTURE_2D, fullhalfsymboltexbind); break;
        }
        glBegin(GL_QUADS);
        glTexCoord2f(0, 1);
        glVertex3f(0, 0, 0);
        glTexCoord2f(0, 0);
        glVertex3f(0, 48, 0);
        glTexCoord2f(1, 0);
        glVertex3f(48, 48, 0);
        glTexCoord2f(1, 1);
        glVertex3f(48, 0, 0);
        glEnd();
      }
      else if (gametype == GAME_9BALL){
        int col;
        if (player[act_player].next_9ball != 8){
          col = options_col_ball[player[act_player].next_9ball];
        }
        else {
          col = 0x888888;
        }
        glColor3ub(col >> 16, (col >> 8) & 0xFF, col & 0xFF);
        textObj_draw(player[act_player].score_text);
      }
      else if (gametype == GAME_SNOOKER){
        textObj_draw(player[act_player].score_text);
      }
      else if (gametype == GAME_CARAMBOL){
        textObj_draw(player[act_player].score_text);
      }
      glPopMatrix();
      /* 2nd player */
      glPushMatrix();
      glColor3f(0.0, 0.0, 1.0);
      glTranslatef(0.94, -0.94, -1.0);
      glScalef(2.0 / win_width, 2.0 / win_height, 1.0);
      if (player[act_player].text != 0){
        textObj_draw_bound(player[act_player ? 0 : 1].text, HBOUND_RIGHT, VBOUND_BOTTOM);
      }
      glTranslatef(0, 30, 0);
      if (gametype == GAME_SNOOKER){
        textObj_draw_bound(player[act_player ? 0 : 1].score_text, HBOUND_RIGHT, VBOUND_BOTTOM);
      }
      else if (gametype == GAME_CARAMBOL){
        textObj_draw_bound(player[act_player ? 0 : 1].score_text, HBOUND_RIGHT, VBOUND_BOTTOM);
      }
      glPopMatrix();


      glDisable(GL_LIGHTING);
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_BLEND);

      glEnable(GL_BLEND);
      //       glBlendFunc(GL_ONE_MINUS_SRC_COLOR,GL_ONE);
      glBlendFunc(GL_ONE, GL_ONE);

      /* strength bar */
      if (!(options_gamemode == options_gamemode_tournament && tournament_state.wait_for_next_match))
        /* disable strength bar if tournament window active */
      {
        glColor3f(0.2, 0.2, 0.2);
        myRect2D(-0.5, -0.805, 0.5, -0.725);
        glColor3f(0.3, 0.3, 0.3);
        myRect2D(-0.5, -0.795, -0.5 + queue_strength, -0.735);
      }

      if (vline_on && queue_view && !balls_moving &&
        ((2 - options_rgaim) == i || options_rgaim == 0 || !options_rgstereo_on)){
        glColor3f(0.3, 0.3, 0.3);
        glLineStipple(1, 0xF0F0);
        glEnable(GL_LINE_STIPPLE);

        glBegin(GL_LINES);
        glVertex3f(0, 1.00, 0.5);
        glVertex3f(0, 0.08, 0.5);
        glEnd();

        glDisable(GL_LINE_STIPPLE);
      }


      glDisable(GL_BLEND);

      //       glRasterPos3f(-0.9,-0.9,-0.5);
      //       for(i=0;str[i]!=0;i++){
      //            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,str[i]);
      ////            glutBitmapCharacter(GLUT_BITMAP_8_BY_13,str[i]);
      //       }

#ifndef USE_SDL
      if (show_fps){
        char str[256];
        glColor3f(1.0, 1.0, 1.0);
        sprintf(str, "fps=%f", fps);
        glRasterPos3f(0.5, -0.9, -0.5);
        for (i = 0; str[i] != 0; i++){
          glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
        }
      }
#endif

#if 0
      if(player[0].winner || player[1].winner)
      {
        int winner=0;
        int width=0;
        if( player[0].winner ) winner=0; else winner=1;
        sprintf(str,"%s  wins",player[winner].name);
        for(i=0;str[i]!=0;i++){
          width+=glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24,str[i]);
        }
        glColor3f(1.0,0.1,0.1);
        glRasterPos3f(-(double)width/(double)win_width,0,-0.5);
        for(i=0;str[i]!=0;i++){
          glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,str[i]);
        }
      }
#endif

      if (options_gamemode == options_gamemode_tournament &&
        tournament_state.overall_winner >= 0)
      {
        printf("!!!!!!!!!!!! overall_winner=%d !!!!!!!!!!!!\n", tournament_state.overall_winner);
        //           textObj_draw(tournament_state.overal_winner_text);
      }

      if ((player[0].winner || player[1].winner) && g_act_menu == (menuType *)0){
        if (options_3D_winnertext){
        }
        else {
          glEnable(GL_TEXTURE_2D);
          glEnable(GL_BLEND);
          glBlendFunc(GL_ONE, GL_ONE);
          if (!options_rgstereo_on){
            glColor3f(1.0, 1.0, 0.0);
          }
          else {
            glColor3f(1.0, 1.0, 1.0);
          }
          glPushMatrix();
          glTranslatef(0, 0, -0.5);
          glScalef(2.0 / win_width, 2.0 / win_height, 1.0);
          glTranslatef(0, 30, -0.5);
          //           textObj_draw_centered( player[player[0].winner?0:1].text );
          textObj_setText(winner_name_text_obj, player[player[0].winner ? 0 : 1].name);
          textObj_draw_centered(winner_name_text_obj);
          glTranslatef(0, -60, 0.0);
          textObj_draw_centered(winner_text_obj);
          //           textObj_draw( winner_text_obj );
          glPopMatrix();
          glDisable(GL_BLEND);
        }
      }


      if (1){//helpscreen_on){
        glColor3f(0.7, 0.7, 0.7);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        glPushMatrix();
        if (helpscreen_on) draw_help_screen(win_width, win_height);

        glPopMatrix();
		// ===
		// Custom window based 2d drawing
		// modify projection
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glOrtho(0, win_width, win_height, 0, -1, 1);

		glMatrixMode(GL_MODELVIEW);

		glColor3f(0.7, 0.7, 0.7);
		glPushMatrix();
		glLoadIdentity();


		mm_draw_2d();

		glPopMatrix();


		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

        // ===

        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
      }

      if (options_gamemode == options_gamemode_tournament &&
        tournament_state.wait_for_next_match)
      {
        int i;
        struct TournamentState_ * ts;
        ts = &tournament_state;
        Display_tournament_tree(&tournament_state);
        //           printf("Pairings: \n");
        for (i = 0; i < (1 << (ts->round_num - ts->round_ind - 1)); i++){
          /*               printf("%s vs. %s\n",
                                ts->roster.player[ts->game[ts->round_ind][i].roster_player1].name,
                                ts->roster.player[ts->game[ts->round_ind][i].roster_player2].name
                                );*/
          /*               printf("%d vs. %d\n",
                                ts->game[ts->round_ind][i].roster_player1,
                                ts->game[ts->round_ind][i].roster_player2
                                );*/
        }
      }

      if (g_act_menu != (menuType *)0){
        //           glColor3f(0.7,0.7,0.7);
        glColor3f(1.0, 1.0, 1.0);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glDisable(GL_LIGHTING);
        glBlendFunc(GL_ONE, GL_ONE);
        glPushMatrix();
        glTranslatef(0.0, 0.0, -1.0);
        glScalef(2.0 / win_width, 2.0 / win_height, 1.0);
        menu_draw(g_act_menu);
        glPopMatrix();
        glDisable(GL_BLEND);
      }

      glEnable(GL_DEPTH_TEST);
      glEnable(GL_LIGHTING);
      glEnable(GL_TEXTURE_2D);
      glPopMatrix();

      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
    }

    glPopMatrix();
  } /* rg stereo */

  //   glutSwapBuffers();
}


void ResizeWindow(int width, int height)
{
  /*   if (width > WIDTH)
         width = WIDTH;
         if (height > HEIGHT)
         height = HEIGHT;*/

#ifndef _WIN32
  win_width=width;
  win_height=height;
  glViewport( 0, 0, width, height );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  //   glFrustum( -1.0, 1.0, -1.0, 1.0, 4.0, 1000.0 );

  //   gluPerspective(cam_FOV, /* field of view in degree */
  //     (GLfloat) width/(GLfloat) height, /* aspect ratio */
  //     0.1, /* Z near */
  //                  10.0); /* Z far */

  //   gluLookAt(0.0, 1.0, 380.0,  /* eye is at (0,8,60) */
  //     0.0, 0.0, 0.0,      /* center is at (0,8,0) */
  //     0.0, 1.0, 0.0);      /* up is in postivie Y direction */
#else
  /* [Win32] remember change so config file can be written to later */
  win_width_change = width;
  win_height_change = height;
#endif
}


static void SetMode(GLuint m)
{
  /* disable everything */
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);

  /* enable what's needed */
  if (m == LIT) {
    glEnable(GL_LIGHTING);
  }
  else if (m == TEXTURED) {
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
  }
  else if (m == REFLECT) {
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
  }
}

void restart_game_common()
{
  player[0].half_full = BALL_ANY;
  player[1].half_full = BALL_ANY;
  player[0].place_cue_ball = 0;
  player[1].place_cue_ball = 0;
  player[0].winner = 0;
  player[1].winner = 0;
  player[0].score = 0;
  player[1].score = 0;
  textObj_setText(player[0].score_text, "0");
  textObj_setText(player[1].score_text, "0");
  create_walls(&walls);
  create_scene(&balls);
  g_shot_due = 1;
}



void restart_game_tournment()
{
  restart_game_common();
  g_motion_ratio = 1.0;
  init_tournament_state(&tournament_state);
  act_player = 0;
  queue_view = player[act_player].queue_view;
}


void restart_game_training()
{
  restart_game_common();
  g_motion_ratio = 1.0;
  player_copy(&player[0], human_player_roster.player[0]);
  player_copy(&player[1], human_player_roster.player[0]);
  act_player = 0;
  queue_view = player[act_player].queue_view;
}


void restart_game_match()
{
  restart_game_common();
  g_motion_ratio = 1.0;
  player_copy(&player[0], human_player_roster.player[0]);
  player_copy(&player[1], human_player_roster.player[1]);
  act_player = 0;
  queue_view = player[act_player].queue_view;
}


void restart_game()
{
  switch (options_gamemode){
  case options_gamemode_tournament: restart_game_tournment(); break;
  case options_gamemode_training:   restart_game_training();  break;
  case options_gamemode_match:      restart_game_match();     break;
  }
}



void control_set(int * control_param)
{
  if (!control__active){
    *control_param = 1;
    control__updated = 1;
    control__active = 1;
  }
}

void control_unset(int * control_param)
{
  if (control__active){
    *control_param = 0;
    control__active = 0;
  }
}

void control_toggle(int * control_param)
{
  if (control__active){
    control_unset(control_param);
  }
  else{
    control_set(control_param);
  }
}


void Key(int key, int modifiers)
{
  float step = 3.0;

  if (g_act_menu != (menuType *)0){
    /* menu keys */
    switch (key) {
    case KSYM_PAGE_UP:
    case KSYM_UP:
      menu_select_prev(g_act_menu); break;
    case KSYM_PAGE_DOWN:
    case KSYM_DOWN:
      menu_select_next(g_act_menu); break;
    case 13:
      menu_choose(&g_act_menu); break;
    case 27:
      menu_exit(&g_act_menu); break;
    default:
      menu_text_keystroke(g_act_menu, key); break;
    }

  }
  else {

    /* general keys */
    switch (key) {
    case KSYM_PAGE_UP:
    case KSYM_UP:
      if (g_act_menu != (menuType *)0){
        menu_select_prev(g_act_menu);
      }
      else {
        if (!player[act_player].is_AI && !balls_moving)
          queue_strength = strength01(queue_strength + 0.01);
      }
      break;
    case KSYM_PAGE_DOWN:
    case KSYM_DOWN:
      if (g_act_menu != (menuType *)0){
        menu_select_next(g_act_menu);
      }
      else {
        if (!player[act_player].is_AI && !balls_moving)
          queue_strength = strength01(queue_strength - 0.01);
      }
      break;
    case KSYM_LEFT:
      Zrot += step;
      break;
    case KSYM_RIGHT:
      Zrot -= step;
      break;
    case KSYM_F1:
      if (g_act_menu == (menuType *)0){
        helpscreen_on = !helpscreen_on;
        if (helpscreen_on == 0) delete_help_screen();
      }
      break;
    case KSYM_F2:
      birdview();
      break;
    case 'o':
      printf("saving config\n");
      save_config();
      break;
    case 'x':
      scale *= 1.1;
      break;
    case 'y':
      scale /= 1.1;
      break;
    case 27:
      //         if( menu_on ){
      if (g_act_menu != (menuType *)0){
        fprintf(stderr, "menu backstep\n");
        menu_exit(&g_act_menu);
        if (g_act_menu == (menuType *)0){
          //                 g_act_menu = g_main_menu;
          menu_on = 0;
        }
      }
      else if (helpscreen_on){
        helpscreen_on = 0;
        delete_help_screen();
      }
      else {
        DPRINTF("menu on\n");
        menu_on = 1;
        g_act_menu = g_main_menu;
      }
      //         exit(0);
      break;
    case ' ':
    case 13:
      if (modifiers == 0){
        if (g_act_menu != (menuType *)0){
          menu_choose(&g_act_menu);
        }
        else {
          /* this has to be the same as middle mouse button !!! - maybe put it in a function some day  */
          if (options_gamemode == options_gamemode_tournament && tournament_state.wait_for_next_match) {
            tournament_state_setup_next_match(&tournament_state);
            tournament_state.wait_for_next_match = 0;
          }
          else if ((!player[act_player].is_net) && (!player[act_player].is_AI)){
            g_shot_due = 0;
            shoot(!queue_view);
          }
        }
      }
      if (modifiers & KEY_MODIFIER_ALT){
        sys_toggle_fullscreen(win_width, win_height);
      }
      break;
    case KSYM_KP_ENTER:
      if (modifiers & KEY_MODIFIER_ALT){
        sys_toggle_fullscreen(win_width, win_height);
      }
      break;
      /*      case w:
                if(modifiers & KEY_MODIFIER_ALT){
                sys_fullscreen( 0, win_width, win_height );
                }
                break;*/
    case '0':
      do_computer_move(0);
      break;
    case 'a':
      player[act_player].is_AI = !player[act_player].is_AI;
      break;
    case 'n':
      restart_game();
      break;
    case 'i':
      show_fps = (show_fps == 0) ? 1 : 0;
      break;
#if 1 // def USE_SDL  /* VRPool compatible key mappings */
      //      case 't': fprintf(stderr,"calling sys_remove_titlebar\n"); sys_remove_titlebar(); break;
    case 's': control_set(&control__mouse_shoot); break;
    case 'b': control_set(&control__cue_butt_updown); break;
    case 'e': control_set(&control__english); break;
    case 'm': control_set(&control__place_cue_ball); break;
#else
    case 's': control_toggle(&control__mouse_shoot); break;
    case 'b': control_toggle(&control__cue_butt_updown); break;
    case 'e': control_toggle(&control__english); break;
    case 'm': control_toggle(&control__place_cue_ball); break;
#endif
    case 'v':
      vline_on = (vline_on == 0) ? 1 : 0;
      break;
    case KSYM_F3:
    case 'c':
      toggle_queue_view();
      break;
    case KSYM_F4:
    case 'f':
      options_free_view_on = (options_free_view_on == 0) ? 1 : 0;
      break;
    case 'l':
      options_ball_fresnel_refl = (options_ball_fresnel_refl == 0) ? 1 : 0;
      break;
      /*      case 's':
          options_rgstereo_on = (options_rgstereo_on==0)?1:0;
          if(!options_rgstereo_on) glColorMask(1, 1, 1, 1);
          delete_queue_texbind();  create_texbinds(&balls);  create_queue_texbind();
          table_obj = create_table( spheretexbind, &walls, gametype==GAME_CARAMBOL );
          break;*/
    case 'r':
      options_ball_reflections_blended = !options_ball_reflections_blended;
      break;
    case 9:  /* TAB */
      if (options_gamemode == options_gamemode_training){
        int old_cue_ball = player[act_player].cue_ball;
        do{
          player[act_player].cue_ball++;
          if (player[act_player].cue_ball >= balls.nr)
            player[act_player].cue_ball = 0;
          if (player[act_player].cue_ball == old_cue_ball) break;
          DPRINTF("cue_ball=%d\n", player[act_player].cue_ball);
        } while (!balls.ball[player[act_player].cue_ball].in_game);
      }
      /*          {
                    BallType * ball;
                    int lookballnr_old;
                    lookballnr_old=g_lookballnr;
                    do{
                    g_lookballnr++;
                    ball=BM_get_ball_by_nr(g_lookballnr,&balls);
                    if( g_lookballnr > balls.nr ) g_lookballnr=0;
                    } while( !ball->in_game && g_lookballnr!=lookballnr_old );
                    if ( g_lookballnr==lookballnr_old ) g_lookballnr=balls.nr;
                    }
                    if( g_lookballnr !=0 ) queue_view=0; else queue_view=1;*/
      break;
    case'u':  /* undo */
      if (options_gamemode == options_gamemode_training){
        copy_balls(&bakballs, &balls);
      }
      break;
    }

  }  /* no menu active */
  //   glutPostRedisplay();
  sys_redisplay();
}


void KeyUp(int key, int modifiers)
{
  if (g_act_menu == (menuType *)0){
    switch (key) {
    case 's': control_unset(&control__mouse_shoot); break;
    case 'b': control_unset(&control__cue_butt_updown); break;
    case 'e': control_unset(&control__english); break;
    case 'm': control_unset(&control__place_cue_ball); break;
    }
  }
}


void host_network_game()
{
  int i;
  int dummy = 42;
  player[0].is_AI = 0;  /* FIXME maybe one can leave this away someday */
  player[1].is_AI = 0;  /* FIXME maybe one can leave this away someday */
  player[1].is_net = 1;    player[0].is_net = 0;
  g_is_host = 1;
  g_network_play = 1;

  g_socket = host_create_socket();
  fprintf(stderr, "host: writing test integer: %d\n", dummy);
  socket_write(g_socket, (char *)&dummy, sizeof(dummy));
  socket_read(g_socket, (char *)&dummy, sizeof(dummy));
  fprintf(stderr, "host: read test integer from client: %d\n", dummy);
  player[1].queue_view = 0;
  player[0].queue_view = 1;

  fprintf(stderr, "host: write gametype: %d\n", gametype);
  socket_write(g_socket, (char *)&gametype, sizeof(gametype));

  fprintf(stderr, "host: write table size: %f\n", options_table_size);
  socket_write(g_socket, (char *)&options_table_size, sizeof(options_table_size));

  fprintf(stderr, "host: write player1-name: %s\n", player[0].name);
  i = -1;
  do{
    i++;
    socket_write(g_socket, (char *)&(player[0].name[i]), sizeof(player[0].name[i]));
  } while (player[0].name[i] != 0);

  fprintf(stderr, "host: read player2-name %s\n", player[1].name);
  i = -1;
  do{
    i++;
    socket_write(g_socket, (char *)&(player[1].name[i]), sizeof(player[1].name[i]));
  } while (player[1].name[i] != 0);

  for (i = 0; i < balls.nr; i++){
    fprintf(stderr, "host: write ball#%02d\n", i);
    socket_write(g_socket, (char *)&(balls.ball[i]), sizeof(balls.ball[i]));
  }
}


void join_network_game()
{
  int i;
  int dummy = 42;
  player[0].is_AI = 0;  /* FIXME maybe one can leave this away someday */
  player[1].is_AI = 0;  /* FIXME maybe one can leave this away someday */
  player[1].is_net = 0;    player[0].is_net = 1;
  g_is_host = 0;
  g_network_play = 1;
  //    options_net_hostname = "10.0.0.1";

  g_socket = client_call_socket(options_net_hostname);
  socket_read(g_socket, (char *)&dummy, sizeof(dummy));
  fprintf(stderr, "client: read test integer from host: %d\n", dummy);
  dummy = 7;
  fprintf(stderr, "client: writing test integer: %d\n", dummy);
  socket_write(g_socket, (char *)&dummy, sizeof(dummy));
  player[0].queue_view = 0;
  player[1].queue_view = 1;
  queue_view = 0;

  socket_read(g_socket, (char *)&gametype, sizeof(gametype));
  fprintf(stderr, "client: read gametype: %d\n", gametype);
  set_gametype(gametype);

  socket_read(g_socket, (char *)&options_table_size, sizeof(options_table_size));
  fprintf(stderr, "client: read table size: %f\n", options_table_size);

  create_scene(&balls);  create_walls(&walls);
  table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);

  i = -1;
  do{
    i++;
    socket_read(g_socket, (char *)&(player[0].name[i]), sizeof(player[0].name[i]));
  } while (player[0].name[i] != 0);
  fprintf(stderr, "client: read player1-name: %s\n", player[0].name);
  textObj_setText(player[0].text, player[0].name);

  i = -1;
  do{
    i++;
    socket_read(g_socket, (char *)&(player[1].name[i]), sizeof(player[1].name[i]));
  } while (player[1].name[i] != 0);
  fprintf(stderr, "client: read player2-name: %s\n", player[1].name);
  textObj_setText(player[1].text, player[1].name);

  for (i = 0; i < balls.nr; i++){
    fprintf(stderr, "client: read ball#%02d\n", i);
    socket_read(g_socket, (char *)&(balls.ball[i]), sizeof(balls.ball[i]));
  }
  /*    create_scene( &balls );  create_walls( &walls );
      table_obj = create_table( spheretexbind, &walls, gametype==GAME_CARAMBOL );*/

}


void free_cuberef_tex()
{
  int i;
  for (i = 0; i < cuberef_allballs_texbind_nr; i++){
    glDeleteTextures(1, &cuberef_allballs_texbind[i]);
  }
  free(cuberef_allballs_texbind);
  cuberef_allballs_texbind = 0;
}


void reassign_and_gen_cuberef_tex()
{
  int i, j, k, l, layer, w, h, target;
  char * data;

  if (cuberef_allballs_texbind != 0 || balls.nr != cuberef_allballs_texbind_nr) free_cuberef_tex();

  //            glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, options_tex_min_filter);
  //            glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, options_tex_mag_filter);
  //            glBindTexture(GL_TEXTURE_2D, cube_tex_bind);

  cuberef_allballs_texbind_nr = balls.nr;

  cuberef_allballs_texbind = malloc(cuberef_allballs_texbind_nr*sizeof(*cuberef_allballs_texbind));

  for (i = 0; i < cuberef_allballs_texbind_nr; i++){
    glGenTextures(1, &cuberef_allballs_texbind[i]);
  }

  for (i = 0; i < 6; i++){
    /*                switch(i){
                    case 0: load_png(posxpng, &w, &h, &depth, &data); break;
                    case 1: load_png(posypng, &w, &h, &depth, &data); break;
                    case 2: load_png(poszpng, &w, &h, &depth, &data); break;
                    case 3: load_png(negxpng, &w, &h, &depth, &data); break;
                    case 4: load_png(negypng, &w, &h, &depth, &data); break;
                    case 5: load_png(negzpng, &w, &h, &depth, &data); break;
                    }*/
    switch (i){
    case 0: target = GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB; break;
    case 1: target = GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB; break;
    case 2: target = GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB; break;
    case 3: target = GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB; break;
    case 4: target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB; break;
    case 5: target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB; break;
    }
    w = cuberef_res; h = cuberef_res;
    DPRINTF(".... w=%d,h=%d\n", w, h);
    data = malloc(w*h * 3);
    for (j = 0; j < w*h; j++){
      if (((j%w) % 9) < 1 || ((j / w) % 9) < 1){
        data[j * 3 + 0] = 255 * (j%w) / w;
        data[j * 3 + 1] = 255 * (j / w) / w;
        data[j * 3 + 2] = 0;
      }
      else{
        data[j * 3 + 0] = ((i % 3) == 0 || i == 5) ? 0xFF : 0;
        data[j * 3 + 1] = ((i % 3) == 1 || i == 3) ? 0xFF : 0;
        data[j * 3 + 2] = ((i % 3) == 2 || i == 4) ? 0xFF : 0;
      }
    }
    //                gluBuild2DMipmaps(target, 3, w, h, GL_RGB, GL_UNSIGNED_BYTE, data);
    DPRINTF("5-%d\n", i);
    for (k = 0; k<cuberef_allballs_texbind_nr; k++){
      glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, cuberef_allballs_texbind[k]);
      for (l = cuberef_res, layer = 0; l>0; l >>= 1, layer++){
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        //                        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_REPEAT);
        //                        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_REPEAT);
        //                        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_REPEAT);
        //                        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //                        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexImage2D(target,
          layer,
          //                                     GL_RGB16, /*3,*/
          //                                     GL_R3_G3_B2, /*3,*/
          GL_RGB, /*3,*/
          l,  /* width */
          l,  /* height */
          0,
          GL_RGB,
          GL_UNSIGNED_BYTE,
          data);
      }
    }
    free(data);
  }
}


void menu_cb(int id, void * arg)
{
  switch (id){
  case MENU_ID_MAIN_QUIT:       save_config(); exit(0); break;
  case MENU_ID_OPTIONS_DISPLAY: fprintf(stderr, "menu_cb:options/display\n"); break;
  case MENU_ID_OPTIONS_SOUND:   fprintf(stderr, "menu_cb:options/sound\n"); break;
  case MENU_ID_OPTIONS_GAME:    fprintf(stderr, "menu_cb:options/game\n"); break;
  case MENU_ID_TABLESIZE_7FOOT:
    options_table_size = 7.0*2.54*12.0 / 100.0;
    create_scene(&balls);  create_walls(&walls);
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    break;
  case MENU_ID_TABLESIZE_8FOOT:
    options_table_size = 8.0*2.54*12.0 / 100.0;
    create_scene(&balls);  create_walls(&walls);
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    break;
  case MENU_ID_TABLESIZE_9FOOT:
    options_table_size = 9.0*2.54*12.0 / 100.0;
    create_scene(&balls);  create_walls(&walls);
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    break;
  case MENU_ID_TABLESIZE_12FOOT:
    options_table_size = 11.708*2.54*12.0 / 100.0;
    create_scene(&balls);  create_walls(&walls);
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    break;
  case MENU_ID_TABLETHEME_GOLDGREEN:
    options_table_color = options_table_color_green;
    options_diamond_color = options_diamond_color_gold;
    options_frame_tex_var = 1;
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    break;
  case MENU_ID_TABLETHEME_GOLDRED:
    options_table_color = options_table_color_red;
    options_diamond_color = options_diamond_color_gold;
    options_frame_tex_var = 1;
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    break;
  case MENU_ID_TABLETHEME_CHROMEBLUE:
    options_table_color = options_table_color_blue;
    options_diamond_color = options_diamond_color_chrome;
    options_frame_tex_var = 1;
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    break;
  case MENU_ID_TABLETHEME_BLACKBEIGE:
    options_table_color = options_table_color_beige;
    options_diamond_color = options_diamond_color_black;
    options_frame_tex_var = 1;
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    break;
  case MENU_ID_TABLETHEME_BLACKWHITE:
    options_table_color = options_table_color_black;
    options_diamond_color = options_diamond_color_black;
    options_frame_color = options_frame_color_white;
    options_frame_tex_var = 0;
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    break;
  case MENU_ID_HELPLINE_ON:
    vline_on = 1;
    break;
  case MENU_ID_HELPLINE_OFF:
    vline_on = 0;
    break;
  case MENU_ID_GAMETYPE_8BALL:
    set_gametype(GAME_8BALL);    restart_game();  /* create_scene( &balls );  create_walls( &walls ); alrerady in restart_game */
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    reassign_and_gen_cuberef_tex();
    break;
  case MENU_ID_GAMETYPE_9BALL:
    set_gametype(GAME_9BALL);    restart_game();  /* create_scene( &balls );  create_walls( &walls ); */
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    reassign_and_gen_cuberef_tex();
    break;
  case MENU_ID_GAMETYPE_CARAMBOL:
    set_gametype(GAME_CARAMBOL); restart_game();  /* create_scene( &balls );  create_walls( &walls ); */
    printf("player[0].cue_ball=%d\n", player[0].cue_ball);
    printf("player[1].cue_ball=%d\n", player[1].cue_ball);
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    reassign_and_gen_cuberef_tex();
    break;
  case MENU_ID_GAMETYPE_SNOOKER:
    set_gametype(GAME_SNOOKER);  restart_game();  /* create_scene( &balls );  create_walls( &walls ); */
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    reassign_and_gen_cuberef_tex();
    break;
  case MENU_ID_FULLSCREEN_ON:
    sys_fullscreen(1, win_width, win_height);
    break;
  case MENU_ID_FULLSCREEN_OFF:
    sys_fullscreen(0, win_width, win_height);
    break;
  case MENU_ID_RGSTEREO_ON:
    options_rgstereo_on = 1;
    delete_queue_texbind();  create_texbinds(&balls);  create_queue_texbind();
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    break;
  case MENU_ID_RGSTEREO_OFF:
    options_rgstereo_on = 0;
    glColorMask(1, 1, 1, 1);
    delete_queue_texbind();  create_texbinds(&balls);  create_queue_texbind();
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    break;
  case MENU_ID_RGAIM_LEFT:   options_rgaim = 1; break;
  case MENU_ID_RGAIM_RIGHT:  options_rgaim = 2; break;
  case MENU_ID_RGAIM_MIDDLE: options_rgaim = 0; break;

  case MENU_ID_LENSFLARE_ON:
    options_lensflare = 1;
    break;
  case MENU_ID_LENSFLARE_OFF:
    options_lensflare = 0;
    break;
  case MENU_ID_MAIN_RESTART:
    restart_game();
    break;
  case MENU_ID_PLAYER1_NAME:
    strcpy(player[0].name, (char *)arg);
    textObj_setText(player[0].text, player[0].name);
    player_copy(&human_player_roster.player[0], player[0]);
    printf("callback:MENU_ID_PLAYER1_NAME\n");
    break;
  case MENU_ID_PLAYER2_NAME:
    strcpy(player[1].name, (char *)arg);
    textObj_setText(player[1].text, player[1].name);
    player_copy(&human_player_roster.player[1], player[1]);
    printf("callback:MENU_ID_PLAYER2_NAME\n");
    break;
  case MENU_ID_PLAYER1_SKILL_EXCEL:  player[0].err = 0.0; player_copy(&human_player_roster.player[0], player[0]); break;
  case MENU_ID_PLAYER1_SKILL_GOOD:   player[0].err = 0.1; player_copy(&human_player_roster.player[0], player[0]); break;
  case MENU_ID_PLAYER1_SKILL_MEDIUM: player[0].err = 0.3; player_copy(&human_player_roster.player[0], player[0]); break;
  case MENU_ID_PLAYER1_SKILL_BAD:    player[0].err = 0.6; player_copy(&human_player_roster.player[0], player[0]); break;
  case MENU_ID_PLAYER1_SKILL_WORSE:  player[0].err = 1.0; player_copy(&human_player_roster.player[0], player[0]); break;

  case MENU_ID_PLAYER2_SKILL_EXCEL:  player[1].err = 0.0; player_copy(&human_player_roster.player[1], player[1]); break;
  case MENU_ID_PLAYER2_SKILL_GOOD:   player[1].err = 0.1; player_copy(&human_player_roster.player[1], player[1]); break;
  case MENU_ID_PLAYER2_SKILL_MEDIUM: player[1].err = 0.3; player_copy(&human_player_roster.player[1], player[1]); break;
  case MENU_ID_PLAYER2_SKILL_BAD:    player[1].err = 0.6; player_copy(&human_player_roster.player[1], player[1]); break;
  case MENU_ID_PLAYER2_SKILL_WORSE:  player[1].err = 1.0; player_copy(&human_player_roster.player[1], player[1]); break;

  case MENU_ID_PLAYER1_TYPE_HUMAN:  player[0].is_AI = 0; player_copy(&human_player_roster.player[0], player[0]); break;
  case MENU_ID_PLAYER2_TYPE_HUMAN:  player[1].is_AI = 0; player_copy(&human_player_roster.player[1], player[1]); break;

  case MENU_ID_PLAYER1_TYPE_AI:
    if (act_player == 0){
      player[0].is_AI = 1;
      player[0].queue_view = 0;
      if (queue_view) toggle_queue_view();
      do_computer_move(1);
    }
    else {
      player[0].is_AI = 1;
      player[0].queue_view = 0;
    }
    player_copy(&human_player_roster.player[0], player[0]);
    break;
  case MENU_ID_PLAYER2_TYPE_AI:
    if (act_player == 1){
      player[1].is_AI = 1;
      player[1].queue_view = 0;
      if (queue_view) toggle_queue_view();
      do_computer_move(1);
    }
    else {
      player[1].is_AI = 1;
      player[1].queue_view = 0;
    }
    player_copy(&human_player_roster.player[1], player[1]);
    break;

  case MENU_ID_BALL_DETAIL_LOW:
    options_max_ball_detail = options_max_ball_detail_LOW;
    options_ball_detail_nearmax = options_ball_detail_nearmax_LOW;
    break;
  case MENU_ID_BALL_DETAIL_MED:
    options_max_ball_detail = options_max_ball_detail_MED;
    options_ball_detail_nearmax = options_ball_detail_nearmax_MED;
    break;
  case MENU_ID_BALL_DETAIL_HIGH:
    options_max_ball_detail = options_max_ball_detail_HIGH;
    options_ball_detail_nearmax = options_ball_detail_nearmax_HIGH;
    break;
  case MENU_ID_BALL_DETAIL_VERYHIGH:
    options_max_ball_detail = options_max_ball_detail_VERYHIGH;
    options_ball_detail_nearmax = options_ball_detail_nearmax_VERYHIGH;
    break;
  case MENU_ID_VIDMODE:
  {
                        sysResolution * mode;
                        mode = (sysResolution *)arg;
                        sys_resize(mode->w, mode->h);
  }
    break;
  case MENU_ID_MAIN_HELP:
    helpscreen_on = !helpscreen_on;
    if (helpscreen_on == 0) delete_help_screen();
    break;
  case MENU_ID_NETWORK_HOST:
    host_network_game();
    break;
  case MENU_ID_NETWORK_JOIN:
    join_network_game();
    break;
  case MENU_ID_NETWORK_IP:
    strcpy(options_net_hostname, (char *)arg);
    break;
  case MENU_ID_NETWORK_PORTNUM:
    sscanf((char *)arg, "%d", &options_net_portnum);
    break;
  case MENU_ID_REFLECTION_SPHERE:
    options_cuberef = 0;
    free_cuberef_tex();
    break;
  case MENU_ID_REFLECTION_RENDERED:
    options_cuberef = 1;
    options_ball_fresnel_refl = 0;
    reassign_and_gen_cuberef_tex();
    break;
  case MENU_ID_REFLECTION_RENDERED_FRESNEL:
    options_cuberef = 1;
    options_ball_fresnel_refl = 1;
    reassign_and_gen_cuberef_tex();
    break;
  case MENU_ID_BALLTRACE_ON:
    options_balltrace = 1;
    break;
  case MENU_ID_BALLTRACE_OFF:
    options_balltrace = 0;
    break;
  case MENU_ID_BUMPREF_ON:
    options_bumpref = 1;
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    break;
  case MENU_ID_BUMPREF_OFF:
    options_bumpref = 0;
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    break;
  case MENU_ID_BUMPWOOD_ON:
    options_bumpwood = 1;
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    break;
  case MENU_ID_BUMPWOOD_OFF:
    options_bumpwood = 0;
    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);
    break;
  case MENU_ID_GAMEMODE_MATCH:      options_gamemode = options_gamemode_match;      break;
  case MENU_ID_GAMEMODE_TRAINING:   options_gamemode = options_gamemode_training;   break;
  case MENU_ID_GAMEMODE_TOURNAMENT: options_gamemode = options_gamemode_tournament; init_tournament_state(&tournament_state); break;
  case MENU_ID_SINGLE_MATCH_START:
    options_gamemode = options_gamemode_match;
    restart_game();
    break;
  case MENU_ID_TOURNAMENT_START:
    options_gamemode = options_gamemode_tournament;
    init_tournament_state(&tournament_state);
    restart_game();
    break;
  case MENU_ID_TRAINING_START:
    options_gamemode = options_gamemode_training;
    restart_game();
    break;
  case MENU_ID_IDLE:
    break;
  }
}


void init_menu()
{
  char str[256];

  menuType * player1_skill_menu;
  menuType * player2_skill_menu;
  menuType * player1_type_menu;
  menuType * player2_type_menu;
  menuType * player1_menu;
  menuType * player2_menu;
  menuType * quit_menu;
  menuType * gamemode_menu;
  menuType * restart_menu;
  menuType * network_menu;
  menuType * network_join_menu;
  menuType * display_menu;
  menuType * game_menu;
  menuType * tablesize_menu;
  menuType * tabletheme_menu;
  menuType * helpline_menu;
  menuType * gametype_menu;
  menuType * fullscreen_menu;
  menuType * rgaim_menu;
  menuType * rgenable_menu;
  menuType * rgstereo_menu;
  menuType * lensflare_menu;
  menuType * balldetail_menu;
  menuType * videomode_menu;
  menuType ** videomode_menus;
  menuType * reflection_menu;
  menuType * balltrace_menu;
  menuType * bumpref_menu;
  menuType * bumpwood_menu;
#define OTHER_RESTART_MENU
#ifdef OTHER_RESTART_MENU
  menuType * network_host_menu;
  menuType * single_match_menu;
  menuType * tournament_menu;
  menuType * training_menu;
#endif


  {
    int i, j;
    char str[256];
    int entr_num = 10;
    int mode_num = 0;
    int menu_num = 0;
    sysResolution * modes;

    /* several pages for the modes: this should fix the problems with many dsplay modes */
    /* however the selected resolution displayed in the parent menu will be wrong
       when selected from other the 1st page */
    modes = sys_list_modes();
    for (mode_num = 0; modes[mode_num].w != 0; mode_num++);
    DPRINTF("mode_num=%d\n", mode_num);
    menu_num = (mode_num + entr_num - 1) / entr_num;
    if (menu_num < 1) menu_num = 1;

    DPRINTF("menu_num=%d\n", menu_num);
    videomode_menus = (menuType **)malloc(menu_num*sizeof(menuType *));

    for (i = 0; i < menu_num; i++){
      videomode_menus[i] = menu_new(menu_cb);
    }
    videomode_menu = videomode_menus[0];

    if (mode_num == 0){
      menu_add_entry(videomode_menus[0], "<not available>", MENU_ID_IDLE);
    }

    i = 0;
    for (j = 0; j < menu_num && i < mode_num; j++){
      if (mode_num != 0) do{
        sprintf(str, "%dx%d", modes[i].w, modes[i].h);
        DPRINTF("%dx%d\n", modes[i].w, modes[i].h);
        menu_add_arg_entry(videomode_menus[j], str, MENU_ID_VIDMODE, (void *)&modes[i]);
        i++;
      } while ((i%entr_num) != 0 && i < mode_num);
      if (i < mode_num){
        menu_add_submenu(videomode_menus[j], "more >", videomode_menus[j + 1], 0);
      }
      menu_add_exit(videomode_menus[j], "< back");
    }

  }

  bumpref_menu = menu_new(menu_cb);
  menu_add_entry(bumpref_menu, "on", MENU_ID_BUMPREF_ON);
  menu_add_entry(bumpref_menu, "off", MENU_ID_BUMPREF_OFF);
  menu_add_exit(bumpref_menu, "< back");

  bumpwood_menu = menu_new(menu_cb);
  menu_add_entry(bumpwood_menu, "on", MENU_ID_BUMPWOOD_ON);
  menu_add_entry(bumpwood_menu, "off", MENU_ID_BUMPWOOD_OFF);
  menu_add_exit(bumpwood_menu, "< back");

  balltrace_menu = menu_new(menu_cb);
  menu_add_entry(balltrace_menu, "on", MENU_ID_BALLTRACE_ON);
  menu_add_entry(balltrace_menu, "off", MENU_ID_BALLTRACE_OFF);
  menu_add_exit(balltrace_menu, "< back");

  reflection_menu = menu_new(menu_cb);
  menu_add_entry(reflection_menu, "spheremap", MENU_ID_REFLECTION_SPHERE);
  menu_add_entry(reflection_menu, "rendered", MENU_ID_REFLECTION_RENDERED);
  menu_add_entry(reflection_menu, "rendered+fresnel", MENU_ID_REFLECTION_RENDERED_FRESNEL);
  menu_add_exit(reflection_menu, "< back");

  balldetail_menu = menu_new(menu_cb);
  menu_add_entry(balldetail_menu, "low", MENU_ID_BALL_DETAIL_LOW);
  menu_add_entry(balldetail_menu, "medium", MENU_ID_BALL_DETAIL_MED);
  menu_add_entry(balldetail_menu, "high", MENU_ID_BALL_DETAIL_HIGH);
  menu_add_entry(balldetail_menu, "very high", MENU_ID_BALL_DETAIL_VERYHIGH);
  menu_add_exit(balldetail_menu, "< back");

  rgaim_menu = menu_new(menu_cb);
  menu_add_entry(rgaim_menu, "middle", MENU_ID_RGAIM_MIDDLE);
  menu_add_entry(rgaim_menu, "left", MENU_ID_RGAIM_LEFT);
  menu_add_entry(rgaim_menu, "right", MENU_ID_RGAIM_RIGHT);
  menu_add_exit(rgaim_menu, "< back");

  rgenable_menu = menu_new(menu_cb);
  menu_add_entry(rgenable_menu, "rg on", MENU_ID_RGSTEREO_ON);
  menu_add_entry(rgenable_menu, "rg off", MENU_ID_RGSTEREO_OFF);
  menu_add_exit(rgenable_menu, "< back");

  rgstereo_menu = menu_new(menu_cb);
  menu_add_submenu(rgstereo_menu, "enable", rgenable_menu, 1);
  menu_add_submenu(rgstereo_menu, "aim eye", rgaim_menu, 1);
  menu_add_exit(rgstereo_menu, "< back");

  lensflare_menu = menu_new(menu_cb);
  menu_add_entry(lensflare_menu, "lensflare on", MENU_ID_LENSFLARE_ON);
  menu_add_entry(lensflare_menu, "lensflare off", MENU_ID_LENSFLARE_OFF);
  menu_add_exit(lensflare_menu, "< back");

  fullscreen_menu = menu_new(menu_cb);
  menu_add_entry(fullscreen_menu, "fullscreen", MENU_ID_FULLSCREEN_ON);
  menu_add_entry(fullscreen_menu, "window", MENU_ID_FULLSCREEN_OFF);
  menu_add_exit(fullscreen_menu, "< back");

  tablesize_menu = menu_new(menu_cb);
  menu_add_entry(tablesize_menu, "7 foot", MENU_ID_TABLESIZE_7FOOT);
  menu_add_entry(tablesize_menu, "8 foot", MENU_ID_TABLESIZE_8FOOT);
  menu_add_entry(tablesize_menu, "9 foot", MENU_ID_TABLESIZE_9FOOT);
  menu_add_entry(tablesize_menu, "12 foot", MENU_ID_TABLESIZE_12FOOT);
  menu_add_exit(tablesize_menu, "< back");

  tabletheme_menu = menu_new(menu_cb);
  menu_add_entry(tabletheme_menu, "gold-green", MENU_ID_TABLETHEME_GOLDGREEN);
  menu_add_entry(tabletheme_menu, "gold-red", MENU_ID_TABLETHEME_GOLDRED);
  menu_add_entry(tabletheme_menu, "chrome-blue", MENU_ID_TABLETHEME_CHROMEBLUE);
  menu_add_entry(tabletheme_menu, "black-white", MENU_ID_TABLETHEME_BLACKWHITE);
  menu_add_entry(tabletheme_menu, "black-beige", MENU_ID_TABLETHEME_BLACKBEIGE);
  menu_add_exit(tabletheme_menu, "< back");

  helpline_menu = menu_new(menu_cb);
  menu_add_entry(helpline_menu, "on", MENU_ID_HELPLINE_ON);
  menu_add_entry(helpline_menu, "off", MENU_ID_HELPLINE_OFF);
  menu_add_exit(helpline_menu, "< back");

  gametype_menu = menu_new(menu_cb);
  menu_add_entry(gametype_menu, "8 ball", MENU_ID_GAMETYPE_8BALL);
  menu_add_entry(gametype_menu, "9 ball", MENU_ID_GAMETYPE_9BALL);
  menu_add_entry(gametype_menu, "carambol", MENU_ID_GAMETYPE_CARAMBOL);
  menu_add_entry(gametype_menu, "snooker", MENU_ID_GAMETYPE_SNOOKER);
  menu_add_exit(gametype_menu, "< back");

  game_menu = menu_new(menu_cb);
  menu_add_submenu(game_menu, "table size", tablesize_menu, 1);
  menu_add_submenu(game_menu, "help line", helpline_menu, 1);
  menu_add_submenu(game_menu, "game type", gametype_menu, 1);
  menu_add_exit(game_menu, "< back");

  display_menu = menu_new(menu_cb);
  menu_add_submenu(display_menu, "resolution", videomode_menu, 1);
  menu_add_submenu(display_menu, "view mode", fullscreen_menu, 1);
  menu_add_submenu(display_menu, "red/green stereo", rgstereo_menu, 1);
  menu_add_submenu(display_menu, "lensflare", lensflare_menu, 1);
  menu_add_submenu(display_menu, "ball detail", balldetail_menu, 1);
  menu_add_submenu(display_menu, "reflections", reflection_menu, 1);
  menu_add_submenu(display_menu, "bump reflections", bumpref_menu, 1);
  menu_add_submenu(display_menu, "bumpy wood frame", bumpwood_menu, 1);
  menu_add_submenu(display_menu, "table theme", tabletheme_menu, 1);
  menu_add_submenu(display_menu, "ball traces", balltrace_menu, 1);
  menu_add_exit(display_menu, "< back");

  quit_menu = menu_new(menu_cb);
  menu_add_entry(quit_menu, "YES  out'a here", MENU_ID_MAIN_QUIT);
  menu_add_exit(quit_menu, "NO  continue");


  player1_skill_menu = menu_new(menu_cb);
  menu_add_entry(player1_skill_menu, "excellent", MENU_ID_PLAYER1_SKILL_EXCEL);
  menu_add_entry(player1_skill_menu, "good", MENU_ID_PLAYER1_SKILL_GOOD);
  menu_add_entry(player1_skill_menu, "medium", MENU_ID_PLAYER1_SKILL_MEDIUM);
  menu_add_entry(player1_skill_menu, "bad", MENU_ID_PLAYER1_SKILL_BAD);
  menu_add_entry(player1_skill_menu, "worse", MENU_ID_PLAYER1_SKILL_WORSE);
  menu_add_exit(player1_skill_menu, "< back");

  player2_skill_menu = menu_new(menu_cb);
  menu_add_entry(player2_skill_menu, "excellent", MENU_ID_PLAYER2_SKILL_EXCEL);
  menu_add_entry(player2_skill_menu, "good", MENU_ID_PLAYER2_SKILL_GOOD);
  menu_add_entry(player2_skill_menu, "medium", MENU_ID_PLAYER2_SKILL_MEDIUM);
  menu_add_entry(player2_skill_menu, "bad", MENU_ID_PLAYER2_SKILL_BAD);
  menu_add_entry(player2_skill_menu, "worse", MENU_ID_PLAYER2_SKILL_WORSE);
  menu_add_exit(player2_skill_menu, "< back");

  player1_type_menu = menu_new(menu_cb);
  menu_add_entry(player1_type_menu, "AI", MENU_ID_PLAYER1_TYPE_AI);
  menu_add_entry(player1_type_menu, "Human", MENU_ID_PLAYER1_TYPE_HUMAN);
  menu_add_exit(player1_type_menu, "< back");

  player2_type_menu = menu_new(menu_cb);
  menu_add_entry(player2_type_menu, "AI", MENU_ID_PLAYER2_TYPE_AI);
  menu_add_entry(player2_type_menu, "Human", MENU_ID_PLAYER2_TYPE_HUMAN);
  menu_add_exit(player2_type_menu, "< back");

  player1_menu = menu_new(menu_cb);
  sprintf(str, "P1 Name: %s", player[0].name);
  menu_add_textfield(player1_menu, str, MENU_ID_PLAYER1_NAME, strlen("P1 Name: "));
  menu_add_submenu(player1_menu, "P1 Type", player1_type_menu, 1);
  menu_add_submenu(player1_menu, "P1 Skill", player1_skill_menu, 1);
  menu_add_exit(player1_menu, "< back");

  player2_menu = menu_new(menu_cb);
  sprintf(str, "P2 Name: %s", player[1].name);
  menu_add_textfield(player2_menu, str, MENU_ID_PLAYER2_NAME, strlen("P2 Name: "));
  menu_add_submenu(player2_menu, "P2 Type", player2_type_menu, 1);
  menu_add_submenu(player2_menu, "P2 Skill", player2_skill_menu, 1);
  menu_add_exit(player2_menu, "< back");

  gamemode_menu = menu_new(menu_cb);
  menu_add_entry(gamemode_menu, "single match", MENU_ID_GAMEMODE_MATCH);
  menu_add_entry(gamemode_menu, "tournament", MENU_ID_GAMEMODE_TOURNAMENT);
  menu_add_entry(gamemode_menu, "training", MENU_ID_GAMEMODE_TRAINING);
  menu_add_exit(gamemode_menu, "< back");

#ifdef OTHER_RESTART_MENU
  network_host_menu = menu_new(menu_cb);
  sprintf(str, "P1 Name: %s", player[0].name);
  menu_add_textfield(network_host_menu, str, MENU_ID_PLAYER1_NAME, strlen("P1 Name: "));
  sprintf(str, "P2 Name: %s", player[1].name);
  menu_add_textfield(network_host_menu, str, MENU_ID_PLAYER2_NAME, strlen("P2 Name: "));
  sprintf(str, "port: %d", options_net_portnum);
  menu_add_textfield(network_host_menu, str, MENU_ID_NETWORK_PORTNUM, strlen("port: "));
  menu_add_entry(network_host_menu, "Start Game", MENU_ID_NETWORK_HOST);
  menu_add_exit(network_host_menu, "< back");
#endif

  network_join_menu = menu_new(menu_cb);
#ifdef OTHER_RESTART_MENU
  sprintf(str, "port: %d", options_net_portnum);
  menu_add_textfield(network_join_menu, str, MENU_ID_NETWORK_PORTNUM, strlen("port: "));
#endif
  sprintf(str, "IP: %s", options_net_hostname);
  menu_add_textfield(network_join_menu, str, MENU_ID_NETWORK_IP, strlen("IP: "));
  menu_add_entry(network_join_menu, "Start Game", MENU_ID_NETWORK_JOIN);
  menu_add_exit(network_join_menu, "< back");

  network_menu = menu_new(menu_cb);
#ifndef OTHER_RESTART_MENU
  sprintf(str,"port: %d",options_net_portnum);
  menu_add_textfield( network_menu, str,        MENU_ID_NETWORK_PORTNUM, strlen("port: ") );
  menu_add_entry  ( network_menu, "As Host",    MENU_ID_NETWORK_HOST );
  menu_add_submenu( network_menu, "Join",       network_join_menu, 0 );
  menu_add_exit   ( network_menu, "< back" );
#else
  menu_add_submenu(network_menu, "As Host", network_host_menu, 0);
  menu_add_submenu(network_menu, "Join", network_join_menu, 0);
  menu_add_exit(network_menu, "< back");
#endif

#ifdef OTHER_RESTART_MENU
  single_match_menu = menu_new(menu_cb);
  menu_add_submenu(single_match_menu, "Player1", player1_menu, 0);
  menu_add_submenu(single_match_menu, "Player2", player2_menu, 0);
  menu_add_entry(single_match_menu, "Start Match", MENU_ID_SINGLE_MATCH_START);
  menu_add_exit(single_match_menu, "< back");

  tournament_menu = menu_new(menu_cb);
  //    menu_add_entry  ( tournament_menu,   "AI-AI fast motion", MENU_ID_TOURNAMENT_AIAI_FMOT );
  menu_add_entry(tournament_menu, "Start Tournament", MENU_ID_TOURNAMENT_START);
  menu_add_exit(tournament_menu, "< back");

  training_menu = menu_new(menu_cb);
  menu_add_entry(training_menu, "Start Training", MENU_ID_TRAINING_START);
  menu_add_exit(training_menu, "< back");
#endif

  restart_menu = menu_new(menu_cb);
#ifndef OTHER_RESTART_MENU
  menu_add_submenu( restart_menu, "Player1",      player1_menu,   0 );
  menu_add_submenu( restart_menu, "Player2",      player2_menu,   0 );
  menu_add_submenu( restart_menu, "Mode",         gamemode_menu,  1 );
  menu_add_entry  ( restart_menu, "restart",      MENU_ID_MAIN_RESTART );
  menu_add_exit   ( restart_menu, "< back");
#else
  menu_add_submenu(restart_menu, "single match", single_match_menu, 0);
  menu_add_submenu(restart_menu, "tournament", tournament_menu, 0);
  menu_add_submenu(restart_menu, "training", training_menu, 0);
  menu_add_submenu(restart_menu, "network game", network_menu, 0);
  menu_add_exit(restart_menu, "< back");
#endif

  g_options_menu = menu_new(menu_cb);
  menu_add_submenu(g_options_menu, "display", display_menu, 0);
  menu_add_entry(g_options_menu, "sound", MENU_ID_OPTIONS_SOUND);
  menu_add_submenu(g_options_menu, "game", game_menu, 0);
  menu_add_exit(g_options_menu, "< back");

  g_main_menu = menu_new(menu_cb);
  menu_add_exit(g_main_menu, "Resume");
  menu_add_submenu(g_main_menu, "Restart Game", restart_menu, 0);
#ifndef OTHER_RESTART_MENU
  menu_add_submenu( g_main_menu, "Network Game", network_menu, 0 );
#endif
  menu_add_submenu(g_main_menu, "Options", g_options_menu, 0);
  menu_add_entry(g_main_menu, "Help", MENU_ID_MAIN_HELP);
  menu_add_submenu(g_main_menu, "Quit", quit_menu, 0);


  g_act_menu = g_options_menu;
  g_act_menu = (menuType *)0;
}


int str_contains(char *s1, char *s2)
{
  int i, j;
  int rval = 0;

  for (i = 0; s1[i] != 0; i++){
    for (j = 0; s2[j] != 0 && s1[i + j] != 0 && s2[j] == s1[i + j]; j++){
    }
    if (s2[j] == 0){
      rval = 1; break;
    }
  }
  return rval;
}

void parse_gl_extensions_string(void)
{
  char * str;
  str = (char *)glGetString(GL_EXTENSIONS);

  extension_cubemap = (str_contains(str, "GL_ARB_texture_cube_map")) ? 1 : 0;
  DPRINTF("extension_cubemap=%d\n", extension_cubemap);

  extension_multitexture = (str_contains(str, "GL_ARB_multitexture")) ? 1 : 0;
  DPRINTF("extension_multitexture=%d\n", extension_multitexture);

#ifdef _WIN32
  // load function addresses (Win32 only) and verify it exists
  if (extension_multitexture) {
    glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)
      wglGetProcAddress("glActiveTextureARB");
    if (glActiveTextureARB == NULL) extension_multitexture = 0;
  }
#endif

  extension_ts_NV = (str_contains(str, "GL_NV_texture_shader")) ? 1 : 0;
  DPRINTF("extension_ts_NV=%d\n", extension_ts_NV);

  extension_rc_NV = (str_contains(str, "GL_NV_register_combiners")) ? 1 : 0;
  DPRINTF("extension_rc_NV=%d\n", extension_rc_NV);

#ifdef _WIN32
  // load function addresses (Win32 only) and verify it exists
  if (extension_rc_NV) {
    glCombinerParameteriNV = (PFNGLCOMBINERPARAMETERINVPROC)
      wglGetProcAddress("glCombinerParameteriNV");
    glCombinerOutputNV = (PFNGLCOMBINEROUTPUTNVPROC)
      wglGetProcAddress("glCombinerOutputNV");
    glCombinerInputNV = (PFNGLCOMBINERINPUTNVPROC)
      wglGetProcAddress("glCombinerInputNV");
    glFinalCombinerInputNV = (PFNGLFINALCOMBINERINPUTNVPROC)
      wglGetProcAddress("glFinalCombinerInputNV");
    glCombinerParameterfvNV = (PFNGLCOMBINERPARAMETERFVNVPROC)
      wglGetProcAddress("glCombinerParameterfvNV");
    if (glCombinerParameteriNV == NULL
      || glCombinerOutputNV == NULL
      || glCombinerInputNV == NULL
      || glFinalCombinerInputNV == NULL
      || glCombinerParameterfvNV == NULL)
      extension_rc_NV = 0;
  }
#endif

  extension_vp_NV = (str_contains(str, "GL_NV_vertex_program")) ? 1 : 0;
  DPRINTF("extension_vp_NV=%d\n", extension_vp_NV);

#ifdef _WIN32
  // load function addresses (Win32 only) and verify it exists
  if (extension_vp_NV) {
    glGenProgramsNV = (PFNGLGENPROGRAMSNVPROC)
      wglGetProcAddress("glGenProgramsNV");
    glBindProgramNV = (PFNGLBINDPROGRAMNVPROC)
      wglGetProcAddress("glBindProgramNV");
    glLoadProgramNV = (PFNGLLOADPROGRAMNVPROC)
      wglGetProcAddress("glLoadProgramNV");
    glProgramParameter4fNV = (PFNGLPROGRAMPARAMETER4FNVPROC)
      wglGetProcAddress("glProgramParameter4fNV");
    glTrackMatrixNV = (PFNGLTRACKMATRIXNVPROC)
      wglGetProcAddress("glTrackMatrixNV");
    if (glGenProgramsNV == NULL
      || glBindProgramNV == NULL
      || glLoadProgramNV == NULL
      || glProgramParameter4fNV == NULL
      || glTrackMatrixNV == NULL)
      extension_vp_NV = 0;
  }
  // convenience report to user for Win32 only
  fprintf(stderr, "FooBillard -- Win32 OpenGL extended features found:\n"
    "GL_ARB_texture_cube_map = %d\n"
    "GL_ARB_multitexture = %d\n"
    "GL_NV_texture_shader = %d\n"
    "GL_NV_register_combiners = %d\n"
    "GL_NV_vertex_program = %d\n",
    extension_cubemap, extension_multitexture, extension_ts_NV,
    extension_rc_NV, extension_vp_NV);
#endif
}


static void Init(void)
{
  //    int i;
  GLfloat fogColor[4] = {0.0, 0.0, 0.0, 1.0};
  int    depth;


  parse_gl_extensions_string();

  if (options_dither)
    glEnable(GL_DITHER);
  else
    glDisable(GL_DITHER);

  /*    lightpos[0]=vec_xyz(-0.3,-0.6,0);
      lightpos[1]=vec_xyz(-0.3,+0.6,0);
      lightpos[2]=vec_xyz(+0.3,-0.6,0);
      lightpos[3]=vec_xyz(+0.3,+0.6,0);*/

  /*    lightpos[0]=vec_xyz(0.0,-0.6,0.7);
      lightpos[1]=vec_xyz(0.6*0.866,0.6*0.5,0.7);
      lightpos[2]=vec_xyz(-0.6*0.866,0.6*0.5,0.7);
      lightnr=3;*/

  /*    lightpos[0]=vec_scale(vec_xyz(0.0,-0.6,0.7),0.6);
      lightpos[1]=vec_scale(vec_xyz(0.6*0.866,0.6*0.5,0.7),0.6);
      lightpos[2]=vec_scale(vec_xyz(-0.6*0.866,0.6*0.5,0.7),0.6);
      lightnr=3;*/

  /*    lightpos[0]=vec_xyz(0.0,0.0,0.7);
      lightnr=1;*/

  lightpos[0] = vec_xyz(0.0, +0.4, 0.7);
  lightpos[1] = vec_xyz(0.0, -0.4, 0.7);
  lightnr = 2;

  /*    lightpos[0]=vec_xyz(0.0,0.0,0.7);
      lightpos[1]=vec_xyz(0.0,3.0,0.7);
      lightpos[2]=vec_xyz(0.0,-3.0,0.7);
      lightpos[3]=vec_xyz(2.0,0.0,0.7);
      lightpos[4]=vec_xyz(-2.0,0.0,0.7);
      lightnr=5;*/


  /*    glActiveTextureARB(GL_TEXTURE1_ARB);
      glEnable(GL_TEXTURE_2D);
      glActiveTextureARB(GL_TEXTURE0_ARB);*/
  glEnable(GL_TEXTURE_2D);

  glGenTextures(1, &spheretexbind);
  load_png("sphere_map_128x128.png", &spheretexw, &spheretexh, &depth, &spheretexdata);
  glBindTexture(GL_TEXTURE_2D, spheretexbind);
  gluBuild2DMipmaps(GL_TEXTURE_2D, 3, spheretexw, spheretexh, GL_RGB,
    GL_UNSIGNED_BYTE, spheretexdata);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options_tex_min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options_tex_mag_filter);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  free(spheretexdata);
  DPRINTF("spheretexbind=%d\n", spheretexbind);

  glGenTextures(1, &fblogotexbind);
  load_png("tabletex_fB_128x128.png", &spheretexw, &spheretexh, &depth, &spheretexdata);
  glBindTexture(GL_TEXTURE_2D, fblogotexbind);
  gluBuild2DMipmaps(GL_TEXTURE_2D, 1, spheretexw, spheretexh, GL_LUMINANCE,
    GL_UNSIGNED_BYTE, spheretexdata);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options_tex_min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options_tex_mag_filter);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  free(spheretexdata);
  DPRINTF("fblogotexbind=%d\n", fblogotexbind);

  glGenTextures(1, &placecueballtexbind);
  load_png("place_cue_ball.png", &spheretexw, &spheretexh, &depth, &spheretexdata);
  glBindTexture(GL_TEXTURE_2D, placecueballtexbind);
  gluBuild2DMipmaps(GL_TEXTURE_2D, 1, spheretexw, spheretexh, GL_LUMINANCE,
    GL_UNSIGNED_BYTE, spheretexdata);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options_tex_min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options_tex_mag_filter);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  free(spheretexdata);
  DPRINTF("placecueballtexbind=%d\n", placecueballtexbind);

  glGenTextures(1, &blendetexbind);
  load_png("blende.png", &spheretexw, &spheretexh, &depth, &spheretexdata);
  glBindTexture(GL_TEXTURE_2D, blendetexbind);
  gluBuild2DMipmaps(GL_TEXTURE_2D, 1, spheretexw, spheretexh, GL_LUMINANCE,
    GL_UNSIGNED_BYTE, spheretexdata);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options_tex_min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options_tex_mag_filter);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  free(spheretexdata);
  DPRINTF("blendetexbind=%d\n", blendetexbind);

  glGenTextures(1, &lightflaretexbind);
  load_png("lightflare.png", &spheretexw, &spheretexh, &depth, &spheretexdata);
  glBindTexture(GL_TEXTURE_2D, lightflaretexbind);
  gluBuild2DMipmaps(GL_TEXTURE_2D, 1, spheretexw, spheretexh, GL_LUMINANCE,
    GL_UNSIGNED_BYTE, spheretexdata);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options_tex_min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options_tex_mag_filter);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  free(spheretexdata);
  DPRINTF("lightflaretexbind=%d\n", lightflaretexbind);

  glGenTextures(1, &fullsymboltexbind);
  load_png("full_symbol.png", &spheretexw, &spheretexh, &depth, &spheretexdata);
  glBindTexture(GL_TEXTURE_2D, fullsymboltexbind);
  gluBuild2DMipmaps(GL_TEXTURE_2D, 1, spheretexw, spheretexh, GL_LUMINANCE,
    GL_UNSIGNED_BYTE, spheretexdata);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options_tex_min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options_tex_mag_filter);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  free(spheretexdata);
  DPRINTF("fullsymboltexbind=%d\n", fullsymboltexbind);

  glGenTextures(1, &halfsymboltexbind);
  load_png("half_symbol.png", &spheretexw, &spheretexh, &depth, &spheretexdata);
  glBindTexture(GL_TEXTURE_2D, halfsymboltexbind);
  gluBuild2DMipmaps(GL_TEXTURE_2D, 1, spheretexw, spheretexh, GL_LUMINANCE,
    GL_UNSIGNED_BYTE, spheretexdata);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options_tex_min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options_tex_mag_filter);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  free(spheretexdata);
  DPRINTF("halfsymboltexbind=%d\n", halfsymboltexbind);

  glGenTextures(1, &fullhalfsymboltexbind);
  load_png("fullhalf_symbol.png", &spheretexw, &spheretexh, &depth, &spheretexdata);
  glBindTexture(GL_TEXTURE_2D, fullhalfsymboltexbind);
  gluBuild2DMipmaps(GL_TEXTURE_2D, 1, spheretexw, spheretexh, GL_LUMINANCE,
    GL_UNSIGNED_BYTE, spheretexdata);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options_tex_min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options_tex_mag_filter);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  free(spheretexdata);
  DPRINTF("fullhalfsymboltexbind=%d\n", fullhalfsymboltexbind);

  if (options_calc_ball_reflections){
    glGenTextures(1, &reftexbind);
    load_png("reflect-earth.png", &spheretexw, &spheretexh, &depth, &spheretexdata);
    //        load_png("reflect.png",&spheretexw,&spheretexh,&depth,&spheretexdata);
    glBindTexture(GL_TEXTURE_2D, reftexbind);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, spheretexw, spheretexh, GL_RGB,
      GL_UNSIGNED_BYTE, spheretexdata);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options_tex_min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options_tex_mag_filter);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    free(spheretexdata);
    DPRINTF("reftexbind=%d\n", spheretexbind);
  }

  lightspheretexdata = 0;
  glGenTextures(1, &lightspheretexbind);
  load_png("sphere_map_128x128_light.png", &spheretexw, &spheretexh, &depth, &lightspheretexdata);
  glBindTexture(GL_TEXTURE_2D, lightspheretexbind);
  gluBuild2DMipmaps(GL_TEXTURE_2D, 3, spheretexw, spheretexh, GL_RGB,
    GL_UNSIGNED_BYTE, lightspheretexdata);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options_tex_min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options_tex_mag_filter);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  DPRINTF("lightspheretexbind=%d\n", lightspheretexbind);
  free(lightspheretexdata);


  glEnable(GL_FOG);
  glFogi(GL_FOG_MODE, GL_LINEAR);
  glHint(GL_FOG_HINT, GL_FASTEST);
  glFogf(GL_FOG_START, 0.0);
  glFogf(GL_FOG_END, 9.0);
  glFogfv(GL_FOG_COLOR, fogColor);

  //    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);


  /*    glEnable (GL_LINE_SMOOTH);
      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
      glLineWidth (1.5);
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);*/
  //    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

  //    glPolygonMode(GL_FRONT,GL_LINE);

  //    glShadeModel (GL_FLAT);

  walls.hole = NULL;
  walls.border = NULL;
  create_walls(&walls);
  balls.ball = NULL;
  bakballs.ball = NULL;
  create_scene(&balls);

#ifdef TIME_INTERPOLATE
  g_lastballs.ball=NULL;
  create_scene( &g_lastballs );
  g_drawballs.ball=NULL;
  create_scene( &g_drawballs );
#endif


  table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);

  /* lighting */
  //    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  //    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
  //    glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);


  DPRINTF("enabling depth test\n");
  glEnable(GL_DEPTH_TEST);
  glFrontFace(GL_CW);
  //   glCullFace(GL_FRONT);
  glEnable(GL_CULL_FACE);
  glDepthMask(GL_TRUE);
  //   glDepthFunc( GL_GREATER );
  //   glDepthFunc( GL_LESS );
  glDepthFunc(GL_LEQUAL);
  //   glDepthFunc( GL_ALWAYS );
  //   glDepthFunc( GL_EQUAL );

  if (options_cuberef){
    reassign_and_gen_cuberef_tex();
  }

  SetMode(REFLECT);
}


int main(int argc, char *argv[])
{

  int auxnr;
  int act_option, option_index;
  int confc;
  char ** confv;

  /* Tell Mesa GLX to use 3Dfx driver in fullscreen mode. */
  //   putenv("MESA_GLX_FX=fullscreen");

  /* Disable 3Dfx Glide splash screen */
  //   putenv("FX_GLIDE_NO_SPLASH=");


  /* initialize hostname */
  strcpy(options_net_hostname, "192.168.1.1");

  /* initialize random seed */
  srand(time_us());

  /* cd to data directory */
  //   fprintf(stderr,"DATA_DIRECTORY=%s\n",DATA_DIRECTORY);
#ifndef _WIN32
  // KHMAN 20040422 is this wrong? this is a bare string...
  if( DATA_DIRECTORY ){
#else
  if (chdir(WinDataPath())){
#endif
    fprintf(stderr, "foobillard seems not to be installed\n");
#ifndef _WIN32
    fprintf(stderr,"  assuming data directory 'data' instead of '%s'\n",DATA_DIRECTORY);
#else
    fprintf(stderr, "  assuming data directory 'data' instead of '%s'\n", WinDataPath());
#endif
    if (chdir("data")){
      fprintf(stderr, "  still no data - assuming data directory '../data'\n");
      if (chdir("../data")){
        fprintf(stderr, "cannot find valid data directory\n");
        fprintf(stderr, "(assuming the current dir contains the data)\n");
        //               exit(0);
      }
    }
  }



  human_human_mode = 0;
  //   if( argc>1 && argv[1][0]=='2' ) human_human_mode=1;

  init_human_player_roster(&human_player_roster);
  init_players();

  //#ifndef _WIN32
  print_help(long_options, appname_str, stderr);

  /* config file */
  load_config(&confv, &confc, argv, argc);
  while ((act_option = getopt_long_only(confc, confv, "+", long_options, &option_index)) >= 0){
    DPRINTF("processing option %d=%s\n", act_option, optarg);
    process_option(act_option);
  }
  DPRINTF("main:rgstereo=%d", options_rgstereo_on);
  
  // ==
  SetProcessDPIAware();
  if (wantFullscreen) {
      fullscreen = 1;
      win_width = GetSystemMetrics(SM_CXSCREEN);
      win_height = GetSystemMetrics(SM_CYSCREEN);
      ;
  }
  
  /* command line options */
  /*   while( ( act_option = getopt_long_only(argc, argv, "+", long_options, &option_index) ) >= 0){
         process_option(act_option);
         }*/
  //#endif


#if 0
  glutInitWindowPosition(0, 0);
  //   glutInitWindowSize( 800, 800 );
  glutInitWindowSize(WIDTH, HEIGHT);

  glutInit( &argc, argv );

  glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
  //   glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE );

  glutCreateWindow( argv[0] );
#endif
#ifdef _WIN32
  /* for Win32, this call sets the flag first in sys_stuff.c before init */
  if (fullscreen) sys_fullscreen(1, win_width, win_height);
#endif
  sys_create_display(&argc, argv, WIDTH, HEIGHT);

  // ===
  mm_init();
  voiceRecog_init();
  // ===

#ifndef _WIN32
  if( fullscreen ) sys_fullscreen( 1, win_width, win_height );
#endif

  Init();

  create_human_player_roster_text(&human_player_roster);
  create_players_text();

  if (options_gamemode == options_gamemode_tournament){
    init_tournament_state(&tournament_state);
  }

  restart_game();

  glGetIntegerv(GL_AUX_BUFFERS, &auxnr);
  DPRINTF("# of AUX-buffers:%d\n", auxnr);

  glEnable(GL_LIGHTING);
  if (g_network_play) {
    int dummy = 42;
    if (g_is_host){
      g_socket = host_create_socket();
      DPRINTF("host: writing test integer: %d\n", dummy);
      socket_write(g_socket, (char *)&dummy, sizeof(dummy));
      while (socket_read(g_socket, (char *)&dummy, sizeof(dummy)) == 0)
      {
        DPRINTF("...\n");
      }
      DPRINTF("host: read test integer from client: %d\n", dummy);
      player[1].queue_view = 0;
      player[0].queue_view = 1;
    }
    else {
      g_socket = client_call_socket(options_net_hostname);
      socket_read(g_socket, (char *)&dummy, sizeof(dummy));
      DPRINTF("client: read test integer from host: %d\n", dummy);
      dummy = 7;
      DPRINTF("client: writing test integer: %d\n", dummy);
      socket_write(g_socket, (char *)&dummy, sizeof(dummy));
      player[0].queue_view = 0;
      player[1].queue_view = 1;
      queue_view = 0;
    }
  }

#ifdef USE_SOUND
  init_sound();
#endif
  //   create_expsin(  17.0, 220.0, &ball_ball_snd.data, &ball_ball_snd.len );
  //   create_expsin_otones(  10.0, 320.0, 0.2, &ball_ball_snd.data, &ball_ball_snd.len );

  //   create_delayed_expsinerr(  10.0, 130.0, SOUND_NULLOFFS, 0.3, &ball_ball_snd.data, &ball_ball_snd.len );

#ifdef USE_SOUND
  {
    FILE * f;
    int i;
    //       create_expsinerr(  10.0, 130.0, 0.3, &ball_ball_snd.data, &ball_ball_snd.len );
    //       create_delayed_expsinerr( 10.0, 130.0, SOUND_NULLOFFS, 0.3, &ball_ball_snd.data, &ball_ball_snd.len );
    //       f=fopen("/mnt/windows/Games/VRBilliard/VRCarom/sounds/ballball.wav", "rb");
    DPRINTF("loading ball-ball sound\n");
    /* ball-ball sounds from samuele catuzzi's kbilliards - thanx */
    if((f=fopen("ball_ball.raw", "rb"))==NULL){
      fprintf(stderr,"couldn't open ball_ball.raw\n");
      exit(1);
    }
    fseek(f, 0L, SEEK_END);
    ball_ball_snd.len = ftell(f)+1+SOUND_NULLOFFS*2*2;
    fseek(f, 0L, SEEK_SET);
    ball_ball_snd.data = malloc(ball_ball_snd.len);
    fread( &ball_ball_snd.data[SOUND_NULLOFFS*2], 1, ball_ball_snd.len-SOUND_NULLOFFS*2*2 , f );
    fclose(f);

#if __BYTE_ORDER == __BIG_ENDIAN
    {
      char *snd=ball_ball_snd.data;
      for(i=0;i<ball_ball_snd.len;i+=2)
      {
        char t=snd[i];
        snd[i]=snd[i+1];
        snd[i+1]=t;
      }
    }
#endif
    for(i=0;i<ball_ball_snd.len/2/2-SOUND_NULLOFFS;i++){
      ball_ball_snd.data[(i+SOUND_NULLOFFS)*2+0]*=/*0.5*/1.0*exp(-(double)i/(double)((ball_ball_snd.len-SOUND_NULLOFFS*2*2)/2/4));
      ball_ball_snd.data[(i+SOUND_NULLOFFS)*2+1]*=/*0.5*/1.0*exp(-(double)i/(double)((ball_ball_snd.len-SOUND_NULLOFFS*2*2)/2/4));
    }
    for(i=0;i<ball_ball_snd.len/2/2-1;i++){
      ball_ball_snd.data[i*2+0]=ball_ball_snd.data[i*2+0]*0.7+ball_ball_snd.data[(i+1)*2+0]*0.3;
      ball_ball_snd.data[i*2+1]=ball_ball_snd.data[i*2+1]*0.7+ball_ball_snd.data[(i+1)*2+1]*0.3;
    }
    for(i=0;i<SOUND_NULLOFFS*2;i++){
      ball_ball_snd.data[i]=0;
    }
  }
#endif

  //   apply_bandpass( 10.0, 30.0, ball_ball_snd );

  /*   {
         TSound s1,s2;
         int i, lenmax;
         double newdata;
         create_expsinerr(  10.0,  130.0, 0.5, &ball_ball_snd.data, &ball_ball_snd.len );
         create_expsinerr(  110.0, 130.0, 1.0, &s1.data, &s1.len );
         create_expsinerr(  5.0,   130.0, 0.5, &s2.data, &s2.len );

         #define MYMAX(a,b) (((a)<(b))?(b):(a))
         lenmax = MYMAX(s2.len,s1.len);

         #define SNDENTRY(snd,index) (((index)<(snd).len/2)?(snd).data[(index)]:0)
         for(i=0;i<s1.len/2;i++){
         newdata = 0.0;
         newdata += SNDENTRY(s1,i)/2.0;
         newdata += SNDENTRY(s2,i)/2.0;
         ball_ball_snd.data[i] = newdata;
         }
         ball_ball_snd.len = lenmax;
         }*/

  //   create_expsinerr(  60.0, 220.0, 0.3, &ball_wall_snd.data, &ball_wall_snd.len );
  //   create_expsinerr_attack( 220.0, 85.0, 0.3, 1.0, &ball_wall_snd.data, &ball_wall_snd.len );

#ifdef USE_SOUND
  create_delayed_expsinerr( 220.0, 465.0, SOUND_NULLOFFS, 0.1, &ball_wall_snd.data, &ball_wall_snd.len );
  apply_attack( SOUND_NULLOFFS, 40.0, &ball_wall_snd.data, &ball_wall_snd.len );
#endif

  //   create_expsinerr(  20.0, 220.0, 0.6, &ball_cue_snd.data,  &ball_cue_snd.len  );
  //   create_delayed_expsinerr(  20.0, 220.0, SOUND_NULLOFFS, 0.6, &ball_cue_snd.data,  &ball_cue_snd.len );
#ifdef USE_SOUND
  create_expsinerr(  20.0, 220.0, 0.6, &ball_cue_snd.data,  &ball_cue_snd.len );
#endif
  /*   {
         FILE * f;
         int i;
         create_expsinerr(  10.0, 130.0, 0.3, &ball_cue_snd.data, &ball_cue_snd.len );
         f=fopen("/mnt/windows/Games/VRBilliard/VRCarom/sounds/cueball.wav", "rb");
         fseek(f, 0L, SEEK_END);
         ball_cue_snd.len = ftell(f);
         fseek(f, 0L, SEEK_SET);
         ball_cue_snd.data = malloc(ball_cue_snd.len);
         fread( ball_cue_snd.data, 1, ball_cue_snd.len , f );
         fclose(f);
         for(i=0;i<ball_ball_snd.len/2/2;i++){
         ball_cue_snd.data[i*2+0]*=0.5*exp(-(double)i/(double)(ball_cue_snd.len/2/5));
         ball_cue_snd.data[i*2+1]*=0.5*exp(-(double)i/(double)(ball_cue_snd.len/2/5));
         }
         }*/

  DPRINTF("creating winner text obj's\n");
  if (!options_3D_winnertext){
    winner_text_obj = textObj_new("wins", options_winner_fontname, 60);
    winner_name_text_obj = textObj_new("hallo", options_winner_fontname, 60);
  }
  else {
    winner_text_obj = textObj3D_new("wins", options_winner_fontname, 0.3, 0.08, 3);
    winner_name_text_obj = textObj3D_new("hallo", options_winner_fontname, 0.3, 0.08, 3);
  }
  DPRINTF("created winner text obj's\n");

  init_menu();

  sys_set_timer(frametime_ms, Idle_timer);     /* assure a framerate of max 50 fps (1frame/20ms) */


  sys_main_loop();


  return 0;
}


#ifdef _WIN32
#include <windows.h>
#include <ctype.h>

/* Win*Path - returns convenient paths (WIN32 only)
 * * note that the memory allocation functions are not terribly robust
 */

char *WinExePath(void)
{
  static char WinExePath[MAX_PATH] = "";
  static int NameLen = 0;

  if (NameLen) return WinExePath; /* further calls cached */

  NameLen = GetModuleFileName(NULL, WinExePath, MAX_PATH);
  if (NameLen) {
    char *WinExeName = WinExePath + NameLen - 1;
    while ((*WinExeName != '\\') && (WinExeName > WinExePath)) {
      WinExeName--;
      NameLen--;
    }
    if (*WinExeName == '\\') {
      *WinExeName = '\0'; /* snip off file name from path */
      return WinExePath;
    }
  }
  return 0;
}

char *WinDataPath(void)
{
  static char *WinDataPath = 0;

  if (WinDataPath) return WinDataPath; /* further calls cached */

  WinDataPath = (char *)malloc(strlen(WinExePath()) + 6);
  strcpy(WinDataPath, WinExePath());
  strcat(WinDataPath, "\\data");
  return WinDataPath;
}

char *WinRCPath(void)
{
  static char *WinRCPath = 0;

  if (WinRCPath) return WinRCPath; /* further calls cached */

  WinRCPath = (char *)malloc(MAX_PATH);

  if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, WinRCPath))) {
    /* got proper per-user application data folder for Windows,
     * make sure string size is okay
     */
    if (strlen(WinRCPath) + 15 <= MAX_PATH) {
      strcat(WinRCPath, "\\.foobillardrc");
      return WinRCPath;
    }
  }
  /* if anything fails, fall back to older version */
  free(WinRCPath);
  WinRCPath = (char *)malloc(strlen(WinExePath()) + 15);
  strcpy(WinRCPath, WinExePath());
  strcat(WinRCPath, "\\.foobillardrc");
  return WinRCPath;
}

#endif
