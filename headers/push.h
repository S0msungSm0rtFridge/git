
#ifndef PUSH_H
#define PUSH_H


int push_object(const char *hash, const char *dir);
int push_commit(const char *hash, const char *dir);
int push_tree(const char *hash, const char *dir);
int copy_file(const char *src, const char *dest);
int mygit_push(int argc, char *argv[]);

#endif