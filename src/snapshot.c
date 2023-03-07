#include "hanh.h"
#include <libgen.h>

// Explaining how this work: 
// This snapshot feature is used for both upgrading system and creating pre-built binaries tarball
// checkDeps() function in hanhbuild won't work in stage mode as it will only check host's data

int cpInfo(char *file, const char *path) {
	char cmd[__PATH] = ""; 
	snprintf(cmd, __PATH, "cp -r %s %s/%s", file, path, basename(file));
	puts(cmd);
	printf("Pkgdir: %s\n", path);
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
	int code                  = 0 ;


	getcwd(cwd, __PATH);

	if ((strcmp(mode, "system")) == 0) {
		snprintf(pkgorderPath, __PATH, "%s/var/lib/pachanh/system/pkgorder", root);
		strcpy(arg, "-bd -s -i");
	}
	else if ((strcmp(mode, "stage")) == 0){
		snprintf(pkgorderPath, __PATH, "%s/pkgorder", buildDir);
		snprintf(infoPath, __PATH, "%s/stage-info", buildDir);
		snprintf(scriptPath, __PATH, "%s/script", buildDir);
		snprintf(dirpkgs, __PATH, "%s/packages/packages", buildDir); 
		snprintf(dirPkgs, __PATH, "%s/packages", buildDir);
		snprintf(baseLib, __PATH, "%s/baselib.tar.xz", buildDir);
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

	chdir(buildDir); 
	char *buf = NULL; 
	char *pkg = strtok_r(buildOrder, ";\n", &buf);
	while (pkg != NULL) {
		snprintf(fetchCmd, __CMD, "hanhbuild -F %s", pkg);
		snprintf(buildCmd, __CMD, "buildroot=\"%s\" %s hanhbuild -b %s %s", installRoot, stageMode, arg, env_optarg);
		snprintf(newTar,   __CMD, "%s/%s", dirpkgs, tarname);

		code = system(fetchCmd); 
		checkCode(code); 
		chdir(pkg);
		code = system(buildCmd);
		checkCode(code); 
		tarname = getName();
		code = LOCALINSTALL(tarname, installRoot, nodepends, 1, ignore, verbose);
		checkCode(code);
		snprintf(mvpkg, __CMD, "mv %s %s", tarname, dirpkgs);

		if (noinstall != 0) {
			code = system(mvpkg);
			checkCode(code);
		}
		chdir(cwd);
		chdir(buildDir);

		pkg = strtok_r(NULL, ";\n", &buf);
	}

	chdir(cwd);
	if (strcmp(mode, "stage") == 0) {
		if (noinstall != 0) {
			printf("aaaaaaaaaa:: %s\n" , dirpkgs);
			code = cpInfo(infoPath, dirpkgs);
			checkCode(code);
			if ((checkPath(scriptPath, "silent")) == 0) {
				code = cpInfo(scriptPath, dirpkgs); 
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
	}


	return 0;
}
