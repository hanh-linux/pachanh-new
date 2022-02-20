#include "hanh.h"

int main(int argc, char **argv)
{	
	/*Actions: 
	 * -i 		install 			= 1
	 * -r 		remove 		= 2
	 * -s 		sync 				= 3 
	 * -S 		Snapshot 	= 4  
	 * -f 		find				=	5
	 * -c		check			= 6
	 * Multiple actions will result in an override*/
	int action 	= 0;
	int nodeps	= 0; // enable checking dependencies by default
	
	// General variables will be used from command-line 
	char packages[__PATHCHARS]		=	"";
	char ROOT[__PATHCHARS]			=	"";
	char download[__PATHCHARS] 	=	"";
	char mirror[__PATHCHARS]			=	"";
	char type[__PATHCHARS]				=	"";
	char PREFIX[__PATHCHARS]			=	"/usr/local";
		
	// Misc
	int exitcode	=	0;
	int opt;
	
	/* Working with command-line argument. Here we use POSIX getopt() function. */
	while ((opt = getopt(argc, argv, "irsSfchvR:d:m:t:DP:")) != -1){
		switch (opt) {
			
			case 'h': 
			help();
			break;
			
			case 'v':
			printver();
			break;
			
			case 'i':
			action=1;
			break;
			
			case 'r': 
			action=2;
			break;
		
			case 's': 
			action=3;
			break;
			
			case 'S':
			action=4;
			break;
			
			case 'f': 
			action=5;
			break;
			
			case 'c': 
			action=6;
			break;
			
			case 'D': 
			nodeps=1;
			break;
						
			case 'R': 
			strcpy(ROOT, optarg);
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
			
			case 'P':
			strcpy(PREFIX, optarg);
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
		
	/*Check for command line error*/	
	exitcode= check_empty(download, "Download command");
	check_code(exitcode);
	exitcode=check_path("Root", ROOT, 1, 1);
	check_code(exitcode);
	exitcode=check_path("Mirror directory", mirror, 1, 1);
	check_code(exitcode);
	
	switch(action) {
		
		case 0: 
		die("No action is specified! Please specify one", 1);
		break;
		
		case 1:
		exitcode = INSTALL(packages, ROOT, PREFIX, nodeps);
		check_code(exitcode);
		break;
		
		case 2: 
		exitcode = REMOVE(packages, ROOT);
		check_code(exitcode);
		break;
		
		case 3:
		exitcode=SYNC(download, ROOT, mirror);
		check_code(exitcode);
		break;
		
		case 4:
		general_die();
		break;
		
		case 5:
		exitcode = FIND(packages, ROOT, type);
		check_code(exitcode); 
		break;
		
		case 6:
		exitcode = CHECK(packages, ROOT, mirror);
		check_code(exitcode);	
		break;
		
		}		
}

