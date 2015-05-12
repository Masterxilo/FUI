/* billard3d.c
**
**    drawing all with OpenGL
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
#include <string.h>
#include <math.h>
#include <unistd.h>

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
#include <GL/glext.h>
#include "SDL.h"
//   #include <sys/timeb.h>   // us time measure
#endif

#ifdef _WIN32
#include <shlobj.h>
/* Mingw's libgw32c has getopt_long_only() function here */
#include <glibc/getopt.h>
#endif

#include "billard.h"
#include "ball.h"
#include "table.h"
#include "png_loader.h"
#include "aiplayer.h"
#include "options.h"
#include "player.h"
#include "evaluate_move.h"

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


int frametime_ms_min = 10;
int frametime_ms_max = 200;
int frametime_ms = 40;


int fullscreen = 0;  /* this is not updated during runtime - its only for startup */

GLuint table_obj = 0;
GLboolean Animate = GL_TRUE;

GLfloat Xrot = -70.0, Yrot = 0.0, Zrot = 0.0;
GLfloat Xque = -83.0, Zque = 0.0;
GLfloat Xrot_offs = 0.0, Yrot_offs = 0.0, Zrot_offs = 0.0;
GLfloat scale = 1.0;



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


int  vline_on = 1;
int  queue_view = 1;
int  old_queue_view = 1;

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




VMvect comp_dir;

int  human_human_mode = 0;
int  act_player = 0;   /* 0 or 1 */
char * player_names[] = {"Human Player", "AI Player", "Human Player 2", "AI Player 2"};
char * half_full_names[] = {"any", "full", "half"};
int  b1_b2_hold = 0;

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


//static int  g_lookballnr;


int   g_shot_due = 1;  /* a shot to be due at the beginning */
float g_motion_ratio = 1.0;  /* a shot to be due at the beginning */



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


#define ROSTER_MAX_NUM 128
struct PlayerRoster{
    int             nr;       /* number of players */
    struct Player   player[ROSTER_MAX_NUM];   /* players */
} human_player_roster;



static textObj * winner_name_text_obj;
static textObj * winner_text_obj;


static char * appname_str = "foobillard";

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
    else{
        _NOT_IMPLEMENTED
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

    strcpy(player[0].name, player_names[0]);
    strcpy(player[1].name, player_names[human_human_mode ? 2 : 1]);
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


void restart_game_common();

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
        PlaySound_fb(&ball_cue_snd, options_snd_volume*queue_strength / 2.0);
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
    _NOT_IMPLEMENTED
}

int do_net_move(void)
{
    _NOT_IMPLEMENTED
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


void shoot(int ani)
{
    int other_player;

    other_player = (act_player == 0) ? 1 : 0;
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
    }

    return(in_region);
}

int
in_table_region(VMvect pos)
{
    int in_region = 1;

    if (pos.x > (TABLE_W - BALL_D) / 2.0)  in_region = 0;
    if (pos.x < -(TABLE_W - BALL_D) / 2.0)  in_region = 0;
    if (pos.y > (TABLE_L - BALL_D) / 2.0)  in_region = 0;
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
        if (frametime_ms < 1) frametime_ms = 1;
        //        if( frametime_ms<frametime_ms_min ) frametime_ms=frametime_ms_min;
        if (frametime_ms > frametime_ms_max) frametime_ms = frametime_ms_max;
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

        while (dt_s_rest > 0.0) /* assure constant time flow */

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

#ifdef USE_SOUND
            {
                int index;
                index = 0;
                do{
                    BM_get_balls_hit_strength_last_index(index++, &bhitstrength, &toffs);
                    bhitstrength = 1.75 * (0.3 * bhitstrength / CUEBALL_MAXSPEED +
                        0.7 * bhitstrength*bhitstrength / CUEBALL_MAXSPEED / CUEBALL_MAXSPEED);
                    if (bhitstrength != 0.0){
                        if (toffs > TIMESTEP || toffs < 0.0){
                            exit(0);
                        }
                        else{
                            //                       printf("toffs/TIMESTEP=%f\n",toffs/TIMESTEP);
                        }
                        PlaySound_offs(&ball_ball_snd, options_snd_volume*((bhitstrength > 1.0) ? 1.0 : bhitstrength), SOUND_NULLOFFS - (TIMESTEP - toffs) * 22050);
                    }
                } while (bhitstrength != 0.0);
                index = 0;
                do{
                    BM_get_walls_hit_strength_last_index(index++, &whitstrength, &toffs);
                    whitstrength = 0.4 * (0.3 * whitstrength / CUEBALL_MAXSPEED +
                        0.7 * whitstrength*whitstrength / CUEBALL_MAXSPEED / CUEBALL_MAXSPEED);
                    if (whitstrength != 0.0){
                        PlaySound_offs(&ball_wall_snd, options_snd_volume*((whitstrength > 1.0) ? 1.0 : whitstrength), SOUND_NULLOFFS - (TIMESTEP - toffs) * 22050);
                        //                  PlaySound_fb(&ball_wall_snd,(whitstrength*0.125>1.0)?1.0:whitstrength*0.125);
                    }
                } while (whitstrength != 0.0);
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
                }
                textObj_setText(player[i].score_text, str);
            }
        }
        if (g_shot_due
            )
        {
            //           first_time=0;
            g_shot_due = 0;
            if (player[act_player].is_AI && !(player[act_player].winner || player[(act_player + 1) % 2].winner)){
                do_computer_move(1);
            }
        }
        other_player = (act_player == 0) ? 1 : 0;

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

int MouseEventEnabled = 0;
void
MouseEvent(MouseButtonEnum button, MouseButtonState  state, int x, int y, int key_modifiers)
//MouseEvent(int button, int state, int x, int y)
{
    if (!MouseEventEnabled) return;

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
            if ((!player[act_player].is_net) && (!player[act_player].is_AI)){
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


    //    fprintf(stderr,"button=%d\n", button);
    //    glutPostRedisplay();
    sys_redisplay();
}


void
ball_displace_clip(VMvect * cue_pos, VMvect offs)
{
    VMvect newpos;

    newpos = vec_add(*cue_pos, offs);

    if (options_gamemode == options_gamemode_training){

        if (newpos.x > (TABLE_W - BALL_D) / 2.0) newpos.x = (TABLE_W - BALL_D) / 2.0;
        if (newpos.x < -(TABLE_W - BALL_D) / 2.0) newpos.x = -(TABLE_W - BALL_D) / 2.0;
        if (newpos.y > (TABLE_L - BALL_D) / 2.0) newpos.y = (TABLE_L - BALL_D) / 2.0;
        if (newpos.y < -(TABLE_L - BALL_D) / 2.0) newpos.y = -(TABLE_L - BALL_D) / 2.0;

    }
    else {

        switch (gametype){
        case GAME_8BALL:
        case GAME_9BALL:
            if (newpos.x > (TABLE_W - BALL_D) / 2.0) newpos.x = (TABLE_W - BALL_D) / 2.0;
            if (newpos.x < -(TABLE_W - BALL_D) / 2.0) newpos.x = -(TABLE_W - BALL_D) / 2.0;
            if (newpos.y > -TABLE_L / 4.0) newpos.y = -TABLE_L / 4.0;
            if (newpos.y < -(TABLE_L - BALL_D) / 2.0) newpos.y = -(TABLE_L - BALL_D) / 2.0;
            break;
        }
    }

    *cue_pos = newpos;
}


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
            if (Xque + Xoffs > 0.0) Xoffs = 0.0 - Xque;
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
            if (Xrot + Xoffs > 0.0) Xoffs = 0.0 - Xrot;
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
            if (cam_FOV < 10.0) cam_FOV = 10.0;
            if (cam_FOV > 110.0) cam_FOV = 110.0;
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

void draw_text(int x, int y, char* s, int height) {
    // === custom text
    textObj* text = textObj_new(s, options_ball_fontname, height);
    glPushMatrix();

    // Screen goes from -1 to 1 in both directions, (0,0) is the center of the screen
    glTranslatef(x,y, 0);
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
// ==
void DisplayFunc(void)
{
    //   int i;
    GLfloat light_position[] = {0.0, 0.0, 0.7, 1.0};
    GLfloat light0_position[] = {0.0, 0.7, 0.7, 1.0};
    GLfloat light0_diff[] = {0.6, 0.6, 0.6, 1.0};
    GLfloat light0_amb[] = {0.35, 0.35, 0.35, 1.0};
    GLfloat light1_position[] = {0.0, -0.7, 0.7, 1.0};
    GLfloat light1_diff[] = {0.6, 0.6, 0.6, 1.0};
    GLfloat light1_amb[] = {0.35, 0.35, 0.35, 1.0};
    myvec cam_pos;
    static GLfloat real_dist = 0.0;
    static double fps;
    int i;
    //   int act_buffer;
    VMmatrix4 mv_matr;
    VMmatrix4 prj_matr;

    static GLfloat rg_eye_dist = 0.05;



    fps = 1000.0 / frametime_ms;

    real_dist = cam_dist;

    // Projection
    double znear, zfar;
    znear = 0.3;
    zfar = 10.0;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-znear*tan(cam_FOV*M_PI / 360.0), znear*tan(cam_FOV*M_PI / 360.0),
        -znear*tan(cam_FOV*M_PI / 360.0)*(double)win_height / (double)win_width,
        +znear*tan(cam_FOV*M_PI / 360.0)*(double)win_height / (double)win_width, znear, zfar);


    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    // Camera position
    if (old_queue_view == 1 && queue_view == 0) /* this is sloppy and ugly */
    { /* set free_view_pos to actual view */
        double th = Xrot / 180.0*M_PI;
        double ph = Zrot / 180.0*M_PI;
        free_view_pos_aim = vec_scale(vec_xyz(sin(th)*sin(ph), sin(th)*cos(ph), cos(th)), cam_dist);
        free_view_pos_aim = vec_add(free_view_pos_aim, CUE_BALL_XYPOS);
        free_view_pos = free_view_pos_aim;
    }
    old_queue_view = queue_view;

    double th = (Xrot + Xrot_offs) / 180.0*M_PI;
    double ph = (Zrot + Zrot_offs) / 180.0*M_PI;
    if (!FREE_VIEW){
        cam_pos = vec_scale(vec_xyz(sin(th)*sin(ph), sin(th)*cos(ph), cos(th)),
            real_dist);
        cam_pos = vec_add(cam_pos, balls.ball[CUE_BALL_IND].r);
    }
    else {
        cam_pos = free_view_pos;
    }

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


    if (!FREE_VIEW){
        glRotatef(Xrot + Xrot_offs, 1.0, 0.0, 0.0);
        glRotatef(Yrot + Yrot_offs, 0.0, 1.0, 0.0);
        glRotatef(Zrot + Zrot_offs, 0.0, 0.0, 1.0);

        glTranslatef(-balls.ball[CUE_BALL_IND].r.x,
            -balls.ball[CUE_BALL_IND].r.y,
            -balls.ball[CUE_BALL_IND].r.z);

    }

    glViewport(0, 0, win_width, win_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // Lights
    light_position[3] = 1.0;
    light0_position[3] = 1.0;
    light1_position[3] = 1.0;

    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diff);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_amb);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diff);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_amb);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);

    // ==
    // Store matrices
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);
    // ==

    /* draw table */
    glCallList(table_obj);


    /* draw balls with reflections and shadows */
    draw_balls(balls, cam_pos, cam_FOV, win_width, spheretexbind, lightpos, lightnr, (int *)0);

    if (options_place_cue_ball_tex && player[act_player].place_cue_ball && !balls_moving){
        int i;
        //        glMaterialfv(GL_FRONT, GL_DIFFUSE, col_shad);
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glDisable(GL_LIGHTING);
        glBlendFunc(GL_ONE, GL_ONE);
        glColor3f(0.5, 0.5, 0.5);
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

    /* hud stuff */
    glColor3f(1.0, 1.0, 1.0);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    /* strength bar */
    {
        glColor3f(0.2, 0.2, 0.2);
        myRect2D(-0.5, -0.805, 0.5, -0.725);
        glColor3f(0.3, 0.3, 0.3);
        myRect2D(-0.5, -0.795, -0.5 + queue_strength, -0.735);
    }

    if (vline_on && queue_view && !balls_moving){
        glColor3f(0.3, 0.3, 0.3);
        glLineStipple(1, 0xF0F0);
        glEnable(GL_LINE_STIPPLE);

        glBegin(GL_LINES);
        glVertex3f(0, 1.00, 0.5);
        glVertex3f(0, 0.08, 0.5);
        glEnd();

        glDisable(GL_LINE_STIPPLE);
    }

    // Custom window based 2d drawing
    // modify projection
    glMatrixMode(GL_PROJECTION);
    glOrtho(0, win_width, win_height, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);

    glColor3f(0.7, 0.7, 0.7);

    mm_draw_2d();

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void ResizeWindow(int width, int height)
{
    _NOT_IMPLEMENTED
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
    case options_gamemode_match:      restart_game_match();     return;
    }
    _NOT_IMPLEMENTED
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

void undo() {
    copy_balls(&bakballs, &balls);
}


void Key(int key, int modifiers)
{
    float step = 3.0;


    /* general keys */
    switch (key) {

    case KSYM_LEFT:
        Zrot += step;
        break;
    case KSYM_RIGHT:
        Zrot -= step;
        break;
    case KSYM_F2:
        birdview();
        break;
    case 'x':
        scale *= 1.1;
        break;
    case 'y':
        scale /= 1.1;
        break;
    case 27:
        exit(0);
    case ' ':
    case 13:
        if (modifiers == 0){ /* this has to be the same as middle mouse button !!! - maybe put it in a function some day  */
            if ((!player[act_player].is_net) && (!player[act_player].is_AI)){
                g_shot_due = 0;
                shoot(!queue_view);
            }
        }
        break;
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
    case 's': control_set(&control__mouse_shoot); break;
    case 'b': control_set(&control__cue_butt_updown); break;
    case 'e': control_set(&control__english); break;
    case 'm': control_set(&control__place_cue_ball); break;
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
        break;
    case'u':  /* undo */
        undo();
        break;
    }


    sys_redisplay();
}


void KeyUp(int key, int modifiers)
{
    switch (key) {
    case 's': control_unset(&control__mouse_shoot); break;
    case 'b': control_unset(&control__cue_butt_updown); break;
    case 'e': control_unset(&control__english); break;
    case 'm': control_unset(&control__place_cue_ball); break;
    }

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
        glCombinerOutputNV = (PFNGLCOMBINEROUTPUTNVPROC)            wglGetProcAddress("glCombinerOutputNV");
        glCombinerInputNV = (PFNGLCOMBINERINPUTNVPROC)            wglGetProcAddress("glCombinerInputNV");
        glFinalCombinerInputNV = (PFNGLFINALCOMBINERINPUTNVPROC)            wglGetProcAddress("glFinalCombinerInputNV");
        glCombinerParameterfvNV = (PFNGLCOMBINERPARAMETERFVNVPROC)            wglGetProcAddress("glCombinerParameterfvNV");
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

    lightpos[0] = vec_xyz(0.0, +0.4, 0.7);
    lightpos[1] = vec_xyz(0.0, -0.4, 0.7);
    lightnr = 2;


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



    walls.hole = NULL;
    walls.border = NULL;
    create_walls(&walls);
    balls.ball = NULL;
    bakballs.ball = NULL;
    create_scene(&balls);


    table_obj = create_table(spheretexbind, &walls, gametype == GAME_CARAMBOL);

    /* lighting */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);


    DPRINTF("enabling depth test\n");
    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    //   glDepthFunc( GL_GREATER );
    //   glDepthFunc( GL_LESS );
    glDepthFunc(GL_LEQUAL);
    //   glDepthFunc( GL_ALWAYS );
    //   glDepthFunc( GL_EQUAL );


    SetMode(REFLECT);
}


int main(int argc, char *argv[])
{
    int act_option, option_index;
    int confc;
    char ** confv;

    /* initialize random seed */
    srand(time_us());

    /* cd to data directory */
    chdir("data");

    human_human_mode = 0;
    //   if( argc>1 && argv[1][0]=='2' ) human_human_mode=1;

    init_human_player_roster(&human_player_roster);
    init_players();


    /* config  */
    SetProcessDPIAware();
    fullscreen = 0;
    win_width = 1024 * (fullscreen + 1);// 512;
    win_height = 768 * (fullscreen + 1);// 512;


    /* for Win32, this call sets the flag first in sys_stuff.c before init */
    if (fullscreen) sys_fullscreen(1, win_width, win_height);

    sys_create_display(&argc, argv, WIDTH, HEIGHT);


    Init();

    create_human_player_roster_text(&human_player_roster);
    create_players_text();


    restart_game();
    glEnable(GL_LIGHTING);

#ifdef USE_SOUND
    init_sound();
#endif

#ifdef USE_SOUND
    {
        FILE * f;
        int i;

        DPRINTF("loading ball-ball sound\n");
        /* ball-ball sounds from samuele catuzzi's kbilliards - thanx */
        if ((f = fopen("ball_ball.raw", "rb")) == NULL){
            fprintf(stderr, "couldn't open ball_ball.raw\n");
            exit(1);
        }
        fseek(f, 0L, SEEK_END);
        ball_ball_snd.len = ftell(f) + 1 + SOUND_NULLOFFS * 2 * 2;
        fseek(f, 0L, SEEK_SET);
        ball_ball_snd.data = malloc(ball_ball_snd.len);
        fread(&ball_ball_snd.data[SOUND_NULLOFFS * 2], 1, ball_ball_snd.len - SOUND_NULLOFFS * 2 * 2, f);
        fclose(f);

        for (i = 0; i < ball_ball_snd.len / 2 / 2 - SOUND_NULLOFFS; i++){
            ball_ball_snd.data[(i + SOUND_NULLOFFS) * 2 + 0] *=/*0.5*/1.0*exp(-(double)i / (double)((ball_ball_snd.len - SOUND_NULLOFFS * 2 * 2) / 2 / 4));
            ball_ball_snd.data[(i + SOUND_NULLOFFS) * 2 + 1] *=/*0.5*/1.0*exp(-(double)i / (double)((ball_ball_snd.len - SOUND_NULLOFFS * 2 * 2) / 2 / 4));
        }
        for (i = 0; i < ball_ball_snd.len / 2 / 2 - 1; i++){
            ball_ball_snd.data[i * 2 + 0] = ball_ball_snd.data[i * 2 + 0] * 0.7 + ball_ball_snd.data[(i + 1) * 2 + 0] * 0.3;
            ball_ball_snd.data[i * 2 + 1] = ball_ball_snd.data[i * 2 + 1] * 0.7 + ball_ball_snd.data[(i + 1) * 2 + 1] * 0.3;
        }
        for (i = 0; i < SOUND_NULLOFFS * 2; i++){
            ball_ball_snd.data[i] = 0;
        }
    }
#endif

#ifdef USE_SOUND
    create_delayed_expsinerr(220.0, 465.0, SOUND_NULLOFFS, 0.1, &ball_wall_snd.data, &ball_wall_snd.len);
    apply_attack(SOUND_NULLOFFS, 40.0, &ball_wall_snd.data, &ball_wall_snd.len);
#endif
#ifdef USE_SOUND
    create_expsinerr(20.0, 220.0, 0.6, &ball_cue_snd.data, &ball_cue_snd.len);
#endif

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

    sys_set_timer(frametime_ms, Idle_timer);     /* assure a framerate of max 50 fps (1frame/20ms) */

    mm_init();

    sys_main_loop();


    return 0;
}

