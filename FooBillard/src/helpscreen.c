/* helpscreen.c
**
**    helpscreen using textobj
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
#include "font.h"
#include "textobj.h"
#include "options.h"
#include "stdlib.h"
#include "config.h"

#define HEAD_FONT_SIZE 32
#define HELP_FONT_SIZE 23
#define HELP_LINE_SPACING 23.0

static int       help_screen_obj = -1;
static textObj ** text;
static textObj ** text0;

static char *help_source[] = {
  "FooBillard v"VERSION" - Help",
  " <button1> ... angular move",
  " <button2> ... radial move",
  " <shift>+<b1> or <b2>after<b1> ... place cue ball",
  " <shift>+<b2> or <b1>after<b2> ... set cue offset",
  " <b2>after<b1> (no move) ... toggle cue/external view",
  " <ctrl>+<button2> ... set FOV (zoom)",
  " <ctrl>+<button1> ... direct mouse-cue-shot",
  " <space> or <enter> or <button3> ... shoot",
  " <up>,<down> or <pg-up><pg-down> ... strength adj.",
  " <left> or <right> ... rotate",
  " <c>,<F3> ... toggle cue/external view",
  " <f>,<F4> ... toggle free move mode in ext view",
  " <0> ... suggest AI-shot",
  " <a> ... toggle actual player AI/Human",
  " <n> ... start a new game",
  //    " <b> ... a little fps benchmark",
  " <v> ... toggle the vertical line",
  " <r> ... toggle simpler reflections",
  //    " <s> ... toggle red/green stereo",
  " <b> ... cue butt up/down",
  " <s> ... mouse shot",
  " <e> ... english adj.",
  " <m> ... place cue ball",
  " <u> ... undo last move (training mode)",
  " <ESC> ... popup menu",
  " <F1> ... toggle this help screen",
  " <F2> ... birdview (same as <F3>+<F4>+centered pos)",
  " <TAB> ... change cueball (training mode)",
  0
};

void create_help_screen(int win_width, int win_height)
{
  char **help_text = &help_source[0];
  text = malloc(100 * sizeof(textObj *));
  text0 = text;
  help_screen_obj = glGenLists(1);

  *text = textObj_new(*help_text, options_help_fontname, HEAD_FONT_SIZE);
  text++;
  help_text++;
  while (*help_text) {
    *text = textObj_new(*help_text, options_help_fontname, HELP_FONT_SIZE);
    text++;
    help_text++;
  }
  *text = (textObj *)0;

  text = text0;
  glNewList(help_screen_obj, GL_COMPILE);
  glTranslatef(-0.95, 0.87, -1.0);
  glScalef(2.0 / win_width*win_height / 768.0, 2.0 / 768.0, 1.0);
  //glScalef(2.0/win_width,2.0/win_height,1.0);
  for (text = text0; *text != 0; text++){
    textObj_draw(*text);
    glTranslatef(0.0, -HELP_LINE_SPACING, -0.0);
  }
  glEndList();
}


void delete_help_screen()
{
  glDeleteLists(help_screen_obj, 1);
  for (text = text0; *text != (textObj *)0; text++){
    textObj_delete(*text);
    free(*text);
  }
  help_screen_obj = -1;
}


int draw_help_screen(int win_width, int win_height)
{

  if (help_screen_obj == -1){
    create_help_screen(win_width, win_height);
  }
  else {
    glCallList(help_screen_obj);
  }

  return 0;
}
