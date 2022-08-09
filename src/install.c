#include "hanh.h"

int getDiff(const char *root, const char *tmp, const char *pkg, const char *pkginfodir) {
	char cmd[__CMD] = ""; 
	char old[__PATH] = "";
	char new[__PATH] = "";
	char infodir[__PATH] = ""; 

	// diffutils has no installed headers by default, and we use non-GNU Linux base tools
	// so run `diff` to filter old files
	snprintf(infodir, __PATH, "%s %s", pkg, pkginfodir); 
	snprintf(old, __PATH, "%s/%s/", root, pkginfodir);
	snprintf(new, __PATH, "%s/%s/", tmp, pkginfodir);
	int code = untar(tmp, infodir); 
	if (code == 0) {
		snprintf(cmd, __CMD, "diff \"%s\" \"%s\" | grep \"<\" | cut -d \"<\" -f 2 > %s/oldfiles", old, new, tmp);
		code = system(cmd);
		if (code != 0) err("Failed to filter old files"); 
	} else {
		err("Failed to filter old files"); 
	}
	return code;
}

int updateDepends(const char *root, const char *pkgname, char depends[]) {
	char *depBuf = NULL; 
	char *dep = strtok_r(depends, " ", &depBuf);
	int code = 0; 
	while (dep != NULL) {
		char sedcmd[__CMD]= ""; 
		snprintf(sedcmd, __CMD, "sed -i \'s/dependants=\"/dependants=\"%s /g\' %s/var/lib/pachanh/system/%s/info", pkgname, root, dep);
		code=system(sedcmd); 
		if (code != 0) {
			err("Failed to update dependencies information");
		}
	       dep = strtok_r(NULL, " ", &depBuf);	
	}
	return code; 
}

int checkConflicts(const char *root, const char *pkgname, char conflicts[], const long int verbose){
	char *conflbuf = NULL; 
	char *confl = strtok_r(conflicts, " ", &conflbuf);
	int code = 0; 
	while (confl != NULL) {
		char conflpath[__PATH] = ""; 
		char container[__PATH] = ""; 
		char Container[__PATH] = ""; 
		code = checkInstalled(root, confl); 
		if (code == 0) {
			snprintf(conflpath, __PATH, "%s/var/lib/pachanh/system/%s", root, confl);
			code = readlink(conflpath, container, __PATH); 
			if (code != -1) {
				if ((strcmp (container, pkgname)) != 0) {
					// Package conflicts with other package that they both provides the same part
					// Read the symbolic link to detect the package contains it
					printf("%s conflicts with %s (both contain %s). \
						\nDo you want to remove %s? [Y/N]\n", pkgname, container, confl, container);
					char ans[1]; 
					scanf("%s", ans); 
					if ((ans[0] == 'y') || (ans[0] == 'Y')) {
						strcpy(Container, container);
						REMOVE(Container, root, "packages", verbose, 1);
					}
					else if ((ans[0] == 'n') || (ans[0] == 'N')) {
						die("Cannot install (conflicting package)", 1);
						
					} else {
						die("Invalid answer! Aborting...", 1);
					}	
				}
			}
			else {
				if ((strcmp (confl, pkgname)) != 0) {
					// Package conflicts with other package that it contains the conflict one 
					// Just remove it
					printf("%s conflicts with %s. \
						\nDo you want to remove %s? [Y/N]\n", pkgname, confl, confl);
					char ans[1]; 
					scanf("%s", ans); 
					if ((ans[0] == 'y') || (ans[0] == 'Y')) {
						REMOVE(confl, root, "packages", verbose, 1); 
					}
					else if ((ans[0] == 'n') || (ans[0] == 'N')) {
						die("Cannot install (conflicting package)", 1);
					} else {
						die("Invalid answer! Aborting...", 1); 	
					}
				}
			}
		}
	confl = strtok_r(NULL, " ", &conflbuf);
	}
	return 0; 
}

void checkConfig(const char *root, char config[]) {
	char *confBuf = NULL; 
	char *cfgfile = strtok_r(config, " ", &confBuf);
	int code = 0;
	while (cfgfile != NULL) {
		// rename() will fail to execute if package file is installed in a 
		// separate partition, better use system `mv`
		char mvcmd[__CMD] = ""; 
		snprintf(mvcmd, __CMD, "mv %s/%s.newfile %s/%s", root, cfgfile, root, cfgfile);
		code = system(mvcmd);
		if (code != 0) printf("WARNING: failed to move %s\n", cfgfile);	

		cfgfile = strtok_r(NULL, " ", &confBuf);
	} 
} 

void removeOld(const char *root, const char *tmp) {
	char filelistpath[__PATH] = ""; 
	snprintf(filelistpath, __PATH, "%s/oldfiles", tmp);

	FILE *filelist = fopen(filelistpath, "r");
	int size = ftell(filelist); 
	char filec[size];
	fread(filec, size, 1, filelist);
	filec[size] = 0; 

	char *filebuf = NULL;
	char *file = strtok_r(filec, "\n", &filebuf); 
	while (file != NULL) {
		char fullpath[__PATH]	= ""; 
		int code = 0;
		snprintf(fullpath, __PATH, "%s/%s", root, file); 
		code = remove(fullpath); 
		if (code != 0) printf("WARNING: failed to remove %s\n", fullpath); 
	
		file = strtok_r(NULL, "\n", &filebuf);
	}
	fclose(filelist); 
}


int INSTALL(char packages[], const char *root, const int nodepends, const long int verbose) {
	char *pkgBuf = NULL; 
	char *pkg  = strtok_r(packages, " ", &pkgBuf); 
	int  code    = 0; 
	
	while(pkg != NULL) {
		char tmpdir[20]      = "/tmp/tmp.XXXXXX" ; 
		char *tmp            = mkdtemp(tmpdir)   ;
		char header[__PATH]  = ""                ;
		char tarHead[__ARG]  = ""                ; 
		char hook[__ARG]     = ""                ;
		char prehook[__PATH] = ""                ;
		char afthook[__PATH] = ""                ;
		
		// Parse config for name, depends, contain (package provide part), config (to backup), pkg_infordir 
		char *name        = NULL                 ;
		char *version     = NULL                 ;
		char *desc        = NULL                 ;
		char *depends     = NULL                 ; 
		char *contain     = NULL                 ; 
		char *config      = NULL                 ; 
		char *pkg_infodir = NULL                 ;
		
		snprintf(header , __PATH, "%s/pre-install"                        , tmp           );
		snprintf(tarHead, __ARG , "%s pre-install"                        , pkg         );
		snprintf(hook   , __ARG , "%s hook"                               , pkg         );
		snprintf(prehook, __PATH, "%s/hook pre_install"                   , tmp           );
		snprintf(afthook, __PATH, "%s/hook post_install"                  , tmp           ); 
		
		printf("Unpacking %s\n", pkg); 
		if (verbose != 0) printf("[DEBUG] Unpacking header\n");
		mkdir(tmp, 0755);
		code = untar(tmp, tarHead);
		checkCode(code);
		if (verbose != 0) printf("[DEBUG] Unpacking hook\n");
		// hookcode will detect if a hook is provided to install 
		// hook will be also installed to data folder for further use (remove package)
		int hookcode = untar(tmp, hook); 
		
		cfg_opt_t opts[] = {
			CFG_SIMPLE_STR("name", &name),
			CFG_SIMPLE_STR("version", &version), 
			CFG_SIMPLE_STR("desc", &desc),
			CFG_SIMPLE_STR("depends", &depends), 
			CFG_SIMPLE_STR("contain", &contain), 
			CFG_SIMPLE_STR("config", &config), 
			CFG_SIMPLE_STR("pkg_infodir", &pkg_infodir), 
			CFG_END()
			};
		cfg_t *cfg       = cfg_init(opts, 0);
		cfg_parse(cfg, header);
		int installCode = checkInstalled(root, name);
		
		// Use package name instead of tarball name
		if (nodepends == 0) {
			if (verbose != 0) debug("Checking depends");	
			code = checkDeps(root, depends); 
			checkCode(code);
			}
		if (verbose != 0) debug("Checking conflicting packages");
		code = checkConflicts(root, name, contain, verbose); 
		checkCode(code);
		if (installCode == 0) {
			if (verbose != 0) debug("Filtering old files");
			code = getDiff(root, tmp, pkg, pkg_infodir);
			checkCode(code);
		}
		if (hookcode == 0) {
			if (verbose != 0) debug("Triggering pre-install hook"); 	
			code = system(prehook); 
			checkCode(code);
		}

		printf("Installing %s\n", name);
		code = untar(root, pkg);
		checkCode(code);	

		printf("Finishing %s\n", name);
		if (nodepends == 0 && installCode != 0) { 
			if (verbose != 0) debug("Updating dependencies information"); 
			code = updateDepends(root, name, depends);
			checkCode(code);
		}
		if (verbose != 0) debug("Working with config");
		checkConfig(root, config);
		if (hookcode == 0) {
			if (verbose != 0) debug("Triggering after install hook");
			code = system(afthook);
			checkCode(code); 
		}
		if (installCode == 0) {
			if (verbose != 0) debug("Removing old files");
			removeOld(root, tmp);
		}
		if (verbose != 0) debug("Cleaning up");
		code = clearTmp(tmp); 
		checkCode(code);
		if (installCode != 0) {
			if(verbose != 0) debug("New package installed! Updating package list");
			code = updatePkglist(root, name);
			checkCode(code); 
		}		
		  		
		pkg = strtok_r(NULL, " ", &pkgBuf); 
	}
	printf("\n\n");
	return code;
}
