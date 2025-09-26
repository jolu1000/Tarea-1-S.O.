#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64
#define MAX_CMDS 10

// Parsing
void parse_command(char *input, char **args) {
    int i = 0;
    // Separa por espacios o tabs
    char *token = strtok(input, " \t");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL;
}

// miprof ejec/ejecsave
void run_miprof(char **args) {
    struct timespec start, end;
    struct rusage usage;
    int status;

    int flag = 0; //flag para ver si se guarda en archivo o no
    int fd = -1;
    char **cmd;

    //Verificar entrada en caso que se ingrese "miprof"
    if (args[1] && strcmp(args[1], "ejec") == 0) {
        cmd = &args[2];
    } else if (args[1] && strcmp(args[1], "ejecsave") == 0) {
        if (!args[2] || !args[3]) {
            fprintf(stderr, "uso: miprof ejecsave <archivo> <comando> [args...]\n");
            return;
        }
        flag = 1;
        fd = open(args[2], O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd < 0) {
            perror("open");
            return;
        }
        cmd = &args[3];
    } else {
        fprintf(stderr, "uso: miprof [ejec|ejecsave archivo] comando [args...]\n");
        return;
    }

    if (cmd[0] == NULL) {
        fprintf(stderr, "falta comando\n");
        if (fd != -1) close(fd);
        return;
    }

    clock_gettime(CLOCK_MONOTONIC, &start); //Empieza a tomar el tiempo

    pid_t pid = fork(); //se crea un proceso hijo para que ejecute el comando
    if (pid == 0) {
        execvp(cmd[0], cmd);
        fprintf(stderr, "mishell: %s: comando no encontrado\n", cmd[0]);
        _exit(127);
    } else if (pid > 0) {
        if (wait4(pid, &status, 0, &usage) == -1) {
            perror("wait4");
            if (fd != -1) close(fd);
            return;
        }

        clock_gettime(CLOCK_MONOTONIC, &end); //Se termina de tomar el tiempo


        //Se calculan las estadísticas de miprof
        double real_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        double user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6;
        double sys_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;
        long max_mem = usage.ru_maxrss;

        char buffer[512];
        int len = snprintf(buffer, sizeof(buffer),
            "\n--- Resultados miprof ---\n"
            "Tiempo real   : %.6f s\n"
            "Tiempo usuario: %.6f s\n"
            "Tiempo sistema: %.6f s\n"
            "Memoria max   : %ld KB\n",
            real_time, user_time, sys_time, max_mem);

        printf("%s", buffer);

        if (flag && fd != -1) {
            if (write(fd, buffer, len) < 0) {
                perror("write");
            }
            close(fd);
        }
    } 
    else {
        perror("fork");
    }
}

//miprof maxtiempo


pid_t child_pid = -1; //variable global que guarda el pid del hijo creado con fork()


//signal handler para SIGALRM
void handle_alarm_direct(int sig) {
    if (child_pid > 0) {
        kill(child_pid, SIGKILL);
        write(STDERR_FILENO, "Proceso terminado por timeout\n", 31);
    }
}

//función miprof maxtiempo
void run_miprof_maxtiempo(char **args) {
    struct timespec start, end;
    struct rusage usage;
    int status;  

    if (!args[2] || !args[3]) {
        fprintf(stderr, "uso: miprof maxtiempo <segundos> <comando> [args...]\n");
        return;
    }

    int limite = atoi(args[2]);
    char **cmd = &args[3];

    struct sigaction sa;
    sa.sa_handler = handle_alarm_direct; //cuando llegue SIGALRM se llamará a handle_alarm_direct
    sigemptyset(&sa.sa_mask); //máscara de señales mientras se ejecute el handler
    sa.sa_flags = SA_RESTART; //esto reduce la posibilidad de que wait4 falle con EINTR
    sigaction(SIGALRM, &sa, NULL); //instala el handler para SIGALRM

    clock_gettime(CLOCK_MONOTONIC, &start);

    child_pid = fork();
    if (child_pid == 0) {
        execvp(cmd[0], cmd);
        fprintf(stderr, "mishell: %s: comando no encontrado\n", cmd[0]);
        _exit(127);
    } 
    else if (child_pid > 0) {
        alarm(limite); //Después de 'limite' segundos el kernel enviará SIGALRM al proceso actual, lo que disparará handle_alarm_direct

        if (wait4(child_pid, &status, 0, &usage) == -1) {
            perror("wait4");
            return;
        }

        alarm(0); //Cancela cualquier alarma pendiente si el proceso hijo terminó antes del límite

        clock_gettime(CLOCK_MONOTONIC, &end);

        double real_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        double user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6;
        double sys_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;
        long max_mem = usage.ru_maxrss;

        //Si el hijo fue terminado por SIGKILL asumimos que fue por timeout
        if (WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL) {
            printf("\n--- miprof maxtiempo ---\n");
            printf("El comando excedió %d segundos y fue terminado.\n", limite);
        } 
        else {
            printf("\n--- Resultados miprof maxtiempo ---\n");
            printf("Tiempo real   : %.6f s\n", real_time);
            printf("Tiempo usuario: %.6f s\n", user_time);
            printf("Tiempo sistema: %.6f s\n", sys_time);
            printf("Memoria max   : %ld KB\n", max_mem);
        }

        child_pid = -1; //limpia la variable global child_pid asignándole -1
    } else {
        perror("fork");
    }
}

int main() {
    char input[MAX_INPUT];
    char input_copy[MAX_INPUT];
    char *comandos[MAX_CMDS];
    char *args[MAX_ARGS];

    while (1) {
        printf("mishell:$ ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        // Quitar salto de línea
        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0){
            continue;
        }
        if (strcmp(input, "exit") == 0){
            break;
        } 

        // Hacemos una copia de input ANTES de usar strtok
        strncpy(input_copy, input, MAX_INPUT);

        // separar por pipes
        int count_comandos = 0;
        char *token = strtok(input, "|");
        while (token != NULL && count_comandos < MAX_CMDS) {
            while (*token == ' '){
                token++;
            } // quitar espacios iniciales
            comandos[count_comandos++] = token;
            token = strtok(NULL, "|");
        }

        if (count_comandos == 0) continue;

        // parsear el primer comando de la copia original
        parse_command(input_copy, args);

        // manejar miprof
        if (args[0] && strcmp(args[0], "miprof") == 0) {
            if (args[1] && (strcmp(args[1], "ejec") == 0 || strcmp(args[1], "ejecsave") == 0)) {
                run_miprof(args);
                continue;
            } 
            else if (args[1] && strcmp(args[1], "maxtiempo") == 0) {
                run_miprof_maxtiempo(args);
                continue;
            }
        }

        // crear pipes si hay más de un comando
        int pipes[MAX_CMDS - 1][2];
        for (int i = 0; i < count_comandos - 1; i++) {
            if (pipe(pipes[i]) < 0) {
                perror("pipe");
                exit(1);
            }
        }

        // fork para cada comando
        for (int i = 0; i < count_comandos; i++) {
            pid_t pid = fork();
            if (pid == 0) {
                if (i > 0){
                    dup2(pipes[i - 1][0], STDIN_FILENO);
                }
                if (i < count_comandos - 1){
                    dup2(pipes[i][1], STDOUT_FILENO);
                }

                // cerrar pipes
                for (int j = 0; j < count_comandos - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                // parsear este comando
                parse_command(comandos[i], args);

                execvp(args[0], args);
                perror("Error: comando ingresado no existe");
                exit(1);
            }
        }

        // cerrar en padre
        for (int i = 0; i < count_comandos - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        for (int i = 0; i < count_comandos; i++) {
            wait(NULL);
        }
    }
    return 0;
}
