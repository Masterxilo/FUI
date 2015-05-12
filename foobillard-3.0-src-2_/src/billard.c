/* billard.c
**
**    code for positioning the balls
**    Copyright (C) 2001  Florian Berger
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

#define BILLARD_C
#include "billard.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


void (*create_scene)( BallsType * balls ) = create_8ball_scene;
void (*create_walls)( BordersType * walls ) = create_6hole_walls;
void * (*billard_malloc)( size_t size ) = malloc;
void (*billard_free)( void * ptr ) = free;

void setfunc_create_scene( void (*func)( BallsType * balls ) )
{
    create_scene=func;
}


void setfunc_create_walls( void (*func)( BordersType * walls ) )
{
    create_walls=func;
}


void setfunc_malloc_free( void * (*func_malloc)( size_t size ), void (*func_free)( void * ptr ) )
{
    billard_malloc = func_malloc;
    billard_free   = func_free;
}


/* positions:            */
/* ==========            */
/*   11  12  13  14  15  */
/*     07  08  09  10    */
/*       04  05  06      */
/*         02  03        */
/*           01          */

void place8ballnrs( BallsType * balls )
{
    int i,j,act;
    int ok=1;

    do{
        for(i=0;i<balls->nr;i++){
            if(i==0){
                balls->ball[i].nr=0;
            }else if(i==5){
                balls->ball[i].nr=8;
            }else{
                int ok;
                act = rand() % balls->nr;
                do {
                    ok=1;
                    act = (act+1) % balls->nr;
                    DPRINTF("   trying %d\n",act);
                    for(j=0;j<i;j++){
                        if( act==balls->ball[j].nr ){ ok=0; break; }
                    }
                    if( act == 8 || act == 0 ) ok=0;
                } while(!ok);
                balls->ball[i].nr=act;
            }
            DPRINTF("i=%d: ball#=%d\n",i,balls->ball[i].nr);
        }
    }while(!ok);
    for(i=0;i<balls->nr;i++){
        DPRINTF("i=%d: ball#=%d\n",i,balls->ball[i].nr);
    }
}


void create_6hole_walls( BordersType * walls )
{
    int i;

    /* borders */
#ifdef USE_ADV_BORDER
    /* borders */
//    walls->nr=30;
    walls->nr=32;
    if( walls->border != NULL ) billard_free( walls->border );
    walls->border = billard_malloc( sizeof(BorderType)*walls->nr );

    /* bonds */
    walls->border[0].pnr = 3;
    walls->border[0].r1 = vec_xyz( +TABLE_W/2.0, +TABLE_L/2.0-HOLE1_W/SQR2, -BALL_D/2.0 );
    walls->border[0].r2 = vec_xyz( +TABLE_W/2.0,              +HOLE2_W/2.0, 0.0 );
    walls->border[0].r3 = vec_xyz( +TABLE_W/2.0, +TABLE_L/2.0-HOLE1_W/SQR2, BALL_D/2.0 );
    walls->border[0].n  = vec_xyz( -1.0, 0.0, 0.0 );

    walls->border[1].pnr = 3;
    walls->border[1].r1 = vec_xyz( +TABLE_W/2.0, -TABLE_L/2.0+HOLE1_W/SQR2, -BALL_D/2.0 );
    walls->border[1].r2 = vec_xyz( +TABLE_W/2.0,              -HOLE2_W/2.0, 0.0 );
    walls->border[1].r3 = vec_xyz( +TABLE_W/2.0, -TABLE_L/2.0+HOLE1_W/SQR2, BALL_D/2.0 );
    walls->border[1].n  = vec_xyz( -1.0, 0.0, 0.0 );

    walls->border[2].pnr = 3;
    walls->border[2].r1 = vec_xyz( -TABLE_W/2.0, +TABLE_L/2.0-HOLE1_W/SQR2, -BALL_D/2.0 );
    walls->border[2].r2 = vec_xyz( -TABLE_W/2.0,              +HOLE2_W/2.0, 0.0 );
    walls->border[2].r3 = vec_xyz( -TABLE_W/2.0, +TABLE_L/2.0-HOLE1_W/SQR2, BALL_D/2.0 );
    walls->border[2].n  = vec_xyz( +1.0, 0.0, 0.0 );

    walls->border[3].pnr = 3;
    walls->border[3].r1 = vec_xyz( -TABLE_W/2.0, -TABLE_L/2.0+HOLE1_W/SQR2, -BALL_D/2.0 );
    walls->border[3].r2 = vec_xyz( -TABLE_W/2.0,              -HOLE2_W/2.0, 0.0 );
    walls->border[3].r3 = vec_xyz( -TABLE_W/2.0, -TABLE_L/2.0+HOLE1_W/SQR2, BALL_D/2.0 );
    walls->border[3].n  = vec_xyz( +1.0, 0.0, 0.0 );

    walls->border[4].pnr = 3;
    walls->border[4].r1 = vec_xyz( -TABLE_W/2.0+HOLE1_W/SQR2, +TABLE_L/2.0, -BALL_D/2.0 );
    walls->border[4].r2 = vec_xyz( +TABLE_W/2.0-HOLE1_W/SQR2, +TABLE_L/2.0, 0.0 );
    walls->border[4].r3 = vec_xyz( -TABLE_W/2.0+HOLE1_W/SQR2, +TABLE_L/2.0, BALL_D/2.0 );
    walls->border[4].n  = vec_xyz( 0.0, -1.0, 0.0 );

    walls->border[5].pnr = 3;
    walls->border[5].r1 = vec_xyz( -TABLE_W/2.0+HOLE1_W/SQR2, -TABLE_L/2.0, -BALL_D/2.0 );
    walls->border[5].r2 = vec_xyz( +TABLE_W/2.0-HOLE1_W/SQR2, -TABLE_L/2.0, 0.0 );
    walls->border[5].r3 = vec_xyz( -TABLE_W/2.0+HOLE1_W/SQR2, -TABLE_L/2.0, BALL_D/2.0 );
    walls->border[5].n  = vec_xyz( 0.0, +1.0, 0.0 );

    /* edges */
    /* upper right */
    walls->border[6].pnr = 2;
    walls->border[6].r1 = vec_xyz( +TABLE_W/2.0, +TABLE_L/2.0-HOLE1_W/SQR2, -BALL_D/2.0 );
    walls->border[6].r2 = vec_xyz( +TABLE_W/2.0, +TABLE_L/2.0-HOLE1_W/SQR2, BALL_D/2.0 );
    walls->border[18].pnr = 3;
    walls->border[18].r1 = vec_xyz( +TABLE_W/2.0, +TABLE_L/2.0-HOLE1_W/SQR2, -BALL_D/2.0 );
    walls->border[18].r2 = vec_xyz( +TABLE_W/2.0, +TABLE_L/2.0-HOLE1_W/SQR2, BALL_D/2.0 );
    walls->border[18].r3 = vec_xyz( +TABLE_W/2.0+1.0, +TABLE_L/2.0-HOLE1_W/SQR2+HOLE1_TAN, 0.0 );
    walls->border[18].n  = vec_unit(vec_cross(vec_diff(walls->border[18].r2,walls->border[18].r1),vec_diff(walls->border[18].r3,walls->border[18].r1)));

    /* upper right */
    walls->border[7].pnr = 2;
    walls->border[7].r1 = vec_xyz( +TABLE_W/2.0-HOLE1_W/SQR2, +TABLE_L/2.0, -BALL_D/2.0 );
    walls->border[7].r2 = vec_xyz( +TABLE_W/2.0-HOLE1_W/SQR2, +TABLE_L/2.0, BALL_D/2.0 );
    walls->border[19].pnr = 3;
    walls->border[19].r1 = vec_xyz( +TABLE_W/2.0-HOLE1_W/SQR2, +TABLE_L/2.0, -BALL_D/2.0 );
    walls->border[19].r2 = vec_xyz( +TABLE_W/2.0-HOLE1_W/SQR2, +TABLE_L/2.0, BALL_D/2.0 );
    walls->border[19].r3 = vec_xyz( +TABLE_W/2.0-HOLE1_W/SQR2+HOLE1_TAN, +TABLE_L/2.0+1.0, 0.0 );
    walls->border[19].n  = vec_unit(vec_cross(vec_diff(walls->border[19].r1,walls->border[19].r2),vec_diff(walls->border[19].r3,walls->border[19].r1)));

    /* upper left */
    walls->border[8].pnr = 2;
    walls->border[8].r1 = vec_xyz( -TABLE_W/2.0, +TABLE_L/2.0-HOLE1_W/SQR2, -BALL_D/2.0 );
    walls->border[8].r2 = vec_xyz( -TABLE_W/2.0, +TABLE_L/2.0-HOLE1_W/SQR2, BALL_D/2.0 );
    walls->border[20].pnr = 3;
    walls->border[20].r1 = vec_xyz( -TABLE_W/2.0, +TABLE_L/2.0-HOLE1_W/SQR2, -BALL_D/2.0 );
    walls->border[20].r2 = vec_xyz( -TABLE_W/2.0, +TABLE_L/2.0-HOLE1_W/SQR2, BALL_D/2.0 );
    walls->border[20].r3 = vec_xyz( -TABLE_W/2.0-1.0, +TABLE_L/2.0-HOLE1_W/SQR2+HOLE1_TAN, 0.0 );
    walls->border[20].n  = vec_unit(vec_cross(vec_diff(walls->border[20].r1,walls->border[20].r2),vec_diff(walls->border[20].r3,walls->border[20].r1)));

    /* upper left */
    walls->border[9].pnr = 2;
    walls->border[9].r1 = vec_xyz( -TABLE_W/2.0+HOLE1_W/SQR2, +TABLE_L/2.0, -BALL_D/2.0 );
    walls->border[9].r2 = vec_xyz( -TABLE_W/2.0+HOLE1_W/SQR2, +TABLE_L/2.0, BALL_D/2.0 );
    walls->border[21].pnr = 3;
    walls->border[21].r1 = vec_xyz( -TABLE_W/2.0+HOLE1_W/SQR2, +TABLE_L/2.0, -BALL_D/2.0 );
    walls->border[21].r2 = vec_xyz( -TABLE_W/2.0+HOLE1_W/SQR2, +TABLE_L/2.0, BALL_D/2.0 );
    walls->border[21].r3 = vec_xyz( -TABLE_W/2.0+HOLE1_W/SQR2-HOLE1_TAN, +TABLE_L/2.0+1.0, 0.0 );
    walls->border[21].n  = vec_unit(vec_cross(vec_diff(walls->border[21].r2,walls->border[21].r1),vec_diff(walls->border[21].r3,walls->border[21].r1)));

    /* lower right */
    walls->border[10].pnr = 2;
    walls->border[10].r1 = vec_xyz( +TABLE_W/2.0, -TABLE_L/2.0+HOLE1_W/SQR2, -BALL_D/2.0 );
    walls->border[10].r2 = vec_xyz( +TABLE_W/2.0, -TABLE_L/2.0+HOLE1_W/SQR2, BALL_D/2.0 );
    walls->border[22].pnr = 3;
    walls->border[22].r1 = vec_xyz( +TABLE_W/2.0, -TABLE_L/2.0+HOLE1_W/SQR2, -BALL_D/2.0 );
    walls->border[22].r2 = vec_xyz( +TABLE_W/2.0, -TABLE_L/2.0+HOLE1_W/SQR2, BALL_D/2.0 );
    walls->border[22].r3 = vec_xyz( +TABLE_W/2.0+1.0, -TABLE_L/2.0+HOLE1_W/SQR2-HOLE1_TAN, 0.0 );
    walls->border[22].n  = vec_unit(vec_cross(vec_diff(walls->border[22].r1,walls->border[22].r2),vec_diff(walls->border[22].r3,walls->border[22].r1)));

    /* lower right */
    walls->border[11].pnr = 2;
    walls->border[11].r1 = vec_xyz( +TABLE_W/2.0-HOLE1_W/SQR2, -TABLE_L/2.0, -BALL_D/2.0 );
    walls->border[11].r2 = vec_xyz( +TABLE_W/2.0-HOLE1_W/SQR2, -TABLE_L/2.0, BALL_D/2.0 );
    walls->border[23].pnr = 3;
    walls->border[23].r1 = vec_xyz( +TABLE_W/2.0-HOLE1_W/SQR2, -TABLE_L/2.0, -BALL_D/2.0 );
    walls->border[23].r2 = vec_xyz( +TABLE_W/2.0-HOLE1_W/SQR2, -TABLE_L/2.0, BALL_D/2.0 );
    walls->border[23].r3 = vec_xyz( +TABLE_W/2.0-HOLE1_W/SQR2+HOLE1_TAN, -TABLE_L/2.0-1.0, 0.0 );
    walls->border[23].n  = vec_unit(vec_cross(vec_diff(walls->border[23].r2,walls->border[23].r1),vec_diff(walls->border[23].r3,walls->border[23].r1)));

    /* lower left */
    walls->border[12].pnr = 2;
    walls->border[12].r1 = vec_xyz( -TABLE_W/2.0, -TABLE_L/2.0+HOLE1_W/SQR2, -BALL_D/2.0 );
    walls->border[12].r2 = vec_xyz( -TABLE_W/2.0, -TABLE_L/2.0+HOLE1_W/SQR2, BALL_D/2.0 );
    walls->border[24].pnr = 3;
    walls->border[24].r1 = vec_xyz( -TABLE_W/2.0, -TABLE_L/2.0+HOLE1_W/SQR2, -BALL_D/2.0 );
    walls->border[24].r2 = vec_xyz( -TABLE_W/2.0, -TABLE_L/2.0+HOLE1_W/SQR2, BALL_D/2.0 );
    walls->border[24].r3 = vec_xyz( -TABLE_W/2.0-1.0, -TABLE_L/2.0+HOLE1_W/SQR2-HOLE1_TAN, 0.0 );
    walls->border[24].n  = vec_unit(vec_cross(vec_diff(walls->border[24].r2,walls->border[24].r1),vec_diff(walls->border[24].r3,walls->border[24].r1)));

    /* lower left */
    walls->border[13].pnr = 2;
    walls->border[13].r1 = vec_xyz( -TABLE_W/2.0+HOLE1_W/SQR2, -TABLE_L/2.0, -BALL_D/2.0 );
    walls->border[13].r2 = vec_xyz( -TABLE_W/2.0+HOLE1_W/SQR2, -TABLE_L/2.0, BALL_D/2.0 );
    walls->border[25].pnr = 3;
    walls->border[25].r1 = vec_xyz( -TABLE_W/2.0+HOLE1_W/SQR2, -TABLE_L/2.0, -BALL_D/2.0 );
    walls->border[25].r2 = vec_xyz( -TABLE_W/2.0+HOLE1_W/SQR2, -TABLE_L/2.0, BALL_D/2.0 );
    walls->border[25].r3 = vec_xyz( -TABLE_W/2.0+HOLE1_W/SQR2-HOLE1_TAN, -TABLE_L/2.0-1.0, 0.0 );
    walls->border[25].n  = vec_unit(vec_cross(vec_diff(walls->border[25].r1,walls->border[25].r2),vec_diff(walls->border[25].r3,walls->border[25].r1)));

    /* middle left */
    walls->border[14].pnr = 2;
    walls->border[14].r1 = vec_xyz( -TABLE_W/2.0, HOLE2_W/2.0, -BALL_D/2.0 );
    walls->border[14].r2 = vec_xyz( -TABLE_W/2.0, HOLE2_W/2.0, BALL_D/2.0 );
    walls->border[26].pnr = 3;
    walls->border[26].r1 = vec_xyz( -TABLE_W/2.0, HOLE2_W/2.0, -BALL_D/2.0 );
    walls->border[26].r2 = vec_xyz( -TABLE_W/2.0, HOLE2_W/2.0, BALL_D/2.0 );
    walls->border[26].r3 = vec_xyz( -TABLE_W/2.0-1.0, HOLE2_W/2.0-HOLE2_TAN, 0.0 );
    walls->border[26].n  = vec_unit(vec_cross(vec_diff(walls->border[26].r2,walls->border[26].r1),vec_diff(walls->border[26].r3,walls->border[26].r1)));

    /* middle left */
    walls->border[15].pnr = 2;
    walls->border[15].r1 = vec_xyz( -TABLE_W/2.0, -HOLE2_W/2.0, -BALL_D/2.0 );
    walls->border[15].r2 = vec_xyz( -TABLE_W/2.0, -HOLE2_W/2.0, BALL_D/2.0 );
    walls->border[27].pnr = 3;
    walls->border[27].r1 = vec_xyz( -TABLE_W/2.0, -HOLE2_W/2.0, -BALL_D/2.0 );
    walls->border[27].r2 = vec_xyz( -TABLE_W/2.0, -HOLE2_W/2.0, BALL_D/2.0 );
    walls->border[27].r3 = vec_xyz( -TABLE_W/2.0-1.0, -HOLE2_W/2.0+HOLE2_TAN, 0.0 );
    walls->border[27].n  = vec_unit(vec_cross(vec_diff(walls->border[27].r1,walls->border[27].r2),vec_diff(walls->border[27].r3,walls->border[27].r1)));

    /* middle right */
    walls->border[16].pnr = 2;
    walls->border[16].r1 = vec_xyz( +TABLE_W/2.0, HOLE2_W/2.0, -BALL_D/2.0 );
    walls->border[16].r2 = vec_xyz( +TABLE_W/2.0, HOLE2_W/2.0, BALL_D/2.0 );
    walls->border[28].pnr = 3;
    walls->border[28].r1 = vec_xyz( +TABLE_W/2.0, HOLE2_W/2.0, -BALL_D/2.0 );
    walls->border[28].r2 = vec_xyz( +TABLE_W/2.0, HOLE2_W/2.0, BALL_D/2.0 );
    walls->border[28].r3 = vec_xyz( +TABLE_W/2.0+1.0, HOLE2_W/2.0-HOLE2_TAN, 0.0 );
    walls->border[28].n  = vec_unit(vec_cross(vec_diff(walls->border[28].r1,walls->border[28].r2),vec_diff(walls->border[28].r3,walls->border[28].r1)));

    /* middle right */
    walls->border[17].pnr = 2;
    walls->border[17].r1 = vec_xyz( +TABLE_W/2.0, -HOLE2_W/2.0, -BALL_D/2.0 );
    walls->border[17].r2 = vec_xyz( +TABLE_W/2.0, -HOLE2_W/2.0, BALL_D/2.0 );
    walls->border[29].pnr = 3;
    walls->border[29].r1 = vec_xyz( +TABLE_W/2.0, -HOLE2_W/2.0, -BALL_D/2.0 );
    walls->border[29].r2 = vec_xyz( +TABLE_W/2.0, -HOLE2_W/2.0, BALL_D/2.0 );
    walls->border[29].r3 = vec_xyz( +TABLE_W/2.0+1.0, -HOLE2_W/2.0+HOLE2_TAN, 0.0 );
    walls->border[29].n  = vec_unit(vec_cross(vec_diff(walls->border[29].r2,walls->border[29].r1),vec_diff(walls->border[29].r3,walls->border[29].r1)));

    /* friction constants and loss factors */
    for(i=0;i<walls->nr;i++){
        walls->border[i].mu          = 0.1;
        walls->border[i].loss0       = 0.2;
        walls->border[i].loss_max    = 0.5;
        walls->border[i].loss_wspeed = 4.0;  /* [m/s] */
    }

    /* table surface */
#define TABLE_W2 (TABLE_W-BALL_D*0.9)
#define TABLE_L2 (TABLE_L-BALL_D*0.9)
    walls->border[30].pnr = 3;
    walls->border[30].r1 = vec_xyz( -TABLE_W/2.0, -TABLE_L/2.0, -BALL_D/2.0-0.0001 );
    walls->border[30].r2 = vec_xyz( +TABLE_W/2.0, -TABLE_L/2.0, -BALL_D/2.0-0.0001 );
    walls->border[30].r3 = vec_xyz( +TABLE_W/2.0, +TABLE_L/2.0, -BALL_D/2.0-0.0001 );
    walls->border[30].n  = vec_xyz( 0.0, 0.0, 1.0 );

    walls->border[31].pnr = 3;
    walls->border[31].r1 = vec_xyz( -TABLE_W/2.0, -TABLE_L/2.0, -BALL_D/2.0-0.0001 );
    walls->border[31].r3 = vec_xyz( +TABLE_W/2.0, +TABLE_L/2.0, -BALL_D/2.0-0.0001 );
    walls->border[31].r2 = vec_xyz( -TABLE_W/2.0, +TABLE_L/2.0, -BALL_D/2.0-0.0001 );
    walls->border[31].n  = vec_xyz( 0.0, 0.0, 1.0 );
#undef TABLE_W2
#undef TABLE_L2

    walls->border[30].mu          = 0.2;
    walls->border[30].loss0       = 0.6;
    walls->border[30].loss_max    = 0.99;
    walls->border[30].loss_wspeed = 1.5;
    walls->border[31].mu          = 0.2;
    walls->border[31].loss0       = 0.6;
    walls->border[31].loss_max    = 0.99;
    walls->border[31].loss_wspeed = 1.5;

#else
    /* borders */
    walls->nr=4;
    walls->border = billard_malloc(sizeof(BorderType)*walls->nr);
    walls->border[0].r = vec_xyz( +TABLE_W/2.0, 0.0, 0.0 );
    walls->border[0].n = vec_xyz( -1.0, 0.0, 0.0 );
    walls->border[1].r = vec_xyz( -TABLE_W/2.0, 0.0, 0.0 );
    walls->border[1].n = vec_xyz( +1.0, 0.0, 0.0 );
    walls->border[2].r = vec_xyz( 0.0, +TABLE_L/2.0, 0.0 );
    walls->border[2].n = vec_xyz( 0.0, -1.0, 0.0 );
    walls->border[3].r = vec_xyz( 0.0, -TABLE_L/2.0, 0.0 );
    walls->border[3].n = vec_xyz( 0.0, +1.0, 0.0 );
#endif

    /* holes */
    walls->holenr = 6;
    if( walls->hole != NULL ) billard_free( walls->hole );
    walls->hole = billard_malloc(sizeof(HoleType)*walls->holenr);
    /* middle right */
    walls->hole[0].aim = vec_xyz( +TABLE_W/2.0-HOLE2_AIMOFFS, 0.0, 0.0 );
    walls->hole[0].pos = vec_xyz( +TABLE_W/2.0+HOLE2_XYOFFS, 0.0, 0.0 );
    walls->hole[0].r   = HOLE2_R;
    /* middle left */
    walls->hole[1].aim = vec_xyz( -TABLE_W/2.0+HOLE2_AIMOFFS, 0.0, 0.0 );
    walls->hole[1].pos = vec_xyz( -TABLE_W/2.0-HOLE2_XYOFFS, 0.0, 0.0 );
    walls->hole[1].r   = HOLE2_R;
    /* upper right */
    walls->hole[2].aim = vec_xyz( +TABLE_W/2.0-HOLE1_AIMOFFS, +TABLE_L/2.0-HOLE1_AIMOFFS, 0.0 );
    walls->hole[2].pos = vec_xyz( +TABLE_W/2.0+HOLE1_XYOFFS, +TABLE_L/2.0+HOLE1_XYOFFS, 0.0 );
    walls->hole[2].r   = HOLE1_R;
    /* upper left */
    walls->hole[3].aim = vec_xyz( -TABLE_W/2.0+HOLE1_AIMOFFS, +TABLE_L/2.0-HOLE1_AIMOFFS, 0.0 );
    walls->hole[3].pos = vec_xyz( -TABLE_W/2.0-HOLE1_XYOFFS, +TABLE_L/2.0+HOLE1_XYOFFS, 0.0 );
    walls->hole[3].r   = HOLE1_R;
    /* lower left */
    walls->hole[4].aim = vec_xyz( -TABLE_W/2.0+HOLE1_AIMOFFS, -TABLE_L/2.0-HOLE1_AIMOFFS, 0.0 );
    walls->hole[4].pos = vec_xyz( -TABLE_W/2.0-HOLE1_XYOFFS, -TABLE_L/2.0-HOLE1_XYOFFS, 0.0 );
    walls->hole[4].r   = HOLE1_R;
    /* lower right */
    walls->hole[5].aim = vec_xyz( +TABLE_W/2.0-HOLE1_AIMOFFS, -TABLE_L/2.0+HOLE1_AIMOFFS, 0.0 );
    walls->hole[5].pos = vec_xyz( +TABLE_W/2.0+HOLE1_XYOFFS, -TABLE_L/2.0-HOLE1_XYOFFS, 0.0 );
    walls->hole[5].r   = HOLE1_R;
}


void create_6hole_walls_snooker( BordersType * walls )
{
    _NOT_IMPLEMENTED
}


void create_0hole_walls( BordersType * walls )
{
    _NOT_IMPLEMENTED
}


void create_8ball_scene( BallsType * balls )
{
    int i;
    myvec dball1, dball2, vdummy;
//    double poserr=0.002;
    double poserr=0.007;

    balls->gametype=GAME_8BALL;
    /* balls */
    balls->nr=16;
    if( balls->ball != NULL ) billard_free( balls->ball );
    balls->ball = billard_malloc(sizeof(BallType)*balls->nr);

    place8ballnrs(balls);

    for(i=0;i<balls->nr;i++){
        balls->ball[i].m=BALL_M;
        /* I_kugel = (m.r^2)2/5 = (m.d^2)/10 */
        balls->ball[i].I=BALL_M*BALL_D*BALL_D/10.0/**0.01*/;
        balls->ball[i].d=BALL_D;
        balls->ball[i].v=vec_xyz(0.0,0.0,0.0);
        balls->ball[i].w=vec_xyz(0.0,0.0,0.0);
        balls->ball[i].b[0]=vec_unit(vec_xyz(rand(),rand(),rand()));
        vdummy=vec_xyz(rand(),rand(),rand());
        balls->ball[i].b[1]=vec_unit(vec_diff(vdummy,vec_proj(vdummy,balls->ball[i].b[0])));
        balls->ball[i].b[2]=vec_cross(balls->ball[i].b[0],balls->ball[i].b[1]);
        balls->ball[i].in_game=1;
        balls->ball[i].in_hole=0;
    }

    dball1=vec_scale( vec_xyz(-0.5, 0.5*sqrt(3.0), 0.0), (1.0+2.0*poserr)*BALL_D );
    dball2=vec_scale( vec_xyz( 1.0,           0.0, 0.0), (1.0+2.0*poserr)*BALL_D );
    /* white ball */
    balls->ball[0].r = vec_xyz(0.0,-TABLE_L/4.0,0.0);
//    balls->ball[0].w = vec_xyz(-M_PI*5.0,0.0,-M_PI*5.0);
    balls->ball[0].w = vec_xyz(0.0,0.0,0.0);
    /* other balls */
    balls->ball[ 1].r = vec_xyz(0.0,TABLE_L/4.0,0.0);
    balls->ball[ 2].r = vec_add( balls->ball[ 1].r, dball1 );
    balls->ball[ 3].r = vec_add( balls->ball[ 2].r, dball2 );
    balls->ball[ 4].r = vec_add( balls->ball[ 2].r, dball1 );
    balls->ball[ 5].r = vec_add( balls->ball[ 4].r, dball2 );
    balls->ball[ 6].r = vec_add( balls->ball[ 5].r, dball2 );
    balls->ball[ 7].r = vec_add( balls->ball[ 4].r, dball1 );
    balls->ball[ 8].r = vec_add( balls->ball[ 7].r, dball2 );
    balls->ball[ 9].r = vec_add( balls->ball[ 8].r, dball2 );
    balls->ball[10].r = vec_add( balls->ball[ 9].r, dball2 );
    balls->ball[11].r = vec_add( balls->ball[ 7].r, dball1 );
    balls->ball[12].r = vec_add( balls->ball[11].r, dball2 );
    balls->ball[13].r = vec_add( balls->ball[12].r, dball2 );
    balls->ball[14].r = vec_add( balls->ball[13].r, dball2 );
    balls->ball[15].r = vec_add( balls->ball[14].r, dball2 );
//    balls->ball[15].r = vec_xyz( 0, -TABLE_L/4, 0 );

    /* add randomness to init positions */
    for( i=1 ; i<balls->nr ; i++ ){
        double ang, ampl;
        myvec verr;
        ang  = (double)rand()/(double)RAND_MAX*2.0*M_PI;
        DPRINTF("ball_placemet_err:   angle=%f    ",ang);
        ampl = (double)rand()/(double)RAND_MAX*(poserr*0.95)*BALL_D;
        DPRINTF("amplitude=%f\n",ampl);
        verr = vec_scale( vec_xyz(cos(ang),sin(ang),0.0), (poserr*0.95)*BALL_D );
        balls->ball[i].r = vec_add( balls->ball[i].r, verr );
    }
    for( i=1 ; i<balls->nr ; i++ ){
        int j;
        for( j=i+1 ; j<balls->nr ; j++ ){
            if (vec_abs(vec_diff(balls->ball[i].r,balls->ball[j].r))/BALL_D<1.5){
                DPRINTF("BALLLDISR(%d,%d)=%f\n",balls->ball[i].nr,balls->ball[j].nr,vec_abs(vec_diff(balls->ball[i].r,balls->ball[j].r))/BALL_D);
            }
        }
    }

    for( i=0 ; i<balls->nr ; i++ ){
        balls->ball[i].path=0;
        balls->ball[i].pathcnt=0;
        balls->ball[i].pathsize=0;
    }

//    balls->ball[0].v=vec_xyz(1.5,2.0,0.0);
    balls->ball[0].v=vec_xyz(0.0,0.0,0.0);
}

void place9ballnrs( BallsType * balls )
{
    _NOT_IMPLEMENTED
}


void create_9ball_scene( BallsType * balls )
{
    _NOT_IMPLEMENTED
}


void placecarambolballnrs( BallsType * balls )
{
    _NOT_IMPLEMENTED
}


void create_carambol_scene( BallsType * balls )
{
    _NOT_IMPLEMENTED
}


void placesnookerballnrs(BallsType * balls)
{
    _NOT_IMPLEMENTED
}

int try_snooker_spot(BallsType *balls,struct Vect spot)
{
    _NOT_IMPLEMENTED
}

#define TABLE_SCALE (TABLE_L/(3.571042))
void spot_snooker_ball(BallsType *balls,int nr)
{
    _NOT_IMPLEMENTED
}

void create_snooker_scene( BallsType * balls )
{
    _NOT_IMPLEMENTED
}



int balls_in_game( BallsType * balls, int full_half )
/* without white, 8 is neither full nor half */
{
    int i;
    int nr=0;
    for(i=1;i<balls->nr;i++) {
        if( ( ( full_half==BALL_FULL && balls->ball[i].nr<8 ) ||
              ( full_half==BALL_HALF && balls->ball[i].nr>8 ) ||
              ( full_half==BALL_ANY ) ) &&
            balls->ball[i].in_game
          ) nr++;
    }
    return nr;
}
