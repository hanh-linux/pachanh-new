#include "hanh.h"

int QUERY(char packages[], const char *root, const char *filetype) {
	char *pkgBuf = NULL; 
	char *pkg  = strtok_r(packages, " ", &pkgBuf);
	int code = 0;
	while (pkg != NULL) {
		int installCode = checkInstalled(root,pkg); 
		if (installCode != 0) {
			printf("ERROR: %s not installed\n", pkg);
			exit(1); 
		}
		char ftype[__ARG] = ""; 
		strcpy(ftype, filetype);
		if (ftype[0] == '\0') strcpy(ftype, "info,filelist");

		char *ftBuf = NULL; 
		char *file  = strtok_r(ftype, ",", &ftBuf); 
		while (file != NULL) {
			if ((strcmp(file, "info")) == 0) {
				
				char *name       = NULL; 
				char *ver        = NULL; 
				char *desc       = NULL; 
				char *home       = NULL; 
				char *license    = NULL;
				char *depends    = NULL; 
				char *contain    = NULL; 
				char *dependants = NULL;

				char infopath[__PATH] = "";
				snprintf(infopath, __PATH, "%s/var/lib/pachanh/system/%s/info", root, pkg);
				
				cfg_opt_t opts[] = {
				CFG_SIMPLE_STR("name", &name), 
				CFG_SIMPLE_STR("version", &ver), 
				CFG_SIMPLE_STR("desc", &desc), 
				CFG_SIMPLE_STR("home", &home),
				CFG_SIMPLE_STR("depends", &depends), 
				CFG_SIMPLE_STR("license", &license), 
				CFG_SIMPLE_STR("contain", &contain), 
				CFG_SIMPLE_STR("dependants", &dependants), 
				CFG_END() 
				};
				cfg_t *cfg = cfg_init(opts, 0);
				cfg_parse(cfg, infopath);

				printf("Name: %s\n", name);
				printf("Version: %s\n", ver); 
				printf("Description: %s\n", desc);
				printf("Homepage: %s\n", home); 
				printf("License: %s\n", license); 
				printf("Dependencies: %s\n", depends);
				printf("Contain: %s\n", contain); 
				printf("Dependants: %s\n", dependants); 
			} 
			else if ((strcmp(file, "filelist")) == 0){ 
				char flpath[__PATH] = "";
				snprintf(flpath, __PATH, "%s/var/lib/pachanh/system/%s/filelist", root, pkg);

				FILE *filelist = fopen(flpath, "r"); 
				fseek(filelist, 0, SEEK_END); 
				int size = ftell(filelist); 
				fseek(filelist, 0, SEEK_SET); 
				char flcon[size];
				fread(flcon, size, 1, filelist); 
				flcon[size] = 0; 
				printf("%s\n", flcon); 
				fclose(filelist);
			} 
			else {
			die("Invalid file to query", 1); 
			}

			printf("\n");
			file = strtok_r(NULL, ",", &ftBuf);
		}

		pkg = strtok_r(NULL, " ", &pkgBuf);
	}
	printf("\n\n");
	return code; 
}
