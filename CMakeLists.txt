cmake_minimum_required(VERSION 2.8.11)

project(PiTimer)



include_directories(
    C:/Users/Johan/OneDrive/Kod/Pi/PiTimer/
    C:/Users/Johan/OneDrive/Kod/Pi/PiTimer/
)

# Linker options
set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS}  ")

# Library path
set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS} -L\".\" ")

# Define the C sources
set ( C_SRCS
    configuration.c
    curing.c
    device_config_list.c
    jsvtellstick.c
    jsvtimerd.c
    logging.c
    mystring.c
    ws_client.c
)

set_source_files_properties(
    ${C_SRCS} PROPERTIES COMPILE_FLAGS 
    " -g -O0 -Wall")

add_executable(PiTimer ${CXX_SRCS} ${C_SRCS})

