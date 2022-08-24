#include "hanh.h"

void FIND(char packages[], const char *root, char repositories[]) {
	char *pkgBuf = NULL; 
	char *pkg    = strtok_r(packages, " ", &pkgBuf);
	while (pkg != NULL) {
		char *repoBuf = NULL; 
		char *repo    = strtok_r(repositories, " ", &repoBuf); 
		while (repo != NULL) {
			char pkgpath[__PATH] = ""; 
			snprintf(pkgpath, __PATH, "%s/var/lib/pachanh/remote/%s/%s/data", root, repo, pkg); 
			if ((checkPath(pkgpath, "silent")) == 0) {
				FILE *data = fopen(pkgpath, "r"); 
				fseek(data, 0, SEEK_END); 
				long int size = ftell(data); 
				fseek(data, 0, SEEK_SET);
				char dataCon[size]; 
				dataCon[size] = 0; 
				fread(dataCon, size, 1, data); 
				printf("%s/%s\n", repo, dataCon);
				fclose(data);
			}
		repo = strtok_r(NULL, " ", &repoBuf);
		}
	pkg = strtok_r(NULL, " ", &pkgBuf);
	}
}
