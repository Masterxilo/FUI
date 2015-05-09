/* options.c
**
**    global variables for options
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


/* gold */
//#define options_diamond_color 0xFFD566
/* chrome */
//#define options_diamond_color 0xBBBBFF
/* copper */
//#define options_diamond_color 0xFF8866

/* green */
//#define options_table_color   0x0D6621
/* blue */
//#define options_table_color   0x346070
/* red */
//#define options_table_color   0x802020
/* ocker */
//#define options_table_color   0x706034

int    options_positional_light=1;

int    options_table_color   = 0x0D6621;
int    options_diamond_color = 0xFFD566;
int    options_frame_color   = 0x401405;

double options_table_size    = 2.1336;  /* 7 ft (smallest normed) */

int    options_lensflare     = 0;

int    options_ball_reflections_blended=1;

int    options_max_ball_detail     = 5;
double options_ball_detail_nearmax = 0.6;
double options_ball_detail_farmin  = 7.0;

int    options_rgstereo_on         = 0;

int    options_free_view_on        = 1;


int    options_col_ball_pool[]={
                            0xFFFFFF,  /* white */

                            0xEEDD00,  /*  1 yellow */
                            0x0000FF,  /*  2 blue   */
                            0xFF0000,  /*  3 red    */
                            0x7700CC,  /*  4 violet */
                            0xFF8800,  /*  5 orange */
                            0x009900,  /*  6 green  */
                            0x883311,  /*  7 brown  */
                            0x000000,  /*  8 black  */

                            0xEEDD00,  /*  9 yellow */
                            0x0000FF,  /* 10 blue   */
                            0xFF0000,  /* 11 red    */
                            0x7700CC,  /* 12 violet */
                            0xFF8800,  /* 13 orange */
                            0x009900,  /* 14 green  */
                            0x883311   /* 15 brown  */
                          };

int    options_col_ball_carambol[]={
                            0xFFFFFF,  /* cueball1 */
                            0xFFFF00,  /* cueball2 */
                            0xFF0000   /* red ball */
                          };

int    options_col_ball_snooker[]={
                            0xFFFFFF,   /* white cueball */
                            0xFF0000,   /* red    balls */
                            0xFFBB00,   /* orange ball */
                            0x00AA33,   /* green  ball */
                            0x883311,   /* brown  ball */
                            0x0000FF,   /* blue   ball */
                            0xFF66AA,   /* pink   ball */
                            0x000000,   /* black  ball */
                          };

int    options_col_ball /*= options_col_ball_pool*/;


char   options_net_hostname [1024];  /* initialized in billard3d.c */
int    options_net_portnum = 56341;

int    options_frame_tex_var = 1;

int    options_cuberef = 0;

int    options_cuberef_res = 128;

int    options_rgaim = 0; /* 0=moddle 1=right 2=left */

int    options_bumpref   = 0;  /* bump reflection of chrome edges */

int    options_bumpwood  = 0;  /* bump maps on wood frame */

int    options_balltrace = 0;

int    options_use_sound = 1;

int    options_gamemode  = 0;  /* 0=options_gamemode_match */

int    options_ball_fresnel_refl = 0;

int    options_avatar_on = 0;

double options_tourfast = 500.0;

int    options_cloth_tex = 1;


/* queries for OpenGL extensions */
int extension_cubemap      = 0;
int extension_multitexture = 0;
int extension_rc_NV        = 0;   /* NVIDIA register combiners */
int extension_ts_NV        = 0;   /* NVIDIA texture shader */
int extension_vp_NV        = 0;   /* NVIDIA vertex program */

#ifdef _WIN32
#include <GL/gl.h>
#include <GL/glext.h>
/* FROM: table.c, ball.c, bumpref.c */
PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
/* FROM: ball.c, bumpref.c */
PFNGLGENPROGRAMSNVPROC glGenProgramsNV;
PFNGLBINDPROGRAMNVPROC glBindProgramNV;
PFNGLLOADPROGRAMNVPROC glLoadProgramNV;
PFNGLPROGRAMPARAMETER4FNVPROC glProgramParameter4fNV;
PFNGLTRACKMATRIXNVPROC glTrackMatrixNV;
/* FROM: bumpref.c */
PFNGLCOMBINERPARAMETERINVPROC glCombinerParameteriNV;
PFNGLCOMBINEROUTPUTNVPROC glCombinerOutputNV;
PFNGLCOMBINERINPUTNVPROC glCombinerInputNV;
PFNGLFINALCOMBINERINPUTNVPROC glFinalCombinerInputNV;
PFNGLCOMBINERPARAMETERFVNVPROC glCombinerParameterfvNV;
#endif

