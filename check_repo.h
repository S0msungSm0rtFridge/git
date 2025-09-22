
#ifndef INIT_H
#define INIT_h

static int dir_exist(const char *check_dir);
static char *get_parent_dir(const char *check_path);
char *find_root_repo(const char *start_dir);

#endif