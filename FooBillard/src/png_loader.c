/* pngloader.c
**
**    load png-textures
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
#define COMPILE_PNG_CODE 1

#if COMPILE_PNG_CODE
#include <png.h>
#else
#endif
#include <math.h>
#include <stdlib.h>
#include "options.h"

int load_png(char * file_name, int * w, int * h, int * depth, char ** data)
{
#if COMPILE_PNG_CODE
  FILE *fp;
  png_structp png_ptr;
  png_infop info_ptr;
  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type,
    compression_type, filter_type;
  int rowbytes, channels;
  int i;
  char * buff;
  char ** row_pointers;

  fp = fopen(file_name, "rb");
  if (!fp){
    return 0;
  }

  /*    fread(header, 1, number, fp);
      is_png = !png_sig_cmp(header, 0, number);
      if (!is_png)
      {
      return 0;
      }*/

  png_ptr = png_create_read_struct
    (PNG_LIBPNG_VER_STRING, (png_voidp)NULL/*user_error_ptr*/,
    NULL/*user_error_fn*/, NULL/*user_warning_fn*/);
  if (!png_ptr)
    return 0;

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    png_destroy_read_struct(&png_ptr,
      (png_infopp)NULL, (png_infopp)NULL);
    return 0;
  }

  //    png_set_invert_alpha(png_ptr);
  png_init_io(png_ptr, fp);

  png_read_info(png_ptr, info_ptr);

  png_get_IHDR(png_ptr, info_ptr, &width, &height,
    &bit_depth, &color_type, &interlace_type,
    &compression_type, &filter_type);

  channels = png_get_channels(png_ptr, info_ptr);

  rowbytes = png_get_rowbytes(png_ptr, info_ptr);

  DPRINTF("width=%ld\n", width);
  DPRINTF("height=%ld\n", height);
  DPRINTF("bit_depth=%d\n", bit_depth);
  DPRINTF("color_type=%d\n", color_type);
  DPRINTF("interlace_type=%d\n", interlace_type);
  DPRINTF("compression_type=%d\n", compression_type);
  DPRINTF("filter_type=%d\n", filter_type);
  DPRINTF("channels=%d\n", channels);
  DPRINTF("rowbytes=%d\n", rowbytes);


  buff = (char *)malloc(height*rowbytes);
  row_pointers = (char **)malloc(height*sizeof(char *));
  for (i = 0; i < height; i++){
    row_pointers[i] = &buff[rowbytes*i];
  }

  png_read_image(png_ptr, (png_bytepp)row_pointers);

  DPRINTF("load_png: rgstereo=%d\n", options_rgstereo_on);

  if (options_rgstereo_on){
    DPRINTF("load_png: graying out texture\n");
    for (i = 0; i < height; i++){
      int j, k, d;
      for (j = 0; j < rowbytes; j += channels){
        d = 0;
        for (k = 0; k < channels; k++) d += (unsigned char)row_pointers[i][j + k];
        d /= channels;
        for (k = 0; k < channels; k++) row_pointers[i][j + k] = d;
      }
    }
  }

  /*    fprintf(stderr,"!!!!!!!!!!!!! alpha correction\n");
      for( i=0;i<height*rowbytes;i+=3 ){
      buff[i]=(buff[i]+buff[i+1]+buff[i+2])/3;
      }*/

  /*    if( channels==4 ){
          fprintf(stderr,"!!!!!!!!!!!!! alpha correction\n");
          for( i=3;i<height*rowbytes;i+=4 )
          buff[i]=0xFF-buff[i];
          }*/

  free(row_pointers);
  free(info_ptr);
  free(png_ptr);
  fclose(fp);


  *data = buff;
  *w = width;
  *h = height;
  *depth = channels*bit_depth;

  return 1;
#else
  *data  = 0;
  *w     = 0;
  *h     = 0;
  *depth = 0;
  return 1;
#endif
}
