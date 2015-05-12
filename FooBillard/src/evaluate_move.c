/* evaluate_move.c
**
**    evaluate moves for dfferent gametypes
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define EVALUATE_MOVE_C
#include "evaluate_move.h"

#define MAX(x,y) ((x)>(y)?(x):(y));

typedef enum
{
  SN_PLAY_RED,
  SN_PLAY_ANY_COLOR,
  SN_PLAY_YELLOW,
  SN_PLAY_GREEN,
  SN_PLAY_BROWN,
  SN_PLAY_BLUE,
  SN_PLAY_PINK,
  SN_PLAY_BLACK,
  SN_DONE
}
SnookerBallToPlay;

typedef struct
{
  SnookerBallToPlay to_play;
}
SnookerState;


void spot_snooker_ball(BallsType *balls, int nr);

void(*evaluate_last_move)(struct Player * player, int * pact_player,
  BallsType * pballs, int * pqueue_view, float * pXque) = evaluate_last_move_8ball;


/*static void white_free_place(double x0, double y0, double * xd, double *yd, BallsType * pballs)
{
int i,exitloop;
double x,y, r,phi;
x=x0; y=y0;
phi=0.0;
do{
exitloop=1;
phi+=0.01;
r=floor(phi/2.0/M_PI)*0.01;
x=x0+r*cos(phi);
y=y0+r*sin(phi);
if(x<(TABLE_W-pballs->ball[0].d)/2.0 &&
x>(-TABLE_W+pballs->ball[0].d)/2.0 &&
y<-TABLE_L/4.0 &&
y>(-TABLE_L+pballs->ball[0].d)/2.0 )
{
} else exitloop=0;
for(i=1;i<pballs->nr;i++){
if ( vec_abs(vec_diff(vec_xyz(x,y,0),pballs->ball[i].r)) <
(pballs->ball[0].d+pballs->ball[i].d)/2.0 )
{ exitloop=0; break; }
}
} while(!exitloop);
*xd=x;
*yd=y;
}*/


static int in_strafraum(VMvect pos)
{
  return(pos.y < -TABLE_L / 4.0);
}

void mm_placing_cue_ball();

void evaluate_last_move_8ball(struct Player * player, int * pact_player,
  BallsType * pballs, int * pqueue_view, float * pXque)
{
#define act_player (*pact_player)
#define balls      (*pballs)
  int out_half = 0;
  int out_full = 0;
  int nextplayer = 1;
  int foul = 0;
  int first_ball_hit;
  /* if balls_out_half!=balls_out_full && human_full_half==BALL_ANY then  */

  out_half = BM_get_balls_out_half();
  out_full = BM_get_balls_out_full();

  nextplayer = 1; foul = 0;

  /* wenn fremde kugel zuerst angespielt -> foul */
  first_ball_hit = BM_get_1st_ball_hit();
  if (player[act_player].half_full == BALL_FULL){
    if (first_ball_hit > 8 && first_ball_hit<16) foul = 1;
  }
  if (player[act_player].half_full == BALL_HALF){
    if (first_ball_hit>0 && first_ball_hit<8) foul = 1;
  }

  /* erst an 2. stelle, da oben kein foul bei break */
  if (player[act_player].half_full == BALL_ANY){
    if (out_half>out_full){
      player[act_player].half_full = BALL_HALF;
      player[!act_player].half_full = BALL_FULL;
      nextplayer = 0;
    }
    if (out_half < out_full){
      player[act_player].half_full = BALL_FULL;
      player[!act_player].half_full = BALL_HALF;
      nextplayer = 0;
    }
  }

  /* wenn 8 zuerst angespielt und noch eigene da -> foul */
  if (first_ball_hit == 8){
    int i, eigene_da;
    eigene_da = 0;
    if (player[act_player].half_full == BALL_FULL){
      for (i = 0; i<pballs->nr; i++){
        if (pballs->ball[i].in_game && pballs->ball[i].nr>0 && pballs->ball[i].nr < 8){
          eigene_da = 1; break;
        }
      }
    }
    else if (player[act_player].half_full == BALL_HALF){
      for (i = 0; i<pballs->nr; i++){
        if (pballs->ball[i].in_game && pballs->ball[i].nr>8 && pballs->ball[i].nr < 16){
          eigene_da = 1; break;
        }
      }
    }
    if (eigene_da) foul = 1;
  }

  /* wenn angespielte kugel im strafraum */
  if (player[act_player].place_cue_ball &&
    in_strafraum(BM_get_1st_ball_hit_pos()) &&
    !BM_get_non_strafraum_wall_hit_before_1st_ball(in_strafraum)){
    foul = 1;
  }

  /* wenn eigene rein naechster */
  if (player[act_player].half_full == BALL_HALF && out_half != 0){
    nextplayer = 0;
  }
  if (player[act_player].half_full == BALL_FULL && out_full != 0){
    nextplayer = 0;
  }
  /* at break */
  if (player[act_player].half_full == BALL_ANY && (out_half != 0 || out_full != 0)){
    nextplayer = 0;
  }


  if (BM_get_balls_hit() == 0) foul = 1;

  /* wenn weisse rein */
  if (BM_get_white_out()){
    nextplayer = 1;
    foul = 1;
    balls.ball[0].in_game = 1;
    balls.ball[0].in_hole = 0;
  }

  /* wenn foul */
  if (foul){
    //        double x,y;
    /* this is done now for all balls in billard3d.c after evaluate_last_move  */
    /* white_free_place(0, -TABLE_L/4.0, &x, &y, pballs); */
    balls.ball[0].v = vec_xyz(0.0, 0.0, 0.0);
    balls.ball[0].w = vec_xyz(0.0, 0.0, 0.0);
    balls.ball[0].r = vec_xyz(0.0, -TABLE_L / 4.0, 0.0);
    //        balls.ball[0].r=vec_xyz(x,y,0.0);
  }

  /* if 8 out */
  if (BM_get_ball_out(8)){
    int(*get_balls_out_own)(void);
    int in_own, i;

    if (player[act_player].half_full == BALL_HALF){
      get_balls_out_own = BM_get_balls_out_half;
    }
    else {
      get_balls_out_own = BM_get_balls_out_full;
    }

    /* count own balls in game */
    in_own = 0;
    for (i = 0; i<pballs->nr; i++){
      if (player[act_player].half_full == BALL_FULL &&
        pballs->ball[i].nr>0 && pballs->ball[i].nr<8 && pballs->ball[i].in_game) in_own++;
      if (player[act_player].half_full == BALL_HALF &&
        pballs->ball[i].nr>8 && pballs->ball[i].nr < 16 && pballs->ball[i].in_game) in_own++;
    }

    /* only one and last one */
    if (
      !foul && get_balls_out_own() == 0 && in_own == 0 &&
      player[act_player].half_full != BALL_ANY   /* potting 8 at break caused a win (this should fix it) */
      )
    {
      player[act_player].winner = 1;
    }
    else
    {
      player[(act_player == 1) ? 0 : 1].winner = 1;
    }

  }

  BM_reset_move_info();

  if (player[act_player].place_cue_ball) player[act_player].place_cue_ball = 0;

  if (foul) nextplayer = 1;

  if (nextplayer){
    //        Xque=player[act_player].Xque;
    //        player[act_player].Zque=Zque;
    player[act_player].queue_view = *pqueue_view;
    act_player = (act_player == 1) ? 0 : 1;
    if (foul) player[act_player].place_cue_ball = 1;
    //        Xque=player[act_player].Xque;
    *pXque = player[act_player].Xque;
    *pqueue_view = player[act_player].queue_view;
  }

  // == 
  if (!player[act_player].is_AI && player[act_player].place_cue_ball)
      mm_placing_cue_ball();
  //==

#undef balls
#undef act_player
}


void evaluate_last_move_9ball(struct Player * player, int * pact_player,
  BallsType * pballs, int * pqueue_view, float * pXque)
{
#define act_player (*pact_player)
  int nextplayer = 1;
  int i, minball, dummy, foul;
  /* if balls_out_half!=balls_out_full && human_full_half==BALL_ANY then  */

  nextplayer = 1; foul = 0;

  minball = 15;
  for (i = 0; i < pballs->nr; i++){
    if (pballs->ball[i].nr != 0 &&
      pballs->ball[i].nr < minball &&
      pballs->ball[i].in_game){
      minball = pballs->ball[i].nr;
    }
  }

  dummy = BM_get_min_ball_out();
  if (dummy<minball && dummy != 0) minball = dummy;

  fprintf(stderr, "eval_move_9ball:      minball: %d\n", minball);
  fprintf(stderr, "eval_move_9ball: 1st ball hit: %d\n", BM_get_1st_ball_hit());

  if (BM_get_1st_ball_hit() == minball &&
    BM_get_balls_out_all() > 0 &&
    !BM_get_white_out()){
    nextplayer = 0;
  }

  if (BM_get_balls_hit() == 0) foul = 1;

  if (BM_get_1st_ball_hit() != minball) foul = 1;

  /* wenn weisse rein */
  if (BM_get_white_out()){
    foul = 1;
    nextplayer = 1;
    pballs->ball[0].in_game = 1;
    pballs->ball[0].in_hole = 0;
  }

  /* wenn foul - weisse platzieren */
  if (foul){
    //        double x,y;
    /* this is done now for all balls in billard3d.c after evaluate_last_move  */
    /*white_free_place(0, -TABLE_L/4.0, &x, &y, pballs);*/
    pballs->ball[0].v = vec_xyz(0.0, 0.0, 0.0);
    pballs->ball[0].w = vec_xyz(0.0, 0.0, 0.0);
    pballs->ball[0].r = vec_xyz(0, -TABLE_L / 4.0, 0.0);
  }

  fprintf(stderr, "foul:%d, nextplayer:%d, BM_get_ball_out(9):%d\n", foul, nextplayer, BM_get_ball_out(9));
  if (!foul && !nextplayer && BM_get_ball_out(9)){
    player[act_player].winner = 1;
  }

  if (foul && BM_get_ball_out(9)){
    player[(act_player == 1) ? 0 : 1].winner = 1;
  }

  BM_reset_move_info();

  if (player[act_player].place_cue_ball) player[act_player].place_cue_ball = 0;

  if (nextplayer){
    //        Xque=player[act_player].Xque;
    //        player[act_player].Zque=Zque;
    player[act_player].queue_view = *pqueue_view;
    act_player = (act_player == 1) ? 0 : 1;
    if (foul) player[act_player].place_cue_ball = 1;
    //        Xque=player[act_player].Xque;
    *pXque = player[act_player].Xque;
    *pqueue_view = player[act_player].queue_view;
  }
#undef act_player
}


void evaluate_last_move_carambol(struct Player * player, int * pact_player,
  BallsType * pballs, int * pqueue_view, float * pXque)
{
#define act_player (*pact_player)
#define CUE_BALL_IND (player[act_player].cue_ball)
  int nextplayer = 1;
  int bhit1, bhit2, i;
  int other_player;

  other_player = (act_player == 1) ? 0 : 1;

  nextplayer = 1;

  bhit1 = BM_get_nth_ball_hit_by_ind(CUE_BALL_IND, 1);
  for (i = 2; (bhit2 = BM_get_nth_ball_hit_by_ind(CUE_BALL_IND, i)) != -1 && bhit2 == bhit1; i++);

  fprintf(stderr, "cueball=%d,%d\n", CUE_BALL_IND, pballs->ball[CUE_BALL_IND].nr);
  fprintf(stderr, "bhit1=%d\n", bhit1);
  fprintf(stderr, "bhit2=%d\n", bhit2);

  if (bhit2 != -1 && bhit1 != -1 && bhit2 != bhit1){
    nextplayer = 0;
    player[act_player].score++;
  }

  BM_reset_move_info();

  if (nextplayer){
    //        Xque=player[act_player].Xque;
    //        player[act_player].Zque=Zque;
    player[act_player].queue_view = *pqueue_view;
    fprintf(stderr, "score of %s: %d\n", player[act_player].name, player[act_player].score);
    fprintf(stderr, "score of %s: %d\n", player[other_player].name, player[other_player].score);
    act_player = (act_player == 1) ? 0 : 1;
    //        if( foul ) player[act_player].place_cue_ball=1;
    //        Xque=player[act_player].Xque;
    *pXque = player[act_player].Xque;
    *pqueue_view = player[act_player].queue_view;
  }

#undef CUE_BALL_IND
#undef act_player
}

void evaluate_last_move_snooker(struct Player * player, int * pact_player,
  BallsType * pballs, int * pqueue_view, float * pXque)
{
#define act_player (*pact_player)
#define IS_RED(x) ( x==1 || x>=8 )
  int red_balls_are_in_game = 0;
  static SnookerState st = {SN_PLAY_RED};
  int color_to_pot;
  int i;
  int act_score = 0;
  int act_penalty = 0;
  int foul = 0;
  int ball_out;
  int other_player = (act_player == 1) ? 0 : 1;
  int b1hit = BM_get_1st_ball_hit();  if (b1hit >= 8) b1hit = 1;

  if (player[act_player].place_cue_ball) player[act_player].place_cue_ball = 0;
  printf("EVAL start\n");
  printf("EVAL to_play=%d\n", st.to_play);
  printf("EVAL b1hit=%d\n", b1hit);
  for (i = 0; i < pballs->nr; i++){
    if (IS_RED(pballs->ball[i].nr) && pballs->ball[i].in_game){
      red_balls_are_in_game = 1;
      break;
    }
  }

  /* wenn weisse rein */
  if (BM_get_white_out()){
    foul = 1;
    printf("EVAL foul 7\n");
    act_penalty = MAX(act_penalty, (BM_get_1st_ball_hit() <= 7 ? BM_get_1st_ball_hit() : 4));
    spot_snooker_ball(pballs, 0);
    player[other_player].place_cue_ball = 1;
  }

  switch (st.to_play)
  {
  case SN_PLAY_RED:
    color_to_pot = 1;
    if (b1hit != 1)
    {
      foul = 1;
      act_penalty = MAX(act_penalty, b1hit);
      printf("EVAL foul 1\n");
    }
    i = 1;
    while ((ball_out = BM_get_nth_ball_out(i++)) >= 0)
    {
      printf("EVAL ball out:%d\n", ball_out);
      if (IS_RED(ball_out))
      {
        act_score += 1;
      }
      else
      {
        act_penalty = MAX(act_penalty, ball_out);
        foul = 1;
        printf("EVAL foul 2\n");
      }
    }
    for (i = 2; i < 8; i++)
    {
      if (BM_get_ball_out(i))
        spot_snooker_ball(pballs, i);
    }
    st.to_play = SN_PLAY_ANY_COLOR;
    break;
  case SN_PLAY_ANY_COLOR:
    if (b1hit == 1)
    {
      foul = 1;
      printf("EVAL foul 3\n");
      act_penalty = MAX(act_penalty, 7);
    }
    color_to_pot = b1hit;
    i = 1;
    while ((ball_out = BM_get_nth_ball_out(i++)) >= 0)
    {
      printf("EVAL ball out:%d\n", ball_out);
      if (ball_out == color_to_pot)
      {
        act_score += ball_out;
      }
      else
      {
        foul = 1;
        printf("EVAL foul 4\n");
        act_penalty = MAX(act_penalty, ball_out == 1 ? 7 : ball_out);
      }
    }
    if (red_balls_are_in_game)
      st.to_play = SN_PLAY_RED;
    else
      st.to_play = SN_PLAY_YELLOW;

    for (i = 2; i < 8; i++)
    {
      if (BM_get_ball_out(i))
        spot_snooker_ball(pballs, i);
    }
    break;
  case SN_PLAY_YELLOW:
  case SN_PLAY_GREEN:
  case SN_PLAY_BROWN:
  case SN_PLAY_BLUE:
  case SN_PLAY_PINK:
  case SN_PLAY_BLACK:
    color_to_pot = st.to_play;
    if (b1hit != color_to_pot)
    {
      foul = 1;
      printf("EVAL foul 5\n");
      act_penalty = MAX(act_penalty, b1hit);
      act_penalty = MAX(act_penalty, color_to_pot);
    }
    i = 1;
    while ((ball_out = BM_get_nth_ball_out(i++)) >= 0)
    {
      printf("EVAL ball out:%d\n", ball_out);
      if (ball_out == color_to_pot)
      {
        act_score += ball_out;
      }
      else
      {
        foul = 1;
        printf("EVAL foul 6\n");
        act_penalty = MAX(act_penalty, b1hit);
        act_penalty = MAX(act_penalty, color_to_pot);
      }
    }
    if (!foul && act_score > 0) st.to_play++;

    for (i = st.to_play; i < 8; i++)
    {
      if (BM_get_ball_out(i))
        spot_snooker_ball(pballs, i);
    }
    break;
  }


  if (foul)
  {
    act_penalty = MAX(act_penalty, 4);
    player[other_player].score += act_penalty;
    printf("EVAL foul\n");
  }
  else
  {
    player[act_player].score += act_score;
    printf("EVAL no foul\n");
  }
  if (act_score == 0 || foul)
  {
    printf("EVAL next player\n");
    if (red_balls_are_in_game)
    {
      st.to_play = SN_PLAY_RED;
    }
    else
    {
      if (st.to_play <= SN_PLAY_ANY_COLOR)
      {
        st.to_play = SN_PLAY_YELLOW;
      }
    }
    player[act_player].queue_view = *pqueue_view;
    act_player = other_player;
    *pXque = player[act_player].Xque;
    *pqueue_view = player[act_player].queue_view;
  }

  player[act_player].snooker_on_red = st.to_play == SN_PLAY_RED;
  player[act_player].snooker_next_color = st.to_play;
  printf("EVAL to_play=%d\n", st.to_play);

  if (st.to_play == SN_DONE)
  {
    int other_player;

    other_player = (act_player + 1) % 2;

    if (player[act_player].score > player[other_player].score){
      player[act_player].winner = 1;
      player[other_player].winner = 0;
    }
    if (player[act_player].score < player[other_player].score) {
      player[act_player].winner = 0;
      player[other_player].winner = 1;
    }
  }

  printf("EVAL done\n");

  BM_reset_move_info();
  fflush(stdout);
}

void setfunc_evaluate_last_move(void(*eval_func)(struct Player * player, int * actual_player,
  BallsType * pballs, int * pqueue_view, float * pXque))
{
  evaluate_last_move = eval_func;
}
