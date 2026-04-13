bin_PROGRAMS = mz77
mz77_SOURCES = \
	       main.c \
	       messages.c \
	       BitWriter.c BitReader.c \
	       compress.c decompress.c \
	       utils.c
mz77_CFLAGS = $(GTK4_CFLAGS) $(PROFILE_CFLAGS)
mz77_LDFLAGS = $(PROFILE_LDFLAGS)
mz77_LDADD = $(GTK4_LIBS) -lpthread

