/* bumpref.h
**
**    bummp-reflection-mapping using
**    NVIDIA vertex-shaders register-combiners and texture-programs
**    Copyright (C) 2002  Florian Berger
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

#ifndef BUMPREF_H
#define BUMPREF_H

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        int init;
        int bumpref_list;
        int cubemap_bind;
        int normalmap_bind;
        unsigned int vert_prog_bind;
        int texgen;
        float bump_light[3];
        float bump_diff_col[4];
        float bump_spec_col[4];
    } BumpRefType;


    /* bump reflections */

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
                                      );

    void bumpref_use( BumpRefType * bumpref );
    void bumpref_restore();


    /* bump only */

    BumpRefType bump_setup_vp_rc( char * map_name, double strength, int texgen );

    void bump_set_light( BumpRefType * bumpref, float x, float y, float z );
    void bump_set_diff( BumpRefType * bumpref, float r, float g, float b );
    void bump_set_spec( BumpRefType * bumpref, float r, float g, float b );

    void bump_use( BumpRefType * bump );
    void bump_restore();


/*int bumpref_create_cubemap( char * posx_name,
                           char * posy_name,
                           char * posz_name,
                           char * negx_name,
                           char * negy_name,
                           char * negz_name );
void bump2normal(int w, int h, char * data, double strength);
int bumpref_create_bumpmap( char * map_name, double strength );*/

#ifdef __cplusplus
}
#endif

#endif  /* BUMPREF_H */
