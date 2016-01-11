#include "state.h"
#include <stdio.h>
#include "main.h"

start_gcrypt () {

  return 1;
}

load_config (char *config_file) {
  yaml_token_t token;
  char key[256], tmpkey[256];

  config_file_handle = fopen (config_file, "r");

  /* Initialize parser */
  if (!yaml_parser_initialize (&parser))
    fputs ("Failed to initialize parser!\n", stderr);
  if (config_file_handle == NULL)
    fputs ("Failed to open file!\n", stderr);


  yaml_parser_set_input_file (&parser, config_file_handle);

  do {
    yaml_parser_scan (&parser, &token);

    /**
     switch(token.type){


    case YAML_STREAM_START_TOKEN: puts("YAML_STREAM_START_TOKEN  "); break;
    case YAML_STREAM_END_TOKEN:   puts("YAML_STREAM_END_TOKEN");   break;

    case YAML_KEY_TOKEN:   puts("YAML_KEY_TOKEN   "); break;
    case YAML_VALUE_TOKEN: puts("YAML_VALUE_TOKEN "); break;

    case YAML_BLOCK_SEQUENCE_START_TOKEN: puts("YAML_BLOCK_SEQUENCE_START_TOKEN"); break;
    case YAML_BLOCK_ENTRY_TOKEN:          puts("YAML_BLOCK_ENTRY_TOKEN");    break;
    case YAML_BLOCK_END_TOKEN:            puts("YAML_BLOCK_END_TOKEN");              break;

    case YAML_BLOCK_MAPPING_START_TOKEN:  puts("YAML_BLOCK_MAPPING_START_TOKEN");            
    
    break;
    
    case YAML_SCALAR_TOKEN:  printf("YAML_SCALAR_TOKEN %s \n", token.data.scalar.value); break;

    default:
      printf("Got token of type %d\n", token.type);
      
       
     } **/
    if (token.type == YAML_BLOCK_MAPPING_START_TOKEN) {
      do {
	yaml_parser_scan (&parser, &token);

	if (token.type == YAML_KEY_TOKEN) {
	  yaml_parser_scan (&parser, &token);
	  if (token.type == YAML_SCALAR_TOKEN) {
	    strncpy (key, token.data.scalar.value, 255);

	  }
	  yaml_parser_scan (&parser, &token);
	  if (token.type == YAML_VALUE_TOKEN) {
	    yaml_parser_scan (&parser, &token);
	    if (token.type == YAML_BLOCK_SEQUENCE_START_TOKEN) {
	      printf ("Block sequence start: %s\r\n", key);

	      yaml_parser_scan (&parser, &token);
	      while (token.type != YAML_BLOCK_END_TOKEN) {
		if (token.type == YAML_BLOCK_ENTRY_TOKEN) {
		  yaml_parser_scan (&parser, &token);
		  if (token.type == YAML_BLOCK_MAPPING_START_TOKEN) {
		    yaml_parser_scan (&parser, &token);
		    if (token.type == YAML_KEY_TOKEN) {
		      yaml_parser_scan (&parser, &token);
		      if (token.type == YAML_SCALAR_TOKEN) {
			strncpy (tmpkey, token.data.scalar.value, 255);
			yaml_parser_scan (&parser, &token);
			if (token.type == YAML_VALUE_TOKEN) {
			  yaml_parser_scan (&parser, &token);
			  if (token.type == YAML_SCALAR_TOKEN) {
			    printf ("%s == %s\r\n", tmpkey, token.data.scalar.value);
			    yaml_parser_scan (&parser, &token);
			    if (token.type == YAML_BLOCK_END_TOKEN) {
			      yaml_parser_scan (&parser, &token);

			    }
			  }
			}

		      }

		    }

		  }

		}
	      }

	      if (token.type == YAML_BLOCK_END_TOKEN) {
		yaml_parser_scan (&parser, &token);

	      }
	    }



	  }


	}

      } while (token.type != YAML_BLOCK_END_TOKEN);
    }
  } while (token.type != YAML_STREAM_END_TOKEN);

  yaml_token_delete (&token);
  return 0;

  return 1;

}

start_networking () {
  return 1;

}

start_threads () {

  return 1;

}

stop_gcrypt () {

  return 1;
}

stop_networking () {

  return 1;

}
stop_threads () {


  return 1;

}

void
unload_config () {
  yaml_parser_delete (&parser);
  fclose (config_file_handle);

}
