
#include "main.h"

int
main (int argc, char **argv) {
//this is just temporary , will code in proper getopts() thingy
  if (argc < 2)
    die (1, "Error,no arguments provided!\r\n");
    
  parse_args(argc,argv,&global);
  
  if (load_config (global.config_file)) {
    die (1,"Error loading config file %s",global.config_file);
      }
      
  start_supervisor ();

  unload_config ();
  return 0;
}
