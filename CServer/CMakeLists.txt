cmake_minimum_required(VERSION 3.6)

project(Group_Project C)

set(CMAKE_C_STANDARD 11)


# sudo apt-get install postgresql-12
# sudo apt-get install libpq-dev postgresql-server-dev-all
include_directories(/usr/include/postgresql)
link_libraries(-lpq)


#link_libraries(/usr/lib/x86_64-linux-gnu)
#set (CMAKE_SHARED_LINKER_FLAGS "-lpq")

#set(OPTIONS "-lpq")
#target_compile_options(-lpq)
#target_compile_options(foo PUBLIC "$<$<CONFIG:RELEASE>:${OPTIONS}>")

#link_directories(/usr/lib)


find_package(PostgreSQL REQUIRED)
set(SOURCE_FILES main.c Decoder.h Decoder.c Database.c Database.h)
add_executable(Group_Project ${SOURCE_FILES})