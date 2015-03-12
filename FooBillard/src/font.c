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
#include "myinclude.h"
#include "options.h"
#ifdef options_use_freetype
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#endif

#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "vmath.h"

#define divisor 64000.0


static int init_me = 1;
#ifdef options_use_freetype
static FT_Library    library;   /* handle to library     */
#endif


struct TessDataVec{
  GLdouble d[3];
  int is_start;
  struct TessDataVec * next;
};

struct TessData {
  int points_per_spline;
  struct GLUtesselator * tobj;
  struct TessDataVec * v;
  FT_Vector from;
  int first_call;
};


struct TessData * new_tessdata()
{
  struct TessData * tessdata;
  tessdata = malloc(sizeof(struct TessData));
  tessdata->first_call = 1;
  tessdata->v = NULL;
  tessdata->tobj = gluNewTess();
  tessdata->points_per_spline = 3;
  return(tessdata);
}


void free_tessdata(struct TessData * tessdata)
{
  struct TessDataVec * v;
  struct TessDataVec * vn;

  v = tessdata->v;
  while (v != NULL){
    vn = v->next; free(v); v = vn;
  }

  gluDeleteTess(tessdata->tobj);

  free(tessdata);
}


void my_draw_bitmap(char * src, int w1, int h1,
  int x0, int y0, char * dst, int w, int h)
{
  int x, y;
  //    fprintf(stderr,"my_draw_bitmap: \n");
  for (y = 0; y < h1; y++){
    //        fprintf(stderr,"my_draw_bitmap: y=%d\n",y);
    for (x = 0; x < w1; x++){
      dst[(y + y0)*w + x + x0] += src[y*w1 + x];
    }
  }
}


void getStringPixmapFT(char *str, char *fontname, int font_height, char ** data, int * dwidth, int * dheight, int * width, int * height)
/* data containes the pixmap */
{
#ifdef options_use_freetype
  FT_Face       face;      /* handle to face object */
  int           pen_x, pen_y, n, i, w, h, error, w1, h1;

  w1 = 0; h1 = 0;
  //.. initialise library ..
  if (init_me){
    error = FT_Init_FreeType(&library);
    if (error) {
      fprintf(stderr, "FT_Init_FreeType error\n");
      exit(1);
    }
    else {
      //        fprintf(stderr,"FT_New_Face OK!\n");
    }
    init_me = 0;
  }
  //#if 0
  //.. create face object ..
  //    error = FT_New_Face( library, "/usr/X11/lib/X11/fonts/win-ttf/comic.ttf", 0, &face );
  error = FT_New_Face(library, fontname, 0, &face);
  //    error = FT_New_Face( library, "/usr/X11/lib/X11/fonts/win-ttf/tahomabd.ttf", 0, &face );
  if (error == FT_Err_Unknown_File_Format){
    fprintf(stderr, "the font file could be opened and read, but it appears that its font format is unsupported\n");
    exit(1);
  }
  else if (error) {
    fprintf(stderr, "another error code means that the font file could not e opened or read, or simply that it is broken\n");
    exit(1);
  }
  else {
    //        fprintf(stderr,"FT_New_Face OK!\n");
  }
  //.. set character size ..
  //    error = FT_Set_Char_Size(
  //                             face,    /* handle to face object           */
  //                             0,       /* char_width in 1/64th of points  */
  //                           16*64,   /* char_height in 1/64th of points */
  //                           300,      /* horizontal device resolution    */
  //                           300 );    /* vertical device resolution      */

  error = FT_Set_Pixel_Sizes(
    face,   /* handle to face object            */
    0,      /* pixel_width                      */
    font_height);   /* pixel_height                     */

  pen_x = 0;
  pen_y = 0;


  //    fprintf(stderr,"getStringPixmapFT: num_glyphs=%d\n",face->num_glyphs);
  w = 0;
  h = font_height;
  for (i = 0; i < 2; i++){

    if (i == 1){
      int j;
      for (w1 = w, w = 8; w < w1; w *= 2);
      for (h1 = h, h = 8; h < h1; h *= 2);
      //            fprintf(stderr,"getStringPixmapFT: allocing  w=%d h=%d\n",w,h);
      (*data) = malloc(w*h);
      for (j = 0; j < w*h; j++) (*data)[j] = 0;
    }

    for (n = 0; str[n] != 0; n++)
    {
      FT_UInt  glyph_index;

      // retrieve glyph index from character code
      glyph_index = FT_Get_Char_Index(face, str[n]);

      // load glyph image into the slot (erase previous one)
      error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
      if (error) { fprintf(stderr, "FT_Load_Glyph:error#%X\n", error); exit(1); }

      // convert to an anti-aliased bitmap
      //            error = FT_Render_Glyph( face->glyph, ft_render_mode_mono );
      error = FT_Render_Glyph(face->glyph, ft_render_mode_normal);
      if (error) { fprintf(stderr, "FT_Render_Glyph:error#%X\n", error); exit(1); }

      /*             fprintf(stderr,"getStringPixmapFT: face->glyph->bitmap_top=%d\n",face->glyph->bitmap_top);
                   fprintf(stderr,"getStringPixmapFT: face->glyph->bitmap.rows=%d\n",face->glyph->bitmap.rows);
                   fprintf(stderr,"getStringPixmapFT: face->glyph->bitmap.pitch=%d\n",face->glyph->bitmap.pitch);
                   fprintf(stderr,"getStringPixmapFT: face->glyph->bitmap.width=%d\n",face->glyph->bitmap.width);
                   fprintf(stderr,"getStringPixmapFT: face->glyph->metrics.horiBearingY=%d\n",face->glyph->metrics.horiBearingY);
                   fprintf(stderr,"getStringPixmapFT: face->height=%d\n",face->height);
                   fprintf(stderr,"getStringPixmapFT: face->ascender=%d\n",face->bbox.yMax);
                   fprintf(stderr,"getStringPixmapFT: face->descender=%d\n",face->descender);*/

      if (i != 0){
        // now, draw to our target surface
        my_draw_bitmap((char *)face->glyph->bitmap.buffer,
          face->glyph->bitmap.width, face->glyph->bitmap.rows,
          //                                pen_x, pen_y,
          pen_x + face->glyph->bitmap_left,
          pen_y + font_height*face->ascender / (face->ascender - face->descender) - face->glyph->bitmap_top,
          *data, w, h);
        pen_x += (face->glyph->advance.x >> 6);
      }
      else {
        //                fprintf(stderr,"getStringPixmapFT: w=%d h=%d\n",w,h);
        w += (face->glyph->advance.x >> 6);
        //                fprintf(stderr,"getStringPixmapFT: w=%d h=%d\n",w,h);
      }
      //            free(face->glyph->bitmap.buffer);
    }
  }

  error = FT_Done_Face(face);
  if (error) {
    fprintf(stderr, "FT_Done_Face error# %d\n", error);
    exit(1);
  }
  //#endif
  /*    fprintf(stderr,"FT_Done_FreeType\n");
      error = FT_Done_FreeType( library );
      if ( error ) {
      fprintf(stderr,"FT_Done_FreeType error# %d\n",error);
      exit(1);
      }*/
  DPRINTF("FT_Done_FreeType ready\n");

  *dwidth = w; *dheight = h;
  if (width != NULL) *width = w1;
  if (height != NULL) *height = h1;
#endif
}


void __stdcall my_Vertex_cb(void * data)
{
  GLdouble * d;
  d = (GLdouble *)data;

  glNormal3f(0, 0, -1);
  glVertex3f(d[0], d[1], d[2]);
}



void tess_add_point(VMvect v, struct TessData * data, int is_start)
{
  struct TessDataVec * tdv;
  struct TessDataVec * tdv_p = NULL;

  tdv = data->v;
  while (tdv != NULL){
    tdv_p = tdv;
    tdv = tdv->next;
  }

  tdv = malloc(sizeof(struct TessDataVec));
  tdv->d[0] = v.x / divisor;
  tdv->d[1] = v.y / divisor;
  tdv->d[2] = v.z / divisor;
  tdv->is_start = is_start;
  tdv->next = NULL;

  if (data->v == NULL){
    data->v = tdv;
  }

  if (tdv_p != NULL){
    tdv_p->next = tdv;
  }

  gluTessVertex(data->tobj, tdv->d, tdv->d);
}



int cb_tess_move_to(FT_Vector * to, void * user)
{
  //    int * first_call = (int *) user;
  VMvect v;
  struct TessData * data = (struct TessData *) user;

  if (!data->first_call){
    gluTessEndContour(data->tobj);
  }
  else {
    data->first_call = 0;
  }
  gluTessBeginContour(data->tobj);

  v = vec_xyz(to->x, to->y, 0.0);
  tess_add_point(v, data, 1);

  data->from = *to;
  return 0;
}

int cb_tess_line_to(FT_Vector * to, void * user)
{
  VMvect v;
  struct TessData * data = (struct TessData *) user;

  v = vec_xyz(to->x, to->y, 0.0);
  tess_add_point(v, data, 0);

  data->from = *to;
  return 0;
}

VMvect conic_spline_point(VMvect vi, VMvect vf, VMvect vc, double t)
{
  VMvect v1, v2, v;
  v1 = vec_add(vi, vec_scale(vec_diff(vc, vi), t));
  v2 = vec_add(vc, vec_scale(vec_diff(vf, vc), t));
  v = vec_add(v1, vec_scale(vec_diff(v2, v1), t));
  return v;
}

VMvect cubic_spline_point(VMvect vi, VMvect vf, VMvect vc1, VMvect vc2, double t)
{
  VMvect v1, v2, v3, v4, v5, v;
  v1 = vec_add(vi, vec_scale(vec_diff(vc1, vi), t));
  v2 = vec_add(vc1, vec_scale(vec_diff(vc2, vc1), t));
  v3 = vec_add(vc2, vec_scale(vec_diff(v1, vc2), t));
  v4 = vec_add(v1, vec_scale(vec_diff(v2, v1), t));
  v5 = vec_add(v2, vec_scale(vec_diff(v3, v2), t));
  v = vec_add(v4, vec_scale(vec_diff(v5, v4), t));
  return v;
}

int cb_tess_conic_to(FT_Vector * ctrl, FT_Vector * to, void * user)
{
  VMvect vi, vf, vc, v;
  double t, dt;
  struct TessData * data = (struct TessData *) user;

  dt = 1.0 / data->points_per_spline;

  vi = vec_xyz(data->from.x, data->from.y, 0.0);
  vf = vec_xyz(to->x, to->y, 0.0);
  vc = vec_xyz(ctrl->x, ctrl->y, 0.0);

  for (t = 0.0; t < 1.0 + dt / 2.0; t += dt){
    v = conic_spline_point(vi, vf, vc, t);
    tess_add_point(v, data, 0);
  }

  data->from = *to;
  return 0;
}

int cb_tess_cubic_to(FT_Vector * ctrl1, FT_Vector * ctrl2, FT_Vector * to, void * user)
{
  VMvect vi, vf, vc1, vc2, v;
  double t, dt;
  struct TessData * data = (struct TessData *) user;

  dt = 1.0 / data->points_per_spline;

  vi = vec_xyz(data->from.x, data->from.y, 0.0);
  vf = vec_xyz(to->x, to->y, 0.0);
  vc1 = vec_xyz(ctrl1->x, ctrl1->y, 0.0);
  vc2 = vec_xyz(ctrl2->x, ctrl2->y, 0.0);

  for (t = 0.0; t < 1.0 + dt / 2.0; t += dt){
    v = cubic_spline_point(vi, vf, vc1, vc2, t);
    tess_add_point(v, data, 0);
  }

  data->from = *to;
  return 0;
}

void makeGLGeometryFT(FT_GlyphSlot glyph, double depth)
{
  int i, j;
  FT_Outline * outline;
  struct GLUtesselator * tobj;
  double start_x, start_y;
  GLdouble v[1000][3];

  outline = &(glyph->outline);

  /*    glBegin(GL_POLYGON);
      for(i=0;i<=outline->contours[0];i++){
      printf("hallo!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
      glNormal3f( 0.0,0.0,1.0 );
      glVertex3f( (double)outline->points[i].x/64.0/1000.0, (double)outline->points[i].y/64.0/1000.0, 0.0 );
      }
      glEnd();*/

#if 0
  tobj=gluNewTess();
  printf("12 tobj=%p\n",tobj);
  gluTessNormal( tobj, 0.0, 0.0, -1.0 );
  gluTessCallback(tobj, GLU_TESS_BEGIN, glBegin);
  gluTessCallback(tobj, GLU_TESS_VERTEX, my_Vertex_cb);
  gluTessCallback(tobj, GLU_TESS_END, glEnd);

  gluTessBeginPolygon(tobj, NULL);
  i=0;
  for(j=0;j<outline->n_contours;j++){
    gluTessBeginContour(tobj);
    for(;i<=outline->contours[j];i++){
      v[i][0]=outline->points[i].x/divisor;
      v[i][1]=outline->points[i].y/divisor;
      v[i][2]=0.0;
      //            if( (outline->tags[i]) & 1 ){
      gluTessVertex(tobj, v[i], v[i]);
      //            }
    }
    gluTessEndContour(tobj);
  }
  gluTessEndPolygon(tobj);
  gluDeleteTess( tobj );

  if(depth!=0.0){
    i=0;
    for(j=0;j<outline->n_contours;j++){
      start_x=outline->points[i].x/divisor;
      start_y=outline->points[i].y/divisor;
      glBegin(GL_QUAD_STRIP);
      for(;i<=outline->contours[j];i++){
        //                if( (outline->tags[i]) & 1 ){
        glNormal3f( 0.0,1.0,0.0 );
        glVertex3f( (double)outline->points[i].x/divisor, (double)outline->points[i].y/divisor, 0.0 );
        glNormal3f( 0.0,1.0,0.0 );
        glVertex3f( (double)outline->points[i].x/divisor, (double)outline->points[i].y/divisor, -depth );
        //                }
      }
      glNormal3f( 0.0,1.0,0.0 );
      glVertex3f( start_x, start_y, 0.0 );
      glNormal3f( 0.0,1.0,0.0 );
      glVertex3f( start_x, start_y, -depth );
      glEnd();
    }
  }
  glTranslatef(glyph->advance.x/divisor,0,0);
#else
  {
    FT_Outline_Funcs funcs;
    struct TessData * tessdata = new_tessdata();

    gluTessNormal(tessdata->tobj, 0.0, 0.0, -1.0);
    gluTessCallback(tessdata->tobj, GLU_TESS_BEGIN, glBegin);
    gluTessCallback(tessdata->tobj, GLU_TESS_VERTEX, my_Vertex_cb);
    gluTessCallback(tessdata->tobj, GLU_TESS_END, glEnd);

    funcs.move_to = cb_tess_move_to;
    funcs.line_to = cb_tess_line_to;
    funcs.conic_to = cb_tess_conic_to;
    funcs.cubic_to = cb_tess_cubic_to;
    funcs.shift = 0;
    funcs.delta = 0;
    gluTessBeginPolygon(tessdata->tobj, NULL);
    FT_Outline_Decompose(outline, &funcs, tessdata);
    gluTessEndPolygon(tessdata->tobj);

    gluTessNormal(tessdata->tobj, 0.0, 0.0, 1.0);
    glPushMatrix();
    glTranslatef(0, 0, -depth);
    gluTessBeginPolygon(tessdata->tobj, NULL);
    FT_Outline_Decompose(outline, &funcs, tessdata);
    gluTessEndPolygon(tessdata->tobj);
    glPopMatrix();

    if (depth != 0.0){
      struct TessDataVec * tdv;
      struct TessDataVec * tdv_p;
      struct TessDataVec * tdv_n = 0;
      struct TessDataVec * tdv_s = 0;
      VMvect n1, n2, n, d1, d2;

      tdv = tessdata->v;
      if (tdv != NULL){ tdv_n = tdv->next; if (tdv_n == NULL || tdv_n->is_start) tdv_n = tdv_s; }

      while (tdv != NULL){
        tdv_s = tdv;
        tdv_p = tdv;
        while (tdv_p->next != NULL && tdv_p->next->is_start == 0){ tdv_p = tdv_p->next; }

        glBegin(GL_QUADS);
        //                glBegin(GL_QUAD_STRIP);

        do {

          d1.x = tdv_p->d[0] - tdv->d[0];
          d1.y = tdv_p->d[1] - tdv->d[1];
          d2.x = tdv_n->d[0] - tdv->d[0];
          d2.y = tdv_n->d[1] - tdv->d[1];
          n1 = vec_unit(vec_xyz(d1.y, -d1.x, 0.0));
          n2 = vec_unit(vec_xyz(-d2.y, d2.x, 0.0));
          /*                    if( tdv_n->d[0]==tdv->d[0] && tdv_n->d[1]==tdv->d[1] ){
                                  n =vec_unit( vec_add(n2, n1) );
                                  }*/
          n = vec_unit(vec_add(n2, n1));
          glNormal3f(n2.x, n2.y, n2.z);
          glVertex3f(tdv->d[0], tdv->d[1], 0.0);
          glNormal3f(n2.x, n2.y, n2.z);
          glVertex3f(tdv->d[0], tdv->d[1], -depth);
          glNormal3f(n2.x, n2.y, n2.z);
          glVertex3f(tdv_n->d[0], tdv_n->d[1], -depth);
          glNormal3f(n2.x, n2.y, n2.z);
          glVertex3f(tdv_n->d[0], tdv_n->d[1], 0.0);

          tdv_p = tdv;
          tdv = tdv->next;
          if (tdv != NULL){ tdv_n = tdv->next; if (tdv_n == NULL || tdv_n->is_start) tdv_n = tdv_s; }

        } while (tdv != NULL && tdv->is_start == 0);

        /*                  glNormal3f( 0.0,1.0,0.0 );
                          glVertex3f( tdv_s->d[0], tdv_s->d[1], 0.0 );
                          glNormal3f( 0.0,1.0,0.0 );
                          glVertex3f( tdv_s->d[0], tdv_s->d[1], -depth );*/

        glEnd();
      }
    }

    glTranslatef(glyph->advance.x / divisor, 0, 0);

    free_tessdata(tessdata);
  }
#endif

}


GLuint getStringGLListFT(char *str, char *fontname, double font_height, float depth, double * width, double * height)
/* data containes the pixmap */
{
  GLuint rval;
#ifdef options_use_freetype
  FT_Face       face;      /* handle to face object */
  int           pen_x, pen_y, n, i, w, h, error, w1, h1;

  //.. initialise library ..
  if (init_me){
    error = FT_Init_FreeType(&library);
    if (error) {
      fprintf(stderr, "FT_Init_FreeType error\n");
      exit(1);
    }
    else {
      //        fprintf(stderr,"FT_New_Face OK!\n");
    }
    init_me = 0;
  }
  error = FT_New_Face(library, fontname, 0, &face);

  if (error == FT_Err_Unknown_File_Format){
    fprintf(stderr, "the font file could be opened and read, but it appears that its font format is unsupported\n");
    exit(1);
  }
  else if (error) {
    fprintf(stderr, "another error code means that the font file could not be opened or read, or simply that it is broken\n");
    exit(1);
  }
  else {
    //        fprintf(stderr,"FT_New_Face OK!\n");
  }
  //.. set character size ..
  error = FT_Set_Char_Size(
    face,    /* handle to face object           */
    0,       /* char_width in 1/64th of points  */
    font_height*divisor,   /* char_height in 1/64th of points */
    72 * 72 / 64,      /* horizontal device resolution    */
    72 * 72 / 64);    /* vertical device resolution      */

  //    error = FT_Set_Pixel_Sizes(
  //                               face,   /* handle to face object            */
  //                               0,      /* pixel_width                      */
  //                               5*font_height );   /* pixel_height                     */


  rval = glGenLists(1);


  glNewList(rval, GL_COMPILE);

  glPushMatrix();

  if (width != NULL)  *width = 0;
  if (height != NULL) *height = font_height;

  for (n = 0; str[n] != 0; n++){

    FT_UInt  glyph_index;

    // retrieve glyph index from character code
    glyph_index = FT_Get_Char_Index(face, str[n]);

    // load glyph image into the slot (erase previous one)
    error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
    //        error = FT_Load_Glyph( face, glyph_index, FT_LOAD_NO_SCALE );
    if (error) { fprintf(stderr, "FT_Load_Glyph:error#%X\n", error); exit(1); }

    // convert to an anti-aliased bitmap
    //            error = FT_Render_Glyph( face->glyph, ft_render_mode_mono );
    //        error = FT_Render_Glyph( face->glyph, ft_render_mode_normal );
    //        if (error) { fprintf(stderr,"FT_Render_Glyph:error#%X\n",error); exit(1); }

    /*             fprintf(stderr,"getStringPixmapFT: face->glyph->bitmap_top=%d\n",face->glyph->bitmap_top);
     fprintf(stderr,"getStringPixmapFT: face->glyph->bitmap.rows=%d\n",face->glyph->bitmap.rows);
     fprintf(stderr,"getStringPixmapFT: face->glyph->bitmap.pitch=%d\n",face->glyph->bitmap.pitch);
     fprintf(stderr,"getStringPixmapFT: face->glyph->bitmap.width=%d\n",face->glyph->bitmap.width);
     fprintf(stderr,"getStringPixmapFT: face->glyph->metrics.horiBearingY=%d\n",face->glyph->metrics.horiBearingY);
     fprintf(stderr,"getStringPixmapFT: face->height=%d\n",face->height);
     fprintf(stderr,"getStringPixmapFT: face->ascender=%d\n",face->bbox.yMax);
     fprintf(stderr,"getStringPixmapFT: face->descender=%d\n",face->descender);*/

    //        printf("face->glyph->format=%d\n !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",face->glyph->format);

    makeGLGeometryFT(face->glyph, depth);
    if (width != NULL) (*width) += (double)(face->glyph->advance.x) / divisor;
    //        if (width!=NULL) (*width) = 0.0;
  }

  error = FT_Done_Face(face);
  if (error) {
    fprintf(stderr, "FT_Done_Face error# %d\n", error);
    exit(1);
  }
  //#endif
  /*    fprintf(stderr,"FT_Done_FreeType\n");
      error = FT_Done_FreeType( library );
      if ( error ) {
      fprintf(stderr,"FT_Done_FreeType error# %d\n",error);
      exit(1);
      }*/
  DPRINTF("FT_Done_FreeType ready\n");

  glPopMatrix();

  glEndList();

#endif

  return rval;
}
