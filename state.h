#ifndef STATE_H
#define STATE_H
#include "yaml.h"
//#include "net/if.h"

#define SEQ_GLOBAL 1		//global config
#define SEQ_APEER  2		//asymmetric peer
#define SEQ_SPEER  3		// symmetric peer

/*
 * This file defines functions used to start and stop the application and load
 * configuration files. they should return 1 on failure,0 on success.
 */
static yaml_parser_t parser;
static FILE *config_file_handle;
static unsigned int sequence;
int start_gcrypt ();
int start_networking ();
int start_threads ();
int load_config (char *config_file);
void start_supervisor ();

int stop_gcrypt ();
int stop_networking ();
int stop_threads ();
void unload_config ();

#endif
