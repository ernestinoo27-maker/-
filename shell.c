#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/file.h>
#include <wait.h>

int flag = 0, flag_for_extra = 0, not_end_inp = 1, errors = 1;

void hand_int_tstp(int sig){
	kill(getpid(), SIGKILL);
}

void get_s(char *buffer){
	char buff_hlp[1024] = {0};
	char extra[1024] = {0};
	char extra_help[1024] ={0};
	flag_for_extra = 0;
	if (fgets(buff_hlp, 1024, stdin) == NULL){
		not_end_inp = 0;
	}
	buff_hlp[strcspn(buff_hlp, "\n")] = '\0';
	if (buff_hlp[0] != '|'){
		for (int i = 0, j = 0; i <= strlen(buff_hlp); i++){
			if (buff_hlp[i] != ' '){
				buffer[j] = buff_hlp[i];
				j++;
				flag = 1;
			} else if ((i != 0) && (buff_hlp[i - 1] != ' ') && (flag != 0)){
				buffer[j] = ' ';
				j++;
			}
		}
		while (((buffer[strlen(buffer) - 1] == '|') || ((buffer[strlen(buffer) - 1] == ' ') && (buffer[strlen(buffer) - 2] == '|'))) && errors){
			printf("|--> ");
			fgets(extra, 1024, stdin);
			extra[strcspn(extra, "\n")] = '\0';
			for (int i = 0, j = 0; i <= strlen(extra); i++){
				if (extra[i] != ' '){
					extra_help[j++] = extra[i];
					flag_for_extra = 1;
				} else if ((i != 0) && (extra[i - 1] != ' ') && (flag_for_extra != 0)){
					extra_help[j++] = ' ';
				}
			}
			if (extra_help[0] == '|'){
				printf("syntax error near unexpected token '|'\n");
				errors = 0;
			}
			strcat(buffer, extra_help);
		}
	} else {
		buffer[0] = '|';
	}
}

int other_errors = 0;

int sep_by_cond_coms(char *shell_command, char **mas_conv_commands, int *mode){
	int i = 0, j = 0, count_conv_commands = 0;
	char *local_buf = (char*)malloc(512 * sizeof(char));
	memset(local_buf, 0, 512);
	while (shell_command[i] != '\0'){
		if ((i == 0) && ((shell_command[i] == '&') || (shell_command[i] == ';'))){
			other_errors = 1;
			break;
		} else if ((shell_command[i] == '&') && (shell_command[i + 1] != '&') && (shell_command[i - 1] != '&')){
			mode[count_conv_commands] = 1;
			local_buf[j] = '\0';
			mas_conv_commands[count_conv_commands++] = strdup(local_buf);
			j = 0;
			memset(local_buf, 0, 512);
		} else if (shell_command[i] == ';'){
			mode[count_conv_commands] = 0;
			local_buf[j] = '\0';
			mas_conv_commands[count_conv_commands++] = strdup(local_buf);
			j = 0;
			memset(local_buf, 0, 512);
		} else {
			local_buf[j++] = shell_command[i];
		}
		i++;
	}
	mode[count_conv_commands] = 0;
	local_buf[j] = '\0';
	mas_conv_commands[count_conv_commands++] = strdup(local_buf);
	free(local_buf);
	return count_conv_commands;
}

int separate_conv(char *buffer, char **argv){
	char *arg = strtok(buffer, "|");
	int i = 0;
	while (arg != NULL){
		argv[i++] = strdup(arg);
		arg = strtok(NULL, "|");
	}
	return i;
}

void pipes(int fd[][2], int count){
	for (int j = 1; j <= count; j++){
		if(pipe(fd[j]) == -1){
			perror("Pipe is failed.");
			exit(1);
		}
	}
}

int file_count = 0, flag_in = 0, flag_out = 0;

void scan_for_spec_sym(char *conv, char *simple_com_args[128], char *io_files[128]){
	int k = 0;
	simple_com_args[k] = strtok(conv, " ");
	if ((strcmp(simple_com_args[k], ">") == 0) || (strcmp(simple_com_args[k], "<") == 0) || (strcmp(simple_com_args[k], ">>") == 0)){
		io_files[file_count++] = strdup(simple_com_args[k]);
		simple_com_args[k] = strtok(NULL, " ");
		io_files[file_count++] = strdup(simple_com_args[k]);
	} else {
		k++;
	}
	while ((simple_com_args[k] = strtok(NULL, " ")) != NULL){
		if ((strcmp(simple_com_args[k], ">") == 0) || (strcmp(simple_com_args[k], "<") == 0) || (strcmp(simple_com_args[k], ">>") == 0)){
			io_files[file_count++] = strdup(simple_com_args[k]);
			simple_com_args[k] = strtok(NULL, " ");
			io_files[file_count++] = strdup(simple_com_args[k]);
		} else {
			k++;
		}
	}
}

void output_input_change(char *io_files[128]){
	for (int p = 0; p < file_count; p++){
    	if (strcmp(io_files[p], "<") == 0){
    		int fd_inp = open(io_files[p + 1], O_RDONLY);
    		dup2(fd_inp, 0);
    		close(fd_inp);
    		flag_in = 1;
    	} else if (strcmp(io_files[p], ">") == 0){
    		int fd_outp = open(io_files[p + 1], O_WRONLY | O_CREAT | O_TRUNC, 0660);
    		dup2(fd_outp, 1);
    		close(fd_outp);
    		flag_out = 1;
    	} else if (strcmp(io_files[p], ">>") == 0){
    		int fd_outp = open(io_files[p + 1], O_WRONLY | O_CREAT | O_APPEND, 0660);
    		dup2(fd_outp, 1);
    		close(fd_outp);
    		flag_out = 1;
    	}
    }
}

void backgrd_io(int key){
	int input = open("/dev/null", O_RDWR);
	dup2(input, key);
	close(input);
}

void execute(char *arg1, char **args){
	execvp(arg1, args);
	int fd_help = open("/dev/tty", O_RDWR);
	dup2(fd_help, 1);
	printf("%s: command not found\n", arg1);
	exit(1);
}

void conv_execute(char *cond_command){
 	char *argv[512], *simple_com_args[128], *io_files[128];
						
	int i = separate_conv(cond_command, argv);
	int virt_pid = i, pr_count = i, fexit = 0;
	pid_t pr_mas[pr_count][2];
	
	int fd[i + 1][2], ext = 1;
	
	if (errors == 1){
		
		pipes(fd, i);
		
		while (virt_pid && ext){
			if((pr_mas[virt_pid - 1][1] = fork()) == 0) {
				ext = 0;
				break; 
			}
			virt_pid--;
		}
		if (virt_pid != 0){
			if ((argv[virt_pid - 1][0] != ' ') || (argv[virt_pid - 1][1] != '\0')){
				
				scan_for_spec_sym(argv[virt_pid - 1], simple_com_args, io_files);
				
				output_input_change(io_files);
				
				if ((virt_pid > 1) && !flag_in) {dup2(fd[virt_pid][0], 0);}
				
				if ((virt_pid < i) && !flag_out) {dup2(fd[virt_pid + 1][1], 1);}
				
				for (int k = 1; k <= i; k++) {
					close(fd[k][1]);
					close(fd[k][0]);
				}
				
				execute(simple_com_args[0], simple_com_args);
				
			} else {
				printf("myshell: syntax error near unexpected token '|'\n");
				exit(1);
			}
		} else {
			for (int k = 1; k <= i; k++) {
			    close(fd[k][1]);
			    close(fd[k][0]);
			}
			int st;
			for (int j = 0; j < pr_count; j++){
				waitpid(pr_mas[j][1], &st, 0);
				if (WIFEXITED(st)){
					fexit = WEXITSTATUS(st);
				}
				free(argv[j]);
			}
		}
	} else {
		if (errors == 2){
			printf("myshell: syntax error near unexpected token '|'\n");
			exit(1);
		}
	}
	exit(fexit);
}

void imp_cond_com(char *cond_coms, int executed){
	char *next_cond_com = cond_coms;
	if ((strstr(next_cond_com, "||") != NULL) && (strstr(next_cond_com, "&&") != NULL) && (strstr(next_cond_com, "||") < strstr(next_cond_com, "&&"))){	
		char *curr_com = next_cond_com;
		next_cond_com = strstr(next_cond_com, "||") + 2;
		pid_t com = fork();
		if (com == 0){
			strstr(curr_com, "||")[0] = '\0';
			if (executed){
				conv_execute(curr_com);
			} 
			exit(1);	
		} else {
			int st;
			waitpid(com, &st, 0);
			if (WIFEXITED(st)){
				if (WEXITSTATUS(st) != 0){
					imp_cond_com(next_cond_com, 1);
					exit(1);
				}
			}
			imp_cond_com(next_cond_com, 0);
			exit(1);
		}
	} else if ((strstr(next_cond_com, "||") != NULL) && (strstr(next_cond_com, "&&") != NULL) && (strstr(next_cond_com, "||") > strstr(next_cond_com, "&&"))){
		char *curr_com = next_cond_com;
		next_cond_com = strstr(next_cond_com, "&&") + 2;
		pid_t com = fork();
		if (com == 0){
			strstr(curr_com, "&&")[0] = '\0';
			if (executed){
				conv_execute(curr_com);
			} 
			exit(1);	
		} else {
			int st;
			waitpid(com, &st, 0);
			if (WIFEXITED(st)){
				if (WEXITSTATUS(st) == 0){
					imp_cond_com(next_cond_com, 1);
					exit(1);
				}
			}
			imp_cond_com(next_cond_com, 0);
			exit(1);
		}
	} else if ((strstr(next_cond_com, "||") == NULL) && (strstr(next_cond_com, "&&") != NULL)){
		char *curr_com = next_cond_com;
		next_cond_com = strstr(next_cond_com, "&&") + 2;
		pid_t com = fork();
		if (com == 0){
			strstr(curr_com, "&&")[0] = '\0';
			if (executed){
				conv_execute(curr_com);
			} 
			exit(1);	
		} else {
			int st;
			waitpid(com, &st, 0);
			if (WIFEXITED(st)){
				if (WEXITSTATUS(st) == 0){
					imp_cond_com(next_cond_com, 1);
					exit(1);
				}
			}
			imp_cond_com(next_cond_com, 0);
			exit(1);
		}
	} else if ((strstr(next_cond_com, "||") != NULL) && (strstr(next_cond_com, "&&") == NULL)){
		char *curr_com = next_cond_com;
		next_cond_com = strstr(next_cond_com, "||") + 2;
		pid_t com = fork();
		if (com == 0){
			strstr(curr_com, "||")[0] = '\0';
			if (executed){
				conv_execute(curr_com);
			} 
			exit(1);
		} else {
			int st;
			waitpid(com, &st, 0);
			if (WIFEXITED(st)){
				if (WEXITSTATUS(st) != 0){
					imp_cond_com(next_cond_com, 1);
					exit(1);
				}
			}
			imp_cond_com(next_cond_com, 0);
			exit(1);
		}
	} else {
		if (executed){
			conv_execute(next_cond_com);
		}
		exit(1);
	}
}

int main(){
	signal(SIGTSTP, SIG_IGN); signal(SIGINT, SIG_IGN);
	char buffer[1024] = {0}, cur_dir[128] = {0}, strerror[128] = {0};
	getcwd(cur_dir, 128);
	printf("|---(gruz1nishe^gruz1nishe)-[~%s]\n", cur_dir);
	printf("|--$ ");
	get_s(buffer);
	
	while ((strstr(buffer, "exit") == NULL) && not_end_inp){
		if (buffer[0] == '|'){
			printf("myshell: syntax error near unexpected token '|'\n");
		} else if ((buffer[0] != '\0') && errors){
			
			char *cond_coms[128];
			int mode[128] = {0}, count_cond_coms;
			count_cond_coms = sep_by_cond_coms(buffer, cond_coms, mode);
			
			for (int cur_cond_com = 0; cur_cond_com < count_cond_coms; cur_cond_com++){
				pid_t seq_pid;
				if ((seq_pid = fork()) == 0){
					signal(SIGTSTP, hand_int_tstp);
					signal(SIGINT, hand_int_tstp);
					if (mode[cur_cond_com] == 0){ // not background
						imp_cond_com(cond_coms[cur_cond_com], 1);
					} else { // background
						if (fork() == 0){
							printf("(myshell) PID: %d\n", getpid());
							signal(SIGINT, SIG_IGN);
							signal(SIGTSTP, SIG_IGN);
							backgrd_io(0);
							/*
							backgrd_io(1);
							*/
							imp_cond_com(cond_coms[cur_cond_com], 1);
						} else {
							exit(0);
						}
					}
				} else {
					waitpid(seq_pid, NULL, 0);
					free(cond_coms[cur_cond_com]);
				}
			} 
		}
		
		getcwd(cur_dir, 128);
		errors = 1;
		printf("|---(gruz1nishe^gruz1nishe)-[~%s]\n", cur_dir);
		printf("|--$ ");
		get_s(buffer);
	}
	
	return 0;
} 
