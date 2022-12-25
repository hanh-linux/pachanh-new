#include "hanh.h"

int SNAPSHOT(const char *root) {
	char cwd[__PATH] = ""; 
	getcwd(cwd, __PATH);
	char pkgorderPath[__PATH] = ""; 
	snprintf(pkgorderPath, __PATH, "%s/var/lib/pachanh/system/pkgorder", root); 
	char buildpkg[__PATH] = "hanhbuild -b -bd -s -i";
	int code = 0;

	FILE *pkgorder = fopen(pkgorderPath, "r");
	fseek(pkgorder, 0, SEEK_END);
	long int size = ftell(pkgorder);
	fseek(pkgorder, 0, SEEK_SET);
	char packages[size]; 
	packages[size] = 0; 
	fread(packages, size, 1, pkgorder);

	char *pkgBuf = NULL; 
	char *pkg    = strtok_r(packages, ";\n", &pkgBuf);
	while (pkg != NULL) {
		char fetchpkg[__PATH] = "";
		snprintf(fetchpkg, __PATH, "hanhbuild -F %s", pkg); 
		
		code = system(fetchpkg); 
		checkCode(code); 
		chdir(pkg); 
		code = system(buildpkg); 
		checkCode(code); 
		chdir(cwd); 
	
	pkg = strtok_r(NULL, ";\n", &pkgBuf); 
	}
	return code; 
}

int SNAPSHOT(const char *root, const char *installRoot, const int ve) 
