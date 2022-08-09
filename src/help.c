#include "hanh.h"

void help() {
	printf("hanh - Hanh Linux package manager\n");
	printf("Usage: hanh [Action] [Options]\n");
	printf("Action\n");
	printf("-i                        Install packages <file>\n");
	printf("-r                        Remove packages\n");
	printf("-q                        Query package information\n");
	printf("-s                        Sync database from remote\n");
	printf("-f                        Find for matched packages\n");
	printf("-S                        Build a snapshot\n");
	printf("Options \n");
	printf("-h                        Print this message\n");
	printf("-v                        Print version\n");
	printf("-R <path>                 Change root directory\n");
	printf("-d <command>              Use another download command\n");
	printf("-m <path>                 Use specified mirror directory\n");
	printf("-t [info,filelist]        Specify file(s) to search\n");
	printf("-D                        Disable dependencies check\n");
	exit(0);
	}
