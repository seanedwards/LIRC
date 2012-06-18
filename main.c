#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "assert.h"

#include <signal.h>
#include <yaml.h>

#include "callbacks.h"

irc_session_t* irc_session = NULL;

void sig_quit(int signal) {
	printf("Caught shutdown signal. Goodbye.\n");
	if (irc_session != NULL && irc_is_connected(irc_session))
		irc_disconnect(irc_session);
}

int main(int argc, char* const* argv) {
	yaml_parser_t yaml_parser;
	
	FILE* config = NULL;

	// Parse command line arguments
	int c;
	while ((c = getopt(argc, argv, "hdfc:")) != -1) {
		switch (c) {
		case 'c':
			config = fopen(optarg, "rb");
			if (config == NULL) {
				fprintf(stderr, "Can not open config file `%s'.\n", optarg);
				return 1;
			}
			break;
		case 'f':
		case 'd':
			break;
		case 'h':
			printf("Usage: %s [OPTION...]\n", argv[0]);
			printf("  -h                 Print this help page and exit.\n");
			printf("  -c <file>          Specify the config file to load (default: ./lirc.yml).\n");
			printf("  -d <db>            Specify the database to use (default: ./lirc.db).\n");
			printf("  -f                 Run the program in the background (fork to daemon).\n");
			printf("\n");
			printf("Report bugs at https://github.com/seanedwards/LIRC\n");
			return 0;
		case '?': return 1;
		}
	}

	if (config == NULL) {
		fprintf(stderr, "No config file specified. Check `%s -h' for program usage.\n", argv[0]);
		return 1;
	}

	// Parse config file
	yaml_event_t event;
	yaml_parser_initialize(&yaml_parser);
	yaml_parser_set_input_file(&yaml_parser, config);

	int done = 0;
	while (!done) {
		if (!yaml_parser_parse(&yaml_parser, &event))
			return 1;

		printf("Event: %d\n", event.type);
		done = (event.type == YAML_STREAM_END_EVENT);
		yaml_event_delete(&event);
	}

	yaml_parser_delete(&yaml_parser);
	fclose(config);

	irc_callbacks_t irc_callbacks = {0};
	irc_session = irc_create_session(&irc_callbacks);

	irc_connect(irc_session,
			"irc.freenode.net",
			6667, "",
			"LIRC_Bot", "LIRC_Bot", "LIRC_Bot");

	signal(SIGINT, sig_quit);
	irc_run(irc_session);

	irc_destroy_session(irc_session);
	return 0;
error:
	irc_destroy_session(irc_session);
	return 1;

}

