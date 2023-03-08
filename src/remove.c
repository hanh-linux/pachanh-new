#include "hanh.h"

// TODO: Support removing dependencies
// Users have to manually delete package(s) old configuration file(s), as these files are for backup
int REMOVE(char packages[], const char *root, const char *mode, const long int verbose, const int ignore) {
	int exitFail;
	if (ignore != 0) 
		exitFail = 0; 
	else 
		exitFail = 1;

	char *buf = NULL; 
	char *pkg = strtok_r(packages, " ", &buf); 
	int code  = 0;
	int rmCode = 0;

	while (pkg != NULL) {
		char movehook[__CMD]  = ""; 
		char prehook[__CMD]   = "";
		char posthook[__CMD]  = ""; 
		char info[__PATH]     = ""; 
		char flpath[__PATH]   = "";
		char sedOrder[__CMD]  = "";
		char link[__PATH]     = "";
		char pkgpath[__PATH]  = "";
		char hookpath[__PATH] = "";

		char tmpdir[__PATH]   = "/tmp/tmp.XXXXXX"; 
		char *tmp             = mkdtemp(tmpdir)  ;	
		
		char *name       = NULL;
		char *ver        = NULL; 
		char *desc       = NULL; 
		char *home       = NULL; 
		char *license    = NULL;
		char *depends    = NULL; 
		char *contain    = NULL; 
		char *dependants = NULL;

		cfg_opt_t opts[] = {
		CFG_SIMPLE_STR("name", &name), 
		CFG_SIMPLE_STR("version" , &ver),
		CFG_SIMPLE_STR("desc", &desc), 
		CFG_SIMPLE_STR("home", &home), 
		CFG_SIMPLE_STR("license", &license), 
		CFG_SIMPLE_STR("depends", &depends), 
		CFG_SIMPLE_STR("contain", &contain),
		CFG_SIMPLE_STR("dependants", &dependants),
		CFG_END()
		};
		cfg_t *cfg = cfg_init(opts, 0); 

		snprintf(prehook , __CMD , "%s/hook pre_remove", tmp); 
		snprintf(posthook, __CMD , "%s/hook post_remove", tmp);
		snprintf(info    , __PATH, "%s/var/lib/pachanh/system/%s/info", root, pkg);
		
		if (verbose != 0) debug("Checking if package is installed");
		int code     = checkInstalled(pkg, root); 
		int hookcode = 0; 
		cfg_parse(cfg, info);

		if (code == 0) {
			if ((strcmp(pkg, name)) != 0) { printf("%s contains %s, removing %s\n", pkg, name, name); }
			printf("Removing %s\n", name);
			
			snprintf(sedOrder, __CMD , "sed -i 's/%s;//g' %s/var/lib/pachanh/system/pkgorder && sed -i '/^$/d' %s/var/lib/pachanh/system/pkgorder", name, root, root);
			snprintf(movehook, __CMD , "cp -r %s/var/lib/pachanh/system/%s/hook %s/", root, name, tmp);
			snprintf(hookpath, __PATH, "%s/var/lib/pachanh/system/%s/hook", root, name); 
			snprintf(flpath  , __PATH, "%s/var/lib/pachanh/system/%s/filelist", root, name);
			snprintf(pkgpath , __PATH, "%s/var/lib/pachanh/system/%s", root, name);

			FILE *filelist = fopen(flpath, "r");
			fseek(filelist, 0, SEEK_END);
			long int size = ftell(filelist);
			fseek(filelist, 0, SEEK_SET);

			char fileinpkg[size]; 
			char dirinpkg[size];
			fread(fileinpkg, size, 1, filelist);
			fileinpkg[size] = 0;
			int empty = 0;

			hookcode = system(movehook);
			if (hookcode == 0) {
				if (verbose != 0) debug("Running pre-remove hook");
				code = system(prehook); 
				checkCode(code); 
			}

			char *fileBuf = NULL; 
			char *file = strtok_r(fileinpkg, "\n", &fileBuf); 
			while (file != NULL) {
				char fullfilepath[__PATH] = ""; 
				snprintf(fullfilepath, __PATH, "%s/%s", root, file);
				int codeDir = checkDir(fullfilepath, "silent"); 	

				if (codeDir == 0) {
					if (empty != 0) { strcat(dirinpkg, fullfilepath); } else { strcpy(dirinpkg, fullfilepath); }
					strcat(dirinpkg, "\n"); 
					empty = 1; 
				} 
				else {
					rmCode = del(fullfilepath, exitFail);
					// should we treat this as error? 
					if (rmCode != 0) {
						if (errno != ENOENT) {
							if (exitFail == 0) {
								printf("WARNING: Failed to remove %s: %s\n", fullfilepath, strerror(errno));
							}
							else {
								printf("ERROR: Failed to remove %s: %s\n", fullfilepath, strerror(errno));
								exit(errno); 
							}
							
						}
					}	
				}

			file = strtok_r(NULL, "\n", &fileBuf);
			}

			dirinpkg[size] = 0; 
			char *dirBuf = NULL; 
			char *dir    = strtok_r(dirinpkg, "\n", & dirBuf); 
			while (dir != NULL) {
				// should we treat this as error?
				rmCode = del(dir, exitFail);
				if (rmCode != 0) {
					if (errno != ENOENT && errno != ENOTEMPTY) {
						if (exitFail == 0) {
							printf("WARNING: Failed to remove %s: %s\n", dir, strerror(errno));
						}
						else {
							printf("ERROR: Failed to remove %s: %s\n", dir, strerror(errno));
							exit(errno); 
						}	
					}
					dir = strtok_r(NULL, "\n", &dirBuf);
				}	
			}

			if (verbose != 0) debug("Removing package contain");
			char *conBuf = NULL; 
			char *con    = strtok_r(contain, " ", &conBuf); 
			while (con != NULL) {
				char conpath[__PATH] = "";
				snprintf(conpath, __PATH, "%s/var/lib/pachanh/system/%s", root, con);
				code = unlink(conpath);
				checkCode(code); 

				con = strtok_r(NULL, " ", &conBuf);	
			} 
			cfg_free(cfg);
			fclose(filelist); 

			if (hookcode == 0) {
				if (verbose != 0) debug("Running post-remove hook");
				code = system(posthook); 
				checkCode(code);
			}

			if (verbose != 0) debug("Updating package information");
			if ((strcmp(mode, "package")) == 0) {
				char *depBuf = NULL; 
				char *dep    = strtok_r(depends, " ", &depBuf);
				while (dep != NULL) {
					char sedDep[__CMD]   = "";
					snprintf(sedDep, __CMD, "sed -i 's/%s //g' %s/var/lib/pachanh/system/%s/info", name, root, dep);
					code = system(sedDep); 
					if (code != 0) printf("Warning: failed to update %s", dep);
					code = 0; 
	
					dep = strtok_r(NULL, " ", &depBuf); 
				}
			} 
			code = system(sedOrder);
			checkCode(code);

			if (verbose != 0) debug("Cleaning package data directory");
			if (hookcode == 0) {
				code = remove(hookpath);
				checkCode(code);
			}
			
			if ((checkDir(pkgpath, "silent")) == 0) {
				code = remove(pkgpath); 
				checkCode(code);
			}

			if (verbose != 0) debug("Cleaning up");
			clearTmp(tmp); 
			}
		else {
			if (exitFail != 0) {
				printf("ERROR: %s not installed\n", pkg);
				exit(exitFail); 
			}
		}
	REMOVE(dependants, root, "dependants", verbose, 1); 	

	pkg = strtok_r(NULL, " ", &buf);
	}
	printf("\n");
	return code; 
}
