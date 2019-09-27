#include <argp.h>
#include <unistd.h> // for access
#include <stdio.h>
#include <stdlib.h>

/* This structure is used by main to communicate with parse_opt. */
struct args
{
  char *graph_path;
  unsigned int reflected;
  unsigned int seed;
};

/*
   OPTIONS.  Field 1 in ARGP.
   Order of fields: {NAME, KEY, ARG, FLAGS, DOC}.
*/
static struct argp_option options[] =
{
  {"graph_path",'g',"path",0,"path to edge list file"},
  {"reflected",'r',"path",0,"set to 0 if file does not have edges reflected"},
  {"seed",'s',"count",0,"random seed for vertex ordering"},
  {0}
};

/*
   PARSER. Field 2 in ARGP.
*/
static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
  struct args *args = state->input;
  switch (key)
    {
    case ARGP_KEY_INIT:
      args->reflected = 1;
      args->seed = 0;
      break;
    case ARGP_KEY_END:
      if (args->graph_path == NULL || access(args->graph_path,R_OK) == -1) {
	fprintf(stderr,"Parse error: need -g to a readable file.\n");
	return ENOENT;
      }
      break;
    case 'g':
      args->graph_path = arg;
      break;
    case 'r':
      args->reflected = strtol(arg,NULL,10);
      break;
    case 's':
      args->seed = strtol(arg,NULL,10);
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

void parse_args(struct args *args, int argc, char **argv)
{
  static struct argp argp = {options, parse_opt, NULL, NULL};
  argp_parse(&argp, argc, argv, 0, 0, args);
  return;
}
