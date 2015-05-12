/* helpscreen.c
**
**    helpscreen using textobj
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

#include "font.h"
#include "textobj.h"
#include "options.h"
#include "stdlib.h"
#include "config.h"

static int       help_screen_obj = -1;
static textObj ** text;
static textObj ** text0;


void create_help_screen(int win_width, int win_height)
{
        text = malloc(100*sizeof(textObj *));
        text0=text;
        help_screen_obj = glGenLists(1);

        *text = textObj_new( "FooBillard v"VERSION" - Help", options_help_fontname, 32 );
        text++;
        *text = textObj_new( " <button1> ... angular move", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <button2> ... radial move", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <shift>+<b1> or <b2>after<b1> ... place cue ball", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <shift>+<b2> or <b1>after<b2> ... set cue offset", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <b2>after<b1> (no move) ... toggle cue/external view", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <ctrl>+<button2> ... set FOV (zoom)", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <ctrl>+<button1> ... direct mouse-cue-shot", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <space> or <enter> or <button3> ... shoot", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <up>,<down> or <pg-up><pg-down> ... strength adj.", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <left> or <right> ... rotate", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <c>,<F3> ... toggle cue/external view", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <f>,<F4> ... toggle free move mode in ext view", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <0> ... suggest AI-shot", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <a> ... toggle actual player AI/Human", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <n> ... start a new game", options_help_fontname, 21);
        text++;
//        *text = textObj_new( " <b> ... a little fps benchmark", options_help_fontname, 21);
//        text++;
        *text = textObj_new( " <v> ... toggle the vertical line", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <r> ... toggle simpler reflections", options_help_fontname, 21);
        text++;
//        *text = textObj_new( " <s> ... toggle red/green stereo", options_help_fontname, 21);
//        text++;
        *text = textObj_new( " <b> ... cue butt up/down", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <s> ... mouse shot", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <e> ... english adj.", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <m> ... place cue ball", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <u> ... undo last move (training mode)", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <ESC> ... popup menu", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <F1> ... toggle this help screen", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <F2> ... birdview (same as <F3>+<F4>+centered pos)", options_help_fontname, 21);
        text++;
        *text = textObj_new( " <TAB> ... change cueball (training mode)", options_help_fontname, 21);
        text++;
        *text = (textObj *)0;

        text=text0;
        glNewList(help_screen_obj, GL_COMPILE);
           glTranslatef(-0.95,0.87,-1.0);
           glScalef(2.0/win_width,2.0/win_height,1.0);
           for(text=text0 ; *text!=0 ; text++ ){
               textObj_draw( *text );
               glTranslatef(0.0,-25.0,-0.0);
           }
        glEndList();
}


void delete_help_screen()
{
    glDeleteLists(help_screen_obj, 1);
    for(text=text0;*text!=(textObj *)0;text++){
        textObj_delete(*text);
        free(*text);
    }
    help_screen_obj=-1;
}


int draw_help_screen(int win_width, int win_height)
{

    if( help_screen_obj==-1 ){
        create_help_screen( win_width, win_height );
    } else {
        glCallList(help_screen_obj);
    }

    return 0;
}
