/* textobj.c
**
**    quad with text as texture using OpenGL
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
#include"options.h"
#include"font.h"
#include"textobj.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef options_use_freetype

void bulb_font(char * data, int w, int h)
{
  int x, y, i, w2, h2, index;
  double d;

  w2 = 5; h2 = 5;
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      d = 0;
      for (i = 0; i<w2*h2; i++) {
        index = (y - h2 / 2 + i / w2)*w + x - w2 / 2 + i%w2;
        if (index>0 && index < w*h){
          d += (unsigned char)data[index];
        }
      }
      if (data[y*w + x] != 0)
        data[y*w + x] = (double)((unsigned char)data[y*w + x])*d / (double)w2 / (double)h2 / 255.0;
    }
  }
}


void diff_x_font(char * data, int w, int h)
{
  int x, y, w2, h2;
  double d;

  w2 = 5; h2 = 5;
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      d = (unsigned char)data[y*w + x] - (unsigned char)data[y*w + x + 1];
      data[y*w + x] = (double)((unsigned char)data[y*w + x])*(200.0 - d) / 255.0;
    }
  }
}

void diff_xy_font(char * data, int w, int h)
{
  int x, y, w2, h2;
  double d;

  w2 = 5; h2 = 5;
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      d = (unsigned char)data[y*w + x] - (unsigned char)data[(y + 1)*w + x + 1];
      data[y*w + x] = (double)((unsigned char)data[y*w + x])*(200.0 - d) / 255.0;
    }
  }
}


int create_string_quad(char * str, char * fontname, int h, int * quad_id, int * tex_id, double * quad_w, double * quad_h)
{
  int    quad_obj;
  int    texbind;
  int    texw, texh, width, height;
  char * texdata;
  double fact;
  int    w1, w2, h1, h2, nx, ny;
#define MAX_TEXW 256
#define MAX_TEXH 256

  getStringPixmapFT(str, fontname, h, &texdata, &texw, &texh, &width, &height);
  glGenTextures(1, &texbind);
  /*    nx=(width+MAX_TEX_W-1)/MAX_TEX_W;
      ny=(width+MAX_TEX_H-1)/MAX_TEX_H;
      w1 = MAX_TEX_W;
      w2 = NEXT_POW_OF_2(MAX_TEX_W & (MAX_TEX_W-1));
      h1 = MAX_TEX_H;
      h2 = NEXT_POW_OF_2(MAX_TEX_H & (MAX_TEX_H-1));*/

  /*    texnr++;
      texnr=nx*ny

      for(iy=0;iy<ny;iy++){
      for(ix=0;ix<nx;ix++){
      glGenTextures(1,&texbind[texnr]);
      }
      }*/


  //    bulb_font(texdata, texw, texh);
  //    diff_xy_font(texdata, texw, texh);
  //    fprintf(stderr,"texw=%d,texh=%d,w=%d,h=%d\n",texw,texh,width,height);
  glBindTexture(GL_TEXTURE_2D, texbind);
  gluBuild2DMipmaps(GL_TEXTURE_2D, 1, texw, texh, GL_LUMINANCE,
    GL_UNSIGNED_BYTE, texdata);
  //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options_tex_min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options_tex_mag_filter);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  free(texdata);
  //    fprintf(stderr,"-texbind=%d\n",texbind);

  //    fprintf(stderr,"texx=%f,texy=%f\n",(double)texw/(double)width,(double)texh/(double)height);
  fact = (double)h;
  quad_obj = glGenLists(1);
  glNewList(quad_obj, GL_COMPILE);
  //    glBindTexture(GL_TEXTURE_2D,lightflaretexbind);
  glBindTexture(GL_TEXTURE_2D, texbind);
  glBegin(GL_QUADS);

  glNormal3f(0, 0, 1);

  glTexCoord2f(0, 0);
  glVertex3f(0, 1.0*fact, 0);

  glTexCoord2f((double)width / (double)texw, 0);
  //    glTexCoord2f(1,0);
  //    glTexCoord2f((double)texw/(double)width,0);
  glVertex3f((double)width / (double)height*fact, 1.0*fact, 0);

  glTexCoord2f((double)width / (double)texw, (double)height / (double)texh);
  //    glTexCoord2f(1,1);
  //    glTexCoord2f((double)texw/(double)width,(double)texh/(double)height);
  glVertex3f((double)width / (double)height*fact, 0, 0);

  glTexCoord2f(0, (double)height / (double)texh);
  //    glTexCoord2f(0,1);
  //    glTexCoord2f(0,(double)texh/(double)height);
  glVertex3f(0, 0, 0);

  glEnd();
  glEndList();

  *quad_h = fact*1.0;
  *quad_w = (double)width / (double)height*fact;

  if (quad_id != 0) *quad_id = quad_obj;
  if (tex_id != 0) *tex_id = texbind;

  return(quad_obj);
}


textObj * textObj_new(char * str, char * fontname, int height)
{
  textObj * obj;
  obj = malloc(sizeof(textObj));
  obj->height = height;
  obj->depth3D = 0.0;  /* for toggling to 3D */
  obj->is_3D = 0;
  strcpy(obj->str, str);
  strcpy(obj->fontname, fontname);
  create_string_quad(obj->str, obj->fontname, obj->height, &(obj->quad_id), &(obj->tex_id), &(obj->quad_w), &(obj->quad_h));
  return obj;
}


textObj * textObj3D_new(char * str, char * fontname, double height, float depth, int ppspline)
{
  textObj * obj;
  obj = malloc(sizeof(textObj));
  obj->height = height;
  obj->depth3D = depth;
  obj->is_3D = 1;
  strcpy(obj->str, str);
  strcpy(obj->fontname, fontname);
  obj->obj3D_id = getStringGLListFT(obj->str, obj->fontname, obj->height, obj->depth3D, &(obj->obj3D_w), &(obj->obj3D_h));
  return obj;
}



void textObj_toggle3D(textObj * obj)
{
  if (!obj->is_3D){
    obj->is_3D = 1;
    glDeleteLists(obj->quad_id, 1);
    obj->quad_id = 0;
    glDeleteTextures(1, &(obj->tex_id));
    obj->tex_id = 0;
    obj->obj3D_id = getStringGLListFT(obj->str, obj->fontname, obj->height, obj->depth3D, &(obj->obj3D_w), &(obj->obj3D_h));
  }
  else {
    obj->is_3D = 0;
    glDeleteLists(obj->obj3D_id, 1);
    obj->obj3D_id = 0;
    create_string_quad(obj->str, obj->fontname, obj->height, &(obj->quad_id), &(obj->tex_id), &(obj->quad_w), &(obj->quad_h));
  }
}


void textObj_setText(textObj * obj, char * str)
{
  if (strcmp(str, obj->str) != 0){
    if (!obj->is_3D){
      DPRINTF("textObj_setText: obj=%p, text=%s\n", obj, str);
      DPRINTF("textObj_setText 0\n");
      glDeleteLists(obj->quad_id, 1);
      DPRINTF("textObj_setText 0.5\n");
      obj->quad_id = 0;
      DPRINTF("obj->tex_id=%d\n", obj->tex_id);
      if (glIsTexture(obj->tex_id) == GL_TRUE)
        glDeleteTextures(1, &(obj->tex_id));
      DPRINTF("textObj_setText 0.75\n");
      obj->tex_id = 0;
      strcpy(obj->str, str);
      DPRINTF("textObj_setText 1\n");
      create_string_quad(obj->str, obj->fontname, obj->height, &(obj->quad_id), &(obj->tex_id), &(obj->quad_w), &(obj->quad_h));
      DPRINTF("textObj_setText 2\n");
    }
    else {
      glDeleteLists(obj->obj3D_id, 1);
      obj->obj3D_id = 0;
      strcpy(obj->str, str);
      obj->obj3D_id = getStringGLListFT(obj->str, obj->fontname, obj->height, obj->depth3D, &(obj->obj3D_w), &(obj->obj3D_h));
    }
  }
}


void textObj_delete_last(textObj * obj)
{
  char str[256];
  int slen;
  strcpy(str, obj->str);
  slen = strlen(str);
  str[slen - 1] = 0;
  textObj_setText(obj, str);
}

void textObj_append_char(textObj * obj, int c)
{
  char str[256];
  int slen;
  strcpy(str, obj->str);
  slen = strlen(str);
  str[slen + 1] = 0;
  str[slen] = c;
  textObj_setText(obj, str);
}


void textObj_setHeight(textObj * obj, int height)
{
  if (!obj->is_3D){
    glDeleteLists(obj->quad_id, 1);
    obj->quad_id = 0;
    glDeleteTextures(1, &(obj->tex_id));
    obj->tex_id = 0;
    obj->height = height;
    create_string_quad(obj->str, obj->fontname, obj->height, &(obj->quad_id), &(obj->tex_id), &(obj->quad_w), &(obj->quad_h));
  }
  else {
    glDeleteLists(obj->obj3D_id, 1);
    obj->obj3D_id = 0;
    obj->height = height;
    obj->obj3D_id = getStringGLListFT(obj->str, obj->fontname, obj->height, obj->depth3D, &(obj->obj3D_w), &(obj->obj3D_h));
  }
}

void textObj_setDepth(textObj * obj, int depth)
{
  if (!obj->is_3D){
    obj->depth3D = depth;
  }
  else {
    glDeleteLists(obj->obj3D_id, 1);
    obj->obj3D_id = 0;
    obj->depth3D = depth;
    obj->obj3D_id = getStringGLListFT(obj->str, obj->fontname, obj->height, obj->depth3D, &(obj->obj3D_w), &(obj->obj3D_h));
  }
}

void textObj_setFont(textObj * obj, char * fontname)
{
  if (!obj->is_3D){
    glDeleteLists(obj->quad_id, 1);
    obj->quad_id = 0;
    glDeleteTextures(1, &(obj->tex_id));
    obj->tex_id = 0;
    strcpy(obj->fontname, fontname);
    create_string_quad(obj->str, obj->fontname, obj->height, &(obj->quad_id), &(obj->tex_id), &(obj->quad_w), &(obj->quad_h));
  }
  else {
    glDeleteLists(obj->obj3D_id, 1);
    obj->obj3D_id = 0;
    strcpy(obj->fontname, fontname);
    obj->obj3D_id = getStringGLListFT(obj->str, obj->fontname, obj->height, obj->depth3D, &(obj->obj3D_w), &(obj->obj3D_h));
  }
}

void textObj_draw(textObj * obj)
{
  if (!obj->is_3D){
    if (obj->quad_id != 0)
      glCallList(obj->quad_id);
  }
  else {
    if (obj->obj3D_id != 0){
      //            glDisable(GL_TEXTURE_2D);
      glCallList(obj->obj3D_id);
      //            glEnable(GL_TEXTURE_2D);
    }
  }
}

void textObj_draw_centered(textObj * obj)
{
  if (!obj->is_3D){
    glPushMatrix();
    glTranslatef(-(double)(obj->quad_w) / 2.0, -(double)(obj->quad_h) / 2.0, 0.0);
    if (obj->quad_id != 0)
      glCallList(obj->quad_id);
    glPopMatrix();
  }
  else {
    glPushMatrix();
    glTranslatef(-(double)(obj->obj3D_w) / 2.0, -(double)(obj->obj3D_h) / 2.0, (double)(obj->depth3D) / 2.0);
    if (obj->obj3D_id != 0){
      //            glDisable(GL_TEXTURE_2D);
      glCallList(obj->obj3D_id);
      //            glEnable(GL_TEXTURE_2D);

    }
    glPopMatrix();
  }
}

void textObj_draw_bound(textObj * obj, int hbound, int vbound)
{
  double x, y;
  if (!obj->is_3D){
    x = 0.0;
    y = 0.0;
    if (hbound == HBOUND_CENTER)  x = -(double)(obj->quad_w) / 2.0;
    if (hbound == HBOUND_RIGHT)  x = -(double)(obj->quad_w);
    if (hbound == HBOUND_LEFT)  x = 0.0;
    if (vbound == VBOUND_CENTER)  y = -(double)(obj->quad_h) / 2.0;
    if (vbound == VBOUND_TOP)  y = -(double)(obj->quad_h);
    if (vbound == VBOUND_BOTTOM)  y = 0.0;
    glPushMatrix();
    glTranslatef(x, y, 0.0);
    if (obj->quad_id != 0)
      glCallList(obj->quad_id);
    glPopMatrix();
  }
  else {
    x = 0.0;
    y = 0.0;
    if (hbound == HBOUND_CENTER)  x = -(double)(obj->obj3D_w) / 2.0;
    if (hbound == HBOUND_RIGHT)  x = -(double)(obj->obj3D_w);
    if (hbound == HBOUND_LEFT)  x = 0.0;
    if (vbound == VBOUND_CENTER)  y = -(double)(obj->obj3D_h) / 2.0;
    if (vbound == VBOUND_TOP)  y = -(double)(obj->obj3D_h);
    if (vbound == VBOUND_BOTTOM)  y = 0.0;
    glPushMatrix();
    glTranslatef(x, y, 0.0);
    if (obj->obj3D_id != 0){
      //            glDisable(GL_TEXTURE_2D);
      glCallList(obj->obj3D_id);
      //            glEnable(GL_TEXTURE_2D);
    }
    glPopMatrix();
  }
}

void textObj_delete(textObj * obj)
{
  if (!obj->is_3D){
    glDeleteLists(obj->quad_id, 1);
    obj->quad_id = 0;
    glDeleteTextures(1, &(obj->tex_id));
    obj->tex_id = 0;
  }
  else {
    glDeleteLists(obj->obj3D_id, 1);
    obj->obj3D_id = 0;
  }
}

#endif
