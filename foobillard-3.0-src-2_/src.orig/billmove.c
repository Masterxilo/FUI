/* billmove.c
**
**    physics of the billard system to calculate timestep
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

#define SQRTM1 100000.0

#include "options.h"
#include "billmove.h"
#include "math.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h" /* memcpy */

#define MAX_EVENT_NR 1000

enum event_type {
    BALL_out,
    BALL_wall,
    BALL_ball
};

struct BallEvent{
    int     ballnr;
    int     ballnr2;
    VMvect  pos;
    VMvect  pos2;
    VMvect  v;
    VMvect  v2;
    VMvect  w;
    VMvect  w2;
    int     timestep_nr;
    BMfloat timeoffs;
    enum event_type event;
};

static struct{
    int timestep_nr;
    BMfloat duration;
    BMfloat duration_last;
    int out_half;
    int out_full;
    int out_white;
    int out_black;
    int eventnr;
    struct BallEvent event[MAX_EVENT_NR];
} move_log;


void BM_reset_move_info()
{
    move_log.out_half=0;
    move_log.out_full=0;
    move_log.out_white=0;
    move_log.out_black=0;
    move_log.eventnr=0;
    move_log.timestep_nr=0;
    move_log.duration=0.0;
    move_log.duration_last=0.0;
}

int BM_get_balls_out_half()  { return move_log.out_half; }

int BM_get_balls_out_full()  { return move_log.out_full; }

int BM_get_balls_out_total() { return move_log.out_full+move_log.out_half; }

int BM_get_balls_out_all() { return move_log.out_full+move_log.out_half+move_log.out_white+move_log.out_black; }

int BM_get_white_out() { return move_log.out_white; }


int BM_get_ball_out(int nr) {
    int i,ballout;

    ballout=0;
    for(i=0;i<move_log.eventnr;i++){
        if( move_log.event[i].event==BALL_out && move_log.event[i].ballnr==nr ){
            ballout=1;
        }
    }
    return ballout;
}


int BM_get_nth_ball_out(int n)
{
    int i,ballout;

    ballout=-1;
    for(i=0;i<move_log.eventnr && n!=0;i++){
        if( move_log.event[i].event==BALL_out ){
            n--;
            if( n==0 ) ballout=move_log.event[i].ballnr;
        }
    }
    return ballout;
}


int BM_get_min_ball_out()  {
    int i,minnr;

    minnr=100;
    for(i=0;i<move_log.eventnr;i++){
        if( move_log.event[i].event==BALL_out && move_log.event[i].ballnr<minnr ){
            minnr=move_log.event[i].ballnr;
        }
    }
    return minnr;
}


int BM_get_1st_ball_hit()
{
    int i,hitball;

    hitball=-1;
    for(i=0;i<move_log.eventnr;i++){
        if( move_log.event[i].event==BALL_ball ){
            if( move_log.event[i].ballnr!=0 ){
                hitball=move_log.event[i].ballnr;
            } else {
                hitball=move_log.event[i].ballnr2;
            }
            break;
        }
    }
    return hitball;
}


int BM_get_non_strafraum_wall_hit_before_1st_ball( int (* in_strafraum)(VMvect) )
{
    int i;
    int rval=0;

    for( i=0 ; i<move_log.eventnr && move_log.event[i].event!=BALL_ball ; i++ ){
        if( move_log.event[i].event==BALL_wall ){
            if( !in_strafraum(move_log.event[i].pos2) ){
                rval=1;
                break;
            }
        }
    }
    return rval;
}


int BM_get_nth_ball_hit(int n)
{
    int i,j,hitball;

    hitball=-1;
    i=0;
    for(j=0;j<(n-1);j++){
        for(   ;i<move_log.eventnr;i++){
            if( move_log.event[i].event==BALL_ball ){
                if( move_log.event[i].ballnr==0 ||
                    move_log.event[i].ballnr2==0 ) break;
            }
        }
        i++;
    }
    for(   ;i<move_log.eventnr;i++){
        if( move_log.event[i].event==BALL_ball ){
            if( move_log.event[i].ballnr==0 ){
                hitball=move_log.event[i].ballnr2;  break;
            } else if( move_log.event[i].ballnr2==0 ){
                hitball=move_log.event[i].ballnr;   break;
            }
        }
    }
    return hitball;
}


int BM_get_nth_ball_hit_by_ind(int ind, int n)
{
    int i,j,hitball;

    hitball=-1;
    i=0;
    for(j=0;j<(n-1);j++){
        for(   ;i<move_log.eventnr;i++){
            if( move_log.event[i].event==BALL_ball ){
                if( move_log.event[i].ballnr==ind ||
                    move_log.event[i].ballnr2==ind ) break;
            }
        }
        i++;
    }
    for(   ;i<move_log.eventnr;i++){
        if( move_log.event[i].event==BALL_ball ){
            if( move_log.event[i].ballnr==ind ){
                hitball=move_log.event[i].ballnr2;  break;
            } else if( move_log.event[i].ballnr2==ind ){
                hitball=move_log.event[i].ballnr;   break;
            }
        }
    }
    return hitball;
}


VMvect BM_get_1st_ball_hit_pos()
{
    int i;
    VMvect hitpos;

    hitpos=vec_xyz(100,100,100);
    for(i=0;i<move_log.eventnr;i++){
        if( move_log.event[i].event==BALL_ball ){
            if( move_log.event[i].ballnr!=0 ){
                hitpos=move_log.event[i].pos;
            } else {
                hitpos=move_log.event[i].pos2;
            }
            break;
        }
    }
    return hitpos;
}


int BM_get_balls_hit()  {
    int i,hits;

    hits=0;
    for(i=0;i<move_log.eventnr;i++){
        if( move_log.event[i].event==BALL_ball ){
            if( move_log.event[i].ballnr==0 || move_log.event[i].ballnr2==0 ){
                hits++;
            }
        }
    }
    return hits;
}


int BM_get_balls_hit_last()  {
    int i,hits;

    hits=0;
    for(i=0;i<move_log.eventnr;i++){
        if( move_log.event[i].event==BALL_ball ){
            if( move_log.event[i].timestep_nr==move_log.timestep_nr ){
                hits++;
            }
        }
    }
    return hits;
}


int BM_get_walls_hit_last()  {
    int i,hits;

    hits=0;
    for(i=0;i<move_log.eventnr;i++){
        if( move_log.event[i].event==BALL_wall ){
            if( move_log.event[i].timestep_nr==move_log.timestep_nr ){
                hits++;
            }
        }
    }
    return hits;
}


double BM_get_balls_hit_strength_last()  {
    int i;
    double hitstrength=0.0;

    for(i=0;i<move_log.eventnr;i++){
        if( move_log.event[i].event==BALL_ball ){
            if( move_log.event[i].timestep_nr==move_log.timestep_nr ){
                hitstrength+=fabs(
                                  vec_mul(
                                          vec_unit(vec_diff(move_log.event[i].pos,move_log.event[i].pos2)),
                                          vec_diff(move_log.event[i].v,move_log.event[i].v2)
                                        )
                                 );
            }
        }
    }
    return hitstrength;
}


void BM_get_balls_hit_strength_last_index(int index, double * strength, double * toffs)
{
    int i;

    for(i=0;i<move_log.eventnr;i++){
        if( move_log.event[i].event==BALL_ball ){
            if( move_log.event[i].timestep_nr==move_log.timestep_nr ){
                index--;
                if( !(index+1) ){
                    *strength=fabs(
                                   vec_mul(
                                           vec_unit(vec_diff(move_log.event[i].pos,move_log.event[i].pos2)),
                                           vec_diff(move_log.event[i].v,move_log.event[i].v2)
                                          )
                                  );
//                    *strength *= *strength;
                    *toffs=move_log.event[i].timeoffs;
                    break;
                }
            }
        }
    }
    if( i==move_log.eventnr ){ *toffs=0.0; *strength=0.0; }
}


double BM_get_walls_hit_strength_last()  {
    int i;
    double hitstrength=0.0;

    for(i=0;i<move_log.eventnr;i++){
        if( move_log.event[i].event==BALL_wall ){
            if( move_log.event[i].timestep_nr==move_log.timestep_nr ){
                hitstrength+=vec_abs( move_log.event[i].v2 );
            }
        }
    }
    return hitstrength;
}


void BM_get_walls_hit_strength_last_index(int index, double * strength, double * toffs)
{
    int i;

    for(i=0;i<move_log.eventnr;i++){
        if( move_log.event[i].event==BALL_wall ){
            if( move_log.event[i].timestep_nr==move_log.timestep_nr ){
                index--;
                if( !(index+1) ){
                    *strength=vec_abs(move_log.event[i].v2);
//                    *strength *= *strength;
                    *toffs=move_log.event[i].timeoffs;
                    break;
                }
            }
        }
    }
    if( i==move_log.eventnr ){ *toffs=0.0; *strength=0.0; }
}


BallType * BM_get_ball_by_nr( int nr, BallsType *pballs )
{
    int i;
    for(i=0;i<pballs->nr;i++){
        if( pballs->ball[i].nr == nr ) break;
    }
    return (i!=pballs->nr) ? &(pballs->ball[i]) : NULL ;
}



void record_move_log_event( enum event_type event,
                            int nr,
                            int nr2,
                            BallsType *pballs,
                            BMfloat timeoffs )
{
    move_log.event[move_log.eventnr].event=event;

    move_log.event[move_log.eventnr].ballnr=nr;
    move_log.event[move_log.eventnr].ballnr2=nr2;

    if(event==BALL_ball){
        move_log.event[move_log.eventnr].pos =BM_get_ball_by_nr(nr,pballs)->r;
        move_log.event[move_log.eventnr].pos2=BM_get_ball_by_nr(nr2,pballs)->r;
        move_log.event[move_log.eventnr].v   =BM_get_ball_by_nr(nr,pballs)->v;
        move_log.event[move_log.eventnr].v2  =BM_get_ball_by_nr(nr2,pballs)->v;
        move_log.event[move_log.eventnr].w   =BM_get_ball_by_nr(nr,pballs)->w;
        move_log.event[move_log.eventnr].w2  =BM_get_ball_by_nr(nr2,pballs)->w;
    }else if(event==BALL_wall){
        move_log.event[move_log.eventnr].pos2=BM_get_ball_by_nr(nr2,pballs)->r;
        move_log.event[move_log.eventnr].v2  =BM_get_ball_by_nr(nr2,pballs)->v;
        move_log.event[move_log.eventnr].w2  =BM_get_ball_by_nr(nr2,pballs)->w;
    }else if(event==BALL_out){
        move_log.event[move_log.eventnr].pos =BM_get_ball_by_nr(nr,pballs)->r;
    }

    move_log.event[move_log.eventnr].timestep_nr = move_log.timestep_nr;
    move_log.event[move_log.eventnr].timeoffs    = timeoffs;
    move_log.eventnr++;
}


int my_rand(nr)
{
    return rand()%nr;
}


static void calc_rand_table(int * rnd, int nr)
{
    int i,i1,dummy;
    for(i=0;i<nr;i++){
        rnd[i]=i;
    }
    for(i=0;i<nr;i++){
        i1    = my_rand(nr);
        dummy = rnd[i];
        rnd[i] = rnd[i1];
        rnd[i1] = dummy;
    }
}


BMfloat plane_dist(VMvect r, VMvect rp, VMvect n)
{ return( vec_mul(vec_diff(r,rp),n) ); }


#ifdef USE_ADV_BORDER
int inrange_advborder( BallType *b, BorderType *w )
{
    VMvect r,dr,dr1,dr2,dr3,n;
    BMfloat d,dra;
    if       (w->pnr==3){
        dr1 = vec_diff(w->r2,w->r1);
        dr2 = vec_diff(w->r3,w->r2);
        dr3 = vec_diff(w->r1,w->r3);
        n   = vec_unit(vec_cross(dr1,dr2));
        return( plane_dist( b->r, w->r1, vec_unit(vec_cross(n,dr1)) ) >= 0.0 &&
                plane_dist( b->r, w->r2, vec_unit(vec_cross(n,dr2)) ) >= 0.0 &&
                plane_dist( b->r, w->r3, vec_unit(vec_cross(n,dr3)) ) >= 0.0 );
    } else if(w->pnr==2){
        r   = vec_diff(b->r,w->r1);
        dr  = vec_diff(w->r2,w->r1);
        dra = vec_abs(dr);
        d   = vec_mul(r,dr)/dra;
        return (d>=0.0 && d<dra);
    } else if(w->pnr==1){
        return 1;
    }
    return 1;
}

BMfloat ball_advborder_dist( BallType *b, BorderType *w )
{
    VMvect r,dr;
    if(inrange_advborder(b,w)){
        if       (w->pnr==3){
            return vec_mul(vec_diff(b->r,w->r1),w->n);
        } else if(w->pnr==2){
            r=vec_diff(b->r,w->r1);
            dr=vec_diff(w->r2,w->r1);
            return vec_abs(vec_diff(r,vec_proj(r,dr)));
        } else if(w->pnr==1){
            return vec_abs(vec_diff(b->r,w->r1));
        }
    } else {
        return -1.0E20;
    }
    return -1.0E20;
}
#endif


#ifdef USE_ADV_BORDER

static BMfloat calc_wall_collision_time( BallType * ball, BorderType * wall )
{
    BMfloat h, vn, rval, ph, q, t1,t2;
    myvec   dr, r, v;
    rval=0.0;
    if       (wall->pnr==3){
        dr = vec_diff( ball->r, wall->r1 );
        h  = vec_mul( dr, wall->n ) - ball->d/2.0;
        vn = vec_mul( ball->v, wall->n );
        rval = -h/vn;
    } else if(wall->pnr==2){
        /* del all comps par to cylinder */
        dr = vec_diff( wall->r2 ,wall->r1 );
        r = vec_diff( ball->r, wall->r1 );
        r = vec_diff( r, vec_proj(r,dr) );
        v = ball->v;
        v = vec_diff( v, vec_proj(v,dr) );
        ph = vec_mul(v,r)/vec_abssq(v);
        q  = (vec_abssq(r) - ball->d*ball->d/4.0)/vec_abssq(v);
        if(ph*ph>q){
           t1 = -ph+sqrt(ph*ph-q);
           t2 = -ph-sqrt(ph*ph-q);
        } else {
           t1 = SQRTM1;
           t2 = SQRTM1;
        }
        /* solve |r+vt|=d/2 */
        rval = (t1<t2)?t1:t2;
    } else if(wall->pnr==1){
        r = vec_diff( ball->r, wall->r1 );
        ph = vec_mul(ball->v,r)/vec_abssq(ball->v);
        q  = (vec_abssq(r) - ball->d*ball->d/4.0)/vec_abssq(v);
        if(ph*ph>q){
	        t1 = -ph+sqrt(ph*ph-q);
   	     t2 = -ph-sqrt(ph*ph-q);
        } else {
           t1 = SQRTM1;
           t2 = SQRTM1;
        }
        rval = (t1<t2)?t1:t2;
    }
    if( !inrange_advborder( ball, wall ) ){
        rval=1E20;
    }
    return(rval);
}

#else

static BMfloat calc_wall_collision_time( BallType * ball, BorderType * wall )
{
    BMfloat h, vn;
    myvec   dr;
    dr = vec_diff( ball->r, wall->r );
    h  = vec_mul( dr, wall->n ) - ball->d/2.0;
    vn = vec_mul( ball->v, wall->n );
    return(-h/vn);
}

#endif

static BMfloat calc_ball_collision_time( BallType * b1, BallType * b2 )
{
    BMfloat p, q, vs, rs, t1, t2, ds;
    myvec   dv, dr;
    dv = vec_diff( b1->v, b2->v );
    dr = vec_diff( b1->r, b2->r );
    vs = dv.x*dv.x + dv.y*dv.y + dv.z*dv.z ;
    rs = dr.x*dr.x + dr.y*dr.y + dr.z*dr.z ;
    ds = (b1->d+b2->d)/2.0;
    ds *= ds;
    p  = ( dv.x*dr.x + dv.y*dr.y + dv.z*dr.z )/vs;
    q  = (rs-ds)/vs;
    q  = (p*p>q)?sqrt(p*p-q):SQRTM1;
    t1 = -p + q;
    t2 = -p - q;
    return( (t1<t2)?t1:t2 );
}


#ifdef USE_ADV_BORDER

myvec perimeter_speed_wall( BallType * ball, BorderType * wall )
/* only for 3-point wall */
{
    return(
            vec_cross(
                      ball->w,
                      vec_scale(wall->n,-ball->d/2.0)
                     )
           );
}

myvec perimeter_speed_normal( BallType * ball, myvec normal )
{
    return(
            vec_cross(
                      ball->w,
                      vec_scale(normal,-ball->d/2.0)
                     )
           );
}


#if 0  /* new ball-wall ia */
static void ball_wall_interaction( BallType * ball, BorderType * wall )
{
//#define CUSHION_LOSS         0.2     /* const loss of energy by cushion */
//#define CUSHION_LOSS_O1      0.07    /* linear (in speed) loss of energy by cushion */
//#define MU_BANDE             0.1     /* friction const between cusion an ball */
#define CUSHION_LOSS_O0      (wall->loss0)       /* const loss of energy by cushion */
#define CUSHION_MAX_LOSS     (wall->loss_max)    /* const loss of energy by cushion */
#define CUSHION_LOSS_WSPEED  (wall->loss_wspeed) /* prop. halbwertsgeschwindigkeit */
#define MU_BANDE             (wall->mu)          /* friction const between cusion an ball */

    myvec dv, dv_tot, dw, dw_tot, dr, n;

    dv_tot  = vec_xyz(0.0,0.0,0.0);
    dw_tot  = vec_xyz(0.0,0.0,0.0);

    if       (wall->pnr==3){
        myvec vp, vn;
        double loss;

        vn      = vec_proj(ball->v, wall->n);
        vp      = vec_diff(ball->v, vn);
        //        dv      = vec_scale(wall->n,vec_mul(wall->n,ball->v)*2.0);

        /* normal component */
        loss = CUSHION_LOSS_O0+
               (CUSHION_MAX_LOSS-CUSHION_LOSS_O0)*(1.0-exp(-vec_abs(vn)/CUSHION_LOSS_WSPEED));
        //        loss    = CUSHION_LOSS;

        dv      = vec_scale( vn, -(1.0+sqrt(1.0-loss)) );
        if( !options_jump_shots ) dv.z = 0.0;
        dv_tot  = vec_add(dv_tot,dv);
/*        dv      = vec_scale( vn, -(1.0+sqrt( 1.0-(1.0-exp(-vec_abs(vn)*CUSHION_LOSS_O1)) )) );
        ball->v = vec_add(ball->v,dv);*/

        /* parallel component */
        dv      = vec_scale( vec_unit(vec_add(perimeter_speed_mormal(ball, wall->n),vp)), -vec_abs(dv)*MU_BANDE );
        if( !options_jump_shots ) dv.z = 0.0;
        /* ang mom change due to friction */
        dw      = vec_cross(vec_scale(dv,ball->m/2.0/ball->I),vec_scale(wall->n,ball->d));
        dw_tot  = vec_add(dw_tot,dw);
/*#if options_jump_shots
#else
        dv.z    = 0.0;
#endif*/
        dv_tot  = vec_add(dv_tot,dv);
//        printf("dv=(%f,%f,%f)\n",vn.x,vn.y,vn.z);
        /* ang mom change due to eccentric hit */
////        dw      = vec_scale(vec_cross(wall->n, vec_proj(dv,ball->v)), ball->d/2.0 * ball->m/ball->I );
////        if( vec_mul(vec_unit(ball->w),dw)<-2.0*vec_abs(vec_proj(ball->w,dw)) ) dw=vec_scale(vec_proj(ball->w,dw),-2.0);
////        dw_tot  = vec_add(dw_tot,dw);

        ball->v = vec_add(ball->v,dv_tot);
        ball->w = vec_add(ball->w,dw_tot);
//        fprintf(stderr,"dv=(%f,%f,%f)",dv.x,dv.y,dv.z);
        /* maybe some angular momentum loss has to be implemented here */
        /* ... */
    } else if(wall->pnr==2){
        myvec vp, vn;
        double loss;
        dr      = vec_diff( wall->r2, wall->r1 );
        n       = vec_diff( ball->r, wall->r1 );
        n       = vec_unit( vec_diff( n, vec_proj(n,dr) ) );
        vn      = vec_proj(ball->v, n);
        vp      = vec_diff(ball->v, vn);
        loss = CUSHION_LOSS_O0+
            (CUSHION_MAX_LOSS-CUSHION_LOSS_O0)*(1.0-exp(-vec_abs(vn)/CUSHION_LOSS_WSPEED));

        dv      = vec_scale(n,-vec_mul(n,ball->v)*2.0);
        if( options_jump_shots ) dv.z = 0.0;
//        dv      = vec_scale(vn, -(1.0+sqrt(1.0-loss)) );
        dv_tot  = vec_add(dv_tot,dv);
        /* ang mom change due to eccentric hit */
        /* dL = eccentricity * F*dt = eccentricity * impact change = ecc * dv * M_BALL */
        /* dw = dL/I */
//        dw      = vec_scale(vec_cross(n, vec_proj(dv,ball->v)), -ball->d/2.0 * ball->m/ball->I );
////        if( vec_mul(vec_unit(ball->w),dw)<-2.0*vec_abs(vec_proj(ball->w,dw)) ) dw=vec_scale(vec_proj(ball->w,dw),-2.0);
        /* ang mom change due to friction (and appropriate speed change) */
        dv      = vec_scale( vec_unit(vec_add(vec_cross(ball->w,vec_scale(n,-ball->d/2.0)),vp)), -vec_abs(dv)*MU_BANDE );
        if( !options_jump_shots ) dv.z = 0.0;
        dv_tot  = vec_add(dv_tot,dv);
        dw      = vec_cross(vec_scale(dv,ball->m/2.0/ball->I),vec_scale(n,ball->d));
        /* ... */
        ball->v = vec_add(ball->v,dv_tot);
        ball->w = vec_add(ball->w,dw);
    } else if(wall->pnr==1){
        n       = vec_unit( vec_diff( ball->r, wall->r1 ) );
        dv      = vec_scale(n,vec_mul(n,ball->v)*2.0);
        ball->v = vec_diff(ball->v,dv);
    }

#undef CUSHION_LOSS
#undef CUSHION_LOSS_O1
#undef MU_BANDE
}
#endif

static void ball_wall_interaction( BallType * ball, BorderType * wall )
{
//#define CUSHION_LOSS         0.2     /* const loss of energy by cushion */
//#define CUSHION_LOSS_O1      0.07    /* linear (in speed) loss of energy by cushion */
//#define MU_BANDE             0.1     /* friction const between cusion an ball */
#define CUSHION_LOSS_O0      (wall->loss0)       /* const loss of energy by cushion */
#define CUSHION_MAX_LOSS     (wall->loss_max)    /* const loss of energy by cushion */
#define CUSHION_LOSS_WSPEED  (wall->loss_wspeed) /* prop. halbwertsgeschwindigkeit */
#define MU_BANDE             (wall->mu)          /* friction const between cusion an ball */

    myvec dv, dw, dr, n, hit_normal;

    if       (wall->pnr==3){
        hit_normal=wall->n;
    } else if(wall->pnr==2){
        dr      = vec_diff( wall->r2, wall->r1 );
        hit_normal = vec_diff( ball->r, wall->r1 );
        hit_normal = vec_unit( vec_diff( hit_normal, vec_proj(hit_normal,dr) ) );
    } else if(wall->pnr==1){
        hit_normal = vec_unit( vec_diff( ball->r, wall->r1 ) );
    }

    if       (wall->pnr==3 || wall->pnr==2 || wall->pnr==1){
        myvec vp, vn;
        double loss;

        vn      = vec_proj(ball->v, hit_normal);
        vp      = vec_diff(ball->v, vn);
        //        dv      = vec_scale(wall->n,vec_mul(wall->n,ball->v)*2.0);

        /* normal component */
        loss = CUSHION_LOSS_O0+
               (CUSHION_MAX_LOSS-CUSHION_LOSS_O0)*(1.0-exp(-vec_abs(vn)/CUSHION_LOSS_WSPEED));
//        loss    = CUSHION_LOSS;
        dv      = vec_scale( vn, -(1.0+sqrt(1.0-loss)) );
        ball->v = vec_add(ball->v,dv);
/*        dv      = vec_scale( vn, -(1.0+sqrt( 1.0-(1.0-exp(-vec_abs(vn)*CUSHION_LOSS_O1)) )) );
        ball->v = vec_add(ball->v,dv);*/

        /* parallel component */
        dv      = vec_scale( vec_unit(vec_add(perimeter_speed_normal(ball, hit_normal),vp)), -vec_abs(dv)*MU_BANDE );
        dw      = vec_cross(vec_scale(dv,ball->m/2.0/ball->I),vec_scale(hit_normal,ball->d));
        {
            myvec dw2;
            dw2 = vec_add(dw,vec_proj(ball->w,dw));
            DPRINTF(" dw=%f,%f,%f\n  w=%f,%f,%f\n",dw.x,dw.y,dw.z,ball->w.x,ball->w.y,ball->w.z);
            if( vec_mul(dw2,ball->w) < 0.0 ){
                DPRINTF("cutting down dw,df\n");
                dw=vec_diff(dw,dw2);
                dv = vec_scale(vec_unit(dv),vec_abs(dw)*2.0*ball->I/ball->m/ball->d);
            }
        }
        ball->w = vec_add(ball->w,dw);
//        ball->w = vec_xyz(0.0,0.0,0.0);
#if options_jump_shots
#else
        dv.z    = 0.0;
#endif
//        printf("dv=(%f,%f,%f)\n",vn.x,vn.y,vn.z);
        ball->v = vec_add(ball->v,dv);

//        fprintf(stderr,"dv=(%f,%f,%f)",dv.x,dv.y,dv.z);
        /* maybe some angular momentum loss has to be implemented here */
        /* ... */
    } else if(wall->pnr==2){
        dr      = vec_diff( wall->r2, wall->r1 );
        n       = vec_diff( ball->r, wall->r1 );
        n       = vec_unit( vec_diff( n, vec_proj(n,dr) ) );
        dv      = vec_scale(n,vec_mul(n,ball->v)*2.0);
        ball->v = vec_diff(ball->v,dv);
        ball->w = vec_xyz(0.0,0.0,0.0);
    } else if(wall->pnr==1){
        n       = vec_unit( vec_diff( ball->r, wall->r1 ) );
        dv      = vec_scale(n,vec_mul(n,ball->v)*2.0);
        ball->v = vec_diff(ball->v,dv);
    }

#undef CUSHION_LOSS
#undef CUSHION_LOSS_O1
#undef MU_BANDE
}

#else

static void ball_wall_interaction( BallType * ball, BorderType * wall )
{
    myvec dv;
    dv      = vec_scale(wall->n,vec_mul(wall->n,ball->v)*(1.0+sqrt(1.0-CUSHION_LOSS)));
    ball->v = vec_diff(ball->v,dv);
    /* maybe some angular momentum loss has to be implemented here */
    /* ... */
}

#endif

#define MU_OMEGA             0.1      /* transmitted ang. mom. ratio (at hit) */


#define MU_BALL              0.1      /* friction const between ball and ball */

static void ball_ball_interaction( BallType * b1, BallType * b2 )
{
    BallType b1s, b2s;  /* balls in coord system of b1 */
    myvec dvec, duvec, dvn, dvp, dw1, dw2;
    myvec dw1max, dw2max;
    myvec dpp, dpn, fric_dir,perimeter_speed_b1, perimeter_speed_b2;

    dvec = vec_diff(b1->r,b2->r);
    duvec = vec_unit(dvec);

    /* stoss */
    b1s=*b1; b2s=*b2;
    b2s.v.x-=b1s.v.x;   b2s.v.y-=b1s.v.y;   b2s.v.z-=b1s.v.z;
    b1s.v.x=0.0;        b1s.v.y=0.0;        b1s.v.z=0.0;

    dvn   = vec_scale(duvec,vec_mul(duvec,b2s.v));
    dvp   = vec_diff( b2s.v, dvn );

    b2s.v = vec_diff( b2s.v, dvn );
    b2s.v = vec_add ( b2s.v, vec_scale(dvn,(b2s.m-b1s.m)/(b1s.m+b2s.m)) );
    b1s.v = vec_scale( dvn, 2.0*b2s.m/(b1s.m+b2s.m) );  /* (momentum transfer)/m1 */
    b2->v = vec_add( b1->v, b2s.v );
    b1->v = vec_add( b1->v, b1s.v );

//    dw1 = vec_scale(b1->w,MU_OMEGA);
//    dw2 = vec_scale(b2->w,MU_OMEGA);

    /* angular momentum transfer */
    dpn  = vec_scale(b1s.v,b1s.m); /* momentum transfer from ball2 to ball1 */
    perimeter_speed_b1=vec_cross( b1->w, vec_scale(duvec,-b1->d/2.0) );
    perimeter_speed_b2=vec_cross( b2->w, vec_scale(duvec, b2->d/2.0) );
    fric_dir = vec_unit(vec_add(vec_diff(perimeter_speed_b2,perimeter_speed_b1),dvp));
    dpp = vec_scale(fric_dir,-vec_abs(dpn)*MU_BALL);  /* dp parallel of ball2 */
    dw2 = vec_scale( vec_cross(dpp,duvec), b2->d/b2->I );
    dw1 = vec_scale( vec_cross(dpp,duvec), b1->d/b1->I );
    dw2max = vec_scale(vec_proj(vec_diff(b2->w,b1->w),dw2),0.5);
    dw1max = vec_scale(vec_proj(vec_diff(b1->w,b2->w),dw2),0.5);
    if( vec_abs(dw1)>vec_abs(dw1max) || vec_abs(dw2)>vec_abs(dw2max) ){
        /* correct momentum transfer to max */
        dpp = vec_scale(dpp,vec_abs(dw2max)/vec_abs(dw2));
        /* correct amg mom transfer to max */
        dw2=dw2max;
        dw1=dw1max;
    }
    b1->w = vec_diff( b1->w,dw1 );
    b2->w = vec_diff( b2->w,dw2 );

    /* parallel momentum transfer due to friction between balls */
    {
        myvec dv1, dv2;
        dv1   = vec_scale(dpp,-b1->m);
        dv2   = vec_scale(dpp, b2->m);
        dv1.z=0.0;
        dv2.z=0.0;
        b1->v = vec_add( b1->v, dv1 );
        b2->v = vec_add( b2->v, dv2 );
    }


//    b1->w = vec_diff( vec_diff(b1->w,dw1), vec_scale(dw2,(b2->I)/(b1->I)) );
//    b2->w = vec_diff( vec_diff(b2->w,dw2), vec_scale(dw1,(b1->I)/(b2->I)) );
}


#define  D_MIN               1.0E-5      /* 1E-5 = 1/100 mm */
#define  V_MIN               1.0E-2      /* 1E-3 = 1 mm/s */
#define  SLIDE_THRESH_SPEED  1.0E-2      /* 1E-2 = 1 cm/s */


static int ball_in_hole(BallType *ball, BordersType *borders)
{
    int i;
    for(i=0;i<borders->holenr;i++){
        if( vec_abs(vec_diff(borders->hole[i].pos,ball->r)) <
            borders->hole[i].r/* - ball->d/2.0*/ ){
            return i+1;
        }
    }
    return 0;
}


int ball_outside( BallType *ball, BordersType *borders )
{
    int rval=0;
    int i;
    for(i=0;i<borders->nr;i++){
        rval = rval || BALL_WALL_DIST_I(ball[0],borders->border[i])<0.0;
    }
    return rval;
}


void remove_balls_from_game( BallsType *balls, BordersType *borders )
{
    int i;
    for(i=0;i<balls->nr;i++){
/*        if( ball_in_hole(&balls->ball[i], borders) ){
            balls->ball[i].v.z-=
        }*/
/*        if( ball_outside(&balls->ball[i],borders) ){*/
        if( balls->ball[i].r.z < -0.1 ){
            balls->ball[i].in_game=0;
            balls->ball[i].in_hole=0;
            balls->ball[i].v = vec_xyz( 0.0, 0.0, 0.0 );
            balls->ball[i].r = vec_xyz( 0.0, 0.0, 0.0 );
            if( balls->ball[i].nr==0 ) move_log.out_white=1;
            if( balls->ball[i].nr==8 ) move_log.out_black=1;
            if( balls->ball[i].nr<8 && balls->ball[i].nr!=0 ) move_log.out_full++;
            if( balls->ball[i].nr>8 && balls->ball[i].nr!=0 ) move_log.out_half++;
/*            move_log.event[move_log.eventnr].event=BALL_out;
            move_log.event[move_log.eventnr].ballnr=balls->ball[i].nr;
            move_log.event[move_log.eventnr].ballnr2=0;
            move_log.event[move_log.eventnr].pos=balls->ball[i].r;
            move_log.event[move_log.eventnr].timestep_nr=move_log.timestep_nr;
            move_log.eventnr++;*/
            record_move_log_event( BALL_out, balls->ball[i].nr, 0, balls, 0.0 );
        }
    }
}


static int proceed_dt_euler(BallsType *balls, BordersType *borders, BMfloat dt, int depth)
/* this one does not remove fallen balls */
{
    myvec dx;
    BMfloat dphi, dt1;
    int i,j,k;
    int rnd[100];
    int collided=0;

//    if (depth>500) return 0;

//    fprintf(stderr,"proceed_dt_euler: depth=%d\n",depth);

    /* move all balls */
    for(i=0;i<balls->nr;i++){
        /* translate ball */
        dx = vec_scale(balls->ball[i].v,dt);
        balls->ball[i].r = vec_add(balls->ball[i].r,dx);
        /* perform rotation */
        dphi = vec_abs(balls->ball[i].w)*dt;
        //        rot_ax( balls->ball[i].w, balls->ball[i].b, 3, -dphi );
        if( dphi > 1.0E-30 )
            rot_ax( balls->ball[i].w, balls->ball[i].b, 3, dphi );
    }
    calc_rand_table(rnd,balls->nr);
    /* checks */
    for(i=0;i<balls->nr;i++) if(balls->ball[i].in_game){ /* check for one collision */
        /* check wall collisions */
        for(j=0;j<borders->nr;j++){
//            fprintf(stderr,"1:bwdist=%f\n",BALL_WALL_DIST(balls->ball[i],borders->border[j]));
            if( BALL_WALL_DIST(balls->ball[i],borders->border[j]) < balls->ball[i].d/2.0 && !ball_in_hole(&(balls->ball[i]),borders) ){
//                fprintf(stderr,"found ball-border collision\n");
                dt1=calc_wall_collision_time(&balls->ball[i],&borders->border[j]);
                /* dt1 should be negative */
//#endif
//                fprintf(stderr,"dt1=%f j=%d\n",dt1,j);
                if( dt1>0.0 || fabs(dt)>dt ){
                    if( dt1>0.0 ){
//                        fprintf(stderr,"wall: dt1>0 !!!\n");
                    }
                    if( fabs(dt)>dt ){
//                        fprintf(stderr,"wall: |dt1| > dt\n");
                    }
                } else {
                    proceed_dt_euler(balls,borders,dt1,depth+1); /* move back to coll. */
//                    fprintf(stderr,"bwdist=%f\n",fabs(BALL_WALL_DIST(balls->ball[i],borders->border[j])));
                    if ( fabs(BALL_WALL_DIST(balls->ball[i],borders->border[j])-balls->ball[i].d/2.0) < 1E-5 /*1E-5 = 1/100 mm*/ ){
                        /* only if ball still interacts */
                        /* (ball may seem to collide with wall but when stepping back out of */
                        /* wall region (wall border)) */
                        ball_wall_interaction(&balls->ball[i],&borders->border[j]);
                        collided=1;
                    }
                    proceed_dt_euler(balls,borders,-dt1,depth+1); /* move rest */
//                    if( collided ) break;
                    /*break;*/ /* no break because of second freeze bug */
                }
            }
        }

        /* check ball collision */
        if (!collided) for(j=0,k=rnd[0];j<balls->nr;j++,k=rnd[j]) if(k!=i){
            if( BALL_BALL_DIST( balls->ball[i], balls->ball[k] ) < (balls->ball[i].d+balls->ball[k].d)/2.0 ){
//                fprintf(stderr,"found ball-ball collision\n");
                dt1=calc_ball_collision_time(&balls->ball[i],&balls->ball[k]); /* dt1 should be negative */
                if( dt1>0.0 || fabs(dt)>dt ){
                    if( dt1>0.0 ){
//                        fprintf(stderr,"ball: dt1>0 !!!\n");
                    }
                    if( fabs(dt)>dt ){
//                        fprintf(stderr,"ball: |dt1| > dt\n");
                    }
                } else {
                    proceed_dt_euler(balls,borders,dt1,depth+1); /* move back to coll. */
                    if ( fabs( BALL_BALL_DIST( balls->ball[i], balls->ball[k] ) - (balls->ball[i].d+balls->ball[k].d)/2.0 ) < 1E-5 /*1E-5 = 1/100 mm*/ ){
                        /* only if balls still interact */
                        ball_ball_interaction(&balls->ball[i],&balls->ball[k]);
                    }
                    proceed_dt_euler(balls,borders,-dt1,depth+1); /* move rest */
                    break;
                }
            }
        }
    }
//    fprintf(stderr,"proceed_dt_euler: end: depth=%d\n",depth);
    return(0);
}


static void proceed_dt_only(BallsType *balls, BordersType *borders, BMfloat dt)
{
    myvec dx;
    BMfloat dphi;
    int i;

    for(i=0;i<balls->nr;i++){
        /* translate ball */
        dx = vec_scale(balls->ball[i].v,dt);
        balls->ball[i].r = vec_add(balls->ball[i].r,dx);
        /* perform rotation */
        dphi = vec_abs(balls->ball[i].w)*dt;
        //        rot_ax( balls->ball[i].w, balls->ball[i].b, 3, -dphi );
        if( dphi > 1.0E-30 )
            rot_ax( balls->ball[i].w, balls->ball[i].b, 3, dphi );
    }
}


static int wall_dv_pos( BallType * ball, BorderType * wall, BMfloat dt )
/* returns 1 if border and ball strobe away fromeach other, 0 else (at time dt) */
{
    static VMvect ballpos;
    if       ( wall->pnr == 3 ){
        return( vec_mul(ball->v,wall->n) > 0.0 );
    } else if( wall->pnr == 2 ){
        ballpos = vec_add(ball->r,vec_scale(ball->v,dt));
        return(
                vec_mul(ball->v,
                        vec_ncomp(
                                  vec_diff(ballpos, wall->r1),
                                  vec_diff(wall->r2,wall->r1)
                                 )
                       ) > 0.0
              );
    } else if( wall->pnr == 1 ){
        ballpos = vec_add(ball->r,vec_scale(ball->v,dt));
        return( vec_mul(vec_diff(ballpos,wall->r1),ball->v) > 0.0 );
    }
    return 1;
}

static int ball_dv_pos( BallType * b1, BallType * b2, BMfloat dt )
/* returns 1 if balls strobe away fromeach other, 0 else (at time dt) */
{
    static VMvect b1pos, b2pos;
    b1pos = vec_add(b1->r,vec_scale(b1->v,dt));
    b2pos = vec_add(b2->r,vec_scale(b2->v,dt));
    return( vec_mul(vec_diff(b2pos,b1pos),vec_diff(b2->v,b1->v)) > 0.0 );
}


static int proceed_dt_euler_new(BallsType *balls, BordersType *borders, BMfloat dt, int depth)
/* this one does not remove fallen balls */
{
    static BMfloat logtime;
    BMfloat dt1, dtmin;
    int i,j;
//    int rnd[100];
//    int collided=0;
//    int overlapnr,walloverlap,ball1overlap,ball2overlap;
    int colltype, collnr, collnr2;
#define COLLTYPE_WALL 1
#define COLLTYPE_BALL 2
#define COLLTYPE_NONE 0

    if(depth==0){
        logtime=dt;
    } else {
        logtime+=dt;
    }

//    if (depth>500) return 0;

//    fprintf(stderr,"proceed_dt_euler: depth=%d\n",depth);

    /* move all balls */
//    fprintf(stderr,"proceed_dt_euler: dt=%f depth=%d\n",dt, depth);
    proceed_dt_only(balls,borders,dt);

    /* checks */
    colltype=COLLTYPE_NONE; collnr=-1; collnr2=-1;
    dtmin=0.0;
    for(i=0;i<balls->nr;i++) if(balls->ball[i].in_game){ /* check for one collision */
        /* check wall collisions */
        for(j=0;j<borders->nr;j++){
            dt1=calc_wall_collision_time(&balls->ball[i],&borders->border[j]);
            if( dt1<dtmin && dt1>-dt ){
                if( !wall_dv_pos(&balls->ball[i],&borders->border[j],-dt*1.0) ){ /* dont strobe apart */
                    dtmin=dt1; colltype=COLLTYPE_WALL; collnr=j; collnr2=i;
                }
            }
        }
        /* check ball collisions */
        for(j=0;j<balls->nr;j++) if( j!=i && balls->ball[j].in_game ){
            dt1=calc_ball_collision_time(&balls->ball[i],&balls->ball[j]); /* dt1 should be negative */
            if( dt1<dtmin && dt1>-dt ){
                if( !ball_dv_pos(&balls->ball[j],&balls->ball[i],-dt*1.0) ){ /* dont strobe apart */
                    dtmin=dt1; colltype=COLLTYPE_BALL; collnr=j; collnr2=i;
                }
            }
        }
    }

    if( colltype==COLLTYPE_NONE ){
    } else if( colltype==COLLTYPE_WALL ){
//        fprintf(stderr,"proceed_dt_euler: wall collision dtmin=%f\n",dtmin);
//        if( !wall_dv_pos(&balls->ball[collnr2],&borders->border[collnr]) ){ /* dont strobe apart */
/*            record_move_log_event(BALL_wall,
                                  collnr,
                                  balls->ball[collnr2].nr,
                                  vec_xyz(0,0,0),
                                  balls->ball[collnr2].r);*/
/*            move_log.event[move_log.eventnr].event=BALL_wall;
            move_log.event[move_log.eventnr].ballnr=collnr;
            move_log.event[move_log.eventnr].ballnr2=balls->ball[collnr2].nr;
            move_log.event[move_log.eventnr].pos2=balls->ball[collnr2].r;
            move_log.event[move_log.eventnr].timestep_nr=move_log.timestep_nr;
            move_log.eventnr++;*/
            record_move_log_event( BALL_wall, collnr, balls->ball[collnr2].nr, balls, logtime+dtmin );

            logtime+=dtmin;
            proceed_dt_only(balls,borders,dtmin);
            ball_wall_interaction(&balls->ball[collnr2],&borders->border[collnr]);
            proceed_dt_euler_new(balls,borders,-dtmin,depth+1);
//        }
    } else if( colltype==COLLTYPE_BALL ){
//        fprintf(stderr,"proceed_dt_euler: ball collision\n");
//        if( !ball_dv_pos(&balls->ball[collnr],&balls->ball[collnr2]) ){ /* dont strobe apart */
/*            move_log.event[move_log.eventnr].event=BALL_ball;
            move_log.event[move_log.eventnr].ballnr=balls->ball[collnr].nr;
            move_log.event[move_log.eventnr].ballnr2=balls->ball[collnr2].nr;
            move_log.event[move_log.eventnr].pos=balls->ball[collnr].r;
            move_log.event[move_log.eventnr].pos2=balls->ball[collnr2].r;
            move_log.event[move_log.eventnr].timestep_nr=move_log.timestep_nr;
            move_log.eventnr++;*/
            record_move_log_event( BALL_ball, balls->ball[collnr].nr, balls->ball[collnr2].nr, balls, logtime+dtmin );

            logtime+=dtmin;
            proceed_dt_only(balls,borders,dtmin);
            ball_ball_interaction(&balls->ball[collnr],&balls->ball[collnr2]);
            proceed_dt_euler_new(balls,borders,-dtmin,depth+1);
//        }
    }

/*    if(depth!=0){
        logtime-=dt;
    }*/

    return(0);
}


#define GRAVITY            9.81      /* m/s^2 */
//#define MU_ROLL            0.01      /* table  roll-friction const */
//#define MU_SLIDE           0.1       /* table slide-friction const */
//#define SPOT_R             6.0e-3    /* 3mm radius der auflageflaeche - not used for rollmom (only rotational-friction around spot) */
#define MU_ROLL            0.03      /* table  roll-friction const */
#define MU_SLIDE           0.2       /* table slide-friction const */
#define SPOT_R             12.0e-3    /* 3mm radius der auflageflaeche - not used for rollmom (only rotational-friction around spot) */
//#define  OMEGA_MIN           M_PI/8.0   /* 22.5°/s */
#define OMEGA_MIN         SLIDE_THRESH_SPEED/SPOT_R   /* 22.5°/s */


myvec perimeter_speed( BallType * ball )
{
    return(
            vec_cross(
                      ball->w,
                      vec_xyz(0.0,0.0,-ball->d/2.0)
                     )
          );
}



int proceed_dt(BallsType *balls, BordersType *borders, BMfloat dt)
{
    int i,j;
    int balls_moving=0;
    myvec accel, waccel, uspeed, uspeed_eff, uspeed2, uspeed_eff2,
          fricaccel, fricmom, rollmom, totmom;
    double uspeed_eff_par, uspeed_eff2_par;


    move_log.timestep_nr++;
    move_log.duration+=dt;
    move_log.duration_last=dt;
    /* timestep with actual speeds, omegas,... */
    proceed_dt_euler_new(balls, borders, dt, 0);

    /* calc new accelerations and speeds */
    for(i=0;i<balls->nr;i++) if(balls->ball[i].in_game){

        /* check if balls still moving */
        if( vec_abs(balls->ball[i].v)!=0 || vec_abs(balls->ball[i].w)!=0 ){
            balls_moving=1;
        }

        /* calc accel 3D */
        accel  = vec_xyz(0.0,0.0,0.0); // init acceleration

        /* absolute and relative perimeter speed */
        uspeed = perimeter_speed(&balls->ball[i]);
        uspeed_eff = vec_add(uspeed,balls->ball[i].v);


        if( balls->ball[i].r.z<=0.0 ){ /* olny if  ball not flying do sliding/rolling */
        if( vec_abs( uspeed_eff ) > SLIDE_THRESH_SPEED ){  /* if sliding */

//            if(i==0) fprintf(stderr,"sliding\n");

            /* acc caused by friction */
            fricaccel = vec_scale( vec_unit(uspeed_eff), -MU_SLIDE*GRAVITY );
            accel = vec_add(accel, fricaccel);
            /* angular acc caused by friction */
            fricmom = vec_scale( vec_cross(fricaccel, vec_xyz(0.0,0.0,-balls->ball[i].d/2.0)), balls->ball[i].m );
            waccel = vec_scale( fricmom,-1.0/balls->ball[i].I );

            /* perform accel */
            balls->ball[i].w = vec_add( balls->ball[i].w, vec_scale(waccel,dt) );
            balls->ball[i].v = vec_add( balls->ball[i].v, vec_scale(accel,dt) );
            uspeed2 = perimeter_speed(&balls->ball[i]);
            uspeed_eff2 = vec_add(uspeed2,balls->ball[i].v);
            /* if uspeed_eff passes 0 */
            //            if( vec_angle(uspeed_eff2,uspeed_eff) > M_PI/2.0 ){
            uspeed_eff_par  = vec_mul( uspeed_eff,  vec_diff(uspeed_eff,uspeed_eff2) );
            uspeed_eff2_par = vec_mul( uspeed_eff2, vec_diff(uspeed_eff,uspeed_eff2) );
            if( vec_ndist(vec_null(),uspeed_eff,uspeed_eff2) <= SLIDE_THRESH_SPEED &&
                ( (uspeed_eff_par > 0.0 && uspeed_eff2_par < 0.0) ||
                  (uspeed_eff2_par > 0.0 && uspeed_eff_par < 0.0) )   ){
                /* make rolling if uspeed_eff passed 0 */
/*                myvec dummy;
                dummy = vec_ncomp(vec_xyz(0.0,0.0,-balls->ball[i].d/2.0),balls->ball[i].w);
                balls->ball[i].w = vec_scale(vec_cross(balls->ball[i].v,dummy),
                                             1.0/vec_abssq(dummy));*/
                balls->ball[i].v = vec_cross( balls->ball[i].w,
                                              vec_xyz(0.0,0.0,balls->ball[i].d/2.0) );
//                if(i==0) fprintf(stderr,"made rolling\n");
            }
            if( vec_abs(balls->ball[i].w) < OMEGA_MIN &&
                vec_abs(balls->ball[i].v) < SLIDE_THRESH_SPEED ){
                balls->ball[i].v = vec_xyz( 0.0, 0.0, 0.0 );
                balls->ball[i].w = vec_xyz( 0.0, 0.0, 0.0 );
            }

        } else {                                             /* rolling forces */

            fricmom = vec_xyz(0.0,0.0,0.0);

//            if(i==0) fprintf(stderr,"rolling\n");

//            if( fabs(vec_mul(vec_xyz(0.0,0.0,SPOT_R),balls->ball[i].w)) > SLIDE_THRESH_SPEED ){
            /* moment of rotation around ballspot */
            if( fabs(vec_mul(vec_xyz(0.0,0.0,1.0),balls->ball[i].w)) > OMEGA_MIN ){
                fricmom = vec_add(fricmom,
                                  vec_scale(
                                            vec_unit(vec_xyz(0.0,0.0,balls->ball[i].w.z)),
                                            MU_SLIDE*balls->ball[i].m*GRAVITY*SPOT_R
                                           )
                                 );
            } else {
            }


//            #define ROLL_MOM_R MU_ROLL*balls->ball[i].d/5.0   /* wirkabstand von rollwid.-kraft */
            /* wirkabstand von rollwid.-kraft */
            #define ROLL_MOM_R MU_ROLL*balls->ball[i].I/balls->ball[i].m/balls->ball[i].d

            rollmom = vec_cross(
                                vec_xyz(0.0,0.0,balls->ball[i].m*GRAVITY * ROLL_MOM_R),
                                vec_unit(balls->ball[i].v)
                               );

            totmom = vec_add(fricmom,rollmom);
            waccel = vec_scale( totmom,-1.0/balls->ball[i].I );

            balls->ball[i].w = vec_add( balls->ball[i].w, vec_scale(waccel,dt) );
            /* align v with w to assure rolling */
            balls->ball[i].v = vec_cross( balls->ball[i].w,
                                          vec_xyz(0.0,0.0,balls->ball[i].d/2.0) );

/*            if( vec_abs(balls->ball[i].v) < SLIDE_THRESH_SPEED ){
                balls->ball[i].v = vec_xyz( 0.0, 0.0, 0.0 );
                balls->ball[i].w = vec_xyz( 0.0, 0.0, 0.0 );
            }*/

            if( vec_abs(balls->ball[i].w) < OMEGA_MIN &&
                vec_abs(balls->ball[i].v) < SLIDE_THRESH_SPEED ){
                balls->ball[i].v = vec_xyz( 0.0, 0.0, 0.0 );
                balls->ball[i].w = vec_xyz( 0.0, 0.0, 0.0 );
            }
//            balls->ball[i].v = vec_add( balls->ball[i].v, vec_xyz(0.0,0.0,-9.81*dt) );

        }
        }

#if options_jump_shots
//#if options_jump_shots
        /*GRAVITY*/
        if( fabs(balls->ball[i].r.z)<0.001 && fabs(balls->ball[i].r.z)>0.00001 && balls->ball[i].v.z<0.4 && !ball_in_hole(&balls->ball[i], borders) ){
            balls->ball[i].r.z=0.0;
            balls->ball[i].v.z=0.0;
        }
        if( balls->ball[i].r.z!=0.0 && balls->ball[i].v.z!=0.0 ){ /* if flying */
            balls->ball[i].v.z += -GRAVITY*dt;
        }
        /* air resistance */
        {
            float v, dv;
            v=vec_abs(balls->ball[i].v);
            if(v!=0.0){
                dv=v*0.001;
                if( dv > v ){
                    balls->ball[i].v = vec_xyz(0.0,0.0,0.0);
                } else {
                    balls->ball[i].v = vec_diff(balls->ball[i].v,vec_scale(balls->ball[i].v,dv/v));
                }
            }
        }
#endif


        /* calc waccel 3D */
//        waccel = vec_scale( vec_add(vec_scale(fricmom,+1.0),rollmom),-1.0/balls->ball[i].I*dt);
//        waccel = vec_scale( vec_proj(fricmom,balls->ball[i].w),
//                            -1.0/balls->ball[i].I*dt );
//        waccel = vec_scale( rollmom,-1.0/balls->ball[i].I*dt);
//        waccel = vec_scale(balls->ball[i].w,-0.1*dt); // fix this

        /* w - omega */
//        balls->ball[i].w = vec_add( balls->ball[i].w, vec_scale(waccel,dt) );
//        if( vec_abs(balls->ball[i].w) < OMEGA_MIN )
//            balls->ball[i].w = vec_xyz( 0.0, 0.0, 0.0 );
        /* v - speed */
/*        balls->ball[i].v = vec_add( balls->ball[i].v, vec_scale(accel,dt) );
        if( vec_abs(balls->ball[i].v) < SLIDE_THRESH_SPEED ){
            balls->ball[i].v = vec_xyz( 0.0, 0.0, 0.0 );
            balls->ball[i].w = vec_xyz( 0.0, 0.0, 0.0 );
        }*/
    }

    remove_balls_from_game( balls, borders );

    for(i=0;i<balls->nr;i++){
        if( (j=ball_in_hole(&balls->ball[i], borders))!=0 ){
            balls->ball[i].in_hole=j;
        }
        if( (j=balls->ball[i].in_hole)!=0 ){
            myvec dr;
            balls->ball[i].v.z-=GRAVITY*dt;
//            balls->ball[i].v.x=0.0;
//            balls->ball[i].v.y=0.0;
//            balls->ball[i].r.x=0.2*borders->hole[j-1].pos.x+0.8*balls->ball[i].r.x;
            //            balls->ball[i].r.y=0.2*borders->hole[j-1].pos.y+0.8*balls->ball[i].r.y;
            dr=vec_diff(balls->ball[i].r,borders->hole[j-1].pos);
            dr.z=0;
            if( vec_abs(dr) > borders->hole[j-1].r ){
                double z;
                balls->ball[i].v=vec_diff(balls->ball[i].v,vec_scale(vec_proj(balls->ball[i].v,dr),2.0));
                dr = vec_scale( vec_unit(dr), borders->hole[j-1].r );
                z=balls->ball[i].r.z;
                balls->ball[i].r=vec_add(borders->hole[j-1].pos, dr);
//                balls->ball[i].r=borders->hole[j-1].pos;
                balls->ball[i].r.z=z;
            }
        }
    }

    return balls_moving;
}


static void realloc_path( BallType * pball, int plussize )
{
    VMvect * newpath;
    newpath = malloc((pball->pathsize+plussize)*sizeof(VMvect));
    memcpy( newpath, pball->path, pball->pathcnt*sizeof(VMvect) );
    free(pball->path);
    pball->path=newpath;
    pball->pathsize+=plussize;
}


void BM_add2path( BallType *pball )
{
    if( pball->pathsize <= pball->pathcnt ) realloc_path(pball, 1024);
    pball->path[pball->pathcnt]=pball->r;
    pball->pathcnt++;
}


void BM_clearpath( BallType *pball )
{
    pball->pathcnt=0;
}
