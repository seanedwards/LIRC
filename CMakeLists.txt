cmake_minimum_required(VERSION 2.8)
project(lirc)

set(CMAKE_BUILD_TYPE Debug)

include_directories(
	"/usr/include/lua5.2"
	)

file(GLOB SCHEMAS RELATIVE ${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/schema/schema.v*.sql
	)

foreach (SCHEMA ${SCHEMAS})
	message(STATUS "Creating build rules for schema script ${SCHEMA}")
	add_custom_command(
		OUTPUT ${SCHEMA}.o
		COMMAND objcopy --input binary --output elf32-i386 --binary-architecture i386 
			${SCHEMA} ${SCHEMA}.o
		DEPENDS ${SCHEMA}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMENT "Compiling schema script ${SCHEMA}"
		)

	set(SCHEMAS_OUT ${SCHEMAS_OUT} ${SCHEMA}.o)

endforeach (SCHEMA ${SCHEMAS})

add_custom_command(
	OUTPUT ${CMAKE_SOURCE_DIR}/schema.c
	COMMAND sh ${CMAKE_SOURCE_DIR}/genschema.sh > ${CMAKE_SOURCE_DIR}/schema.c
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	COMMENT "Building schema rev function."
	)

add_executable(lirc
	${SCHEMAS_OUT}
	main.c
	callbacks.c
	schema.c
	)

target_link_libraries(lirc
	yaml
	lua5.2
	ircclient
	sqlite3
	)

