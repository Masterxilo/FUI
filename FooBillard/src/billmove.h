/* billmove.h
**
**    includefile: physics of the billard system to calculate timestep
**    Copyright (C) 2001  Florian Berger
**    Email:  harpin_floh@yahoo.de,  florian.berger@jk.uni-linz.ac.at
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

#ifndef BILLMOVE_H
#define BILLMOVE_H
#include "myinclude.h"
#include "vmath.h"

typedef VMvect myvec;


#define USE_ADV_BORDER
//#undef USE_ADV_BORDER
#define BILLMOVE_SINGLE_PRECISION
//#undef BILLMOVE_SINGLE_PRECISION


#ifndef USE_ADV_BORDER

#define  BorderType             OldBorderType
#define  BALL_WALL_DIST(b,w)    fabs(vec_mul(vec_diff(b.r,w.r),w.n))
#define  BALL_WALL_DIST_I(b,w)  vec_mul(vec_diff(b.r,w.r),w.n)

#else

#define  BorderType             AdvBorderType
#define  BALL_WALL_DIST_I(b,w)  ball_advborder_dist(&(b),&(w))
#define  BALL_WALL_DIST(b,w)    fabs(ball_advborder_dist(&(b),&(w)))

#endif

#define  BALL_BALL_DIST(b1,b2)  vec_abs(vec_diff(b1.r,b2.r))


#ifdef BILLMOVE_SINGLE_PRECISION
typedef float BMfloat;     /* floating point type for billard physics */
#else
typedef double BMfloat;    /* floating point type for billard physics */
#endif


enum gameType { GAME_8BALL, GAME_9BALL, GAME_CARAMBOL, GAME_SNOOKER };


typedef struct{
    BMfloat  m;           // mass                   [kg]
    BMfloat  I;           // massentraegheitsmom    [kg*m^2]
    BMfloat  d;           // diameter               [m]
    myvec    r;           // position               [m]
    myvec    v;           // speed                  [m/s]
    myvec    w;           // rotation speed and axe [rad./s] in table coords
    myvec    b[3];        // x,y,z base of ball in table coords
    int      nr;          // 0=white, ...
    int      in_game;     // ball in game
    int      in_fov;      // ball in field of view - used internally
    int      in_hole;     // ball still in game but already versenkt
    myvec *  path;        // path of ball
    int      pathsize;    // number of reserved path points
    int      pathcnt;     // counter of path points
} BallType;

typedef struct{
    int            nr;
    enum gameType  gametype;
    BallType *     ball;
} BallsType;

typedef struct{
    myvec r;             // pos
    myvec n;             // normal vector
} OldBorderType;

typedef struct{
    int   pnr;           // 4=arc 3=triangle 2=line 1=point
    myvec r1;            // pos
    myvec r2;            // pos
    myvec r3;            // pos (tangent vec for arc)
    myvec n;             // normal vector
    double mu;           // friction const
    double loss0;        // const loss per hit (0th order in speed)
    double loss_max;     // max loss
    double loss_wspeed;  // width of higher order loss curve
} AdvBorderType;

typedef struct{
    myvec pos;           // pos
    myvec aim;           // position to aim for ai-player
    BMfloat r;           // radius of hole (wall-coll free zone)
} HoleType;

typedef struct{
    int          nr;
    BorderType * border;
    int          holenr;
    HoleType *   hole;
} BordersType;


void       BM_reset_move_info(void);

int        BM_get_balls_out_half(void);
int        BM_get_balls_out_full(void);
int        BM_get_balls_out_total(void);  // halves and full (no black or white)
int        BM_get_balls_out_all(void);    // all balls inclusive black and white
int        BM_get_white_out(void);
int        BM_get_ball_out(int nr);
int        BM_get_nth_ball_out(int n);
int        BM_get_min_ball_out(void);
int        BM_get_1st_ball_hit(void);
int        BM_get_non_strafraum_wall_hit_before_1st_ball( int (* in_strafraum)(VMvect) );
int        BM_get_nth_ball_hit(int n);
int        BM_get_nth_ball_hit_by_ind(int ind, int n);
VMvect     BM_get_1st_ball_hit_pos(void);
int        BM_get_balls_hit(void);
int        BM_get_balls_hit_last();
int        BM_get_walls_hit_last();
double     BM_get_balls_hit_strength_last();
void       BM_get_balls_hit_strength_last_index(int index, double * strength, double * toffs);
double     BM_get_walls_hit_strength_last();
void       BM_get_walls_hit_strength_last_index(int index, double * strength, double * toffs);

BallType * BM_get_ball_by_nr( int nr, BallsType *pballs );

int proceed_dt(BallsType *balls, BordersType *borders, BMfloat dt);

#ifdef USE_ADV_BORDER
BMfloat ball_advborder_dist( BallType *b, BorderType *w );
#endif

void       BM_add2path( BallType *pball );
void       BM_clearpath( BallType *pball );

#endif  /* BILLMOVE_H */
