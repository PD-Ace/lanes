#define _GNU_SOURCE      //we need this for recvmmsg/sendmmsg
#define _USE_GNU 1

#include <stdio.h>


#include "state.h"
#include "main.h"
#include "util.h"
#include "network.h"

start_gcrypt () {
init_gcrypt();
  return 1;
}

load_config (char *config_file) {
  yaml_token_t token;
  char key[256], tmpkey[256], name[256], last_name[256];
  struct peer_context *peer;
  int peer_init = 0, peerid = 1;

  config_file_handle = fopen (config_file, "r");

  /* Initialize parser */
  if (!yaml_parser_initialize (&parser))
    fputs ("Failed to initialize parser!\n", stderr);
  if (config_file_handle == NULL)
    fputs ("Failed to open file!\n", stderr);


  yaml_parser_set_input_file (&parser, config_file_handle);
 /***** I'm sure there is a better way to do this,this works for now
  * not a big deal since it is just a config loading function****/

  do {
    yaml_parser_scan (&parser, &token);

    if (token.type == YAML_BLOCK_MAPPING_START_TOKEN) {
      do {
	yaml_parser_scan (&parser, &token);

	if (token.type == YAML_KEY_TOKEN) {
	  yaml_parser_scan (&parser, &token);
	  if (token.type == YAML_SCALAR_TOKEN) {
	    strncpy (key, token.data.scalar.value, 255);
	    //  printf("Got key - %s\r\n",key);
	  }
	  yaml_parser_scan (&parser, &token);
	  if (token.type == YAML_VALUE_TOKEN) {
	    yaml_parser_scan (&parser, &token);
// 	    if (token.type == YAML_BLOCK_SEQUENCE_START_TOKEN) {
// //                        printf ("Block sequence start: %s\r\n", key);
// 
// 	      if (strncmp ("global", key, 6) == 0)
// 		sequence = SEQ_GLOBAL;
// 	      else if (strncmp ("symmetric-peer", key, 14) == 0)
// 		sequence = SEQ_SPEER;
// 	      else if (strncmp ("assymetric-peer", key, 15) == 0)
// 		sequence = SEQ_APEER;
// 
// 	      yaml_parser_scan (&parser, &token);
// 	      while (token.type != YAML_BLOCK_END_TOKEN) {
// 		if (token.type == YAML_BLOCK_ENTRY_TOKEN) {
// 		  yaml_parser_scan (&parser, &token);
// 		  if (token.type == YAML_BLOCK_MAPPING_START_TOKEN) {
// 		    yaml_parser_scan (&parser, &token);
// 		    if (token.type == YAML_KEY_TOKEN) {
// 		      yaml_parser_scan (&parser, &token);
// 		      if (token.type == YAML_SCALAR_TOKEN) {
// 			strncpy (tmpkey, token.data.scalar.value, 255);
// 			yaml_parser_scan (&parser, &token);
// 			if (token.type == YAML_VALUE_TOKEN) {
// 			  yaml_parser_scan (&parser, &token);
// 			  if (token.type == YAML_SCALAR_TOKEN) {
// 			    if (strncmp ("name", tmpkey, 4) == 0)
// 			      strncpy (name, token.data.scalar.value, 255);
// 			    else
// 			      //printf
// 			      //  ("%i  <<%s>> %s == %s\r\n",
// 			      //  sequence, name,
// 			      //  tmpkey,
// 			      //  token.data.scalar.
// 			      //  value);
// 
// 			      yaml_parser_scan (&parser, &token);
// 			    if (token.type == YAML_BLOCK_END_TOKEN) {
// 			      // printf ("BLOCK END \r\n");
// 
// 			      yaml_parser_scan (&parser, &token);
// 
// 			    }
// 			  }
// 			}
// 
// 		      }
// 
// 		    }
// 
// 		  }
// 
// 		}
// 	      }
// 	      // yaml_parser_scan (&parser, &token);
// 	      if (token.type == YAML_BLOCK_END_TOKEN) {
// 		//  yaml_parser_scan (&parser, &token);
// 		//     printf ("BLOCK END \r\n");
// 	      }
// 	    }



	  }


	}

      }
      while (token.type != YAML_BLOCK_END_TOKEN);
    }
    else if (token.type == YAML_KEY_TOKEN) {
      yaml_parser_scan (&parser, &token);
      if (token.type == YAML_SCALAR_TOKEN) {
	strncpy (key, token.data.scalar.value, 255);
      }
      yaml_parser_scan (&parser, &token);
      if (token.type == YAML_VALUE_TOKEN) {
	yaml_parser_scan (&parser, &token);
	if (token.type == YAML_BLOCK_SEQUENCE_START_TOKEN) {
	  //        printf ("--Block sequence start: %s\r\n", key);
	  if (strncmp ("symmetric-peers", key, 15) == 0)
	    sequence = SEQ_SPEER;
	  else if (strncmp ("assymetric-peers", key, 16) == 0)
	    sequence = SEQ_APEER;
	  yaml_parser_scan (&parser, &token);
	  if (token.type == YAML_BLOCK_ENTRY_TOKEN) {
	    do {
	      yaml_parser_scan (&parser, &token);
	      if (token.type == YAML_BLOCK_MAPPING_START_TOKEN) {
		yaml_parser_scan (&parser, &token);

		while (token.type != YAML_BLOCK_ENTRY_TOKEN && token.type != YAML_BLOCK_END_TOKEN) {
		  if (token.type == YAML_KEY_TOKEN) {
		    yaml_parser_scan (&parser, &token);
		    if (token.type == YAML_SCALAR_TOKEN) {
		      strncpy (key, token.data.scalar.value, 255);
		      yaml_parser_scan (&parser, &token);
		      if (token.type == YAML_VALUE_TOKEN) {
			yaml_parser_scan (&parser, &token);
			if (token.type == YAML_SCALAR_TOKEN) {
			  if (strncmp ("name", key, 4)
			      == 0)
			    strncpy (name, token.data.scalar.value, 255);

			  if (sequence == SEQ_SPEER) {

			    if (peer_init && (strncmp (key, "name", strlen (key)) != 0)
				&& strncmp (last_name, name, strlen (last_name)) == 0) {
			      if (strncmp (key, "type", strlen (key)) == 0) {
				if (strncmp
				    (token.data.scalar.value, "ingress",
				     strlen (token.data.scalar.value)) == 0)
				  peer->direction = IN;
				else
				  if (strncmp
				      (token.data.scalar.value, "egress",
				       strlen (token.data.scalar.value)) == 0)
				  peer->direction = OUT;
				else
				  if (strncmp
				      (token.data.scalar.value, "map",
				       strlen (token.data.scalar.value)) == 0)
				  peer->direction = MAP;
			      }
			      else if (strncmp (key, "ip-version", strlen (key)) == 0) {
				if (strncmp
				    (token.data.scalar.value, "4",
				     strlen (token.data.scalar.value)) == 0)
				  peer->ip_version = 4;
				else
				  if (strncmp
				      (token.data.scalar.value, "6",
				       strlen (token.data.scalar.value)) == 0)
				  peer->ip_version = 6;
				else
				  die (1, "Invalid ip-version value:%s", token.data.scalar.value);
			      }
			      else if (strncmp (key, "threads", IFNAMSIZ) == 0) {

				peer->threads = atoi (token.data.scalar.value);
				if (!peer->threads) {
				  die (0,
				       "No valid threadcount specified for %s.setting thread count to 1",
				       peer->name);
				}
			      }
			      else if (strncmp (key, "interface", IFNAMSIZ) == 0) {
				strncpy (peer->ifname, token.data.scalar.value, 255);

			      }
			      else if (strncmp (key, "map-interface", IFNAMSIZ) == 0) {
				strncpy (peer->ifmap, token.data.scalar.value, 255);
				peer->direction = MAP;
			      }
			      else if (strncmp (key, "tunnel", IFNAMSIZ) == 0) {
				strncpy (peer->tifname, token.data.scalar.value, 255);
			      }
			      else if (strncmp (key, "peer-ip", strlen (key)) == 0) {
				strncpy (peer->peer_ip, token.data.scalar.value, 255);
				peer->peerip = host_to_ip (token.data.scalar.value);
				if (!peer->peerip)
				  die (1, "Peer-IP %s resolution failure!",
				       token.data.scalar.value);

			      }
			      else if (strncmp (key, "next-hop", strlen (key)) == 0) {
				strncpy (peer->next_hop, token.data.scalar.value, 255);
			      }
			      else if (strncmp (key, "local-ip", strlen (key)) == 0) {
				peer->myip = host_to_ip (token.data.scalar.value);
				if (!peer->myip)
				  die (1, "Local-IP %s resolution failure!",
				       token.data.scalar.value);


			      }
			      else if (strncmp (key, "cipher", strlen (key)) == 0) {
				strncpy (peer->cipher, token.data.scalar.value, 255);
			      }
			      else if (strncmp (key, "key", strlen (key)) == 0) {
				strncpy (peer->key, token.data.scalar.value, 16000);
			      }
			      else if (strncmp (key, "socket", strlen (key)) == 0) {
				strncpy (peer->socket_path, token.data.scalar.value, 255);
			      }
			      else if (strncmp (key, "pipe", strlen (key)) == 0) {
				strncpy (peer->pipe_path, token.data.scalar.value, 255);
			      }

			    }
			    else {
			      if (peer_init) {
				add_to_peer_list (peer);



			      }
			      else {
				init_peer_list ();
				peer_init = 1;
			      }
			      peer = (struct peer_context *) malloc (sizeof (struct peer_context));
			      peer->id = peerid;
			      ++peerid;
			      strncpy (peer->name, name, 255);
			      strncpy (last_name, name, 255);

			    }

			  }
			  yaml_parser_scan (&parser, &token);
			  if (token.type == YAML_BLOCK_END_TOKEN) {
			    //   printf("***BLOCK END\r\n");
			    yaml_parser_scan (&parser, &token);
			  }
			}
		      }
		    }
		  }

		}
	      }


	    }
	    while (token.type != YAML_BLOCK_END_TOKEN);
	  }




	}
      }
    }
    else {
      //printf ("wah....\r\n");
    }
  }
  while (token.type != YAML_STREAM_END_TOKEN);
  if (peer_init) {
    add_to_peer_list (peer);
  }
  yaml_token_delete (&token);
  //peer_list_dump();
  return 0;

  return 1;

}

start_networking () {
  enumerate_interfaces ();

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
start_supervisor () {
  global.run=1;
  global.running=1;
  init_thread_list ();
  new_thread (SUP, &supervisor, NULL);
}

void
unload_config () {
  yaml_parser_delete (&parser);
  fclose (config_file_handle);

}
