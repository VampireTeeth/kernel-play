//
// Created by steven on 25/8/26.
//

#ifndef FIRST_ASM_PPARSER_H
#define FIRST_ASM_PPARSER_H
#define PPARSER_MAX_PATH 256

typedef struct path_part
{
    const char *path;
    struct path_part *next;
} path_part_t;

typedef struct path_root
{
    int drive_no;
    path_part_t* parts;
} path_root_t;

int pparser_parse_path_root(const char *path_str, path_root_t* path_root_ptr);
void pparser_free_path_root(path_root_t* path_root);

#endif //FIRST_ASM_PPARSER_H
