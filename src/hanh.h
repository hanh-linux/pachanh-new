#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <confuse.h>
#include <errno.h>
#include <getopt.h>

#define __PREFIX "prefix"
#define __PATH 2048
#define __VER  "2.1"
#define __ARG  1024 
#define __CMD  512

#define mode_package 1
#define mode_stage 2

void err(const char *msg);
void die(const char *msg, const int code); 
void checkCode(const int code);
void debug(const char *msg);

int checkEmpty(const char *obj, const char *msg); 
int checkPath(const char *obj, const char *msg);
int checkFile(const char *obj, const char *msg);
int checkDir(const char *obj, const char *msg); 
int checkDeps(const char *root, char depends[]);
int checkInstalled(const char *root, const char *pkg); 
int untar(const char *untarPath, const char *file); 
int updatePkglist(const char *root, const char *pkg);
int clearTmp(const char *tmpdir);
int getSize(const char *filepath);

void help(); 
int INSTALL(char packages[], const char *opts, const char *mode, const char *insroot, const char *root, const char *mirror, const char *download, char allrepo[], const int nodepends, const long int verbose); 
int REMOVE(char packages[], const char *root, const char *mode, const long int verbose, const int exitFail);
int QUERY(char packages[], const char *root, const char *filetype);
int SYNC(char repositories[], const char *root, const char *mirror, const char *fetchCmd, const long int verbose);
void FIND(char packages[], const char *root, char repositories[]);
int SNAPSHOT(const char *root); 
