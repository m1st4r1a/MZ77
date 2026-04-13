#include<config.h>
#include<stdlib.h>
#include<stdnoreturn.h>
#include<stdio.h>
#include "dt.h"
#include "messages.h"

noreturn void
show_help (STR callable_name)
{
  fprintf (stderr, "Usage: %s [OPTIONS]...\n"
	   "\t-c, --compress\t\tCompression mode\n"
	   "\t-d, --decompression\t\tDecompression mode\n"
	   "\t-i, --input=FILENAME\t\tInput file path\n"
	   "\t-o, --output=FILENAME\t\tOutput file path\n"
	   "\t-V, --version\t\tOutput version information and exit\n"
	   "\t-v, --verbose\t\tEnable verbose output\n"
	   "\t-h, --help\t\tDisplay this help and exit\n", callable_name);
  exit (EXIT_SUCCESS);
}

noreturn void
show_version (void)
{
  fprintf (stderr, "%s\n"
	   "Copyright (C) 2026 mb6ockatf & m1st4r1a\n"
	   "License GPLv3+: GNU GPL version 3 or later\n"
	   "This is free software: you are free to change and redistribute it.\n"
	   "There is NO WARRANTY, to the extent permitted by law.\n",
	   PACKAGE_NAME);
  exit (EXIT_SUCCESS);
}
