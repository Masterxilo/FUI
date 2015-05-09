/* aiplayer.c
**
**    code for positioning artifitial intelligence player
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
#define AIPLAYER_C
#include "aiplayer.h"
#include "vmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

VMvect (*ai_get_stroke_dir)( BallsType * balls, BordersType * walls, struct Player *pplayer ) = ai_get_stroke_dir_8ball;

void ai_set_skill( double skill ) /* not used ...yet */
{
    _NOT_IMPLEMENTED
}


void ai_set_err( double err )
{
    _NOT_IMPLEMENTED
}


static int my_rand(int nr)
{
    _NOT_IMPLEMENTED
}


static double my_rand01()
{
    _NOT_IMPLEMENTED
}


VMfloat stroke_angle( BallType * bcue, BallType * bhit, HoleType * hole )
{
    _NOT_IMPLEMENTED
}


int ball_in_way( int ballnr, VMvect aim, BallsType * balls )
{
    _NOT_IMPLEMENTED
}


int ind_ball_nr( int nr, BallsType * balls )
{
    _NOT_IMPLEMENTED
}


int nth_in_game( int n, BallsType * balls, int full_half )
{
    _NOT_IMPLEMENTED
}


VMvect ai_get_stroke_dir_8ball( BallsType * balls, BordersType * walls, struct Player * pplayer )
{
    _NOT_IMPLEMENTED
}

VMvect ai_get_stroke_dir_9ball(BallsType * balls, BordersType * walls, struct Player * pplayer)
{
    _NOT_IMPLEMENTED
}

int snooker_ball_legal(int ball,struct Player *player)
{
    _NOT_IMPLEMENTED
}

VMvect ai_get_stroke_dir_snooker( BallsType * balls, BordersType * walls, struct Player * pplayer )
{
    _NOT_IMPLEMENTED
}

VMvect ai_get_stroke_dir_carambol( BallsType * balls, BordersType * walls, struct Player * pplayer )
{
    _NOT_IMPLEMENTED
}


void setfunc_ai_get_stroke_dir(VMvect (*func)( BallsType * balls, BordersType * walls, struct Player * pplayer ))
{
    _NOT_IMPLEMENTED
}
