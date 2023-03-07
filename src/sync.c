#include "hanh.h"

int SYNC(char repositories[], const char *root, const char *mirror, const char *fetchCmd, const long int verbose) {
	char cwd[__PATH] = "";
	char remotePath[__PATH] = ""; 
	char *rBuf = NULL; 
	char *repo = strtok_r(repositories, " ", &rBuf); 
	int code  = 0;

	getcwd(cwd, __PATH); 
	snprintf(remotePath, __PATH, "%s/var/lib/pachanh/remote", root);
	code = chdir(remotePath);
	if (code != 0) die("Remote dbdir not found!", 1);
	checkCode(code);
	printf("Cleaning old database...\n");
	code = system("rm -rf *");
	checkCode(code);

	while (repo != NULL) {
		char repoPath[__PATH] = "";
		snprintf(repoPath, __PATH, "%s/%s", mirror, repo);
		code = checkPath(repoPath, repo); 
		checkCode(code); 

		FILE *repoFile = fopen(repoPath, "r");
		int size = getSize(repoFile);
		char repoCon[size]; 
		fread(repoCon, size, 1, repoFile);

		char *mirBuf = NULL;
		char *mir = strtok_r(repoCon, "\n", &mirBuf); 
		printf("Fetching %s.database\n", repo);
		while (mir != NULL) {
			char fetchdb[__CMD] = ""; 
			char database[__PATH] = ""; 
			char repoUpdate[__PATH] = ""; 
			char triggerCmd[__CMD]  = ""; 

			snprintf(fetchdb, __CMD, "%s %s.database %s/%s.database", fetchCmd, repo, mir, repo);
			snprintf(database, __PATH, "%s/%s.database", remotePath, repo);
			snprintf(repoUpdate, __PATH, "%s/%s/%s.sh", remotePath, repo, repo); 
			snprintf(triggerCmd, __CMD, "sh %s", repoUpdate);

			code = system(fetchdb); 
			if (code == 0) {
				if (verbose != 0) debug("Unpacking repo database");
				mkdir(repo, 0755);
				code = untar(repo, database); 
				checkCode(code); 
				if((checkPath(repoUpdate, "silent")) == 0) {
					if (verbose != 0) debug("Triggering update script");
					code = system(triggerCmd); 
					checkCode(code);
				}
				break;
			}
			mir = strtok_r(NULL, "\n", &mirBuf);
		}
		repo = strtok_r(NULL, " ", &rBuf);
	}	
	return code; 
}
