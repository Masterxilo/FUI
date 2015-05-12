/* font.c
**
**    create pixmaps from text using freetype
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

#ifndef FONT_H
#define FONT_H

#include<GL/gl.h>

void my_draw_bitmap( char * src, int w1, int h1,
                    int x0, int y0, char * dst , int w, int h );

void getStringPixmapFT(char *str, char * fontname, int font_height, char ** data, int * dwidth, int * dheight, int * width, int * height);

GLuint getStringGLListFT(char *str, char * fontname, double font_height, float depth, double * width, double * height);

#endif  /* FONT_H */
