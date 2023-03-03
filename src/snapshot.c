#include "hanh.h"
#include <libgen.h>

// Explaining how this work: 
// This snapshot feature is used for both upgrading system and creating pre-built binaries tarball
// checkDeps() function in hanhbuild won't work in stage mode as it will only check host's data

int cpInfo(const char *path, const char *file) {
	char newFile[__PATH] = ""; 
	char *oldFile        = NULL; 
	strcpy(oldFile, file);
	snprintf(newFile, __PATH, "%s/%s", path, basename(oldFile));

	int code = rename(oldFile, newFile); 
	return code;
}

int SNAPSHOT(const char *installRoot, const char *buildDir, const char *env_optarg, const char *mode, const int noinstall, const int nodepends, const char *root, const int ignore, const int verbose) {
	char cwd[__PATH]          = "";
	char pkgorderPath[__PATH] = "";
	char infoPath[__PATH]     = ""; 
	char scriptPath[__PATH]   = "";
	char parentPkgdir[__PATH] = "";
	char pkgdir[__PATH]       = "";
	char fetchCmd[__CMD]      = "";
	char buildCmd[__CMD]      = ""; 
	char baseLib[__PATH]      = "";
	char arg[__ARG]           = "";
	char *tarname             = "";
	char newTar[__PATH]       = "";
	int code                  = 0 ;


	getcwd(cwd, __PATH);

	if ((strcmp(mode, "system")) == 0) {
		snprintf(pkgorderPath, __PATH, "%s/var/lib/pachanh/system/pkgorder", root);
		strcpy(arg, "-bd -s -i");
	}
	else if ((strcmp(mode, "stage")) == 0){
		snprintf(pkgorderPath, __PATH, "%s/pkgorder", buildDir);
		snprintf(infoPath, __PATH, "%s/stage-info", buildDir);
		snprintf(parentPkgdir, __PATH, "%s/packages", buildDir);
		snprintf(pkgdir, __PATH, "%s/packages/packages/", buildDir);
		snprintf(scriptPath, __PATH, "%s/script", buildDir);
		snprintf(baseLib, __PATH, "%s/baselib.tar.gz", buildDir);
		snprintf(arg, __ARG, "-S -bd -ir=\"%s\"", installRoot);

		mkdir(parentPkgdir, 0755); 
		mkdir(pkgdir, 0755);
	}

	char *name = NULL; 
	char *ver  = NULL; 
	char *desc = NULL;

	cfg_opt_t opts[] = {
		CFG_SIMPLE_STR("name", &name), 
		CFG_SIMPLE_STR("ver", &ver), 
		CFG_SIMPLE_STR("desc", &desc),
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
		tarname = getName(); 
		snprintf(fetchCmd, __CMD, "hanhbuild -F %s", pkg);
		snprintf(buildCmd, __CMD, "buildroot=\"%s\" hanhbuild -b %s %s", installRoot, arg, env_optarg);
		snprintf(newTar,   __CMD, "%s/%s", pkgdir, tarname);

		chdir(pkg);
		code = system(fetchCmd); 
		checkCode(code); 
		code = system(buildCmd);
		code = LOCALINSTALL(tarname, installRoot, nodepends, 1, ignore, verbose);
		checkCode(code);

		if (noinstall != 0) {
			code = rename(tarname, newTar);
			checkCode(code);
		}
		chdir(cwd);

		pkg = strtok_r(NULL, ";\n", &buf);
	}

	chdir(cwd);
	if (strcmp(mode, "stage") == 0) {
		if (noinstall != 0) {
			code = cpInfo(infoPath, pkgdir);
			checkCode(code); 
			code = cpInfo(scriptPath, pkgdir); 
			checkCode(code);
			code = createTar(parentPkgdir, name);
			checkCode(code);
		}
		else {
			code = cpInfo(infoPath, installRoot);
			checkCode(code); 
			code = cpInfo(scriptPath, installRoot); 
			checkCode(code);
			code = createTar(installRoot, name);
			checkCode(code);	
		}
	}


	return 0;
}
