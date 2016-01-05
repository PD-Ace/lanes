
#include "main.h"

int
main (int argc, char **argv) {
//this is just temporary , will code in proper getopts() thingy
  if (argc < 2)
    die (1, "Error,no configuration file provided!\r\n");

  if (load_config (argv[1])) {
    perror ("error loading config!!");
    exit (1);
  }
  start_supervisor ();

  unload_config ();
  return 0;
}
