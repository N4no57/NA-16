CMAKE_MINIMUM_REQUIRED(VERSION 4.2)
project(linker)

set(CMAKE_C_STANDARD 23)

add_executable(linker
        src/main.c
        src/lib/linklib.h
        src/object_file_reader/obj_file.h
        src/object_file_reader/obj_reader.c src/object_file_reader/obj_reader.h
        src/lib/global_symbol_table.c src/lib/global_symbol_table.h
        src/lib/linked_section_table.c src/lib/linked_section_table.h
        src/lib/section_map.c src/lib/section_map.h
        src/linker.c src/linker.h
)