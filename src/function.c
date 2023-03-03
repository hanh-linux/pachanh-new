#include "hanh.h"

void err(const char *msg) {
	printf("ERROR: %s\n", msg); 
}

void die(const char *msg, const int code) {
	printf("ERROR: %s\n", msg); 
	exit(code);  
	}


void checkCode(const int code) {
	if (code != 0) {
		die("Exit due to previous error", code);
		}
	}
	
void debug(const char *msg) {
	printf("[DEBUG] %s\n", msg); 
}


int checkEmpty(const char *obj, const char *msg) {
	if (obj[0] == '\0') {
		printf("%s is empty \n", msg);
		return 1;
	} 
	else {
		return 0; 
	}
}
	
int checkPath(const char *obj, const char *msg) {
	struct stat buf;
	int code = stat(obj, &buf);
	if (code != 0) {
		if ((strcmp(msg, "silent")) != 0) {
			printf("%s not found\n", msg);  
			}
		}
	return code; 
	}

int checkFile(const char *obj, const char *msg) {
	struct stat buf; 
	stat(obj, &buf);
	int code = S_ISREG(buf.st_mode);
	if (code == 0)
		code = 1; 
	else 
		code = 0; 

	if (code != 0) {
		if ((strcmp(msg, "silent")) != 0) {
			printf("%s not found or not a file\n", msg);
		}
	}
	return code;
}

// We don't believe in returned value of stat function (for directory)
// Using opendir() to do it (<dirent.h> is not available on Windows)
int checkDir(const char *obj, const char *msg) {
	DIR *dir = opendir(obj);
	int code = 0; 
	if (dir) {
		code = 0; 	
	}
	else if (ENOENT == errno) {
		if ((strcmp(msg, "silent")) != 0) {
			printf("%s not found\n", msg);
		}
		code = errno;
	}
	else if (ENOTDIR == errno) {
		if ((strcmp(msg, "silent")) != 0) {
			printf("%s is not a directory", msg);
		} 
		code = errno;
	}
	else {
		if ((strcmp(msg, "silent")) != 0) {
			printf("Failed to open %s: %s", obj, strerror(errno));
		}
		code = errno;
	} 
	return code; 
}

int checkDeps(const char *root, char depends[]) {
	char *depbuf = NULL;
	char *dep = strtok_r(depends, " ", &depbuf);
	int code = 0;
	while (dep != NULL) {
		// Just check if the data folder is there
		char deppath[__PATH] = "";
		snprintf(deppath, __PATH, "%s/var/lib/pachanh/system/%s", root, dep);
		code = checkDir(deppath, dep);
		if (code != 0) {
			printf("ERROR: %s not installed\n", dep);
			break;
		}	

		dep = strtok_r(NULL, " ", &depbuf);
	}
	return code;
}

int checkInstalled(const char *root, const char *pkg) {
	int code = 0;
	// Just check if the data folder is there
	char pkgpath[__PATH] = "";
	snprintf(pkgpath, __PATH, "%s/var/lib/pachanh/system/%s", root, pkg);
	code = checkDir(pkgpath, "silent");
	return code;
}


int untar(const char *untarPath, const char *file) {
	// `file` doesn't contain only tarball, it can use to 
	// extract a file from tarball
	char cmd[__CMD] = ""; 
	// No GNU tar is installed by default, use `tar` command 
	// to unpack the package
	snprintf(cmd, __CMD, "tar -C %s -xf %s", untarPath, file);
	int code = system(cmd);
	if (code != 0) err("Failed to unpack needed file"); 	
	return code; 
}

int keepOldUntar(const char *untarPath, const char *file) {
	// Like untar(), but keep old file
	char cmd[__CMD] = ""; 
	snprintf(cmd, __CMD, "tar -k -C %s -xf %s", untarPath, file);
	int code = system(cmd);
	if (code != 0) err("Failed to unpack needed file or file existed");
	return code; 
}

int createTar(const char *dir, const char *name) {
	char cwd[__PATH] = ""; 
	char cmd[__CMD] = "";
	int code = 0;
	getcwd(cwd, __PATH); 
	snprintf(cmd, __CMD, "tar -cjf %s/%s.tar.xz *", cwd, name);

	chdir(dir);
	code = system(cmd);
	chdir(cwd);

	return code; 
}

int updatePkglist(const char *root, const char *pkg) {
	// Update package install order or your system will be fcked up 
	char orderpath[__PATH] = "";
	int code = 0;	
	snprintf(orderpath, __PATH, "%s/var/lib/pachanh/system/pkgorder", root);

	FILE *orderfile = fopen(orderpath , "a+");  
	code = fprintf(orderfile, "%s;\n", pkg);
	fclose(orderfile);
	if (code > 0) { code = 0; } else { code = 1; } 
	fclose(orderfile); 
	return code;
}

int clearTmp(const char *tmpdir) {
	char cleanCmd[__CMD] = ""; 
	snprintf(cleanCmd, __CMD, "rm -rf %s && rm -rf /tmp/oldfiles", tmpdir);
	int code = system(cleanCmd);

	return code;
}

int getSize(FILE *file) {
	fseek(file, 0, SEEK_END);
	int size = ftell(file); 
	fseek(file, 0, SEEK_SET);
	
	return size;
}

int checkDirEmpty(const char *dirpath) {
	char name[__PATH] = "";
	int code = 0;
	struct dirent *pDirent;
	DIR *dir = opendir(dirpath);
	if (dir) {
		while ((pDirent = readdir(dir)) != NULL) {
			strcpy(name, pDirent->d_name);
			if  ((strcmp(name, ".")) != 0 && (strcmp(name, "..")) != 0 ) {
				code = 1;
				break;
			}
		}
	}
	else {
		code = 1;
	}
	return code;
}

int del(const char *path, const int exitIfFail) {
	int code = checkDir(path, "silent");
	if (code == 0) {
		int empty = checkDirEmpty(path); 
		if (empty == 0) {
			code = remove(path);
			if (code != 0) {
				if (exitIfFail != 0) {
					printf("ERROR: Failed to remove %s: %s", path, strerror(errno));
					return errno;
				}
			}
		}
	} 
	else if (ENOTDIR == code) {
		code = remove(path);
		if (code != 0) {
			if (exitIfFail != 0) {
				printf("ERROR: Failed to remove %s: %s", path, strerror(errno));
				return errno;
			}
		}
	}
	else if (ENOENT == code) {}
	else {
		printf("ERROR: Failed to access %s: %s", path, strerror(errno));
		return errno;	
	}
	return 0; 
}

char* getName() {
	static char name[__ARG];
	FILE *inp = popen("hanhbuild -gi", "r");
	fgets(name, __ARG, inp); 
	return name;
}
