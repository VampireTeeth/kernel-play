//
// Created by steven on 25/8/26.
//

#include "pparser.h"

#include "config.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "string/string.h"

static int pparser_parse_drive_no(const char** path_str, int* drive_no)
{
    char first_byte = **path_str;
    if (!is_digit(first_byte))
    {
        return -EINVARG;
    }
    *drive_no = char_to_numeric(first_byte);
    (*path_str) += 1;
    return 0;
}

static int validate_start_of_path(const char** path_str)
{
    int r = 0;
    char* first_2_bytes = ":/";
    r = memcmp(first_2_bytes, *path_str, 2);
    if (r < 0) return -EINVARG;
    *path_str += 2;
    return 0;
}

static int pparser_parse_path_parts(const char** path_str, path_part_t* path_part)
{
    int name_len = 0;
    const char* p = *path_str;
    while (*p != '/' && *p != '\0')
    {
        p++;
        name_len++;
    }
    char* name = kheap_zalloc((name_len+1)*sizeof(char));
    memcpy(name, p - name_len, name_len);
    name[name_len] = '\0';
    path_part->path = name;
    if (*p == '\0')
    {
        path_part->next = NULL;
        return 0;
    }
    path_part_t* next_part = kheap_zalloc(sizeof(path_part_t));
    *path_str = p + 1; // current p points to '/', so move 1 more step forward
    int r = pparser_parse_path_parts(path_str, next_part);
    if (r < 0)
    {
        path_part->next = NULL;
        return r;
    }
    path_part->next = next_part;
    return 0;
}

int pparser_parse_path_root(const char* const path_str, path_root_t* path_root)
{
    const char* p_str = path_str;
    int len = strlen(path_str);
    if (len > PPARSER_MAX_PATH) return -EINVARG;
    int drive_no = 0;
    int r = 0;
    r = pparser_parse_drive_no(&p_str, &drive_no);
    if (r < 0) return r;
    r = validate_start_of_path(&p_str);
    if (r < 0) return r;
    path_part_t* path_part = kheap_zalloc(sizeof(path_part_t));
    r = pparser_parse_path_parts(&p_str, path_part);
    if (r < 0) return r;
    path_root->drive_no = drive_no;
    path_root->parts = path_part;
    return 0;
}

static void pparser_free_path_part(path_part_t* path_part)
{
    kheap_free((void*)path_part->path);
    if (path_part->next != NULL)
    {
        pparser_free_path_part(path_part->next);
    }
    kheap_free(path_part);
}

void pparser_free_path_root(path_root_t* const path_root)
{
    pparser_free_path_part(path_root->parts);
    kheap_free(path_root);
}

