/* options.h
**
**    global defines for options
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

#include<GL/gl.h>


//#undef DEBUG

#ifdef DEBUG
#define DPRINTF(format, args...)	fprintf(stderr, format, ## args)
#else
#define DPRINTF(format, args...)
#endif


//#undef  options_use_freetype
#define options_use_freetype

#define options_tex_min_filter GL_LINEAR_MIPMAP_LINEAR
#define options_tex_mag_filter GL_LINEAR
//#define options_tex_min_filter GL_NEAREST
//#define options_tex_mag_filter GL_NEAREST

#define options_dither 1

#define options_ball_reflect 1
#define options_queue_reflect 1
#define options_diamond_reflect 1
#define options_ball_tex 1
#define options_queue_tex 1
#define options_frame_tex options_frame_tex_var
#define options_place_cue_ball_tex 1
#define options_ball_shadows 1
#define options_queue_shadows 1
#define options_table_tex 1
#define options_simple_ball_shadows 0
#define options_simple_queue_shadows 0
#define options_calc_ball_reflections 0
#define options_ball_stencil_reflections 1

#define options_jump_shots 0


#define options_diamond_color_gold    0xFFD566
#define options_diamond_color_chrome  0xFFFFFF
#define options_diamond_color_copper  0xFF66D5
#define options_diamond_color_black   0x888888
#define options_table_color_red       0x802020
#define options_table_color_green     0x0D6621
#define options_table_color_blue      0x346070
#define options_table_color_black     0x383838
#define options_table_color_beige     0x807060
#define options_frame_color_white     0xCCCCCC


/* ball detail settings - very slow machines */
#define options_max_ball_detail_LOW 3
#define options_ball_detail_nearmax_LOW 0.7
#define options_ball_detail_farmin_LOW 5.0

/* ball detail settings - slow machines */
#define options_max_ball_detail_MED 4
#define options_ball_detail_nearmax_MED 0.7
#define options_ball_detail_farmin_MED 5.5

/* ball detail settings - fast machines */
#define options_max_ball_detail_HIGH 5
#define options_ball_detail_nearmax_HIGH 0.6
#define options_ball_detail_farmin_HIGH 7.0

/* ball detail settings - very fast machines */
#define options_max_ball_detail_VERYHIGH 7
#define options_ball_detail_nearmax_VERYHIGH 0.4
#define options_ball_detail_farmin_VERYHIGH  7.0

#define options_autocreate_balltex 1

//#define options_player_fontname  "/usr/X11/lib/X11/fonts/win-ttf/tahomabd.ttf"
#define options_shared_data_path   "/usr/local/shared/foobillard"

#define options_player_fontname    "iomanoid.ttf"
#define options_help_fontname      "youregon.ttf"
#define options_menu_fontname      "youregon.ttf"
#define options_winner_fontname    "youregon.ttf"
#define options_ball_fontname      "bluebold.ttf"
#define options_score_fontname     "bluebold.ttf"
#define options_roster_fontname    "bluebold.ttf"

#define options_snd_volume 1.0

#define options_3D_winnertext 1

#define options_gamemode_match      0
#define options_gamemode_training   1
#define options_gamemode_tournament 2


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

extern int     options_positional_light;

extern int     options_diamond_color;
extern int     options_table_color;
extern int     options_frame_color;

extern double  options_table_size;

extern int     options_ball_reflections_blended;

extern int     options_lensflare;

extern int     options_max_ball_detail;
extern double  options_ball_detail_nearmax;
extern double  options_ball_detail_farmin;

extern int     options_rgstereo_on;

extern int *   options_col_ball;
extern int     options_col_ball_pool[];
extern int     options_col_ball_carambol[];
extern int     options_col_ball_snooker[];

extern char    options_net_hostname[];
extern int     options_net_portnum;

extern int     options_free_view_on;

extern int     options_frame_tex_var;

extern int     options_cuberef;
extern int     options_cuberef_res;

extern int     options_rgaim;

extern int     options_bumpref;  /* bump reflection of chrome edges */
extern int     options_bumpwood; /* bumpmaps on wood frame */

extern int     options_balltrace;

extern int     options_use_sound;   /* for taking care of non-sound systems */

extern int     options_gamemode;

extern int     options_ball_fresnel_refl;

extern int     options_avatar_on;

extern double  options_tourfast;

extern int     options_cloth_tex;


/* queries for OpenGL extensions */
extern int extension_cubemap;
extern int extension_multitexture;
extern int extension_rc_NV;   /* NVIDIA register combiners */
extern int extension_ts_NV;   /* NVIDIA texture shader */
extern int extension_vp_NV;   /* NVIDIA vertex program */


/* compatibility defines */

#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB
#ifdef GL_TEXTURE_CUBE_MAP_POSITIVE_X
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB GL_TEXTURE_CUBE_MAP_POSITIVE_X
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB GL_TEXTURE_CUBE_MAP_POSITIVE_Y
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB GL_TEXTURE_CUBE_MAP_POSITIVE_Z
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB GL_TEXTURE_CUBE_MAP_NEGATIVE_X
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
#define GL_TEXTURE_CUBE_MAP_ARB GL_TEXTURE_CUBE_MAP
#define GL_REFLECTION_MAP_ARB GL_REFLECTION_MAP
#endif
#endif

#ifndef GL_TEXTURE0_ARB
#ifdef GL_TEXTURE0
#define GL_TEXTURE0_ARB GL_TEXTURE0
#define GL_TEXTURE1_ARB GL_TEXTURE1
#define GL_TEXTURE2_ARB GL_TEXTURE2
#define GL_TEXTURE3_ARB GL_TEXTURE3
#endif
#endif
