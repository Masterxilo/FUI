/* queue.h
**
**    includefile: create the billard-queue display lists
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

#ifndef BILLARD_QUEUE_H
#define BILLARD_QUEUE_H

void delete_queue_texbind( void );
void create_queue_texbind( void );
int create_queue();
void draw_queue( VMvect pos, GLfloat Xrot, GLfloat Zrot, GLfloat zoffs,
                 GLfloat xoffs, GLfloat yoffs, int spheretexbind, VMvect * lightpos, int lightnr );

#endif  /* BILLARD_QUEUE_H */
