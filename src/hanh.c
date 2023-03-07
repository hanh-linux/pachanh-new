#include "hanh.h"

int main(int argc, char **argv)
{	
	/*Actions: 
	 * -i 		install 			= 1
	 * -r 		remove 				= 2
	 * -q 		query 				= 3 
	 * -s		sync	 			= 4  
	 * -f		find				= 5
	 * -S		snapshot			= 6
	 * Multiple actions will result in an override*/
	
	int action 	= 0;
	int nodeps	= 0; // enable checking dependencies by default
	int exitcode	= 0;
	int dircode	= 0; 
	int noinstall   = 0;
	int ignore      = 0;
	int opt            ;

	// General variables will be used from command-line 
	char packages[__ARG]         = ""  ;
	char type[__ARG]             = ""  ;
	char buildtype[__ARG]        = ""  ; 
	char installtype[__ARG]      = ""  ;
	char source[__ARG]           = ""  ;
	char builddir[__PATH]        = ""  ;
	char installRoot[__PATH]     = ""  ;
	char localdbpath[__PATH]     = ""  ;
	char remotedbpath[__PATH]    = ""  ;
	char pkgtarballspath[__PATH] = ""  ;
	char stgtarballspath[__PATH] = ""  ; 
	char stgdbpath[__PATH]       = ""  ; 
	char *env_optarg                   ;

	static char *sysroot       = NULL;
	static char *download      = NULL;
	static char *mirror        = NULL;
	static char *repo          = NULL; 
	static long int verbose    = 0   ;

	static struct option long_opts[] = {
		{"install",    0, 0, 'i'}, 
		{"remove",     0, 0, 'r'},
		{"sync",       0, 0, 's'}, 
		{"find",       0, 0, 'f'}, 
		{"query",      0, 0, 'q'},
		{"snapshot",   0, 0, 'S'},
		{"root",       1, 0, 'R'}, 
		{"mirror",     1, 0, 'm'}, 
		{"download",   1, 0, 'd'},
		{"nodeps",     0, 0, 'D'}, 
		{"infotype",   1, 0, 't'},
		{"instype",    1, 0, 'I'},
		{"buildtype",  1, 0, 'b'},
		{"noinstall",  0, 0, 'n'},
		{"help",       0, 0, 'h'},
		{"version",    0, 0, 'v'},
		{"builddir",   1, 0, 'B'},
		{"stageroot",  1, 0, 'T'},
		{"ignore",     0, 0, 'g'},
		{NULL, 0, NULL, 0}
	};
	
	cfg_opt_t options[] = {
	CFG_SIMPLE_STR("sysroot", &sysroot), 
	CFG_SIMPLE_STR("download", &download),
	CFG_SIMPLE_STR("mirror", &mirror), 
	CFG_SIMPLE_STR("repo", &repo),
	CFG_SIMPLE_INT("verbose", &verbose),
	CFG_END()
	};
	cfg_t *cfg = cfg_init(options, 0); 
	cfg_parse(cfg, "CONFDIR/hanh.conf");
	
	/* Working with command-line argument. Here we use POSIX getopt() function. */
	while ((opt = getopt_long(argc, argv, "irqsfShvR:d:m:t:DI:b:B:T:nIg", long_opts, NULL)) != -1){
		switch (opt) {
			
			case 'h': 
			help();
			break;
			
			case 'v':
			printf("%s\n", __VER); 
			break;
			
			case 'i':
			action=1;
			break;
			
			case 'r': 
			action=2;
			break;
		
			case 'q': 
			action=3;
			break;
			
			case 's':
			action=4;
			break;
			
			case 'f': 
			action=5;
			break;
			
			case 'S': 
			action=6;
			break;
			
			case 'D': 
			nodeps=1;
			break;

			case 'n': 
			noinstall=1; 
			break;

			case 'g': 
			ignore=1;
			break;
						
			case 'R': 
			strcpy(sysroot, optarg);
			break;
			
			case 'd': 
			strcpy(download, optarg);
			break;
			
			case 'm': 
			strcpy(mirror, optarg);
			break;
			
			case 't':
			strcpy(type, optarg);
			break;
			
			case 'b': 
			strcpy(buildtype, optarg); 
			break;
			
			case 'B': 
			strcpy(builddir, optarg);
			break;

			case 'I': 
			strcpy(installtype, optarg); 
			break; 

			case 'w': 
			strcpy(source, optarg); 
			break;

			case 'T': 
			strcpy(installRoot, optarg);
			break;
			
			case '?':
			printf("Please use \"hanh -h\" for more information\n");
			return 1;
			}
	}


	for (; optind < argc; optind++){
		strcat(packages, argv[optind]);
		strcat(packages, " ");
		}
		
	// Set some default value for empty variables
	snprintf(localdbpath, __PATH, "%s/%s", sysroot, localdbdir);
	snprintf(remotedbpath, __PATH, "%s/%s", sysroot, remotedbdir);
	snprintf(pkgtarballspath, __PATH, "%s/%s", sysroot, pkgtarballs); 
	snprintf(stgtarballspath, __PATH, "%s/%s", sysroot, stgtarballs); 
	snprintf(stgdbpath, __PATH, "%s/%s", sysroot, stgdbdir);

	if (action == 1) {
		if (installtype[0] == '\0')
			strcpy(installtype, "packages"); 
		env_optarg = getenv("optarg");
		if (env_optarg == NULL)
			env_optarg = " ";

	}
	else if (action == 3) {
		if (type[0] == '\0')
			strcpy(type, "info,filelist");
	}
	else if (action == 6) {
		if (buildtype[0] == '\0') {
			strcpy(buildtype, "system");
		} 
		else {
			if ((strcmp(buildtype, "system")) != 0 && (strcmp(buildtype, "stage")) != 0) 
				die("Invalid stage mode", 1);
		}
	
		if (builddir[0] == '\0') {
			if ((strcmp(buildtype, "stage")) == 0) {
				die("Stage mode set but no build directory set", 1);
			}
			else if ((strcmp(buildtype, "system")) == 0){ 
				time_t t_var = time(NULL);
				struct tm* tm = localtime(&t_var); 
	
				snprintf(builddir, __PATH, "%s/usr/cache/%d-%d-%d/", sysroot, tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
				mkdirRecursive(builddir, 0755);
			}
		}

		if (installRoot[0] == '\0') {
			if ((strcmp(buildtype, "system")) == 0) {
				strcpy(installRoot, sysroot);
			}
			else if ((strcmp(buildtype, "stage")) == 0) {
				snprintf(installRoot, __PATH, "%s/sysroot", builddir);
			}
		}

		env_optarg = getenv("optarg");
		if (env_optarg == NULL)
			env_optarg = " ";
		exitcode = checkDir(builddir, "Build directory"); 
		checkCode(exitcode);
	}

	if (verbose != 0) {
		printf("[DEBUG] Variables: \n");
		printf("sysroot: %s\n", sysroot); 
		printf("download: %s\n", download); 
		printf("mirror: %s\n", mirror); 
		printf("repo: %s\n", repo);
		printf("verbose: %ld\n", verbose);
		printf("\n"); 
	}	

		
	/*Check for command line error*/	
	exitcode = checkEmpty(download, "Download command");
	checkCode(exitcode);
	exitcode = checkDir(sysroot, "Root");
	checkCode(exitcode);
	exitcode = checkDir(mirror, "Mirror directory");
	checkCode(exitcode);

	mkdirRecursive(localdbpath, 0755); 
	mkdirRecursive(remotedbpath, 0755); 
	mkdirRecursive(pkgtarballspath, 0755);
	mkdirRecursive(stgtarballspath, 0755); 
	mkdirRecursive(stgdbpath, 0755);

	switch(action) {
		
		case 0: 
		err("No action is specified! Please specify one");
		exitcode = 1;
		break;
	
		case 1:
		exitcode = INSTALL(packages, env_optarg, installtype, installRoot, sysroot, mirror, download, repo, nodeps, 0, ignore, verbose);
		checkCode(exitcode);
		break;

		
		case 2: 
		exitcode = REMOVE(packages, sysroot, "package", verbose, 1);
		checkCode(exitcode);
		break;
		
		case 3:
		exitcode = QUERY(packages, sysroot, type); 
		checkCode(exitcode);
		break;

		case 4:
		exitcode = SYNC(repo, sysroot, mirror, download, verbose);
		checkCode(exitcode);
		break;
	
		case 5:
		FIND(packages, sysroot, repo); 
		break;

		case 6:
		exitcode = SNAPSHOT(installRoot, builddir, env_optarg, buildtype, noinstall, nodeps, sysroot, ignore, verbose);
		checkCode(exitcode);	
		break;
		}		
	return exitcode; 
}
