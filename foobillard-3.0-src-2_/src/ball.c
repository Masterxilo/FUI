/* ball.c
**
**    code for creating the GL-ball display-lists
**    Copyright (C) 2001-2004  Florian Berger
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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glu.h>
#include <GL/gl.h>
#ifdef _WIN32
#include <GL/glext.h>
#endif
#include "billard.h"
#include "billmove.h"
#include "ball.h"
#include "png_loader.h"
#include "options.h"
#include "font.h"


#ifdef GL_VERTEX_ARRAY
   #define USE_VERTEX_ARRAYS
#else
   #undef USE_VERTEX_ARRAYS
#endif

#ifdef _WIN32
//   #undef MULTITEX_ENABLED
//   #undef GL_VERTEX_PROGRAM_NV
#endif

#ifdef GL_ARB_multitexture
   #define MULTITEX_ENABLED
#else
   #undef MULTITEX_ENABLED
#endif

#define USE_TRISTRIPS
//#undef USE_VERTEX_ARRAYS

#ifndef NO_NV_FRESNEL
#define USE_BALL_FRESNEL
#endif

//#define TEST_FRESNEL

static int             fresnel_vert_prog_bind;
static const unsigned char * fresnel_vert_prog_world_coord_str=
            "!!VP1.0 \n"
//            #
//            # c[0-3]   = projection matrix
//            #            TrackMatrixNV(GL_VERTEX_PROGRAM_NV, 0, GL_PROJECTION, GL_IDENTITY_NV);
//            # c[4-7]   = texture matrix
//            #            TrackMatrixNV(GL_VERTEX_PROGRAM_NV, 4, GL_TEXTURE, GL_IDENTITY_NV);
//            # c[8].z   = offset for correct z-buffering
//            # c[8].x   = 0.0
//            # c[8].y   = 1.0
//            # c[8].w   = Rmax(=1) - Rmin
//               # c[9]  = ball_pos
//               # c[10].x  = ball_r
//               # c[10].y  = cuberef offset in viewer dir
//            # c[12-15] = modelview matrix
//            #            TrackMatrixNV(GL_VERTEX_PROGRAM_NV, 12, GL_MODELVIEW, GL_IDENTITY_NV);
//            #
            /* R1 = pos = -campos */
            "DP4   R1.x, c[12], v[OPOS];         \n"
            "DP4   R1.y, c[13], v[OPOS];         \n"
            "DP4   R1.z, c[14], v[OPOS];         \n"
            "DP4   R1.w, c[15], v[OPOS];         \n"
            /* R0 = nrml */
            "DP3   R0.x, c[12], v[NRML];         \n"
            "DP3   R0.y, c[13], v[NRML];         \n"
            "DP3   R0.z, c[14], v[NRML];         \n"
            /* R4=camdir=dist(cam,pos) */
            "MOV   R4, -R1;                  \n"
               /* normalize */
               "MOV   R5, R4;                    \n"
               "DP3   R4.w, R5, R5;              \n"
               "RSQ   R4.w, R4.w;                \n"
               "MUL   R4.xyz, R5, R4.w;          \n"
            /* R5 = refdir = -cam+2n*cam.n */
            "DP3   R3.w, R4, R0;                 \n"
            "ADD   R3.w, R3.w, R3.w;             \n"
            "MAD   R5, R3.w, R0, -R4;            \n"
            /* offset consideration for R5 */
/*               "ADD   R3, -c[9], v[OPOS];           \n"
               "MOV   R1, c[15];                    \n"
               "ADD   R1, R1, -c[9];                \n"
               "DP3   R5.w, R2, R1;                 \n"
               "MUL   R2.xyz, R1, R5.w;             \n"
               "DP3   R5.w, R1, R1;                 \n"
               "RCP   R5.w, R5.w;                   \n"
               "MUL   R2.xyz, R2, R5.w;             \n"
               "RCP   R5.w, c[10].x;                \n"
               "MUL   R2.xyz, R2, R5.w;             \n"*/
            /* col0 = fresnel(n) = e.n */
            "DP3   R3.x, R4, R0;                 \n"
//            "ADD   R3.x, c[8].y, -R3.x;          \n"
//            "MUL   R3.x, R3.x, R3.x;             \n"
//            "ADD   R3.x, c[8].y, -R3.x;          \n"
            "MUL   R3.x, R3.x, c[8].w;           \n"
            "ADD   R3.x, R3.x, R3.x;             \n"
            "ADD   R3.x, R3.x, -c[8].w;          \n"
            "ADD   o[COL0].xyz, c[8].y, -R3.x;   \n"
            /* o[TEX0] = refl vect in table coords */
            "DP3   o[TEX0].x, R5, c[4];          \n"
            "DP3   o[TEX0].y, R5, c[5];          \n"
            "DP3   o[TEX0].z, R5, c[6];          \n"
            /* apply modelview + projection */
            "DP4   o[HPOS].x, c[0], R1;          \n"
            "DP4   o[HPOS].y, c[1], R1;          \n"
            "DP4   R3.z,      c[2], R1;          \n"
            "DP4   o[HPOS].w, c[3], R1;          \n"
            "ADD   o[HPOS].z, R3.z, -c[8].z;     \n"
            "END                                 \n";

static const unsigned char * fresnel_vert_prog_obj_coord_str=  /* object coordinates */
            "!!VP1.0 \n"
//            #
//            # c[0-3]   = modelview projection (composite) matrix
//            #            TrackMatrixNV(GL_VERTEX_PROGRAM_NV, 0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);
//            # c[4-7]   = texture matrix
//            #            TrackMatrixNV(GL_VERTEX_PROGRAM_NV, 4, GL_TEXTURE, GL_IDENTITY_NV);
//            # c[8].z   = offset for correct z-buffering
//            # c[8].x   = 0.0
//            # c[8].y   = 1.0
//            # c[8].w   = Rmax(=1) - Rmin
//               # c[10].x  = ball_r
//               # c[10].y  = cuberef offset in viewer dir
//            # c[12-15] = modelview inverse matrix
//            #            TrackMatrixNV(GL_VERTEX_PROGRAM_NV, 12, GL_MODELVIEW, GL_INVERSE_TRANSPOSE);
//            #              => eye vector is one line and upper 3x3 matrix = modelview identity
//            #
            /* offset consideration */
               "MOV   R0, c[15];  \n"
                /* normalize */
                  "MOV   R5, R0;                    \n"
                  "DP3   R0.w, R5, R5;              \n"
                  "RSQ   R0.w, R0.w;                \n"
                  "MUL   R0.xyz, R5, R0.w;          \n"
               "DP3   R1.w, v[OPOS], R0; \n"
               "ADD   R1.w, R1.w, -c[10].y; \n"
               "RCP   R3.w, c[10].x; \n"
               "MUL   R1.w, R1.w, R3.w; \n"
               "MUL   R2.xyz, R0, R1.w; \n"
/*               "MUL   R0.xyz, R0, c[10].y;          \n"
               "ADD   R2, -R0, v[OPOS];           \n"
               "MOV   R1, c[15];                    \n"
               "ADD   R1, R1, -R0;                \n"
               "DP3   R5.w, R2, R1;                 \n"
               "MUL   R2.xyz, R1, R5.w;             \n"
               "DP3   R5.w, R1, R1;                 \n"
               "RCP   R5.w, R5.w;                   \n"
               "MUL   R2.xyz, R2, R5.w;             \n"
               "RCP   R5.w, c[10].x;                \n"
               "MUL   R2.xyz, R2, R5.w;             \n"*/
            /* R4=camdir=dist(cam,pos) */
            "ADD   R4, c[15], -v[OPOS];          \n"
               /* normalize */
               "MOV   R5, R4;                    \n"
               "DP3   R4.w, R5, R5;              \n"
               "RSQ   R4.w, R4.w;                \n"
               "MUL   R4.xyz, R5, R4.w;          \n"
            /* R5 = refdir = -cam+2n*cam.n */
            "DP3   R3.w, R4, v[NRML];            \n"
            "ADD   R3.w, R3.w, R3.w;             \n"
            "MAD   R5, R3.w, v[NRML], -R4;       \n"
            /* col0 = fresnel(n) = e.n */
            "DP3   R3.x, R4, v[NRML];            \n"
//            "ADD   R3.x, c[8].y, -R3.x;          \n"
//            "MUL   R3.x, R3.x, R3.x;             \n"
//            "ADD   R3.x, c[8].y, -R3.x;          \n"
            "MUL   R3.x, R3.x, c[8].w;           \n"
            "ADD   R3.x, R3.x, R3.x;             \n"
            "ADD   R3.x, R3.x, -c[8].w;          \n"
            "ADD   o[COL0].xyz, c[8].y, -R3.x;   \n"
            /* R3 = refl vect in world coords */
               "ADD   R5, R5, -R2;                   \n"
            "DP3   R3.x, R5, c[12];              \n"
            "DP3   R3.y, R5, c[13];              \n"
            "DP3   R3.z, R5, c[14];              \n"
            /* o[TEX0] = refl vect in table coords */
            "DP3   o[TEX0].x, R3, c[4];          \n"
            "DP3   o[TEX0].y, R3, c[5];          \n"
            "DP3   o[TEX0].z, R3, c[6];          \n"
            /* apply modelview + projection */
            "DP4   o[HPOS].x, c[0], v[OPOS];     \n"
            "DP4   o[HPOS].y, c[1], v[OPOS];     \n"
            "DP4   R3.z,      c[2], v[OPOS];     \n"
            "DP4   o[HPOS].w, c[3], v[OPOS];     \n"
            "ADD   o[HPOS].z, R3.z, -c[8].z;     \n"
            "END                                 \n";
#define fresnel_vert_prog_str fresnel_vert_prog_obj_coord_str



static char * balltexdata[22];
static GLuint balltexbind[22];
static int    balltexw,balltexh;
static int    shadowtexw,shadowtexh;
static char * shadowtexdata;
static GLuint shadowtexbind;
static int    depth;

static GLfloat col_null [4] = {0.0, 0.0, 0.0, 0.0}; /* dont need any specular because of reflections */
static GLfloat col_spec [4] = {0.0, 0.0, 0.0, 1.0}; /* dont need any specular because of reflections */
static GLfloat col_spec2[4] = {1.0, 1.0, 1.0, 1.0}; /* dont need any specular because of reflections */
//static GLfloat col_refl [4] = {1.0, 1.0, 1.0, 0.28};
static GLfloat col_refl [4] = {1.0, 1.0, 1.0, 0.28};
static GLfloat col_refl3[4] = {1.0, 1.0, 1.0, 0.60};   /* for rendered cubemap */
static GLfloat col_refl4[4] = {1.0, 1.0, 1.0, 1.0};   /* for rendered cubemap (fresnel) */
static GLfloat col_diff [4] = {0.7, 0.7, 0.7, 1.0};
static GLfloat col_diff2[4] = {0.75, 0.75, 0.75, 1.0};
static GLfloat col_diff3[4] = {0.69, 0.69, 0.69, 1.0};
static GLfloat col_amb  [4] = {0.2, 0.2, 0.2, 1.0};
static GLfloat col_amb2 [4] = {0.45, 0.45, 0.45, 1.0}; /* for simple reflections */
static GLfloat col_amb3 [4] = {0.31, 0.31, 0.31, 1.0}; /* for simple reflections */
static GLfloat col_shad [4] = {0.5, 0.0, 0.0, 0.0};


enum BallSet { BALLSET_POOL, BALLSET_CARAMBOL, BALLSET_SNOOKER, BALLSET_NONE };

enum BallSet g_ballset=BALLSET_NONE;



void normalize(float *v) /*FOLD00*/
{
    GLfloat d;
    d = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    v[0] /= d; v[1] /= d; v[2] /= d;
}


void rescale(float *v, float r)
{
    GLfloat d;
    d = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2])/r;
    v[0] /= d; v[1] /= d; v[2] /= d;
}


/*
void normalize_my(myfloat *v)
{
    GLfloat d;
    d = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    v[0] /= d; v[1] /= d; v[2] /= d;
}
*/


static int in_array_old( ElemArray * array, GLfloat * v, GLfloat tex_x, GLfloat tex_y )
{
    int i;

    for(i=0;i<array->vnr;i++){
        if( v[0]==array->vert[i*3+0] &&
            v[1]==array->vert[i*3+1] &&
            v[2]==array->vert[i*3+2] &&
            tex_x==array->tex[i*2+0] &&
            tex_y==array->tex[i*2+1] ){
            return i;
        }
    }
    for(i=0;i<array->vnr;i++){
        if( v[0]==array->vert[i*3+0] &&
            v[1]==array->vert[i*3+1] &&
            v[2]==array->vert[i*3+2] ){
            return -2;
        }
    }

    return -1;
}


static int in_array( ElemArray * array, GLfloat * v, GLfloat tex_x, GLfloat tex_y )
{
    int i;

    for(i=0;i<array->vnr;i++){
        if( fabs(v[0]-array->vert[i*3+0])<1.0E-6 &&
            fabs(v[1]-array->vert[i*3+1])<1.0E-6 &&
            fabs(v[2]-array->vert[i*3+2])<1.0E-6 &&
            fabs(tex_x-array->tex[i*2+0])<1.0E-6 &&
            fabs(tex_y-array->tex[i*2+1])<1.0E-6 ){
            return i;
        }
    }
    for(i=0;i<array->vnr;i++){
        if( fabs(v[0]-array->vert[i*3+0])<1.0E-6 &&
            fabs(v[1]-array->vert[i*3+1])<1.0E-6 &&
            fabs(v[2]-array->vert[i*3+2])<1.0E-6 ){
            return -2;
        }
    }

    return -1;
}


void createvertex( float * v, float * n,
                   float tex_x, float tex_y, ElemArray * array )
{
    static int count;
    int pos;
    if ( array == NULL ){
//        fprintf(stderr,"creating glMultiTexCoord2fARB\n");
//        glMultiTexCoord2fARB(GL_TEXTURE0_ARB,tex_x, tex_y );
//        glMultiTexCoord2fARB(GL_TEXTURE1_ARB,tex_x, tex_y );
        DPRINTF("creating vertex (non-array)\n");
        glTexCoord2f( tex_x, tex_y );
        glNormal3fv(n);
        glVertex3fv(v);
    } else {
        if(array->indnr==0) count=0;
        pos = in_array( array,v, tex_x, tex_y );
        if( pos>-1 ){
            array->index[array->indnr] = pos;
            array->indnr++;
        } else {
            array->vert[array->vnr*3+0]=v[0];
            array->vert[array->vnr*3+1]=v[1];
            array->vert[array->vnr*3+2]=v[2];
            if( pos!=-1 ){
                count++;
                DPRINTF("count=%d\n",count);
//                fprintf(stderr,"texx,texy=(%f,%f);\n",tex_x,tex_y);
//                array->vert[array->vnr*3+0]*=1.1;
//                array->vert[array->vnr*3+1]*=1.1;
//                array->vert[array->vnr*3+2]*=1.1;
            }
            array->norm[array->vnr*3+0]=n[0];
            array->norm[array->vnr*3+1]=n[1];
            array->norm[array->vnr*3+2]=n[2];
            array->tex[array->vnr*2+0]=tex_x;
            array->tex[array->vnr*2+1]=tex_y;
            array->index[array->indnr] = array->vnr;
            array->vnr++;
            array->indnr++;
            DPRINTF("createvertex: array->vnr=%d\n",array->vnr);
        }
    }
}

// because used in ball_subdivide_rec
void ball_subdivide9_rec(float *v1, float *v2, float *v3, const int depth, float r, ElemArray *array);

void ball_subdivide_rec(float *v1, float *v2, float *v3, const int depth, float r, ElemArray *array)
/* if array=0 just make vertices */
{
    GLfloat v12[3], v23[3], v31[3], v123[3];
    GLfloat n1[3], n2[3], n3[3];
    GLint i;
    int otherside;
    double xt,yt,rho;

//    DPRINTF("ball_subdivide_rec\n");

//    fprintf(stderr,"depth=%d\n",depth);
    if (depth == 0 || depth==1) {

        for(i=0;i<3;i++){
            n1[i]=v1[i];   n2[i]=v2[i];   n3[i]=v3[i];
        }
        normalize(n1);  normalize(n2);  normalize(n3);
        rescale(v1,r);  rescale(v2,r);  rescale(v3,r);

//        otherside = ( v1[2]>0.0 || v2[2]>0.0 || v3[2]>0.0 );
        otherside = ( v1[2]+v2[2]+v3[2]>0.0 );
//        glEnable(GL_NORMALIZE);
//        glTexCoord2f(0.5+0.5*v1[0], 0.5-0.5*(otherside?v1[1]:-v1[1]) );
        rho = sqrt(v1[0]*v1[0]+v1[1]*v1[1]);
        xt=asin(rho)/M_PI*2.3*v1[0]/rho;
        yt=asin(rho)/M_PI*2.3*v1[1]/rho;
        xt=v1[0]/r;
        yt=v1[1]/r;
        createvertex( v1, n1, 0.5-0.5*(otherside?xt:-xt), 0.5+0.5*yt, array);
//        glTexCoord2f(0.5+0.5*xt, 0.5-0.5*(otherside?yt:-yt) );
//        glNormal3fv(n1);
//        glVertex3fv(v1);

        rho = sqrt(v2[0]*v2[0]+v2[1]*v2[1]);
        xt=asin(rho)/M_PI*2.3*v2[0]/rho;
        yt=asin(rho)/M_PI*2.3*v2[1]/rho;
        xt=v2[0]/r;
        yt=v2[1]/r;
        createvertex( v2, n2, 0.5-0.5*(otherside?xt:-xt), 0.5+0.5*yt, array);
//        glTexCoord2f(0.5+0.5*xt, 0.5-0.5*(otherside?yt:-yt) );
//        glNormal3fv(n2);
//        glVertex3fv(v2);

        rho = sqrt(v3[0]*v3[0]+v3[1]*v3[1]);
        xt=asin(rho)/M_PI*2.3*v3[0]/rho;
        yt=asin(rho)/M_PI*2.3*v3[1]/rho;
        xt=v3[0]/r;
        yt=v3[1]/r;
        createvertex( v3, n3, 0.5-0.5*(otherside?xt:-xt), 0.5+0.5*yt, array);
//        glTexCoord2f(0.5+0.5*xt, 0.5-0.5*(otherside?yt:-yt) );
//        glNormal3fv(n3);
//        glVertex3fv(v3);

    } else if( depth==1 ){
        for (i = 0; i < 3; i++) v123[i] = v1[i]+v2[i]+v3[i];
        rescale(v123,r);
        ball_subdivide_rec( v123, v1, v2, depth-1, r, array);
        ball_subdivide_rec( v123, v2, v3, depth-1, r, array);
        ball_subdivide_rec( v123, v3, v1, depth-1, r, array);
    } else if( depth==3 ){
        ball_subdivide9_rec( v1, v2, v3, depth-2, r, array);
/*    } else if( depth==4 ){
        for (i = 0; i < 3; i++) {
            v12[i] = v1[i]+v2[i];
            v23[i] = v2[i]+v3[i];
            v31[i] = v3[i]+v1[i];
        }
        rescale(v12,r);
        rescale(v23,r);
        rescale(v31,r);

        create_strip(v1,v12,v31,v23,v2);*/
    } else {
        for (i = 0; i < 3; i++) {
            v12[i] = v1[i]+v2[i];
            v23[i] = v2[i]+v3[i];
            v31[i] = v3[i]+v1[i];
        }
        rescale(v12,r);
        rescale(v23,r);
        rescale(v31,r);
        ball_subdivide_rec( v1, v12, v31, depth-2, r, array);
        ball_subdivide_rec( v2, v23, v12, depth-2, r, array);
        ball_subdivide_rec( v3, v31, v23, depth-2, r, array);
        ball_subdivide_rec(v12, v23, v31, depth-2, r, array);
    }
//    fprintf(stderr,"back again all (depth=%d)\n",depth);
}


void ball_subdivide9_rec(float *v1, float *v2, float *v3, const int depth, float r, ElemArray *array)
{
    GLfloat v112[3], v122[3], v223[3], v233[3], v331[3], v311[3], v123[3];
    GLint i;

//    fprintf(stderr,"depth=%d\n",depth);
    if (depth == 0) {
        ball_subdivide_rec(v1,v2,v3,0,r, array); // draw the triangle
    } else {
        for (i = 0; i < 3; i++) {
            v112[i] = v1[i]+v1[i]+v2[i];
            v122[i] = v1[i]+v2[i]+v2[i];
            v223[i] = v2[i]+v2[i]+v3[i];
            v233[i] = v2[i]+v3[i]+v3[i];
            v331[i] = v3[i]+v3[i]+v1[i];
            v311[i] = v3[i]+v1[i]+v1[i];
            v123[i] = v1[i]+v2[i]+v3[i];
        }
        rescale(v112,r);
        rescale(v122,r);
        rescale(v223,r);
        rescale(v233,r);
        rescale(v331,r);
        rescale(v311,r);
        rescale(v123,r);
        ball_subdivide9_rec( v1  , v112, v311, depth-1, r, array);
        ball_subdivide9_rec( v112, v122, v123, depth-1, r, array);
        ball_subdivide9_rec( v112, v123, v311, depth-1, r, array);
        ball_subdivide9_rec( v311, v123, v331, depth-1, r, array);
        ball_subdivide9_rec( v122, v2  , v223, depth-1, r, array);
        ball_subdivide9_rec( v122, v223, v123, depth-1, r, array);
        ball_subdivide9_rec( v123, v223, v233, depth-1, r, array);
        ball_subdivide9_rec( v123, v233, v331, depth-1, r, array);
        ball_subdivide9_rec( v331, v233, v3  , depth-1, r, array);
    }
//    fprintf(stderr,"back again all (depth=%d)\n",depth);
}



void ball_subdivide_nonrec(float *v1, float *v2, float *v3, int depth, float r, ElemArray *array)
/* if array=0 just make vertices */
{
    GLfloat v12[3], v13[3], v[3];
    GLfloat n[3];
    GLint i,j,k,x,y;
    double xf,yf,vl;
    int otherside;
    double xt,yt,rho;
    int subdiv;

    if(depth==1) depth=0;
    if(!(depth&1)){ /* even */
        subdiv=1<<(depth/2);
    }else{          /* odd */
        subdiv=(1<<((depth-3)/2))*3;
    }


    for (i = 0; i < 3; i++) {
        v12[i] = v2[i]-v1[i];
        v13[i] = v3[i]-v1[i];
    }
    otherside = ( v1[2]+v2[2]+v3[2]>0.0 );
    for( i=0; i<subdiv; i++ ){ /* make tristrips */
        if(array==NULL){
            glBegin(GL_TRIANGLE_STRIP);
        } else {
            array->prim_size[array->num_prim]=2*(subdiv-i)+1;
            (array->num_prim)++;
        }
        for(j=0;j<2*(subdiv-i)+1;j++){
            if(!(j&1)){ /* even */
                x=j/2;     y=i;
            } else {    /* odd */
                x=(j-1)/2; y=i+1;
            }
            xf=(double)x/(double)subdiv;
            yf=(double)y/(double)subdiv;
            vl=0.0;
            for(k=0;k<3;k++){
                v[k]=v1[k]+v13[k]*xf+v12[k]*yf;
                vl+=v[k]*v[k];
            }
            vl=sqrt(vl);
            for(k=0;k<3;k++){
                n[k]=v[k]/vl;
                v[k]=n[k]*r;
            }
            //        glEnable(GL_NORMALIZE);
            //        glTexCoord2f(0.5+0.5*v1[0], 0.5-0.5*(otherside?v1[1]:-v1[1]) );
/*            rho = sqrt(v[0]*v[0]+v[1]*v[1]);
            xt=asin(rho)/M_PI*2.3*v[0]/rho;
            yt=asin(rho)/M_PI*2.3*v[1]/rho;*/
            xt=n[0];
            yt=n[1];
            createvertex( v, n, 0.5-0.5*(otherside?xt:-xt), 0.5+0.5*yt, array);
        }
        if(array==NULL){
            glEnd();
        }
    }
//    fprintf(stderr,"back again all (depth=%d)\n",depth);
}


void create_ball_icosa( float r, int ddepth, int id ) /*FOLD00*/
{
#define X .525731112119133606
#define Z .850650808352039932

    int i;

    static GLfloat vdata[12][3] = {
        {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},
        {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},
        {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0}
    };

    static GLint tindices[20][3] = {
        {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},
        {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},
        {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
        {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11}
    };

    DPRINTF("create_ball_icosa\n");

    glDisable( GL_NORMALIZE );   /* remove this */
    glNewList(id, GL_COMPILE);
//      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
//      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glPushMatrix();
//      glScalef( r, r, r );
//      glEnable( GL_NORMALIZE );   /* remove this */
#ifndef USE_TRISTRIPS
      glBegin(GL_TRIANGLES);
#endif
         for (i = 0; i < 20; i++) {
#ifndef USE_TRISTRIPS
             ball_subdivide_rec(
#else
             ball_subdivide_nonrec(
#endif
                                &vdata[tindices[i][0]][0],
                                &vdata[tindices[i][1]][0],
                                &vdata[tindices[i][2]][0],
                                ddepth, r, NULL
                               );
         }
#ifndef USE_TRISTRIPS
      glEnd();
#endif
      glPopMatrix();
//      glDisable( GL_NORMALIZE );   /* remove this */
    glEndList();

}


#ifdef USE_VERTEX_ARRAYS

ElemArray * create_ball_icosa_array( float r, int ddepth, int id )
{
#define X .525731112119133606
#define Z .850650808352039932

    ElemArray * array;
    int i,pnr,fnr;
    double volume, scale_ratio;
    VMvect v1,v2,v3;

    static GLfloat vdata[12][3] = {
        {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},
        {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},
        {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0}
    };

    static GLint tindices[20][3] = {
        {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},
        {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},
        {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
        {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11}
    };

    static int init = 0;


    if(!init){
        init=1;
         for (i = 0; i < 12; i++) {
             rescale(vdata[i],r);
         }
    }

    if(ddepth%2==0){
        fnr = 20*(1<<(ddepth/2*2));
        pnr = 10*((1<<(ddepth/2*2))-1) + 12;
    } else if(ddepth!=1) {
        fnr = 20*(1<<((ddepth-3)/2*2));
        pnr = 10*((1<<((ddepth-3)/2*2))-1) + 12;
        pnr += fnr * 4;
        fnr *= 9;
    } else if(ddepth==1) {
        fnr = 20*(1<<(ddepth/2*2));
        pnr = 10*((1<<(ddepth/2*2))-1) + 12;
        pnr += fnr; fnr *= 3;
    }
    DPRINTF("pnr-ideal=%d\n",pnr);
    pnr+=(int)(sqrt(pnr)+0.5)*2;  /* could (still?) get narrow (theoret. *sqrt(PI) instead of *2) */
    DPRINTF("pnr-nachher=%d\n",pnr);

    array = (ElemArray *)malloc( sizeof(ElemArray) );
    array->vert  = (GLfloat *)malloc( pnr*3*sizeof(GLfloat) );
    array->norm  = (GLfloat *)malloc( pnr*3*sizeof(GLfloat) );
    array->tex   = (GLfloat *)malloc( pnr*2*sizeof(GLfloat) );
    array->reftex = (GLfloat *)malloc( pnr*2*sizeof(GLfloat) );
    array->index = (int *)    malloc( fnr*3*sizeof(int)     );
    array->num_prim = 0;

    array->indnr=0;
    array->vnr=0;

    DPRINTF("fnr*3=%d\n",fnr*3);
    DPRINTF("pnr=%d\n",pnr);

    for (i = 0; i < 20; i++) {
#ifndef USE_TRISTRIPS
        ball_subdivide_rec(
#else
        ball_subdivide_nonrec(
#endif
                           &vdata[tindices[i][0]][0],
                           &vdata[tindices[i][1]][0],
                           &vdata[tindices[i][2]][0],
                           ddepth, r, array
                          );
    }

    DPRINTF("array->indnr=%d\n",array->indnr);
    DPRINTF("array->vnr=%d\n",array->vnr);
/*
    glNewList(id, GL_COMPILE);
      glPushMatrix();
      glBegin(GL_TRIANGLES);
         for (i = 0; i < array->indnr; i++) {
             glTexCoord2fv( &array->tex [array->index[i]*2] );
             glNormal3fv(   &array->norm[array->index[i]*3] );
             glVertex3fv(   &array->vert[array->index[i]*3] );
             fprintf(stderr,"v=<%f,%f,%f>\n",
                     array->vert[array->index[i]*3],
                     array->vert[array->index[i]*3+1],
                     array->vert[array->index[i]*3+2] );
         }
      glEnd();
      glPopMatrix();
    glEndList();
*/

#if 1
    DPRINTF("create_ball_icosa_array: rescaling vertices\n");
#ifndef USE_TRISTRIPS
    volume=0.0;
    for(i=0;i<array->indnr;i+=3){

        v1.x = array->vert[array->index[i+0]*3+0];
        v1.y = array->vert[array->index[i+0]*3+1];
        v1.z = array->vert[array->index[i+0]*3+2];

        v2.x = array->vert[array->index[i+1]*3+0];
        v2.y = array->vert[array->index[i+1]*3+1];
        v2.z = array->vert[array->index[i+1]*3+2];

        v3.x = array->vert[array->index[i+2]*3+0];
        v3.y = array->vert[array->index[i+2]*3+1];
        v3.z = array->vert[array->index[i+2]*3+2];

        volume+=tri_vol_xy( v1, v2, v3 );
    }
    DPRINTF("create_ball_icosa_array: volume=%f\n",volume);
#else
    volume=0.0;
    {
        int j;
        int actind=0;
        for(i=0;i<array->num_prim;i++){
            for(j=0;j<array->prim_size[i]-2;j++){

                v1.x = array->vert[array->index[actind+j+0]*3+0];
                v1.y = array->vert[array->index[actind+j+0]*3+1];
                v1.z = array->vert[array->index[actind+j+0]*3+2];

                v2.x = array->vert[array->index[actind+j+1]*3+0];
                v2.y = array->vert[array->index[actind+j+1]*3+1];
                v2.z = array->vert[array->index[actind+j+1]*3+2];

                v3.x = array->vert[array->index[actind+j+2]*3+0];
                v3.y = array->vert[array->index[actind+j+2]*3+1];
                v3.z = array->vert[array->index[actind+j+2]*3+2];

                if(!(j&1)){
                    volume+=tri_vol_xy( v1, v2, v3 );
                } else {
                    volume-=tri_vol_xy( v1, v2, v3 );
                }
            }
            actind+=array->prim_size[i];
        }
    }
    DPRINTF("create_ball_icosa_array: volume=%f\n",volume);
#endif
    scale_ratio=pow((4.0/3.0*r*r*r*M_PI)/fabs(volume),1.0/3.0);
    DPRINTF("create_ball_icosa_array: scale_ratioe=%f\n",scale_ratio);
    for(i=0;i<array->vnr*3;i++){
        array->vert[i] = array->vert[i]*scale_ratio;
    }
    DPRINTF("create_ball_icosa_array: compiling list\n");
    glNewList(id, GL_COMPILE);
      glPushMatrix();
        /* call array */
        glEnableClientState( GL_VERTEX_ARRAY );
        glEnableClientState( GL_NORMAL_ARRAY );
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        glVertexPointer( 3, GL_FLOAT, 0, array->vert );
        glNormalPointer( GL_FLOAT, 0, array->norm );
        glTexCoordPointer( 2, GL_FLOAT, 0, array->tex );

        DPRINTF("indnr=%d\n",array->indnr);
        DPRINTF("vnr=%d\n",array->vnr);

#ifndef USE_TRISTRIPS
        glDrawElements( GL_TRIANGLES, array->indnr, GL_UNSIGNED_INT, array->index );
#else
        {
            int actind=0;
            int i;
            for(i=0;i<array->num_prim;i++){
                glDrawElements( GL_TRIANGLE_STRIP, array->prim_size[i], GL_UNSIGNED_INT, &(array->index[actind]) );
                actind+=array->prim_size[i];
            }
        }
#endif

//      glEnd();
      glPopMatrix();
    glEndList();
#endif
    
//    fprintf(stderr,"array compiled %d, NE=%d, %d\n",glGetError(),GL_NO_ERROR, array);

    return array;
}

#endif


void calc_reftex( BallType * ball, myvec cam_pos,
                  GLfloat * vp, GLfloat * np,
                  GLfloat * tex, int nr )
{
    int i;
    double phi,th,v_abs,v_xy;
    myvec v_ref,v_in,n,norm;

    for(i=0;i<nr;i++){
        n.x=np[i*3+0];
        n.y=np[i*3+1];
        n.z=np[i*3+2];
        norm = vec_xyz(0,0,0);
        norm = vec_add( norm, vec_scale(ball->b[0],n.x) );
        norm = vec_add( norm, vec_scale(ball->b[1],n.y) );
        norm = vec_add( norm, vec_scale(ball->b[2],n.z) );
        v_in  = vec_diff( ball->r, cam_pos );
        v_ref = vec_diff( v_in, vec_scale(vec_proj(v_in,norm),2.0) );
        v_abs = vec_abs(v_ref);
        v_xy  = sqrt(v_ref.x*v_ref.x+v_ref.y*v_ref.y);
        th    = acos(v_ref.z/v_abs);
        phi   = atan2(v_ref.y,v_ref.x);
        tex[i*2+0] = 0.5*(1.0+phi/M_PI);
        tex[i*2+1] = 0.5*th/M_PI;
    }
}


VMvect cutphi0( VMvect p1, VMvect p2 )
{
    return
    vec_scale(
              vec_add(vec_scale(p1,fabs(p2.y)),vec_scale(p2,fabs(p1.y))),
              1.0/fabs(p2.y-p1.y)
              );
}


void myDrawElements( BallType * ball, ElemArray * array, myvec cam_pos )
{
    int i,j,k,l,m,n,tchanged;
    VMvect t[3], p[3];
    int cutnr;
    int cutnr1[3];
    int cutnr2[3];
    double phi,th,v_abs,v_xy;
    myvec v_ref[3],v_in;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBegin(GL_TRIANGLES);

    for(i=0;i<array->indnr;i+=3){
        j=array->index[i];
        k=array->index[i+1];
        l=array->index[i+2];

        t[0].x=array->reftex[j*2];  t[0].y=array->reftex[j*2+1];  t[0].z=0.0;
        t[1].x=array->reftex[k*2];  t[1].y=array->reftex[k*2+1];  t[1].z=0.0;
        t[2].x=array->reftex[l*2];  t[2].y=array->reftex[l*2+1];  t[2].z=0.0;

        p[0]=vec_xyz(0,0,0);
        p[0]=vec_add(p[0],vec_scale(ball->b[0],array->norm[j*3]));
        p[0]=vec_add(p[0],vec_scale(ball->b[1],array->norm[j*3+1]));
        p[0]=vec_add(p[0],vec_scale(ball->b[2],array->norm[j*3+2]));
        p[1]=vec_xyz(0,0,0);
        p[1]=vec_add(p[1],vec_scale(ball->b[0],array->norm[k*3]));
        p[1]=vec_add(p[1],vec_scale(ball->b[1],array->norm[k*3+1]));
        p[1]=vec_add(p[1],vec_scale(ball->b[2],array->norm[k*3+2]));
        p[2]=vec_xyz(0,0,0);
        p[2]=vec_add(p[2],vec_scale(ball->b[0],array->norm[l*3]));
        p[2]=vec_add(p[2],vec_scale(ball->b[1],array->norm[l*3+1]));
        p[2]=vec_add(p[2],vec_scale(ball->b[2],array->norm[l*3+2]));

        for(m=0;m<3;m++){
            v_in     = vec_diff( ball->r, cam_pos );
//            v_ref[m] = vec_unit(vec_diff( v_in, vec_scale(vec_proj(v_in,p[m]),2.0) ));
            v_ref[m] = vec_unit(p[m]);
            v_abs    = vec_abs(v_ref[m]);
            v_xy     = sqrt(v_ref[m].x*v_ref[m].x+v_ref[m].y*v_ref[m].y);
            th       = acos(v_ref[m].z/v_abs);
            phi      = atan2(v_ref[m].y,v_ref[m].x);
            t[m].x   = 0.5*(1.0+phi/M_PI);
            t[m].y   = 0.5*th/M_PI;
//            fprintf(stderr,"tex%d=%f,%f,%f\n",m,p[m].x,p[m].y,p[m].z);
        }

/*        if( tri_area_xy( t1,t2,t3 ) < 0.0 ){
            if( 1.0-t1.x > t1.x ) t1.x+=1.0;
            if( 1.0-t2.x > t2.x ) t2.x+=1.0;
            if( 1.0-t3.x > t3.x ) t3.x+=1.0;
            }*/

        tchanged=0;
        cutnr=0;
        for(m=0;m<3;m++){ /* lines 1-2  2-3  3-1 */
            n=(m+1)%3;
            if        ( v_ref[m].y>0.0 && v_ref[n].y<0.0 ){
                if( cutphi0(v_ref[m],v_ref[n]).x < 0.0 ){
//                if( v_ref[m].x < 0.0 ){
                    cutnr1[cutnr]=m; cutnr2[cutnr]=n;  /* der kleinere zuerst */
                    cutnr++;
                }
            } else if ( v_ref[m].y<0.0 && v_ref[n].y>0.0 ){
                if( cutphi0(v_ref[m],v_ref[n]).x < 0.0 ){
//                if( v_ref[m].x < 0.0 ){
                    cutnr1[cutnr]=n; cutnr2[cutnr]=m;
                    cutnr++;
                }
            }
        }
//        fprintf(stderr,"cutnr=%d\n",cutnr);
                                   
        if(cutnr==2){ /* 2 schnitte mit phi=0 ebene */
//            fprintf(stderr,"  cutnr2[0]=%d\n",cutnr2[0]);
//            fprintf(stderr,"  cutnr2[1]=%d\n",cutnr2[1]);
            t[cutnr1[0]].x-=1.0;
            if(cutnr1[1]!=cutnr1[0]) t[cutnr1[1]].x-=1.0;
                tchanged=1;
        } else if(cutnr==1){
#if 1
            int other;
//            if( t[0].y<0.25 && t[1].y<0.25 && t[2].y<0.25 ){
            if( t[0].y<0.25 ){
                t[cutnr1[0]].x-=1.0;
                other = ((cutnr1[0]+1)^(cutnr2[0]+1))-1;
//                fprintf(stderr,"other=%d\n",other);
                t[other].x-=0.5;
                t[other].y=-t[other].y;
                tchanged=1;
            }else{
                t[cutnr1[0]].x-=1.0;
                other = ((cutnr1[0]+1)^(cutnr2[0]+1))-1;
//                fprintf(stderr,"other=%d\n",other);
                t[other].x-=0.5;
                t[other].y=1.0-t[other].y;
                tchanged=1;
            }
#endif
        }

        if(cutnr!=1){
        glTexCoord2f( t[0].x,t[0].y );
        glNormal3f  ( array->norm[j*3],array->norm[j*3+1],array->norm[j*3+2] );
        glVertex3f  ( array->vert[j*3],array->vert[j*3+1],array->vert[j*3+2] );

        glTexCoord2f( t[1].x,t[1].y );
        glNormal3f  ( array->norm[k*3],array->norm[k*3+1],array->norm[k*3+2] );
        glVertex3f  ( array->vert[k*3],array->vert[k*3+1],array->vert[k*3+2] );

        glTexCoord2f( t[2].x,t[2].y );
        glNormal3f  ( array->norm[l*3],array->norm[l*3+1],array->norm[l*3+2] );
        glVertex3f  ( array->vert[l*3],array->vert[l*3+1],array->vert[l*3+2] );
        }
    }
    glEnd();
}



void free_elem_array( ElemArray * array )
{
    free( array->vert );
    free( array->norm );
    free( array->tex );
    free( array->reftex );
    free( array->index );
}



void draw_ball( BallType * ball, myvec cam_pos, GLfloat cam_FOV, int win_width, int reflect )
/* expects to be on table center */
{
    float cnear, cfar, geomfact;
    static int glballlist[]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
    static float ballmatr[]={
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0
    };
    int i,detail;
    static int maxdetail=-1;
    static ElemArray * array[11];


    if( maxdetail != options_max_ball_detail ){
        for(i=0;i<=maxdetail;i++){
            if(glballlist[i]!=-1) glDeleteLists(glballlist[i],1);
            glballlist[i]=-1;
#ifdef USE_VERTEX_ARRAYS
            for(i=0;i<=maxdetail;i++){
                free_elem_array( array[i] );
                array[i]=(ElemArray *)0;
            }
#endif
        }
        maxdetail = options_max_ball_detail;
    }

    if( glballlist[0]==-1 ){
        for(i=0;i<=maxdetail;i++){
            glballlist[i] = glGenLists(1);
#ifdef USE_VERTEX_ARRAYS
            array[i] = create_ball_icosa_array( BALL_D/2.0, i, glballlist[i] );
#else
            create_ball_icosa( BALL_D/2.0, i, glballlist[i] );
#endif
        }
    }

    glPushMatrix();

    glTranslatef( ball->r.x, ball->r.y, ball->r.z );

    ballmatr[ 0]=ball->b[0].x;
    ballmatr[ 1]=ball->b[0].y;
    ballmatr[ 2]=ball->b[0].z;

    ballmatr[ 4]=ball->b[1].x;
    ballmatr[ 5]=ball->b[1].y;
    ballmatr[ 6]=ball->b[1].z;

    ballmatr[ 8]=ball->b[2].x;
    ballmatr[ 9]=ball->b[2].y;
    ballmatr[10]=ball->b[2].z;

    ballmatr[15]=1.0;

    glMultMatrixf( ballmatr );

/*    if(showReflections){
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
    }*/

//    glEnable(GL_TEXTURE_2D);

    geomfact=40.0/cam_FOV*(double)win_width/800;
    cnear = options_ball_detail_nearmax*geomfact;
    cfar  = options_ball_detail_farmin*geomfact;
    //    detail=(int)((double)maxdetail-(vec_abs(vec_diff(cam_pos,ball->r))-cnear)/(cfar-cnear)*(double)maxdetail);

    /* this would lead to a constant face (triangle) size */
//    detail=maxdetail-2.0*log(vec_abs(vec_diff(cam_pos,ball->r))/options_ball_detail_nearmax)/log(2.0);
    /* 1.0 instead of 2.0 because angles get smaller when smaller detail */
    detail=maxdetail-1.0*log(vec_abs(vec_diff(cam_pos,ball->r))/cnear)/log(2.0);

    if( detail>maxdetail ) detail=maxdetail;
    if( detail<0 ) detail=0;
    #ifdef USE_VERTEX_ARRAYS
      #if 0
      glPushMatrix();
        glEnableClientState( GL_VERTEX_ARRAY );
        glEnableClientState( GL_NORMAL_ARRAY );
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        glVertexPointer( 3, GL_FLOAT, 0, array[detail]->vert );
        glNormalPointer( GL_FLOAT, 0, array[detail]->norm );
        if( !reflect ){
            glTexCoordPointer( 2, GL_FLOAT, 0, array[detail]->tex );
        } else {
            if(options_calc_ball_reflections){
/*                calc_reftex( ball, cam_pos,
                             array[detail]->vert, array[detail]->norm,
                             array[detail]->reftex, array[detail]->vnr);
                glTexCoordPointer( 2, GL_FLOAT, 0, array[detail]->reftex );*/
            } else {
                glTexCoordPointer( 2, GL_FLOAT, 0, array[detail]->tex );
            }
        }
        if(options_calc_ball_reflections && reflect){
//            myDrawElements( ball, array[detail], cam_pos );
            glDrawElements( GL_TRIANGLES, array[detail]->indnr, GL_UNSIGNED_INT, array[detail]->index );
        } else {
            glDrawElements( GL_TRIANGLES, array[detail]->indnr, GL_UNSIGNED_INT, array[detail]->index );
        }
      glPopMatrix();
      #endif
      glCallList(glballlist[detail]);
    #else
      glCallList(glballlist[detail]);
    #endif

    glColor3f(1.0, 1.0, 1.0);
    glPopMatrix();

    /* ang mom vec */
/*    glPushMatrix();
    glTranslatef( ball->r.x, ball->r.y, ball->r.z );
      wabs=vec_abs(ball->w);
      vabs=vec_abs(ball->v);
      if( wabs ){
      glBegin(GL_LINES);
        glVertex3f( 0.0, 0.0, 0.0 );
        glVertex3f(
                    ball->w.x/wabs*0.1,
                    ball->w.y/wabs*0.1,
                    ball->w.z/wabs*0.1
                  );
        glVertex3f( 0.0, 0.0, 0.0 );
        glVertex3f(
                    ball->v.x/vabs*0.1,
                    ball->v.y/vabs*0.1,
                    ball->v.z/vabs*0.1
                  );
      glEnd();
      }
    glPopMatrix();*/
}


void my_copy_area_1_3( char * src, int w1, int h1, int wc, int hc,
                       int x0, int y0, char * dst , int w, int h )
{
    int x,y,i;
    for(y=0;y<hc;y++){
        for(x=0;x<wc;x++){
            for(i=0;i<3;i++){
                if( src[y*w1+x]!=(char)0xFF )
                    dst[((y+y0)*w+x+x0)*3+i]=src[y*w1+x];
            }
        }
    }
}

//void create_poolballtex( int nr, int * w, int * h, int * depth, char ** data)
void create_balltex( int nr, int * w, int * h, int * depth, char ** data)
{
    int x,y;
    int bg_r, bg_g, bg_b;
    int fg_r, fg_g, fg_b;
    double r_hole;

    r_hole=60;
    if(nr<=9) r_hole=50;
    if(nr==0) r_hole=16;

    options_col_ball=options_col_ball_pool;
    fg_r = ((options_col_ball[nr])>>16) & 0xFF;
    fg_g = ((options_col_ball[nr])>> 8) & 0xFF;
    fg_b = ((options_col_ball[nr])>> 0) & 0xFF;

    if(nr==0){ bg_r=0x00; bg_g=0x00; bg_b=0x00; }
    else     { bg_r=0xFF; bg_g=0xFF; bg_b=0xFF; }

    *w=256;
    *h=256;
    *depth=24;
    *data=malloc((*w)*(*h)*3);

    /* bg color */
    for(y=0;y<*h;y++){
        for(x=0;x<*w;x++){
            (*data)[(*w*y+x)*3+0]=bg_r;
            (*data)[(*w*y+x)*3+1]=bg_g;
            (*data)[(*w*y+x)*3+2]=bg_b;
        }
    }

    /* ball color (strip for half) */
    for(y=0;y<*h;y++){
        for(x=0;x<*w;x++){
            if( nr<9 || (y>=64 && y<=192) ){
                (*data)[(*w*y+x)*3+0]=fg_r;
                (*data)[(*w*y+x)*3+1]=fg_g;
                (*data)[(*w*y+x)*3+2]=fg_b;
            }
        }
    }

    /* number circle */
    for(y=0;y<*h;y++){
        for(x=0;x<*w;x++){
            double r,dx,dy,fact;
            dx=x-(*w)/2;
            dy=y-(*h)/2;
            r=sqrt(dx*dx+dy*dy);
            if(r<r_hole+0.5){
                if(r>r_hole-0.5)
                    fact=r-r_hole+0.5;
                else
                    fact=0.0;
                (*data)[(*w*y+x)*3+0]=(double)fg_r*fact+(double)bg_r*(1.0-fact);
                (*data)[(*w*y+x)*3+1]=(double)fg_g*fact+(double)bg_g*(1.0-fact);
                (*data)[(*w*y+x)*3+2]=(double)fg_b*fact+(double)bg_b*(1.0-fact);
            }
        }
    }

    /* number */
    if(nr!=0){
        char str[80];
        char * nrdata;
        int width, height, dw, dh, i, xmin,xmax, ymin,ymax;
        double sx,sy,sum, sx2,sy2;
        sprintf(str,"%d",nr);
        getStringPixmapFT(str, options_ball_fontname, 124, &nrdata, &dw, &dh, &width, &height);
        sx=0.0; sy=0.0; sum=0.0;
        xmax=0; xmin=dw; ymax=0; ymin=dh;
        for(i=0;i<dw*dh;i++){
            if(nrdata[i]==(char)0xFF){
                if( (i%dw)<xmin ) xmin=(i%dw);
                if( (i%dw)>xmax ) xmax=(i%dw);
                if( (i/dw)<ymin ) ymin=(i/dw);
                if( (i/dw)>ymax ) ymax=(i/dw);
            }
            sx+=(double)(i%dw)*(double)((unsigned char)nrdata[i]);
            sy+=(double)(i/dw)*(double)((unsigned char)nrdata[i]);
            sum+=(double)((unsigned char)nrdata[i]);
//            fprintf(stderr,"%d",(unsigned char)nrdata[i]);
            nrdata[i]=~nrdata[i];
        }
        if(nr==6 || nr==9){
            for(y=ymax+5;y<ymax+10;y++){
                for(x=xmin+6;x<xmax-6;x++){
                    nrdata[y*dw+x]=0x00;
                }
            }
            ymax=ymax+10;
        }
        sx/=sum;
        sy/=sum;
        sx2=(double)(xmin+xmax)/2.0;
        sy2=(double)(ymin+ymax)/2.0;
        sx=(sx+sx2)/2.0;
        sy=(sy+sy2)/2.0;
//        fprintf(stderr,"sx=%f, sy=%f\n");
        my_copy_area_1_3( nrdata, dw, dh, width, height,
                          (*w)/2-(sx+0.5), (*h)/2-(sy+0.5), *data, *w, *h );
    }

    if( options_rgstereo_on ){
        int i;
        double d;
        /* graying out texture */
        for(i=0;i<(*h)*(*w);i++){
            d=((unsigned char)(*data)[i*3+0]+
               (unsigned char)(*data)[i*3+1]+
               (unsigned char)(*data)[i*3+2])/3.0;
            (*data)[i*3+0]=d;
            (*data)[i*3+1]=d;
            (*data)[i*3+2]=d;
        }
    }

}


void create_balltex_snooker( int nr, int * w, int * h, int * depth, char ** data)
{
    _NOT_IMPLEMENTED
}


void create_balltex_carambol( int nr, int * w, int * h, int * depth, char ** data)
{
    _NOT_IMPLEMENTED

}


void free_pooltexbinds( void )
{
    int i;
    for(i=0;i<16;i++){
        glDeleteTextures(1,&balltexbind[i]);
    }
    g_ballset=BALLSET_NONE;
}


void free_caramboltexbinds( void )
{
    _NOT_IMPLEMENTED
}


void free_snookertexbinds( void )
{
    _NOT_IMPLEMENTED
}


void create_pooltex_binds( void )
{
    char str[80];
    int i;

    g_ballset=BALLSET_POOL;
    for(i=0;i<16;i++){
        glGenTextures(1,&balltexbind[i]);
        sprintf(str,"billball%02d.png",i);
        if(options_autocreate_balltex){
            create_balltex(i,&balltexw,&balltexh,&depth,&balltexdata[i]);
        } else {
            load_png(str,&balltexw,&balltexh,&depth,&balltexdata[i]);
        }
        glBindTexture(GL_TEXTURE_2D,balltexbind[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options_tex_min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options_tex_mag_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, balltexw, balltexh, GL_RGB,
                          GL_UNSIGNED_BYTE, balltexdata[i]);
        free(balltexdata[i]);
        DPRINTF("balltexbind[%d]=%d\n",i,balltexbind[i]);
    }
}


void create_caramboltex_binds( void )
{
    _NOT_IMPLEMENTED
}


void create_snookertex_binds( void )
{
    _NOT_IMPLEMENTED
}


enum BallSet ballset_for_gametype(enum gameType g)
{
            switch(g){
            case GAME_8BALL:    return(BALLSET_POOL);     break;
            case GAME_9BALL:    return(BALLSET_POOL);     break;
            case GAME_CARAMBOL: return(BALLSET_CARAMBOL); break;
            case GAME_SNOOKER:  return(BALLSET_SNOOKER);  break;
            default:            return(BALLSET_NONE);
            }
}


void create_texbinds( BallsType *balls )
{
    switch(g_ballset){
    case BALLSET_POOL:      free_pooltexbinds();     break;
    case BALLSET_CARAMBOL:  free_caramboltexbinds(); break;
    case BALLSET_SNOOKER:   free_snookertexbinds();  break;
    case BALLSET_NONE:      break;
    }
    switch(balls->gametype){
    case GAME_8BALL:    create_pooltex_binds();     break;
    case GAME_9BALL:    create_pooltex_binds();     break;
    case GAME_CARAMBOL: create_caramboltex_binds(); break;
    case GAME_SNOOKER:  create_snookertex_binds();  break;
    }
}


void draw_balls( BallsType balls, myvec cam_pos, GLfloat cam_FOV, int win_width, int spheretexbind, VMvect * lightpos, int lightnr, int * cuberef_binds )
{
    static int init = 0;
    static int fresnel_init = 0;
    int i,j;
    double fact;
    VMvect v,vn;
    GLfloat stretch_matrix[16];


    if( !init ){

        if(options_ball_shadows){
            glGenTextures(1,&shadowtexbind);
            if(options_simple_ball_shadows){   // one threefold shadow under balls
                load_png("shadow_alpha.png",&shadowtexw,&shadowtexh,&depth,&shadowtexdata);
            }else{
                load_png("shadow2.png",&shadowtexw,&shadowtexh,&depth,&shadowtexdata);
            }
            glBindTexture(GL_TEXTURE_2D,shadowtexbind);
            gluBuild2DMipmaps(GL_TEXTURE_2D, 1, shadowtexw, shadowtexh, GL_LUMINANCE,
                          GL_UNSIGNED_BYTE, shadowtexdata);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options_tex_min_filter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options_tex_mag_filter);
            DPRINTF("shadowtexbind=%d\n",shadowtexbind);
        }

        if(options_ball_tex){
            create_texbinds(&balls);
        }

        init=1;
    }

#ifdef GL_VERTEX_PROGRAM_NV
#ifdef USE_BALL_FRESNEL
        if(options_ball_fresnel_refl && !fresnel_init
	   && extension_vp_NV){
            glGenProgramsNV(1, &fresnel_vert_prog_bind);
            glBindProgramNV(GL_VERTEX_PROGRAM_NV, fresnel_vert_prog_bind);
            glLoadProgramNV(GL_VERTEX_PROGRAM_NV, fresnel_vert_prog_bind, strlen(fresnel_vert_prog_str), fresnel_vert_prog_str);
            fresnel_init=1;
        }
#endif
#endif

    { /* whole ball culling */
        int i;
        GLfloat mat[16],x,y,w;
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glGetFloatv(GL_MODELVIEW_MATRIX,mat);
#define BORDER_SHIFT 2.0
        glTranslatef(0,0,-BORDER_SHIFT*balls.ball[0].d); /* half FOV mut be >= asin(1/BORDER_SHIFT) */
        glMultMatrixf(mat);
        glGetFloatv(GL_PROJECTION_MATRIX,mat);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        for(i=0;i<balls.nr;i++) if(balls.ball[i].in_game){
            x = mat[0]*balls.ball[i].r.x +
                mat[4]*balls.ball[i].r.y +
                mat[8]*balls.ball[i].r.z +
                mat[12];
            y = mat[1]*balls.ball[i].r.x +
                mat[5]*balls.ball[i].r.y +
                mat[9]*balls.ball[i].r.z +
                mat[13];
            w = mat[3]*balls.ball[i].r.x +
                mat[7]*balls.ball[i].r.y +
                mat[11]*balls.ball[i].r.z +
                mat[15];
            balls.ball[i].in_fov = ( x>-w && x<w && y>-w && y<w );
//            balls.ball[i].in_fov = 1;
        }
    }

    if(options_ball_tex && g_ballset!=ballset_for_gametype(balls.gametype)){
        create_texbinds(&balls);
    }


/*    glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB,&i);
    fprintf(stderr,"max_ARB=%d\n",i);*/

    if(options_ball_tex || options_ball_reflect){
        glEnable(GL_TEXTURE_2D);
    }

    if( options_ball_reflect && !options_ball_reflections_blended
	&& extension_multitexture ){
      #ifdef MULTITEX_ENABLED
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glBindTexture(GL_TEXTURE_2D,spheretexbind);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, col_refl);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glEnable(GL_TEXTURE_2D);
        glActiveTextureARB(GL_TEXTURE0_ARB);
      #endif
    }
        //    glClientActiveTextureARB(GL_TEXTURE0_ARB);

    /* draw balls */
    if(  options_ball_reflect && !options_ball_reflections_blended ){
        glMaterialfv(GL_FRONT, GL_DIFFUSE,   col_diff2);
        glMaterialfv(GL_FRONT, GL_AMBIENT,   col_amb2 );
        glMaterialfv(GL_FRONT, GL_SPECULAR,  col_spec2);
        glMaterialf (GL_FRONT, GL_SHININESS, 20.0     );
    }else if( !options_ball_reflect ){
        glMaterialfv(GL_FRONT, GL_DIFFUSE,   col_diff2);
        glMaterialfv(GL_FRONT, GL_AMBIENT,   col_amb2 );
        glMaterialfv(GL_FRONT, GL_SPECULAR,  col_spec2);
        glMaterialf (GL_FRONT, GL_SHININESS, 10.0     );
    }else if( options_ball_reflect && options_cuberef ){
        glMaterialfv(GL_FRONT, GL_DIFFUSE,   col_diff3);
        glMaterialfv(GL_FRONT, GL_AMBIENT,   col_amb3 );
        glMaterialfv(GL_FRONT, GL_SPECULAR,  col_spec );
        glMaterialf (GL_FRONT, GL_SHININESS, 0.0      );
    }else{
        glMaterialfv(GL_FRONT, GL_DIFFUSE,   col_diff);
        glMaterialfv(GL_FRONT, GL_AMBIENT,   col_amb );
        glMaterialfv(GL_FRONT, GL_SPECULAR,  col_spec);
        glMaterialf (GL_FRONT, GL_SHININESS, 0.0     );
    }
    if( (!options_ball_tex) && (!options_ball_reflect) ){
        glDisable(GL_TEXTURE_2D);
    }
    /* draw balls */
/*    if (options_ball_stencil_reflections){
        glClearStencil(0x0);
    }*/
    if( options_ball_tex || (options_ball_reflect && !options_ball_reflections_blended) ){
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        for(i=0;i<balls.nr;i++) if(balls.ball[i].in_game && balls.ball[i].in_fov){
            glBindTexture(GL_TEXTURE_2D,balltexbind[balls.ball[i].nr]);
/*            if (options_ball_stencil_reflections){
                glClear(GL_STENCIL_BUFFER_BIT);
                glDisable(GL_STENCIL_TEST);
                glEnable(GL_DEPTH_TEST);
            }*/
/*            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glColor3f(0.0,0.0,0.0);*/
#ifdef TEST_FRESNEL
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glColor3f(0,0,0);
#endif
            draw_ball(&balls.ball[i],cam_pos,cam_FOV,win_width,0);
/*            if (options_ball_stencil_reflections){
                glEnable(GL_STENCIL_TEST);
                glDisable(GL_DEPTH_TEST);
                glStencilFunc(GL_EQUAL,1,1);
            }*/
        }
    }
    /* only plain balls (no tex no reflexion) */
    if( !(options_ball_tex || options_ball_reflect) ){
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        for(i=0;i<balls.nr;i++) if(balls.ball[i].in_game && balls.ball[i].in_fov){
            draw_ball(&balls.ball[i],cam_pos,cam_FOV,win_width,0);
        }
    }

    if( options_ball_reflect && !options_ball_reflections_blended
	&& extension_multitexture ){
      #ifdef MULTITEX_ENABLED
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glDisable(GL_TEXTURE_2D);
        glActiveTextureARB(GL_TEXTURE0_ARB);
      #endif
    }

    /* draw extra blended ball-reflections */
    if( options_ball_reflect && options_ball_reflections_blended && !(options_cuberef && cuberef_binds==0) ){
        float texmat[16];
        glDepthMask (GL_FALSE);
        glEnable(GL_BLEND);
        glPolygonOffset( 0.0, -2.0 );
        glEnable( GL_POLYGON_OFFSET_FILL );
//        glBlendFunc (GL_SRC_ALPHA, GL_ONE);
//        glBlendFunc (GL_ONE_MINUS_DST_COLOR, GL_ONE);
//        glBlendFunc (GL_ONE, GL_ONE);
        if(options_cuberef){
#ifdef USE_BALL_FRESNEL
            if(options_ball_fresnel_refl) {
                glBlendFunc (GL_ONE, GL_ONE);
//                glBlendFunc (GL_ONE_MINUS_DST_COLOR, GL_ONE);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, col_refl4);
/*                glDisable(GL_LIGHTING);
                glColor3f(1.0,1.0,1.0);*/
            } else
#endif
            {
                glBlendFunc (GL_ONE_MINUS_DST_COLOR, GL_ONE);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, col_refl3);
            }
        } else {
            glBlendFunc (GL_SRC_ALPHA, GL_ONE);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, col_refl);
        }
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        if( options_cuberef && cuberef_binds!=0 ) {
            glDisable(GL_TEXTURE_2D);
#ifndef TEST_FRESNEL
            glEnable(GL_TEXTURE_CUBE_MAP_ARB);
#endif
        } else {
            glBindTexture(GL_TEXTURE_2D,spheretexbind);
        }

        if( options_calc_ball_reflections ){
        }else if( options_cuberef && cuberef_binds!=0 ){
           float dummy;
           glEnable(GL_TEXTURE_GEN_S);
           glEnable(GL_TEXTURE_GEN_T);
           glEnable(GL_TEXTURE_GEN_R);
           glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
           glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
           glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
           glGetFloatv(GL_MODELVIEW_MATRIX, texmat);
           /* maybe set this per ball to camaera direction */
           texmat[12]=0.0;
           texmat[13]=0.0;
           texmat[14]=0.0;
/*           printf("texmat offs=%f %f %f\n",
                  texmat[3],
                  texmat[7],
                  texmat[11]);*/
           /* transpose */
           dummy=texmat[1]; texmat[1]=texmat[4]; texmat[4]=dummy;
           dummy=texmat[2]; texmat[2]=texmat[8]; texmat[8]=dummy;
           dummy=texmat[6]; texmat[6]=texmat[9]; texmat[9]=dummy;
           glMatrixMode(GL_TEXTURE);
           glLoadMatrixf(texmat);
           glMatrixMode(GL_MODELVIEW);
#ifdef GL_VERTEX_PROGRAM_NV
#ifdef USE_BALL_FRESNEL
           if( options_ball_fresnel_refl && extension_vp_NV ){
               glDisable(GL_TEXTURE_GEN_S);
               glDisable(GL_TEXTURE_GEN_T);
               glDisable(GL_TEXTURE_GEN_R);
               glEnable(GL_VERTEX_PROGRAM_NV);
               /* for world coord program */
/*               glTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  0, GL_PROJECTION,              GL_IDENTITY_NV );
               glTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  4, GL_TEXTURE,                 GL_IDENTITY_NV  );
               glTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 12, GL_MODELVIEW,               GL_IDENTITY_NV  );*/
               /* for object coord program */
               glTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV  );
               glTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  4, GL_TEXTURE,                 GL_IDENTITY_NV  );
               glTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 12, GL_MODELVIEW,               GL_INVERSE_TRANSPOSE_NV  );
//               glTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 9, GL_MODELVIEW, GL_IDENTITY_NV);
               glProgramParameter4fNV( GL_VERTEX_PROGRAM_NV, 8, /* c[8] */
                                      0.0,     /* needed by vertex prog as constant 0.0 */
                                      1.0,     /* dummy - not used */
                                      0.00001,   /* z-shift for correct zbuffering when multisampling */
                                      0.6       /* Rmax(=1)-Rmin */
                                     );
//            # c[8].z  = offset for correct z-buffering
               glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
               glBindProgramNV(GL_VERTEX_PROGRAM_NV, fresnel_vert_prog_bind);
           }
#endif
#endif
        }else{
           glEnable(GL_TEXTURE_GEN_S);
           glEnable(GL_TEXTURE_GEN_T);
           glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
           glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        }
        for(i=0;i<balls.nr;i++) if(balls.ball[i].in_game && balls.ball[i].in_fov){
            if( options_calc_ball_reflections ){
                draw_ball(&balls.ball[i],cam_pos,cam_FOV,win_width,1);
            } else if( options_cuberef && cuberef_binds!=0 ){
                glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, cuberef_binds[i]);
#ifdef GL_VERTEX_PROGRAM_NV
                if ( extension_vp_NV ) {
                    myvec cam_pos2;
//                    cam_pos2=vec_scale(vec_unit(vec_diff(cam_pos,balls.ball[i].r)),BALL_D/2.5);
                    cam_pos2=balls.ball[i].r;
                    glProgramParameter4fNV( GL_VERTEX_PROGRAM_NV, 10, /* c[10] */
                                           BALL_D/2.0,BALL_D/2.5,0,0 );
                }
#endif
                draw_ball(&balls.ball[i],cam_pos,cam_FOV,win_width,0);
            }else{
                draw_ball(&balls.ball[i],cam_pos,cam_FOV,win_width,0);
            }
        }
        if( options_calc_ball_reflections ){
        }else if( options_cuberef && cuberef_binds!=0 ){
#ifdef GL_VERTEX_PROGRAM_NV
#ifdef USE_BALL_FRESNEL
            if( options_ball_fresnel_refl && extension_vp_NV ){
                glDisable(GL_VERTEX_PROGRAM_NV);
            }
#endif
#endif
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_GEN_R);
            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_CUBE_MAP_ARB);
            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
            glEnable(GL_TEXTURE_2D);
        }else{
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glDisable(GL_BLEND);
        }
        glDisable( GL_POLYGON_OFFSET_FILL );
        glDepthMask (GL_TRUE);
    }


    /* draw shadows */
    if(options_simple_ball_shadows)   // one threefold shadow under balls
    if( options_ball_shadows ){
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, col_shad);
        glDepthMask (GL_FALSE);
        glEnable(GL_BLEND);
//        glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc (GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        // glBlendFunc ( GL_ONE, GL_SRC_ALPHA );
        // glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glBindTexture(GL_TEXTURE_2D,shadowtexbind);
#define SH_SZ 0.047
        for(i=0;i<balls.nr;i++) if(balls.ball[i].in_game){
            glBegin(GL_POLYGON);
            glNormal3f( 0.0,0.0,1.0 );
            glTexCoord2f(0.0,1.0);
            glVertex3f( balls.ball[i].r.x-SH_SZ, balls.ball[i].r.y+SH_SZ, balls.ball[i].r.z-balls.ball[i].d/2.02 );
            glTexCoord2f(1.0,1.0);
            glVertex3f( balls.ball[i].r.x+SH_SZ, balls.ball[i].r.y+SH_SZ, balls.ball[i].r.z-balls.ball[i].d/2.02 );
            glTexCoord2f(1.0,0.0);
            glVertex3f( balls.ball[i].r.x+SH_SZ, balls.ball[i].r.y-SH_SZ, balls.ball[i].r.z-balls.ball[i].d/2.02 );
            glTexCoord2f(0.0,0.0);
            glVertex3f( balls.ball[i].r.x-SH_SZ, balls.ball[i].r.y-SH_SZ, balls.ball[i].r.z-balls.ball[i].d/2.02 );
            glEnd();
        }
        glDisable(GL_BLEND);
        glDepthMask (GL_TRUE);
#undef SH_SZ
    }

   /* draw shadows */
    if(!options_simple_ball_shadows)   // one threefold shadow under balls
    if( options_ball_shadows ){
        glMaterialfv(GL_FRONT, GL_AMBIENT,   col_amb );
        glMaterialfv(GL_FRONT, GL_SPECULAR,  col_spec);
        glMaterialf (GL_FRONT, GL_SHININESS, 0.0     );
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glDepthMask (GL_FALSE);
        glEnable(GL_BLEND);
//        glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc (GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
//         glBlendFunc ( GL_ONE, GL_SRC_ALPHA );
//         glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glBindTexture(GL_TEXTURE_2D,shadowtexbind);
#define SH_SZ 0.040
#define SH_FACT 0.32
        for(i=0;i<balls.nr;i++) if(balls.ball[i].in_game) for(j=0;j<lightnr;j++){
            v=vec_diff(balls.ball[i].r,vec_xyz(lightpos[j].x,lightpos[j].y,0.0));
//            v=vec_diff(balls.ball[i].r,vec_xyz(0,0,0));
            fact=1.0+vec_abs(v)*SH_FACT;
            col_shad[0]=0.5-0.3*vec_abs(v);   if(col_shad[0]<0.0) col_shad[0]=0.0;
            col_shad[1]=col_shad[0];
            col_shad[2]=col_shad[0];
            col_shad[3]=1.0;
            glMaterialfv(GL_FRONT, GL_DIFFUSE, col_shad);
            glColor3f(0.0,0.0,0.0);   /* against shadow flicker bug */
//            fact=1.3;
            if( v.x==0.0 && v.y==0.0 && v.z==0.0 ){
                v=vec_xyz(1,0,0);
            } else {
                v=vec_unit(v);
            }
            vn=vec_xyz(-v.y,v.x,v.z);
            stretch_matrix[ 0]=vn.x*vn.x+fact*v.x*v.x;
            stretch_matrix[ 5]=vn.y*vn.y+fact*v.y*v.y;
            stretch_matrix[10]=1.0;
            stretch_matrix[15]=57.15E-3/BALL_D;
            stretch_matrix[ 3]=0.0;
            stretch_matrix[ 7]=0.0;
            stretch_matrix[11]=0.0;
            stretch_matrix[12]=0.0;
            stretch_matrix[13]=0.0;
            stretch_matrix[14]=0.0;
            stretch_matrix[ 4]=vn.x*vn.y+fact*v.x*v.y;
            stretch_matrix[ 8]=vn.x*vn.z+fact*v.x*v.z;
            stretch_matrix[ 9]=vn.y*vn.z+fact*v.y*v.z;
            stretch_matrix[ 1]=vn.x*vn.y+fact*v.x*v.y;
            stretch_matrix[ 2]=vn.x*vn.z+fact*v.x*v.z;
            stretch_matrix[ 6]=vn.y*vn.z+fact*v.y*v.z;
            glPushMatrix();
            glTranslatef( lightpos[j].x+(balls.ball[i].r.x-lightpos[j].x)*1.025,
                          lightpos[j].y+(balls.ball[i].r.y-lightpos[j].y)*1.025,
                          -balls.ball[i].d/2.02 );
            glMultMatrixf(stretch_matrix);
            glBegin(GL_QUADS);
            glNormal3f( 0.0,0.0,1.0 );
            glTexCoord2f(0.0,1.0);
            glVertex3f( -SH_SZ, +SH_SZ, 0.0 );
            glTexCoord2f(1.0,1.0);
            glVertex3f( +SH_SZ, +SH_SZ, 0.0 );
            glTexCoord2f(1.0,0.0);
            glVertex3f( +SH_SZ, -SH_SZ, 0.0 );
            glTexCoord2f(0.0,0.0);
            glVertex3f( -SH_SZ, -SH_SZ, 0.0 );
            glEnd();
            glPopMatrix();
        }
        glDisable(GL_BLEND);
        glDepthMask (GL_TRUE);
#undef SH_SZ
    }

}


void draw_ballpath( BallType * pball )
{
    _NOT_IMPLEMENTED
}
