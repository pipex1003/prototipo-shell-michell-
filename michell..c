#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARG_SIZE 100

void prompt() {
    printf("myshell> ");
}

int main() {
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARG_SIZE];
    char *token;
    int status;
    pid_t pid;

    while (1) {
        prompt();
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = 0;

        int i = 0;
        token = strtok(input, " ");
        while (token!= NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        if (strcmp(args[0], "exit") == 0) {
            break;
        }

        // Manejar el comando echo de manera especial
        if (strcmp(args[0], "echo") == 0) {
            // Construir el comando echo como una sola cadena
            char cmd[MAX_INPUT_SIZE];
            strcpy(cmd, "echo "); // Iniciar la cadena con "echo "
            for(int j = 1; args[j]!= NULL; j++) { // Iterar sobre los argumentos restantes
                strcat(cmd, args[j]); // Añadir cada argumento a la cadena
                if(args[j+1]!= NULL) { // Si hay más argumentos, añadir un espacio entre ellos
                    strcat(cmd, " ");
                }
            }
            system(cmd); // Ejecutar el comando echo con system()
        } else {
            pid = fork();
            if (pid == 0) {
                if (execvp(args[0], args) == -1) {
                    perror("myshell");
                }
                exit(EXIT_FAILURE);
            } else if (pid < 0) {
                perror("myshell");
            } else {
                do {
                    waitpid(pid, &status, WUNTRACED);
                } while (!WIFEXITED(status) &&WIFSIGNALED(status));
            }
        }
    }

    return 0;
}
