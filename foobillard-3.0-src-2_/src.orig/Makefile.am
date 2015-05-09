bin_PROGRAMS = foobillard

foobillard_SOURCES = billard3d.c \
	       	     billmove.c \
	       	     billmove.h \
		     billard.c \
		     billard.h \
                     vmath.c \
                     vmath.h \
                     ball.c \
                     ball.h \
                     png_loader.c \
                     png_loader.h \
                     table.c \
                     table.h \
                     queue.c \
                     queue.h \
                     aiplayer.c \
                     aiplayer.h \
                     options.c \
                     options.h \
                     evaluate_move.c \
                     evaluate_move.h \
                     helpscreen.c \
                     helpscreen.h \
                     textobj.c \
                     textobj.h \
                     font.c \
                     font.h \
                     sys_stuff.c \
                     sys_stuff.h \
                     net_socket.c \
                     net_socket.h \
                     sound_stuff.c \
                     sound_stuff.h \
                     menu.c \
                     menu.h \
                     gamemenu.h \
                     player.h \
                     bumpref.c \
                     bumpref.h
                     



INCLUDES = -DNDEBUG -DDATA_DIRECTORY='"${pkgdatadir}/"'
#LIBDIR = /usr/X11R6/lib
#INCDIR = /usr/X11R6/include

#CC     = gcc
#CXXFLAGS = -O6 -s
#CFLAGS = -DXMESA -Wall -O3 -I$(INCDIR)
#CFLAGS = -Wall -O3 -I$(INCDIR) `freetype-config --cflags`
if USE_SDL
SDL_CFLAGS = `sdl-config --cflags` -DUSE_SDL
SDL_LIBS   = `sdl-config --libs`
GLUT_LIBS  =
else
GLUT_LIBS  = -lglut
SDL_CFLAGS =
SDL_LIBS   =
endif

if USE_DEBUG
DEBUG_CFLAGS = -DDEBUG
else
DEBUG_CFLAGS =
endif

if USE_NVIDIA
NVIDIA_CFLAGS =
else
NVIDIA_CFLAGS = -DNO_NV_BUMPREF -DNO_NV_FRESNEL
endif

if USE_SOUND
SOUND_CFLAGS = -DUSE_SOUND
else
SOUND_CFLAGS =
endif

AM_CFLAGS = -Wall `freetype-config --cflags` ${SDL_CFLAGS} ${NVIDIA_CFLAGS} ${SOUND_CFLAGS} ${DEBUG_CFLAGS}

#LIBS   = -L$(LIBDIR) -lglut -lGLU -lGL -lXext -lXaw -lm -lXi -lpng -lz
#LIBS   = -L$(LIBDIR) -lglut -lGLU -lGL -lXext -lXaw -lm -lXi -lpng -lz
AM_LDFLAGS = ${all_libraries} `freetype-config --libs` ${SDL_LIBS} ${GLUT_LIBS}
#LDADD = ${X_PRE_LIBS} ${X_LIBS} ${X_EXTRA_LIBS} -lGL -lGLU -lXaw -lm -lXi -lpng -lz -L/home/floh/nvsdk/OpenGL/lib/ -lnvparse
LDADD = ${X_PRE_LIBS} ${X_LIBS} ${X_EXTRA_LIBS} -lGL -lGLU -lXaw -lm -lXi -lpng -lz
