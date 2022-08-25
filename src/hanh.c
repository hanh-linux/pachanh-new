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
	int opt            ;

	// General variables will be used from command-line 
	char packages[__ARG]       = ""  ;
	char type[__ARG]           = ""  ;
	static char *sysroot       = NULL;
	static char *download      = NULL;
	static char *mirror        = NULL;
	static char *repo          = NULL; 
	static long int verbose    = 0   ;
	
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
	while ((opt = getopt(argc, argv, "irqsfShvR:d:m:t:D")) != -1){
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
			
			case '?':
			printf("Please use \"hanh -h\" for more information\n");
			return 1;
			}
	}


	for (; optind < argc; optind++){
		strcat(packages, argv[optind]);
		strcat(packages, " ");
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
	exitcode = checkPath(sysroot, "Root");
	checkCode(exitcode);
	exitcode = checkPath(mirror, "Mirror directory");
	checkCode(exitcode); 
	switch(action) {
		
		case 0: 
		err("No action is specified! Please specify one");
		exitcode = 1;
		break;
	
		case 1:
		exitcode = INSTALL(packages, sysroot, nodeps, verbose);
		checkCode(exitcode);
		break;

		
		case 2: 
		exitcode = REMOVE(packages, sysroot, "packages", verbose, 1);
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
		exitcode = SNAPSHOT(sysroot);
		checkCode(exitcode);	
		break;
		}		
	return exitcode; 
}

