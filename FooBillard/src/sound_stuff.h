/* sound_stuff.h
**
**    code for sound sample data
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

#ifndef SOUND_STUFF_H
#define SOUND_STUFF_H
#include "myinclude.h"
#ifdef USE_SOUND

typedef struct SoundStruct{
    short int * data;
    int len;  /* length in bytes */
} TSound;

#define TSoundENTRY(snd,index) (( (index)<(snd).len/2 && (index)>=0 )?(snd).data[(index)]:0)

void PlaySound_fb(TSound * snd, double ampl);
void PlaySound_offs(TSound * snd, double ampl, int offs_samps);
void PlayData(short int *data, int len, double ampl);
int init_sound(void);
int exit_sound(void);

#ifndef USE_SDL
void mixaudio(void);
#endif

void apply_bandpass( double period_samps, double width_samps, TSound snd );
void apply_attack( int offset, double attack, short int ** data, int * len );
void create_expsinerr( double period_samps, double tau_samps, double err, short int ** data, int * len );
void create_delayed_expsinerr( double period_samps, double tau_samps, int delay_samps, double err, short int ** data, int * len );
void create_expsinerr_attack( double period_samps, double tau_samps, double err, double attack, short int ** data, int * len );
void create_expsin( double period_samps, double tau_samps, short int ** data, int * len );

#endif /* #ifdef USE_SOUND */

#endif /* SOUND_STUFF_H */
