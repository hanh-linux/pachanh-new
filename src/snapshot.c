#include "hanh.h"
#include <libgen.h>

// Explaining how this work: 
// This snapshot feature is used for both upgrading system and creating pre-built binaries tarball
// checkDeps() function in hanhbuild won't work in stage mode as it will only check host's data

int cpInfo(char *file, const char *dir) {
	char cmd[__PATH] = ""; 
	snprintf(cmd, __PATH, "install -Dm755 %s %s/%s", file, dir, basename(file));
	int code = system(cmd);
	return code;
}

int SNAPSHOT(const char *installRoot, const char *buildDir, const char *env_optarg, const char *mode, const int noinstall, const int nodepends, const char *root, const int ignore, const int verbose) {
	char cwd[__PATH]          = "";
	char c[__PATH]            = "";
	char pkgorderPath[__PATH] = "";
	char infoPath[__PATH]     = ""; 
	char scriptPath[__PATH]   = "";
	char fetchCmd[__CMD]      = "";
	char buildCmd[__CMD]      = ""; 
	char baseLib[__PATH]      = "";
	char stageMode[__ARG]     = "";
	char arg[__ARG]           = "";
	char mvpkg[__CMD]         = "";
	char *tarname             = "";
	char newTar[__PATH]       = "";
	char dirpkgs[__PATH]      = ""; 
	char dirPkgs[__PATH]      = "";
	char builddir[__PATH]     = "";
	int code                  = 0 ;
	strcpy(builddir, buildDir);

	getcwd(cwd, __PATH);
	chdir(buildDir); 
	getcwd(builddir, __PATH);

	if ((strcmp(mode, "system")) == 0) {
		snprintf(pkgorderPath, __PATH, "%s/var/lib/pachanh/system/pkgorder", root);
		strcpy(arg, "-bd -s");
	}
	else if ((strcmp(mode, "stage")) == 0){
		snprintf(pkgorderPath, __PATH, "%s/pkgorder", builddir);
		snprintf(infoPath, __PATH, "%s/stage-info", builddir);
		snprintf(scriptPath, __PATH, "%s/script", builddir);
		snprintf(dirpkgs, __PATH, "%s/packages/packages", builddir);
		snprintf(dirPkgs, __PATH, "%s/packages", builddir);
		snprintf(baseLib, __PATH, "%s/baselib.tar.xz", builddir);
		snprintf(arg, __ARG, "-S -bd -ir=\"%s\"", installRoot);
		strcpy(stageMode, "stagemode=y");

		mkdirRecursive(dirpkgs, 0755);
	}		

	char *name = NULL; 
	char *ver  = NULL; 
	char *desc = NULL;
	char *path = NULL;

	cfg_opt_t opts[] = {
		CFG_SIMPLE_STR("name", &name), 
		CFG_SIMPLE_STR("ver", &ver), 
		CFG_SIMPLE_STR("desc", &desc),
		CFG_SIMPLE_STR("path", &path),
		CFG_END()
	};
	cfg_t *cfg = cfg_init(opts, 0); 
	cfg_parse(cfg, infoPath);

	if ((strcmp(mode, "system")) == 0) 
		printf("Building system snapshot"); 
	else 
		printf("Building stage tarball %s\n", name);
	
	code = checkPath(pkgorderPath, "Package build list");
	checkCode(code);
	FILE *pkgorder = fopen(pkgorderPath, "r");
	int size = getSize(pkgorder);

	char buildOrder[size];
	snprintf(buildOrder, size, "");
	fread(buildOrder, size, 1, pkgorder);
	buildOrder[size] = 0;

	if ((strcmp (mode, "stage")) == 0) {
		code = checkPath(baseLib, "silent");
		if (code == 0) {
			code = untar(installRoot, baseLib);
			checkCode(code);
		}
	}

	char *buf = NULL; 
	char *pkg = strtok_r(buildOrder, ";\n", &buf);
	while (pkg != NULL) {
		if (verbose != 0) printf("[DEBUG]: Building %s\n", pkg);
		snprintf(fetchCmd, __CMD, "hanhbuild -F %s", pkg);
		snprintf(buildCmd, __CMD, "hanhbuild -b %s %s snapshot=y buildroot=\"%s\" %s", arg, env_optarg, installRoot, stageMode) ;
		snprintf(newTar,   __CMD, "%s/%s", dirpkgs, tarname);

		code = system(fetchCmd); 
		checkCode(code); 
		chdir(pkg);
		code = system(buildCmd);
		checkCode(code);
		 

		if ((checkPath(".up-to-date", "silent")) != 0) {
			tarname = getName();
			code = LOCALINSTALL(tarname, env_optarg, installRoot, nodepends, 1, ignore, verbose);
			checkCode(code);
		
			snprintf(mvpkg, __CMD, "mv %s %s/%s", tarname, dirpkgs, tarname);
	
			if (noinstall != 0) {
				code = system(mvpkg);
				checkCode(code);
			}
		}
		chdir(builddir);

		pkg = strtok_r(NULL, ";\n", &buf);
	}

	chdir(cwd);
	printf("Finishing stage tarball\n");
	if (strcmp(mode, "stage") == 0) {
		if (noinstall != 0) {
			code = cpInfo(infoPath, dirPkgs);
			checkCode(code);
			if ((checkPath(scriptPath, "silent")) == 0) {
				code = cpInfo(scriptPath, dirPkgs); 
				checkCode(code);
			}
			code = createTar(dirPkgs, name);
			checkCode(code);
		}
		else {
			code = cpInfo(infoPath, installRoot);
			checkCode(code); 
			if ((checkPath(scriptPath, "silent")) == 0) {
				code = cpInfo(scriptPath, installRoot); 
				checkCode(code);
			}
			code = createTar(installRoot, name);
			checkCode(code);	
		}
	printf("Stage tarball created! (%s.tar.xz)\n", name);
	} 
	else if ((strcmp(mode, "system")) == 0) {
		printf("Snapshot installed to the system\n");
	}
	
	return 0;
}
