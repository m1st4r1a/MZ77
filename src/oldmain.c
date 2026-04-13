#define MAX_NAME_LEN 256
#define VERBOSE_OPT 'v'
#define HELP_OPT 'h'
#define VERSION_OPT 'V'
#define INPUT_OPT 'i'
#define OUTPUT_OPT 'o'
#define COMPRESS_OPT 'c'
#define DECOMPRESS_OPT 'd'
int32_t opt, option_index = 0;
struct option long_options[] = {
  {"verbose", no_argument, 0, VERBOSE_OPT},
  {"help", no_argument, 0, HELP_OPT},
  {"version", no_argument, 0, VERSION_OPT},
  {"input", required_argument, 0, INPUT_OPT},
  {"output", required_argument, 0, OUTPUT_OPT},
  {"compress", no_argument, 0, COMPRESS_OPT},
  {"decompress", no_argument, 0, DECOMPRESS_OPT},
  {0, 0, 0, 0}
};

int
main (int argc, char *argv[])
{
  bool verbose = false;
  (void) verbose;
  char input_filename[MAX_NAME_LEN] = "", output_filename[MAX_NAME_LEN] = "";
  enum Mode selected_mode = NONE;
  while ((opt =
	  getopt_long (argc, argv, "vhVi:o:cd", long_options,
		       &option_index)) != -1)
    {
      switch (opt)
	{
	case VERBOSE_OPT:
	  verbose = true;
	  break;
	case HELP_OPT:
	  show_help (argv[0]);
	case VERSION_OPT:
	  show_version ();
	case INPUT_OPT:
	  strncpy (input_filename, optarg, MAX_NAME_LEN - 1);
	  input_filename[MAX_NAME_LEN - 1] = 0;
	  break;
	case OUTPUT_OPT:
	  strncpy (output_filename, optarg, MAX_NAME_LEN - 1);
	  output_filename[MAX_NAME_LEN - 1] = 0;
	  break;
	case COMPRESS_OPT:
	  selected_mode = COMPRESS;
	  break;
	case DECOMPRESS_OPT:
	  selected_mode = DECOMPRESS;
	  break;
	default:
	  fprintf (stderr, "Try '%s --help' for more information.\n",
		   argv[0]);
	  return EXIT_FAILURE;
	}
    }
  if (selected_mode == COMPRESS)
    compress (input_filename, output_filename);
  else if (selected_mode == DECOMPRESS)
    decompress (input_filename, output_filename);
  else
    {
      fprintf (stderr,
	       "Mode not specified. Try '%s --help' for more information.\n",
	       argv[0]);
      return EXIT_FAILURE;
    }
  puts ("hello human");
  return EXIT_SUCCESS;
}
