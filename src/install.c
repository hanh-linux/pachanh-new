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
		// Get rid of whitespace 
		snprintf(cmd, __CMD, "(diff \"%s\" \"%s\" | grep \"<\" | cut -d \"<\" -f 2 | sed 's/ //g') > %s/oldfiles", old, new, tmp);
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
		char presed[__CMD]= "";
		char sedcmd[__CMD]= "";
		// Remove existing package in dependants string for further package installation 
		snprintf(presed, __CMD, "sed -i \'s/%s //g\' %s/var/lib/pachanh/system/%s/info", pkgname, root, dep);
		snprintf(sedcmd, __CMD, "sed -i \'s/dependants=\"/dependants=\"%s /g\' %s/var/lib/pachanh/system/%s/info", pkgname, root, dep);
		code = system(presed);
		code = system(sedcmd); 
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
		char configFullPath[__PATH] = "";
		snprintf(configFullPath, __PATH, "%s/%s", root, cfgfile);
		if ((checkFile(configFullPath, "silent")) != 0) {
		// rename() will fail to execute if package file is installed in a 
		// separate partition, better use system `mv`
		char mvcmd[__CMD] = ""; 
		snprintf(mvcmd, __CMD, "mv %s/%s.newfile %s/%s", root, cfgfile, root, cfgfile);
		code = system(mvcmd);
		if (code != 0) printf("WARNING: failed to move %s\n", cfgfile);	
		}

		cfgfile = strtok_r(NULL, " ", &confBuf);
	} 
} 

// TODO: Remove all old version folders
void removeOld(const char *root, const char *tmp) {
	char filelistpath[__PATH] = ""; 
	snprintf(filelistpath, __PATH, "%s/oldfiles", tmp);

	FILE *filelist = fopen(filelistpath, "r");
	int size = getSize(filelistpath);
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

int LOCALINSTALL(char packages[], const char *root, const int nodepends, const long int verbose) {
	char *pkgBuf = NULL; 
	char *pkg  = strtok_r(packages, " ", &pkgBuf); 
	int  code    = 0; 
	
	while(pkg != NULL) {
		char tmpdir[20]      = "/tmp/tmp.XXXXXX" ; 
		char *tmp            = mkdtemp(tmpdir)   ;
		char rhead[__PATH]   = ""                ;
		char header[__PATH]  = ""                ;
		char tarHead[__ARG]  = ""                ; 
		char hook[__ARG]     = ""                ;
		char Hook[__PATH]    = ""                ;

		char preinstall[__PATH] = ""             ;
		char aftinstall[__PATH] = ""             ;
		char preupgrade[__PATH] = ""             ;
		char aftupgrade[__PATH] = ""             ;

		// Parse config for name, depends, contain (package provide part), config (to backup), pkg_infodir 
		char *name        = NULL                 ;
		char *version     = NULL                 ;
		char *desc        = NULL                 ;
		char *depends     = NULL                 ; 
		char *contain     = NULL                 ; 
		char *config      = NULL                 ; 
		char *pkg_infodir = NULL                 ;
		
		snprintf(Hook   , __PATH, "%s/hook"                               , root          );
		snprintf(header , __PATH, "%s/pre-install"                        , tmp           );
		snprintf(rhead  , __PATH, "%s/pre-install"                        , root          ); 
		snprintf(tarHead, __ARG , "%s pre-install"                        , pkg           );
		snprintf(hook   , __ARG , "%s hook"                               , pkg           );
		
		// should we use 'source'? 	
		snprintf(preinstall, __PATH, "%s/hook pre_install"      , tmp );
		snprintf(aftinstall, __PATH, "%s/hook post_install"     , tmp ); 
		snprintf(preupgrade, __PATH, "%s/hook pre_upgrade"      , tmp ); 
		snprintf(aftupgrade, __PATH, "%s/hook post_upgrade"     , tmp ); 
		
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
			if (installCode != 0) {
				if (verbose != 0) debug("Triggering pre-install hook"); 	
				code = system(preinstall); 
				checkCode(code);
			}
			else {
				if (verbose != 0) debug("Triggering pre-upgrade hook");
				code = system(preupgrade);
				checkCode(code);
			}
		}

		printf("Installing %s\n", name);
		code = untar(root, pkg);
		checkCode(code);	

		printf("Finishing %s\n", name);
		if (nodepends == 0) { 
			if (verbose != 0) debug("Updating dependencies information"); 
			code = updateDepends(root, name, depends);
			checkCode(code);
		}
		if (verbose != 0) debug("Working with config");
		checkConfig(root, config);
		if (hookcode == 0) {
			if (installCode == 0) {
				if (verbose != 0) debug("Triggering post install hook");
				code = system(aftinstall);
				checkCode(code); 
			}
			else {
				if (verbose != 0) debug("Triggering post upgrade hook");
				code = system(aftupgrade); 
				checkCode(code);
			}
		}
		if (installCode == 0) {
			if (verbose != 0) debug("Removing old files");
			removeOld(root, tmp);
		} 
		if (verbose != 0) debug("Cleaning up");
		code = remove(rhead); 
		checkCode(code);
		code = remove(Hook); 
		checkCode(code);
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

// This function now supports both binary and source installation. 
int PACKAGEINSTALL(char packages[], const char *opts, const char *root, const char *mirror, const char *download, char allrepo[], const int nodepends, const long int verbose) {
	char *pkgBuf         = NULL; 
	char *pkg            = strtok_r(packages, " ", &pkgBuf); 
	int  code            = 0;
	int  repoBufSize     = strlen(allrepo);
	int  file            = 1;

	while (pkg != NULL) {
		file = checkFile(pkg, "silent");
		// If it is a package tarball, install it into sysroot
		if (file == 0) {
			if (verbose != 0) debug("Installing local package");
			code = LOCALINSTALL(pkg, root, nodepends, verbose);
			checkCode(code);
		}
		else {
			if (verbose != 0) debug("Trying to find for remote package");
			printf("Checking if %s has a binary release\n", pkg);
			
			char allRepo[repoBufSize]; 
			strcpy(allRepo, allrepo);
			char *repoBuf = NULL; 
			char *repo    = strtok_r(allRepo, " ", &repoBuf); 
			int  success  = 1;
			int  found    = 0;
			int  relcode  = 0;
			int  pkgcode  = 0;

			while (repo != NULL) {
				char pkgrelpath[__PATH]  = "";
				char repopath[__PATH]    = "";
				char pkgpath[__PATH]     = "";

				snprintf(pkgpath    , __PATH, "%s/var/lib/pachanh/remote/%s/%s" , root, repo, pkg);
				snprintf(pkgrelpath , __PATH, "%s/release"                      , pkgpath        );
				snprintf(repopath   , __PATH, "%s/%s"                           , mirror, repo   );
			
				// Check if package is available and binary supported
				relcode = checkFile(pkgrelpath, "silent");
				pkgcode = checkDir(pkgpath, "silent");
				if (relcode == 0) {
					FILE *mirFile = fopen(repopath, "r"); 
					int msize = getSize(repopath); 
					char allMir[msize]; 
					allMir[msize] = '\0';
					fread(allMir, msize, 1, mirFile);
	
					char allmir[msize]; 
					strcpy(allmir, allMir); 

					FILE *pkgRel = fopen(pkgpath, "r");
					int tsize = getSize(pkgpath);
					char pkgTar[tsize];
					pkgTar[tsize] = '\0';
					fread(pkgTar, tsize, 1, pkgRel);
					pkgTar[strcspn(pkgTar, "\n")] = 0;

					char pathtopkg[__PATH] = "";
					snprintf(pathtopkg, __CMD, "%s/var/cache/pachanh/tarballs/packages/%s", root, pkgTar);
					found = checkFile(pathtopkg, "silent"); 
					// If the package downloaded earlier, we will install it
					// Otherwise, fetch it.
					if (found == 0) {
						printf("WARNING: Package downloaded earlier. Using it\n");
						code = LOCALINSTALL(pathtopkg, root, nodepends, verbose); 
						checkCode(code);
						success = 0;
					}
					else {
						// Try to fetch package
						printf("Fetching %s from remote\n", pkg);
						char *urlBuf = NULL;
						char *url    = strtok_r(allmir, "\n", &urlBuf);
						
						while (url != NULL) {
							char fetchCmd[__CMD] = ""; 
							snprintf(fetchCmd, __CMD, "%s %s %s/binaries/%s", download, pathtopkg, url, pkgTar);
		
							// Use process exit code to detect if the package is fetched (should we?)
							success = system(fetchCmd);
							if (success == 0) {
								if (verbose != 0) debug("Installing fetched package");
								code = LOCALINSTALL(pathtopkg, root, nodepends, verbose);
								checkCode(code);
								break;
							}
		
							url = strtok_r(NULL, "\n", &urlBuf);
						}
					}
					fclose(pkgRel);
					fclose(mirFile);
	
					if (success != 0) {
						die("Failed to fetch package from remote", success);
					}
				}
				else {
					if (pkgcode != 0) { 
						die("Package doesn't exist", relcode);
					}
					else {
						// Build package is the only way now.
						if (verbose != 0) debug("No binary package found! Building from source");
						char cwd[__PATH] = "";
						char fetchCmd[__CMD] = ""; 
						char buildCmd[__CMD] = "";
						snprintf(fetchCmd, __CMD, "hanhbuild -F %s"   , pkg );
						snprintf(buildCmd, __CMD, "hanhbuild -b -i %s", opts);
				
						getcwd(cwd, __PATH);
						
						printf("Fetching build file from remote\n");
						code = system(fetchCmd);
						checkCode(code);
						chdir(pkg);
						printf("Building package\n");
						code = system(buildCmd);
						checkCode(code);
						chdir(cwd);
					}
				}
				repo = strtok_r(NULL, " ", &repoBuf); 
			}
		
		}
		pkg = strtok_r(NULL, " ", &pkgBuf);
	}
	return code;
}

int LOCALSTAGEINSTALL(char tarballs[], const char *root, const char *installRoot, const long int verbose) {
	char *stageBuf           = NULL; 
	char *stageFile          = strtok_r (tarballs, " ", &stageBuf);
	int  code                = 0; 
	int  found               = 1;

	while (stageFile != NULL) {
		found = checkFile(stageFile, "silent");
		if (found != 0) {
			printf("ERROR: %s not found\n", stageFile);
			exit(found);
		}
		printf("Installing stage tarball\n");
		code = untar(installRoot, stageFile);
		checkCode(code);
	
		char runScript[__PATH]  = "";
		char scriptPath[__PATH] = "";
		char stageCfg[__PATH]   = "";
		char tarCfg[__PATH]     = ""; 
		char *sName             = NULL; 
		char *sVer              = NULL; 
		char *sDesc             = NULL;
		snprintf(stageCfg,  __PATH, "%s/stage-info", installRoot);

		cfg_opt_t opts[] = {
		CFG_SIMPLE_STR("name", &sName),
		CFG_SIMPLE_STR("ver",  &sVer), 
		CFG_SIMPLE_STR("desc", &sDesc),
		CFG_END()
		};
		cfg_t *cfg = cfg_init(opts, 0); 
		cfg_parse(cfg, stageCfg);

		// Trigger the stage script if it is presented
		snprintf(scriptPath, __PATH, "%s/var/lib/pachanh/system/stage/%s/script", installRoot, sName); 
		snprintf(runScript, __PATH, "%s", scriptPath);
		if ((checkFile(scriptPath, "silent")) == 0) {
			if (verbose != 0) debug("Triggering script");
			code = system(runScript);
			if (code != 0) die("Failed to trigger stage script", code);
		}
		printf("%s stage tarball installed successfully!\n", name);
		stageFile = strtok_r(NULL, " ", &stageBuf);
	}
	return code;
}

int STAGEINSTALL(char tarballs[], const char *installRoot, const char *root, const char *mirror, const char *download, const long int verbose) {
	char *stageBuf  = NULL; 
	char *stageFile = strtok_r(tarballs, " ", &stageBuf);
	int  code       = 0;
	int  found      = 0;
	int  fetched    = 0;
	int  success    = 1;
	int  rootFound  = 0;
	char sRepo[__PATH] = "";
	snprintf(sRepo, __PATH, "%s/stage", mirror);

	rootFound = checkDir(installRoot, "silent"); 
	if (rootFound != 0) {
		die("Root directory not found", rootFound);
	}

	while (stageFile != NULL) {
		success = 1;
		int found = checkFile(stageFile, "silent"); 
		if (found == 0) {
			code = LOCALSTAGEINSTALL(stageFile, root, installRoot, verbose);	
		}
		else {
			printf("Checking if remote has a %s stage tarball\n", stageFile);
			char stageNamePath[__PATH] = ""; 
			snprintf(stageNamePath, __PATH, "%s/var/lib/pachanh/remote/stage/%s", root, stageFile); 

			found = checkFile(sRepo, "silent"); 
			if (found != 0)
				die("Your system does not have the stage repository", 1); 
			
			code  = SYNC("stage", root, mirror, download, verbose);
			found = checkFile(stageNamePath, "silent");
			if (found != 0) {
				printf("ERROR: %s stage tarball is not available\n", stageFile);
				exit(1);
			}
			else {
				FILE *repoFile = fopen(sRepo, "r"); 
				int  rSize     = getSize(sRepo); 
				char allMir[rSize]; 
				allMir[rSize] = '\0'; 
				fread(allMir, rSize, 1, repoFile);
				
				printf("Fetching %s.stage\n", stageFile);
				char *urlBuf = NULL; 
				char *url    = strtok_r(allMir, "\n", &urlBuf); 
				while (url != NULL) {
					char fetchCmd[__PATH] = "";
					char pathtostage[__PATH] = ""; 
					snprintf(pathtostage, __PATH, "%s/var/cache/pachanh/tarballs/stage/%s.stage", root, stageFile);
					snprintf(fetchCmd, __PATH, "%s %s %s/stage/%s.stage", download, pathtostage, url, stageFile);
					
					success = system(fetchCmd);
					if (success == 0) {
						code = LOCALSTAGEINSTALL(pathtostage, root, installRoot, verbose);
						checkCode(code);
						break;
					}

				url = strtok_r(NULL, "\n", &urlBuf); 	
				}
			}
		if (success != 0) die("Failed to fetch required stage tarball", success); 
	}
	stageFile = strtok_r(NULL, "\n", &stageBuf);
	}
	return code;
}

int INSTALL(char packages[], const char *opts, const char *mode, const char *insroot, const char *root, const char *mirror, const char *download, char allrepo[], const int nodepends, const long int verbose) {
	int code = 0;
	if ((strcmp(mode, "packages")) == 0) {
		// int PACKAGEINSTALL(char packages[], const char *root, const char *mirror, const char *download, char allrepo[], const int nodepends, const long int verbose)
		code = PACKAGEINSTALL(packages, opts, root, mirror, download, allrepo, nodepends, verbose);
	}
	else if ((strcmp(mode, "stage")) == 0) {
		// int STAGEINSTALL(char tarballs[], const char *installRoot, const char *root, const char *mirror, const char *download, const long int verbose)
		code = STAGEINSTALL(packages, insroot, root, mirror, download, verbose); 
	}
	return code;
}
