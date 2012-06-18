#!/bin/sh

# Schema generator for IRC bot.

# I'm sorry.

echo "#include <stdio.h>"
echo "#include <stdlib.h>"
echo "#include <unistd.h>"

echo "#include <sqlite3.h>"

echo "int schema_rev(sqlite3* db) {"
echo "size_t schematext_size = 0;"
echo "char* schematext = 0;"
echo "char* sqlerr = 0;"
for f in schema/*.o
do

	SCHEMA_VERSION=`echo $f | sed -r 's/schema\/schema\.v([0-9]+)\.sql\.o/\1/'`
	echo "extern const char _binary_schema_schema_v${SCHEMA_VERSION}_sql_start;"
	echo "extern const char _binary_schema_schema_v${SCHEMA_VERSION}_sql_end;"
	echo "extern const unsigned char _binary_schema_schema_v${SCHEMA_VERSION}_sql_size;"

	echo "schematext_size = 
		&_binary_schema_schema_v${SCHEMA_VERSION}_sql_end - 
		&_binary_schema_schema_v${SCHEMA_VERSION}_sql_start + 1;"
	echo "schematext = (char*)malloc(schematext_size);"
	echo "schematext[schematext_size - 1] = 0;"

	echo "if (sqlite3_exec(db, schematext, 0, 0, &sqlerr) != SQLITE_OK) {"
		echo "fprintf(stderr, \"Failed to update schema to v%d: '%s'\\\\n\", ${SCHEMA_VERSION}, sqlerr);"
		echo "sqlite3_free(sqlerr);"
		echo "return 1;"
	echo "}"

	echo "free(schematext);"
done

echo "return 0;"
echo "}"

