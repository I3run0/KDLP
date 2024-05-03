#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/wait.h>

#define MAXARGS 10

int split_args(const char *line, char **args) {
	if (line == NULL || args == NULL) {
		return -1;
	}
    int arg_count = 0;
    const char *delimiters = " \t\n"; 

    char *token = strtok((char *)line, delimiters);
    while (token != NULL && arg_count < MAXARGS) {
		if (*token != '\0') {
        	args[arg_count] = strdup(token); 
			arg_count++;
        	token = strtok(NULL, delimiters);
		}
    }

    return arg_count;
}

typedef struct {
	char *name;
	int name_len;
	int (*execute)(int argc, char *argvs[]);
} Command;

int cd_command(int argc, char *argvs[]) {
    if (2 < argc) {
		printf("Usage message\n");
		return -1;
	}
	return chdir(argvs[1]);
}

int exit_command(int argc, char *argvs[]) {
    if (1 < argc) {
		printf("Exit dons't have ags:");
		for(int i = 1; i < argc; i++)
			printf(" %s", argvs[i]);
		printf("\n");
	}
	exit(1);
} 

int fork_exec_command(int argc, char *argvs[]) {
	int pid, endId, status;
	if(pid = fork() == -1) {
		printf("Fork error\n");
	} else if (pid == 0) {
		execv(argvs[0] + 1, argvs)
	} else {
		while ((endId = waitpid(pid, &status, WNOHANG|WUNTRACED)) == 0);
	}
	
}

int execute_command(int argc, char *argvs[]) {
    Command command_table[] = {
        {"cd", strlen("cd"), cd_command},
        {"exit", strlen("exit"), exit_command},
		{"./", strlen("./"), fork_exec_command},
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
	char buffer[200];
	char *prompt;
	size_t prompt_len;
	int rt, args_c;
	char *argvs[MAXARGS];
	do{
		getcwd(buffer, 200);
		printf("%s$ ", buffer);
		rt = getline(&prompt, &prompt_len, stdin);
		args_c = split_args(prompt, argvs); 
		execute_command(args_c, argvs);
			//printf("Unrecongnized command: %s\n", argvs[0] != NULL ? argvs[0] : "");
		//}
	} while (rt != -1);
	return 0;	
}
