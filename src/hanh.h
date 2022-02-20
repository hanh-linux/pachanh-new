#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#define __PATHCHARS 2000
#define __HANHVER "2.0"

void help() {
	printf("hanh - Hanh Linux package manager\n");
	printf("Usage: hanh [Action] [Options]\n");	
	printf("Action\n"); 
	printf("-s                        Sync database from remote \n");
	printf("-i                        Install packages <file>\n");
	printf("-r                        Remove packages\n");
	printf("-f                        Search for installed packages\n");
	printf("-c                        Check if a package is available\n"); 
	printf("-S                        Build a snapshot\n");
	printf("Options \n");
	printf("-h                        Print this message\n");
	printf("-v                        Print version\n");
	printf("-R <path>                 Change root directory\n");
	printf("-P <path>                 Change prefix\n");
	printf("-d <command>              Use another download command\n");
	printf("-m <path>                 Use specified mirror directory\n");
	printf("-t [info,filelist,header] Specify file(s) to search\n");
	printf("-D                        Disable dependencies check\n");
	printf("For any questions, please consider asking on GitHub: https://github.com/hanh-linux/pachanh\n");
	exit(0);
	}
	
void printver() {
	printf("%s\n", __HANHVER);
	exit(0);
	}

/*ERROR message*/
void err(char x[]) {
	/* x: error message */
	printf("ERROR: %s\n", x);
	}

/*ERROR message and exit with non-zero code*/
void die (char x[], int y) {
	/* x: error message
	 * y: exit code*/
	err(x);
	exit(y);
	}

/*Check exitcode*/
void check_code(int a) {
	/* a: exit code (if not 0)*/
	if (a != 0) {
		die("Exit program due to previous error", 1);
		}
	}
	
/*Check if a string is empty*/	
int check_empty(char a[], char b[]) {
	/* a: object to check empty
	 * b: object name for error message*/
	char bruh[__PATHCHARS];
	
	strcpy(bruh, b);
	if (a[0] == '\0') {
		strcat(bruh, " is empty!\n");
		printf("%s", bruh); 
		
		return 1;
		}

	return 0;
	}
	
/*Function for checking path*/
int check_path(char a[], char b[], int c, int d) {
	/* a: Object name (not variable)
	 * b: Object to check
	 * c: Non-zero exit code
	 * d: Check mode (0 for file, 1 for directory)*/
	char bruh[__PATHCHARS], errMsg[__PATHCHARS];
	int if_found, code;
	struct stat buf;
	
	strcpy(bruh, a);
	// Check if it is empty
	code = check_empty(b, a);
	check_code(code);
	
	// Check if the file exists	
	if_found=stat(b, &buf);
	if (if_found != 0){
		strcat(bruh, " not found!\n");
		err(bruh);
		
		return c; 
		}
	
	// Return the value depends on used mode	
	// Mode: File
	if (d == 0) {
		return 0;
	} 
	// Mode: directory
	else if (d == 1) {
		int code = S_ISREG(buf.st_mode);
		
		if (code != 0) {
		strcat(errMsg, a);
		strcat(errMsg, "is not a directory!");
		err(errMsg);	
			}
		
		return code;
		}
	// Mode: invalid
	else {
		err("Requested mode is invalid\n");
		return c;
		}
	}

int silent_check_path(char b[], int c, int d) {
	/* b: Object to check
	 * c: Non-zero exit code
	 * d: Check mode (0 for file, 1 for directory)*/
	int if_found, code;
	struct stat buf;
	
	// Check if the file exists	
	if_found=stat(b, &buf);
	if (if_found != 0) {
		return if_found;
		}
	// Return the value depends on used mode	
	// Mode: File
	if (d == 0) {
		return 0;
	} 
	// Mode: directory
	else if (d == 1) {
		code = S_ISREG(buf.st_mode);
		
		return code;
		}
	// Mode: invalid
	else {
		return c;
		}
	}

/*Function for get size of a file*/	
int getSize(char a[]) {
	/* a: File path*/
	FILE *file; 
	int filechars;
	
	file = fopen(a, "r");
	fseek(file, 0, SEEK_END);
	filechars = ftell(file) + 1; 
	
	return filechars;
	}
	
/*Function for unpacking packages*/
int untar(char a[], char b[]) {
	/* a: Where to unpack? 
	 * b: File to unpack*/
	char command[__PATHCHARS] = "";
	int c = 0;
	
	snprintf(command, __PATHCHARS, "tar -C %s -xf %s", a, b);
	c = system(command);
	
	return c;
	}
	
int fetch(char a[], char b[], char c[]) {
	/* a: Download command
	 * b: Mirror
	 * c: Repo */
	 char command[__PATHCHARS] = "";
	 int code = 0;
	 
	 snprintf(command, __PATHCHARS, "%s %s/%s.database", a, b, c);
	 printf("Fetching %s.database\n", c);
	 code = system(command);
	 
	 return code;
	}

/*Function for installing package(s)*/			
int INSTALL(char a[], char b[], char d[], int c) {
	/* a: Package tarballs
	 * b: Root directory
	 * d: Prefix
	 * c: Depends check or not?*/
	char *token = strtok(a, " ");
	
	while ( token != NULL) {
		// Prepare to unpack
		int code = 0; 
		char tmpdir[20] 		= "/tmp/tmp.XXXXXX"	;
		char *tmp			= mkdtemp(tmpdir)	;
		char header[__PATHCHARS]	= ""			;
		char depsh[__PATHCHARS]		= ""			;
		char conflsh[__PATHCHARS] 	= ""			;
		char diffsh[__PATHCHARS]	= ""			;
		char rmsh[__PATHCHARS] 		= ""			;
		char confsh[__PATHCHARS]	= ""			;
		char root_header[__PATHCHARS]	= ""			;
		
		snprintf(root_header, __PATHCHARS, "%s/pre-install", b);
		snprintf(header	, __PATHCHARS, "%s pre-install",token);
		snprintf(depsh	, __PATHCHARS, "%s/%s/share/pachanh/scripts/check-deps.sh \"%s\" %s	", b, d, b, tmp);
		snprintf(rmsh   , __PATHCHARS, "%s/%s/share/pachanh/scripts/rm-old.sh \"%s\" %s		", b, d, b,tmp);
		snprintf(conflsh, __PATHCHARS, "%s/%s/share/pachanh/scripts/check-conflict.sh \"%s\" %s	", b, d, b,tmp);
		snprintf(confsh , __PATHCHARS, "%s/%s/share/pachanh/scripts/mv-conf.sh \"%s\" %s	", b, d, b,tmp);
		snprintf(diffsh , __PATHCHARS, "%s/%s/share/pachanh/scripts/get-old.sh \"%s\" %s %s	", b, d, b, tmp, token);

		// Create temporary directory so we can get the header
		printf("Unpacking %s\n", token);
		mkdir(tmp, 0755);
		code = untar(tmp, header);
		check_code(code);
		if (c == 0) {
			// Run a script to check dependencies so that we don't need
			// libconfig to be installed (or write a parser as well :P)
			code = system(depsh);
			check_code(code);
			}
		code = system(conflsh);
		check_code(code);
		// Also for filtering old files
		printf("Filtering old files\n");
		code = system(diffsh);
		check_code(code);
		// Installing the package
		printf("Installing %s\n", token);
		code = untar(b, token);
		check_code(code);
		// Check for configuration file
		printf("Checking for configuration files\n");
		code = system(confsh);
		check_code(code);
		// Remove the old files and /tmp/tmp.XXXXXX directory
		printf("Cleaning up...\n");
		code = system(rmsh);
		check_code(code);
		rmdir(tmp);
		remove(root_header);
		
		token = strtok(NULL, " ");
		}
		
	return 0;
	}  
	
/*Function for synchonizing(?) database */
int SYNC(char a[], char b[], char c[]) {
	/* a: Download command
	 * b: Root directory
	 * c: Mirror directory*/
	char path[__PATHCHARS] = "", dbdir[__PATHCHARS]= "", dirc[100]="";
	char *bufA = NULL, *bufB = NULL; 
	int code = 0;
	DIR *list;
	struct dirent *pDirent;
		
	snprintf(dbdir,__PATHCHARS,"%s/var/lib/pachanh/remote/",b);
	strcpy(path, c); 

	// Change directory to database directory and remove all existing files
	printf("Cleaning old database\n");
	chdir(dbdir); 
	code = system("rm -rf *");
	check_code(code);

	// List all files (repo)
	list = opendir(path);
	while ((pDirent = readdir(list)) != NULL) {
		if (pDirent -> d_name[0] != '.') {
			strcat(dirc, pDirent -> d_name);
			strcat(dirc, "\n");
			}
		} 
		
	char *token = strtok_r(dirc, "\n", &bufA); 
	
	while (token != NULL) {
		char fullpath[__PATHCHARS] = "", charc[5] = "", tarfile[__PATHCHARS] = "";
		struct stat buf;
		int size, d, if_found;
		FILE *text;	
		int i = 0;
		
		snprintf(tarfile, __PATHCHARS, "%s.database", token);
		snprintf(fullpath, __PATHCHARS, "%s/%s", path, token);
			
		size = getSize(fullpath) + 1;
		char filec[size]; 
		if_found = stat(fullpath, &buf);
		check_code(if_found);
		text = fopen(fullpath, "r") ;
			
		while (text != NULL) {
			d = getc(text);
			if ( feof(text) ) {
				break;
				}
			snprintf(charc, 5, "%c", d);
			if (i == 0) {
				strcpy(filec, charc);
				}
			else {
				strcat(filec, charc);
				}
			i++;
			}
				
			char *filetok = strtok_r(filec, "\n", &bufB);
			
			while (filetok != NULL) {
				int code = 0;
				int found = 0;
				struct stat buff; 
				
				fetch(a, filetok, token);
				found = stat(tarfile, &buff); 
				if (found == 0) {
					printf("Unpacking database (%s)\n", tarfile);
					code = mkdir(token, 0755);
					check_code(code);
					code = untar(token, tarfile);
					check_code(code);
					break;
				} 
				 else {
					 char errMsg[__PATHCHARS]= "";
					 
					 snprintf(errMsg, __PATHCHARS, "Failed to fetch %s", tarfile); 
					 err(errMsg);
					 }
				
				filetok = strtok_r(NULL, "\n", &bufB);
				}

			fclose(text);
			token = strtok_r(NULL, "\n", &bufA);
		}

	return 0;
	}
	
int FIND (char a[], char b[], char c[]) {
	/* a: Packages
	 * b: Root directory
	 * c: File type (info, filelist, header)*/
	char *token, *filename, *bufA = NULL, *bufB = NULL;
	FILE *file;
	int d, code;
	
	token = strtok_r(a, " ", &bufA);
		if (c[0] == '\0') {
			strcpy(c, "info,filelist,header");
		}
		
	while (token != NULL) {
		char fullpath[__PATHCHARS] = "", filetype[25]="";

		snprintf(fullpath, __PATHCHARS, "%s/var/lib/pachanh/system/%s/", b, token);
        	strcpy(filetype, c);
        	filename = strtok_r(filetype, ",", &bufB);
		code = check_path(token, fullpath, 1, 1);
		check_code(code);

		while (filename != NULL) {
			char path[__PATHCHARS]="";
			
			snprintf(path, __PATHCHARS, "%s/%s", fullpath, filename);
			code = check_path(filename , path, 1, 0);
			check_code(code);
			file = fopen(path, "r");
			printf("Printing %s/%s:\n", token,filename);
			while (1) {
				d = fgetc(file); 
				
				if ( feof(file) ) {
					break;
					}

				printf("%c", d);
				}

			fclose(file);
			filename = strtok_r(NULL, ",", &bufB);
			}
			
			token=strtok_r(NULL, " ", &bufA);
		}
		return 0;
	}

int REMOVE(char a[], char b[]) {
	/* a: Packages
	 * b: Root directory */
	 char *token, *filetok, *bufA = NULL, *bufB = NULL; 
	 char pkg[__PATHCHARS] = "", datadir[__PATHCHARS] = "", pkgfl[__PATHCHARS] = "", d[5]="", pkgDir[__PATHCHARS] = "", errMsg[__PATHCHARS]= "";
	 int code = 0, c; 
	 FILE *file;
	 
	 snprintf(datadir, __PATHCHARS, "%s/var/lib/pachanh/system/", b);
	 strcpy(pkg, a);
	 token = strtok_r(pkg, " ", &bufA); 
	 while (token != NULL) {
		 int i = 0, size=0;
		 // Let's check if the package is installed or not. 
		 snprintf(pkgDir, __PATHCHARS, "%s/%s/", datadir,token);
		 snprintf(pkgfl, __PATHCHARS, "%s/%s/filelist", datadir, token);
		 printf("Check if package %s is installed\n", token);
		 code = silent_check_path(pkgDir, 1, 0); 
		 if (code != 0) {
			 snprintf(errMsg,__PATHCHARS, "%s is not installed", token); 
			 die(errMsg, 1);  
			 }
		 // We will check if the filelist is available and read it
		 code = check_path(pkgfl, pkgfl, 1, 0);
		 check_code(code); 
		 size = getSize(pkgfl) + 1; 
		 char flc[size]; 
		 file = fopen(pkgfl, "r"); 
		 while (1) {
			 c = fgetc(file); 
			 
			 if (feof(file)) {
				 break;
				 }
				 
			 snprintf(d, 5, "%c", c); 
			 if (i == 0) {
				 strcpy(flc, d);
				 }
			 else {
				 strcat(flc, d);
				 }
			 i++; 
			 }			
		 fclose(file);
		 	 
		 // Remove all available files, we won't delete directory 
		 // since it may delete standard folders that may destroy
		 // the system
		 filetok = strtok_r(flc, "\n", &bufB); 
		 printf("Removing %s...\n", token);
		 while (filetok != NULL) {
			 char filepath[__PATHCHARS] = "";
			 
			 snprintf(filepath, __PATHCHARS, "%s/%s", b, filetok);
			 int if_dir = silent_check_path(filepath, 1, 1); 
			 if (if_dir != 0) {
				 remove(filepath);
				 }
			 
			 filetok = strtok_r(NULL, "\n", &bufB);
			 }
		 
		 remove(pkgfl);
		 rmdir(pkgDir);
		 token = strtok_r(NULL, "\n", &bufA); 
		 }
		 return 0; 
	}

int CHECK(char a[], char b[], char c[]) {
	/* a: Packages 
	 * b: Root directory
	 * c: Mirror directory*/
	char *buf=NULL, *bufA=NULL, *token, *reponame;
       	char root[__PATHCHARS]="", path[__PATHCHARS]= "", all_repo[__PATHCHARS]= "";
	struct dirent *pDirent; 
	DIR *dir; 
	
	strcpy(root, b);	
	strcpy(path, c);
	
	// List all repo 
	dir = opendir(path);
	while ((pDirent = readdir(dir)) != NULL ) {
		if (pDirent -> d_name[0] != '.') {
			strcat(all_repo, pDirent -> d_name);
			strcat(all_repo, "\n"); 
		}
	}
	
	token = strtok_r(a, " ", &buf);
	
	while (token != NULL) { 
	char allRepo[__PATHCHARS] = "";

	strcpy(allRepo, all_repo);		 
	reponame = strtok_r(allRepo, "\n ", &bufA);
		while (reponame != NULL) {
			char fullpath[__PATHCHARS] = "";
			int if_found, d; 
			FILE *file; 
			
			// Check if the data file is available. If found, print it
			// Otherwise check the next repo
			snprintf(fullpath, __PATHCHARS, "%s/var/lib/pachanh/remote/%s/%s/data", root, reponame, token); 
			if_found = silent_check_path(fullpath, 1, 0);
			if (if_found == 0) {
			printf("Printing database of %s/%s...\n", reponame, token);
			file = fopen(fullpath, "r"); 
			while (1) {
				d = fgetc(file); 
				if (feof(file)) {
					break;
					}
					printf("%c", d);
				}
			fclose(file);
			}

			reponame=strtok_r(NULL, "\n", &bufA);
			}

		token = strtok_r(NULL, " ", &buf);
		}
	return 0;
	}
	
void general_die() {
	die("Feature is not ready to be used", 1);
	}
	
	
