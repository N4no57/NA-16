#ifndef NA_16_LINKER_H
#define NA_16_LINKER_H

#include "object_file_reader/obj_file.h"

void link(ObjectFile *objs, u64 obj_count, char *outfile);

#endif //NA_16_LINKER_H
