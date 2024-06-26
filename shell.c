#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAXARGS 20
#define MAXLEN 256

char ** alloc_argvs() {
	char **argvs;
	
	argvs = (char **)malloc(MAXARGS * sizeof(char *));
	for(int i = 0; i < MAXARGS; i++) {
		argvs[i] = (char *)malloc(MAXLEN * sizeof(char));
	}

	return argvs;
}

void free_argvs(char **argvs) {
	for(int i = 0; i < MAXARGS; i++) {
		free(argvs[i]);
	}

	free(argvs);
}

int split_args(const char *line, char **argvs, char *delimiters) {
	if (line == NULL || argvs == NULL) {
		return -1;
	}
    	int argc = 0;
    	//const char *delimiters = delimiters; 

    	char *token = strtok((char *)line, delimiters);
    	while (token != NULL && argc < MAXARGS - 1) {
        	strcpy(argvs[argc], strdup(token)); 
		argc++;
        	token = strtok(NULL, delimiters);
    	}
	
	argvs[argc + 1] = NULL; // Ensure that the argsv is NULL terminated
	return argc;
}

void replace_to_home(int argc, char **argvs) {
	char *env, *token, arg[MAXLEN];
       
	env = getenv("HOME");	
	int i, j;
	for (i = 0; i < argc; i++) {
		token = strtok(argvs[i], "/\n");
		if (token != NULL && (strncmp(token, "~", 1) == 0)) {
			strcpy(arg, env);
			strcat(arg, "/");
			strcat(arg,e);
			strcpy(argvs[i], arg);
		}
		printf("%s\n", argvs[i]);
		
	}
}

typedef struct {
	char *name;
	int name_len;
	int (*execute)(int argc, char **argvs);
} Command;

int cd_command(int argc, char **argvs) {
    if (2 < argc) {
		printf("Usage message\n");
		return -1;
	}
	return chdir(argvs[1]);
}

int exit_command(int argc, char **argvs) {
    if (1 < argc) {
		printf("Exit dons't have ags:");
		for(int i = 1; i < argc; i++)
			printf(" %s", argvs[i]);
		printf("\n");
	}
	exit(1);
} 

int fork_exec_command(int argc, char **argvs) {
	int pid, endId, status;
	if((pid = fork()) == -1) {
		printf("Fork error\n");
	} else if (pid == 0) {
		if (execv(argvs[0], (char * const*) argvs) == -1) {
			printf("Execv error\n");
			exit(1);
		}
	} else {
		while ((endId = waitpid(pid, &status, WNOHANG|WUNTRACED)) == 0);
		exit(1);
	}

	return 0;
}

int env_exec_command(int argc, char**argvs) {
	char *env;
	char path_to_search[2 * MAXLEN];
	char **paths = alloc_argvs();
	int pathsc;
	struct stat sb;

	env = getenv("PATH");
	pathsc = split_args(env, paths, ":\n");
	if (pathsc < 1) return 1;
	
	for (int i = 0; i < pathsc; i++) {
		/* Construct the path to search */
		strcpy(path_to_search, paths[i]);
		strcat(path_to_search, "/");
		strcat(path_to_search, argvs[0]);
		
		/* If find the binary execute them */
		if (stat(path_to_search, &sb) != -1) {
			if (strlen(path_to_search) < MAXLEN - 2) {
				strcpy(argvs[0], path_to_search);
				return fork_exec_command(argc, argvs);
			} else {
				printf("Path to long to execute");
				break;
			}		
		} 
		
	}

	return 1;
}

int execute_command(int argc, char **argvs) {
	if(env_exec_command(argc, argvs))
		return -1;
	
	Command command_table[] = {
        	{"cd", strlen("cd"), cd_command},
        	{"exit", strlen("exit"), exit_command},
		{"./", strlen("/"), fork_exec_command}
    	};

    char *cmd_name = argvs[0];
    int cmd_table_size = sizeof(command_table) / sizeof(command_table[0]);
	
    for (int i = 0; i < cmd_table_size; i++) {
        if (strncmp(command_table[i].name, cmd_name, command_table[i].name_len) == 0) {
            if (command_table[i].execute != NULL) {
                return command_table[i].execute(argc, argvs);
            }
            break;
        }
    }
	
    return -1;
}

int main() {
	char buffer[1000];
	char *prompt = NULL;
	size_t prompt_len;
	int rt, argc;
	char **argvs = alloc_argvs();
	do{
		getcwd(buffer, 200);
		printf("%s$ ", buffer);
		rt = getline(&prompt, &prompt_len, stdin);
		argc = split_args(prompt, argvs, " \t\n");
	       	replace_to_home(argc, argvs);
		/*
		if(argc != 0 && execute_command(argc, argvs)) {
			printf("Unrecongnized command: %s\n", argvs[0]);
		}*/
	} while (rt != -1);
	return 0;	
}
