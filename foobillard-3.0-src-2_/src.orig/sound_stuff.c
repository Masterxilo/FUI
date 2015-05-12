/* sound_stuff.c
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
#ifdef USE_SOUND

#include "sound_stuff.h"
#include "options.h"

#ifdef USE_SDL
#include "SDL.h"
#include "SDL_audio.h"
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef USE_SDL
#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <fcntl.h>
#endif


#define NUM_SOUNDS 20

#ifdef USE_SDL
static struct sample {
    Uint8 *data;
    Uint32 dpos;
    Uint32 dlen;
    int    vol;
} sounds[NUM_SOUNDS];
#else
static struct sample {
    int16_t *data;
    int dpos;
    int dlen;
    int vol;
} sounds[NUM_SOUNDS];
#endif

#ifndef USE_SDL
#define SNDBUFF_SIZE 512
#define MAXAMP_128  0x3FFF80
#define MINAMP_128 -0x3FFF80
int h_audio;
long int  mixbuff[SNDBUFF_SIZE];
int16_t   sndbuff[SNDBUFF_SIZE];
#endif


#ifdef USE_SDL
static void mixaudio(void *userdata, Uint8 *stream, int len)
{
    int i;
    Uint32 amount;

    for ( i=0; i<NUM_SOUNDS; ++i ) {
        amount = (sounds[i].dlen-sounds[i].dpos);
        if ( amount > len ) {
            amount = len;
        }
/*        SDL_MixAudio(stream, &sounds[i].data[sounds[i].dpos], amount, SDL_MIX_MAXVOLUME);*/
        SDL_MixAudio(stream, &sounds[i].data[sounds[i].dpos], amount, sounds[i].vol);
        sounds[i].dpos += amount;
    }
}
#else
void mixaudio(void)
{
    int i,j;
    int amount;
    int sound_occured;
    audio_buf_info info;

    ioctl(h_audio, SNDCTL_DSP_GETOSPACE, &info);
    sound_occured=1; /* for inital loop entry */

    while ( info.bytes > SNDBUFF_SIZE*2  &&  sound_occured ){
        sound_occured=0;

        for ( i=0; i<NUM_SOUNDS; ++i ) {
            if(sounds[i].dlen!=sounds[i].dpos){

                if(!sound_occured){
                    sound_occured=1;
                    for(j=0;j<SNDBUFF_SIZE;j++) mixbuff[j]=0;
                }

                amount = (sounds[i].dlen-sounds[i].dpos);
                if ( amount > SNDBUFF_SIZE ) {
                    amount = SNDBUFF_SIZE;
                }
                for(j=0;j<amount;j++){
                    mixbuff[j]+=sounds[i].data[sounds[i].dpos+j]*sounds[i].vol;
                }
                sounds[i].dpos += amount;
            }
        }

        if(sound_occured){
            for(j=0;j<SNDBUFF_SIZE;j++){
                if        (mixbuff[j]>MAXAMP_128) {
                    sndbuff[j] = MAXAMP_128/128;
                } else if (mixbuff[j]<MINAMP_128) {
                    sndbuff[j] = MINAMP_128/128;
                } else {
                    sndbuff[j] = mixbuff[j]/128;
                }
            }
            write(h_audio,sndbuff,SNDBUFF_SIZE*2);
        }

        ioctl(h_audio, SNDCTL_DSP_GETOSPACE, &info);
    }
}
#endif

void PlayData(short int *data, int len, double ampl)
{
    int index;
//    SDL_AudioSpec wave;
//    Uint32 dlen;
//    SDL_AudioCVT cvt;

    if(options_use_sound){
        /* Look for an empty (or finished) sound slot */
        for ( index=0; index<NUM_SOUNDS; ++index ) {
            if ( sounds[index].dpos == sounds[index].dlen ) {
                break;
            }
        }
        if ( index == NUM_SOUNDS )
            return;

        /* Put the sound data in the slot (it starts playing immediately) */
        if ( sounds[index].data ) {
            //        free(sounds[index].data);
        }
#ifdef USE_SDL
        SDL_LockAudio();
#endif
#ifdef USE_SDL
        sounds[index].data = (Uint8 *)data;
        sounds[index].dlen = len;
#else
        sounds[index].data = (int16_t *)data;
        sounds[index].dlen = len/2;
#endif
        sounds[index].dpos = 0;
        sounds[index].vol  = (int)(128.0*ampl);
#ifdef USE_SDL
        SDL_UnlockAudio();
#endif
    }
}


void PlaySound(TSound * snd, double ampl)
{
    PlayData(snd->data, snd->len, ampl);
}


void PlaySound_offs(TSound * snd, double ampl, int offs_samps)
{
    PlayData(&snd->data[offs_samps*2], snd->len-offs_samps*2*2, ampl);
}


int init_sound(void)
{
#ifdef USE_SDL
    SDL_AudioSpec fmt;

    /* Set 16-bit stereo audio at 22Khz */
    fmt.freq = 22050;
    fmt.format = AUDIO_S16SYS;
    fmt.channels = 2;
    fmt.samples = 512;        /* A good value for games */
    fmt.callback = mixaudio;
    fmt.userdata = NULL;

    /* Open the audio device and start playing sound! */
    if ( SDL_OpenAudio(&fmt, NULL) < 0 ) {
        fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
        options_use_sound=0;
    } else {
        options_use_sound=1;
    }

    SDL_PauseAudio(0);
    return 1;
#else
    audio_buf_info info;
    int dummy;

    h_audio = open("/dev/dsp", O_WRONLY, 0);
    dummy = (6<<16) + 10;
    ioctl(h_audio, SNDCTL_DSP_SETFRAGMENT, &dummy);
    dummy = AFMT_S16_LE;
    ioctl(h_audio, SNDCTL_DSP_SAMPLESIZE, &dummy);
    dummy = 1;
    ioctl(h_audio, SNDCTL_DSP_STEREO, &dummy);
    dummy = 22050;
    ioctl(h_audio, SNDCTL_DSP_SPEED, &dummy);

    ioctl(h_audio, SNDCTL_DSP_GETOSPACE, &info);
    DPRINTF("Info for output\n");
    DPRINTF("   fragments  = %d\n",info.fragments );
    DPRINTF("   fragstotal = %d\n",info.fragstotal);
    DPRINTF("   fragsize   = %d\n",info.fragsize  );
    DPRINTF("   bytes      = %d\n",info.bytes     );

    return 1;
#endif
}


int exit_sound(void)
{
#ifdef USE_SDL
    SDL_CloseAudio();
    return 1;
#else
    close(h_audio);
    return 1;
#endif
}



double errsin(double x, double err)
{
    double y;
    y=(x-M_PI/4.0)/M_PI;
    y-=floor(y);
/*    if( y<0.5 ){
        return (1.0-4.0*y);
    }
    return (-1.0+4.0*(y-0.5));*/
    return(err*(double)rand()/(double)RAND_MAX+(1.0-err)*sin(x));
}

void create_expsinerr( double period_samps, double tau_samps, double err, short int ** data, int * len )
{
    int i;
    *len=2*2*3*tau_samps;
    *data=(short int *)malloc(*len);
    for(i=0;i<(*len)/2/2;i++){
        (*data)[i*2]=32000.0*errsin((double)i/period_samps*2.0*M_PI,err)*exp(-(double)i/tau_samps);
        (*data)[i*2+1]=(*data)[i*2];
    }
    for(i=1;i<(*len)/2/2;i++){
        (*data)[i*2]=0.5*(*data)[i*2]+0.5*(*data)[(i-1)*2];
        (*data)[i*2+1]=(*data)[i*2];
    }
    for(i=30;i<(*len)/2/2;i++){
        (*data)[i*2]=0.7*(*data)[i*2]+0.3*(*data)[(i-30)*2];
        (*data)[i*2+1]=(*data)[i*2];
    }
}

void create_delayed_expsinerr( double period_samps, double tau_samps, int delay_samps, double err, short int ** data, int * len )
{
    int i;
    *len=2*2*3*tau_samps+2*2*delay_samps;
    *data=(short int *)malloc(*len);
    for(i=0;i<delay_samps;i++){
        (*data)[i*2]=0;
        (*data)[i*2+1]=0;
    }
    for(i=delay_samps+0;i<(*len)/2/2;i++){
        (*data)[i*2]=32000.0*errsin((double)(i-delay_samps)/period_samps*2.0*M_PI,err)*exp(-(double)(i-delay_samps)/tau_samps);
        (*data)[i*2+1]=(*data)[i*2];
    }
    for(i=delay_samps+1;i<(*len)/2/2;i++){
        (*data)[i*2]=0.5*(*data)[i*2]+0.5*(*data)[(i-1)*2];
        (*data)[i*2+1]=(*data)[i*2];
    }
    for(i=delay_samps+30;i<(*len)/2/2;i++){
        (*data)[i*2]=0.7*(*data)[i*2]+0.3*(*data)[(i-30)*2];
        (*data)[i*2+1]=(*data)[i*2];
    }
}

void apply_bandpass( double period_samps, double width_samps, TSound snd )
{
    int len2, i, j;
    double * filter, filterint;

#define FILTERENTRY(filter,len,index) (((index)<(len)&&(index)>=0)?(filter)[(index)]:0.0)
    len2 = 3*width_samps;
    filter = malloc( len2*sizeof(double) );

    /* construct filter */
    filterint=0.0;
    for(i=0;i<len2;i++){
        filter[i]=
            exp(-(double)(i-len2/2)*(double)(i-len2/2)/(double)width_samps/(double)width_samps) *
            cos(2.0*M_PI*(double)(i-len2/2)/(double)period_samps);
        filterint+=filter[i];
    }

    /* apply filter */
    for(i=0;i<snd.len/2/2;i++){
        double newdata1, newdata2;
        newdata1=0.0;
        newdata2=0.0;
        for(j=0;j<len2;j++){
            newdata1 += (double)TSoundENTRY(snd,(i+(j-len2/2))*2)   * FILTERENTRY(filter,len2,j);
            newdata2 += (double)TSoundENTRY(snd,(i+(j-len2/2))*2+1) * FILTERENTRY(filter,len2,j);
        }
/*        newdata1 = (double)TSoundENTRY(snd,i*2)   * filterint;
        newdata2 = (double)TSoundENTRY(snd,i*2+1) * filterint;*/
        snd.data[i*2]   = newdata1/filterint;
        snd.data[i*2+1] = newdata2/filterint;
    }

    free( filter );
}

void create_expsinerr_attack( double period_samps, double tau_samps, double err, double attack, short int ** data, int * len )
{
    int i;
    *len=2*2*3*tau_samps;
    *data=(short int *)malloc(*len);
    for(i=0;i<(*len)/2/2;i++){
        (*data)[i*2]=32000.0*errsin((double)i/period_samps*2.0*M_PI,err)*exp(-(double)i/tau_samps);
        (*data)[i*2+1]=(*data)[i*2];
    }
    for(i=1;i<(*len)/2/2;i++){
        (*data)[i*2]=0.5*(*data)[i*2]+0.5*(*data)[(i-1)*2];
        (*data)[i*2+1]=(*data)[i*2];
    }
    for(i=30;i<(*len)/2/2;i++){
        (*data)[i*2]=0.7*(*data)[i*2]+0.3*(*data)[(i-30)*2];
        (*data)[i*2+1]=(*data)[i*2];
    }

    for(i=0;i<(*len)/2/2;i++){
        (*data)[i*2]*=1.0-exp(-(double)i/(double)attack*(double)i/(double)attack);
        (*data)[i*2+1]=(*data)[i*2];
    }
}


void apply_attack( int offset, double attack, short int ** data, int * len )
{
    int i;

    for(i=offset;i<(*len)/2/2;i++){
        (*data)[i*2+0]*=1.0-exp(-(double)(i-offset)/(double)attack*(double)(i-offset)/(double)attack);
        (*data)[i*2+1]*=1.0-exp(-(double)(i-offset)/(double)attack*(double)(i-offset)/(double)attack);
    }
}


void create_expsin( double period_samps, double tau_samps, short int ** data, int * len )
{
    int i;
    *len=2*2*3*tau_samps;
    *data=(short int *)malloc(*len);
    for(i=0;i<(*len)/2/2;i++){
        (*data)[i*2]=32000.0*sin((double)i/period_samps*2.0*M_PI)*exp(-(double)i/tau_samps);
        (*data)[i*2+1]=(*data)[i*2];
    }
}


void create_expsin_otones( double period_samps, double tau_samps, double otonefact, short int ** data, int * len )
{
    int i,j;
    double fact;
    *len=2*2*3*tau_samps;
    *data=(short int *)malloc(*len);
    for(i=0;i<(*len)/2/2;i++){
        fact=1.0;
        (*data)[i*2]=0.0;
        for(j=0;j<6;j++){
            (*data)[i*2]+=fact*25000.0*sin((double)i/(period_samps/(double)j)*2.0*M_PI)*exp(-(double)i/tau_samps);
            fact*=otonefact;
        }
        (*data)[i*2+1]=(*data)[i*2];
    }
}


#endif  /* #ifdef USE_SOUND */
