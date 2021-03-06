/* vmath.h
**
**    includefile: some vector mathematics and structures (float or double)
**    Copyright (C) 2001  Florian Berger
**    Email:  harpin_floh@yahoo.de,  florian.berger@jk.uni-linz.ac.at
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

#ifndef VMATH_H
#define VMATH_H
#include "myinclude.h"
#define VMATH_SINGLE_PRECISION   /* this should be in the using code */
//#undef VMATH_SINGLE_PRECISION   /* this should be in the using code */

#ifdef VMATH_SINGLE_PRECISION
typedef float VMfloat;
#else
typedef double VMfloat;
#endif

struct Vect{
          VMfloat x,y,z;
       };
typedef struct Vect VMvect;


struct Matrix4{
          VMfloat m[16];
       };
typedef struct Matrix4 VMmatrix4;


struct Shape{
          int pnr;
          int closed;
          struct Vect *p;
       };
typedef struct Shape VMshape;


struct Outline{
          int shnr;
          struct Shape * sh;
       };
typedef struct Outline VMoutline;


struct Triangle{
          struct Vect v1,v2,v3;
       };
typedef struct Triangle VMtriangle;


struct Mesh{
          int tnr;
          struct Triangle *t;
          struct Triangle *n;
       };
typedef struct Mesh VMmesh;


struct Point{
    struct Vect pos;
    struct Point * next;
};
typedef struct Point VMpoint;


struct PolyPoint{
    int pnr;
    struct Point pointlist;
    struct PolyPoint * next;
};
typedef struct PolyPoint VMpolypoint;



void         shape_copy  ( struct Shape shs, struct Shape *shd );
void         shape_flip  ( struct Shape *sh );
void         shape_trans ( struct Shape *s, struct Vect t );

struct Vect  vec_cross ( struct Vect v1, struct Vect v2 );
VMfloat      vec_mul   ( struct Vect v1, struct Vect v2 );
struct Vect  vec_diff  ( struct Vect v1, struct Vect v2 );
struct Vect  vec_add   ( struct Vect v1, struct Vect v2 );

struct Vect  vec_scale( struct Vect v1, VMfloat scale );
struct Vect  vec_rotate( struct Vect v1, struct Vect ang );
VMfloat      vec_abs   ( struct Vect v );
VMfloat      vec_abssq ( struct Vect v );
struct Vect  vec_unit  ( struct Vect v );

int          vec_equal        ( struct Vect v1, struct Vect v2 );
int          vec_nearly_equal ( struct Vect v1, struct Vect v2, VMfloat tolerance );

struct Vect  vec_xyz( VMfloat x, VMfloat y, VMfloat z );
struct Vect  vec_ex();
struct Vect  vec_ey();
struct Vect  vec_ez();
struct Vect  vec_null();
void         rot_ax( struct Vect ax, struct Vect *v, int nr, VMfloat phi );
void         shape_rot_ax( struct Vect ax, struct Shape *s, struct Vect m, VMfloat phi,
                           struct Vect nax, VMfloat sc_x, VMfloat sc_y, VMfloat sc_z );

struct Vect  meshpt_nr( struct Point * plist, int index );

int remove_idle_points( struct Shape * sh, struct Shape * shd );

VMfloat      vec_angle ( struct Vect v1, struct Vect v2 );

struct Vect  vec_proj  ( struct Vect v1, struct Vect v2 );
struct Vect  vec_ncomp ( struct Vect v1, struct Vect v2 );
VMfloat      vec_ndist ( struct Vect v, struct Vect v1, struct Vect v2 );
struct Vect  vec_surf_proj( struct Vect center, struct Vect point, struct Vect n, struct Vect npos );

struct Vect    matr4_rdot( struct Matrix4 m,  struct Vect v );
struct Matrix4 matr4_mul ( struct Matrix4 m1, struct Matrix4 m2);

VMfloat      shape_area( struct Shape sh );

struct Vect  tri_center ( struct Vect v1, struct Vect v2, struct Vect v3 );
VMfloat      tri_area_xy( struct Vect v1, struct Vect v2, struct Vect v3 );
VMfloat      tri_vol_xy ( struct Vect v1, struct Vect v2, struct Vect v3 );

#endif  //VMATH_H
