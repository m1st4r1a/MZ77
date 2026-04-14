bin_PROGRAMS = mz77
mz77_SOURCES = \
	       main.c \
	       hashtable.c \
	       messages.c \
	       BitWriter.c BitReader.c \
	       compress.c decompress.c \
	       queue.c \
	       utils.c
mz77_CFLAGS = $(GTK4_CFLAGS) $(PROFILE_CFLAGS)
mz77_LDFLAGS = $(PROFILE_LDFLAGS)
mz77_LDADD = $(GTK4_LIBS) -lpthread

