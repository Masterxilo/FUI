/* bumpref.c
**
**    bummp-reflection-mapping using
**    NVIDIA vertex-shaders register-combiners and texture-programs
**    Copyright (C) 2002-2004  Florian Berger
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

#include <GL/gl.h>
#include <GL/glu.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bumpref.h"
#include "png_loader.h"
#include "options.h"
#include "vmath.h"

#ifndef NO_NV_BUMPREF  /* force not to use bumpref */

#ifdef GL_NV_register_combiners
#ifdef GL_NV_texture_shader
#ifdef GL_NV_vertex_program

#define USE_NV_BUMPREF

#endif /*GL_NV_vertex_program*/
#endif /*GL_NV_texture_shader*/
#endif /*GL_NV_register_combiners*/

#endif /* NO_NV_BUMPREF */

int bumpref_create_cubemap( char * posx_name,
                           char * posy_name,
                           char * posz_name,
                           char * negx_name,
                           char * negy_name,
                           char * negz_name )
{
    _NOT_IMPLEMENTED
}



void gaussian_blur( int w, int h, double * ddata, int d )
{
    _NOT_IMPLEMENTED
}

void bump2normal(int w, int h, char * data, double strength)
/* data has to be rgb */
{
    _NOT_IMPLEMENTED
}



signed short * bump2normal_HILO(int w, int h, char * data, double strength)
/* data has to be rgb */
{
    _NOT_IMPLEMENTED
}



int bumpref_create_bumpmap( char * map_name, double strength, int hilo )
/* returns texture-bind-id */
{
    _NOT_IMPLEMENTED
}













BumpRefType bumpref_setup_vp_ts_rc(
                            char * map_name, double strength,
                            char * posx_name,
                            char * posy_name,
                            char * posz_name,
                            char * negx_name,
                            char * negy_name,
                            char * negz_name,
                            float zoffs, int hilo,
                            int texgen
                           )
/* dont call this 1st time inside a GL display list */
/* doesnt have any effect when called 1st */
{
    _NOT_IMPLEMENTED
}

void bumpref_use(BumpRefType * bumpref)
{
    _NOT_IMPLEMENTED
}



void bumpref_restore()
{
    _NOT_IMPLEMENTED
}






int bump_create_bumpmap( char * map_name, double strength )
{
    _NOT_IMPLEMENTED
}


BumpRefType bump_setup_vp_rc( char * map_name, double strength, int texgen )
{
    _NOT_IMPLEMENTED
}


void bump_set_light( BumpRefType * bumpref, float x, float y, float z )
{
    _NOT_IMPLEMENTED
}


void bump_set_diff( BumpRefType * bumpref, float r, float g, float b )
{
    _NOT_IMPLEMENTED
}


void bump_set_spec( BumpRefType * bumpref, float r, float g, float b )
{
    _NOT_IMPLEMENTED
}


void bump_use( BumpRefType * bumpref )
{
    _NOT_IMPLEMENTED
}



void bump_restore()
{
    _NOT_IMPLEMENTED
}
