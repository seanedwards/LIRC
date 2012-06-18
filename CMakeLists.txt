cmake_minimum_required(VERSION 2.6)
project(lirc)


include_directories(
	"/usr/include/lua5.2"
	)

add_executable(lirc
	main.c
	callbacks.c
	)

target_link_libraries(lirc
	yaml
	lua5.2
	ircclient
	)

