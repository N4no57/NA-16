CMAKE_MINIMUM_REQUIRED(VERSION 4.2)
project(linker)

set(CMAKE_C_STANDARD 23)

add_executable(linker
        src/main.c
        src/object_file_reader/obj_reader.c src/object_file_reader/obj_reader.h
        src/lib/linklib.h
        src/object_file_reader/linked_section.c
        src/object_file_reader/obj_file.h
)

target_include_directories(linker PRIVATE
        dependencies/include
)

target_link_directories(linker PRIVATE
    dependencies/libs
)

target_link_libraries(linker
    collectc
)