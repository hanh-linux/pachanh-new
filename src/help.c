#include "hanh.h"

void help() {
	printf("Usage: hanh [Action] [Options]\n");
	printf("Action\n"); 
	printf("-i [--install     ]                       Install packages <file>\n");
	printf("-r [--remove      ]                       Remove packages\n");
	printf("-q [--query       ]                       Query package information\n");
	printf("-s [--sync        ]                       Sync database from remote\n"); 
	printf("-f [--find        ]                       Find for matched packages\n");
	printf("-S [--snapshot    ]                       Build a snapshot\n");
	printf("-h [--help        ]                       Print this message\n"); 
	printf("-v [--version     ]                       Print version\n");
	printf("Options: \n"); 
	printf("-R [--root        ] <path>                Change root directory\n"); 
	printf("-d [--download    ] <command>             Use another download command\n"); 
	printf("-m [--mirror      ] <path>                Use specified mirror directory\n");
	printf("-g [--ignore      ]                       Ignore acceptable errors and overwritting process\n");
	printf("Options: query only\n");
	printf("-t [--infotype    ] [info,filelist]       Specify file(s) to query\n");
	printf("Options: install only\n");
	printf("-D [--nodeps      ]                       Disable dependencies check\n");
	printf("-I [--instype     ] [packages/stage]      Modify install type\n");
	printf("-T [--stageroot   ] <path>                Specify where to install stage tarball\n");
	printf("Options: snapshot only\n");
	printf("-n [--noinstall   ]                       Disable installing package after built\n");
	printf("-b [--buildtype   ] [system/stage]        Modify snapshot buildtype\n"); 
	printf("-B [--builddir    ] <path>                Packages build directory\n");
	printf("-T [--stageroot   ] <path>                Directory to install stage packages for building\n");
	exit(0);
	}
