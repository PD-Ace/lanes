
#include "main.h"

int
main (int argc, char **argv) {
      int i=0;
      
    if (argc < 2)
    die (1, "Error,no arguments provided!\r\n");
    
    for (i; i < 32; i++)
    signal (i, signal_handler); //Setup signal handler
    
  parse_args(argc,argv,&global);
  global.cpu_available=sysconf(_SC_NPROCESSORS_CONF);

  if (load_config (global.config_file)) {        // Get yaml to parse the config
    die (1,"Error loading config file %s",global.config_file);
      }
      
  start_supervisor (); //Start supervisor thread which will start/manage all other threads.

  unload_config (); //supervisor is done...do other stuff and exit peacefully.
  return 0;
}
