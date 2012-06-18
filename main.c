#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "assert.h"

#include <signal.h>
#include <yaml.h>

#include <sqlite3.h>

#include "callbacks.h"

int schema_rev(sqlite3* db);

irc_session_t* irc_session = NULL;
sqlite3* db = NULL;

void sig_quit(int signal) {
	printf("Caught shutdown signal. Goodbye.\n");
	if (irc_session != NULL && irc_is_connected(irc_session))
		irc_disconnect(irc_session);
}

void onquit() {
	if (db != NULL) sqlite3_close(db);
	if (irc_session != NULL) irc_destroy_session(irc_session);
}

int main(int argc, char* const* argv) {
	atexit(onquit);
	
	FILE* config = NULL;

	// Parse command line arguments
	int c;
	while ((c = getopt(argc, argv, "hdfc:")) != -1) {
		switch (c) {
		case 'c':
			config = fopen(optarg, "rb");
			if (config == NULL) {
				fprintf(stderr, "Can not open config file `%s'.\n", optarg);
				exit(1);
			}
			break;
		case 'f':
			printf("Will run in background.");
			break;
		case 'd': {
			int rc = sqlite3_open(optarg, &db);
			if (rc) {
				fprintf(stderr, "Cannot open database `%s': %s", optarg, sqlite3_errmsg(db));
				if (config != NULL) fclose(config);
				exit(1);
			}
			} break;
		case 'h':
			printf("Usage: %s [OPTION...]\n", argv[0]);
			printf("  -h                 Print this help page and exit.\n");
			printf("  -c <file>          Specify the config file to load (default: ./lirc.yml).\n");
			printf("  -d <db>            Specify the database to use (default: ./lirc.db).\n");
			printf("  -f                 Run the program in the background (fork to daemon).\n");
			printf("\n");
			printf("Report bugs at https://github.com/seanedwards/LIRC\n");
			exit(0);
		case '?': exit(1);
		}
	}

	if (config == NULL) {
		fprintf(stderr, "No config file specified. Check `%s -h' for program usage.\n", argv[0]);
		exit(1);
	}

	if (db == NULL) {
		fprintf(stderr, "No database file specified. Check `%s -h' for program usage.\n", argv[0]);
		exit(1);
	}

	schema_rev(db);

	// Parse config file
	yaml_document_t document;
	yaml_parser_t yaml_parser;

	yaml_parser_initialize(&yaml_parser);
	yaml_parser_set_input_file(&yaml_parser, config);

	int done = 0;
	while (!done) {
		if (!yaml_parser_load(&yaml_parser, &document)) {
			fprintf(stderr, "Could not parse config file: %s on line %d.\n", 
					yaml_parser.problem, yaml_parser.problem_mark.line);
			yaml_parser_delete(&yaml_parser);
			assert(!fclose(config));
			exit(1);
		}
		done = (!yaml_document_get_root_node(&document));

		yaml_document_delete(&document);
	}

	yaml_parser_delete(&yaml_parser);
	assert(!fclose(config));

	irc_callbacks_t irc_callbacks = {0};
	irc_session = irc_create_session(&irc_callbacks);

	irc_connect(irc_session,
			"irc.freenode.net",
			6667, "",
			"LIRC_Bot", "LIRC_Bot", "LIRC_Bot");

	signal(SIGINT, sig_quit);
	irc_run(irc_session);

	return 0;
}

