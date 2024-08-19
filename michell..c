include <stdio.h>      // Incluye la biblioteca estándar de entrada y salida.
#include <stdlib.h>     // Incluye la biblioteca estándar de utilidades.
#include <string.h>     // Incluye la biblioteca estándar de manipulación de cadenas.
#include <unistd.h>     // Incluye las funciones POSIX.
#include <sys/wait.h>   // Incluye las funciones para esperar procesos.
#include <sys/stat.h>   // Incluye las funciones de estado de archivos.
#include <fcntl.h>      // Incluye las funciones de control de archivos.
#include <dirent.h>     // Incluye las funciones para manipular directorios.

#define MAX_INPUT_SIZE 1024       // Define el tamaño máximo de la entrada del usuario.
#define BUFFER_SIZE 4096          // Define el tamaño del buffer para leer archivos.
#define MAX_ARG_SIZE 100          // Define el número máximo de argumentos.
char prev_dir[MAX_INPUT_SIZE];    // Variable global para almacenar el directorio anterior.

// Declaración de la función count_words antes de su uso
int count_words(const char *line);

// Función para contar líneas, palabras y caracteres en un archivo.
void wc(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("michell");
        return;
    }

    long lines = 0, words = 0, chars = 0;
    char line[256];

    while (fgets(line, sizeof(line), file)) {
        lines++;
        words += count_words(line);  // Ahora la función está declarada
        chars += strlen(line);
    }

    fclose(file);

    printf("Lines: %ld Words: %ld Characters: %ld\n", lines, words, chars);
}

// Función para contar palabras en una línea.
int count_words(const char *line) {
    const char *word = line;
    while (*word != '\0') {
        word = strpbrk(word, " \t\n");
        if (word != line) {
            word++;  // Incrementar word para evitar contar espacios en blanco
        }
    }
    return word - line;
}

// Función para mostrar el prompt.
void prompt() {
    printf("michell> ");
}

//funcion para buscar una palabra especifica en un archivo.
void grep(const char *pattern, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("michell");
        return;
    }

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, pattern) != NULL) {
            printf("%s", line);
        }
    }

    fclose(file);
}


// Función para copiar archivos.
void cp(char *src, char *dest) {
    int src_fd = open(src, O_RDONLY);
    if (src_fd < 0) {
        perror("michell");
        return;
    }

    int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (dest_fd < 0) {
        perror("michell");
        close(src_fd);
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes;
    while ((bytes = read(src_fd, buffer, sizeof(buffer))) > 0) {
        if (write(dest_fd, buffer, bytes) != bytes) {
            perror("michell");
            close(src_fd);
            close(dest_fd);
            return;
        }
    }

    if (bytes < 0) {
        perror("michell");
    }

    close(src_fd);
    close(dest_fd);
}

// Función para mover/renombrar archivos.
void mv(char *src, char *dest) {
    if (rename(src, dest) < 0) {
        perror("michell");
    }
}

// Función para crear un archivo.
void touch(char *filename) {
    int fd = open(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        perror("michell");
    } else {
        close(fd);
    }
}

// Función para mostrar el contenido de un archivo.
void cat(char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("michell");
        return;
    }

    char buffer[1024];
    ssize_t bytes;
    while ((bytes = read(fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytes);
    }

    close(fd);
}

// Función para imprimir texto, con posibilidad de redirigir a archivo.
void echo(int argc, char *argv[], int to_file, char *filename) {
    int fd = STDOUT_FILENO;

    if (to_file) {
        fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            perror("michell");
            return;
        }
    }

    for (int i = 1; i < argc; i++) {
        dprintf(fd, "%s ", argv[i]);
    }
    dprintf(fd, "\n");

    if (to_file) {
        close(fd);
    }
}

// Función para listar el contenido del directorio actual.
void ls() {
    DIR *d = opendir(".");
    if (d == NULL) {
        perror("michell");
        return;
    }

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        printf("%s ", dir->d_name);
    }
    printf("\n");

    closedir(d);
}

// Función para eliminar un archivo.
void rm(char *filename) {
    if (unlink(filename) < 0) {
        perror("michell");
    }
}

// Función para crear un directorio.
void mkdir_cmd(char *dirname) {
    if (mkdir(dirname, 0755) < 0) {
        perror("michell");
    }
}

// Función para eliminar un directorio.
void rmdir_cmd(char *dirname) {
    if (rmdir(dirname) < 0) {
        perror("michell");
    }
}

// Función para cambiar de directorio.
void cd(char *path) {
    if (chdir(path) < 0) {
        perror("michell");
    } else {
        char cwd[MAX_INPUT_SIZE];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Current directory: %s\n", cwd);
        } else {
            perror("myshell");
        }
    }
}

// Función para cambiar al directorio previo.
void cd_prev() {
    if (chdir(prev_dir) < 0) {
        perror("michell");
    }
}

// Función para mostrar el tamaño de un archivo.
void du(char *filename) {
    struct stat st;
    if (stat(filename, &st) == -1) {
        perror("michell");
        return;
    }

    printf("Size: %lld bytes\n", st.st_size);
}

// Función principal que maneja el bucle de la shell.
int main() {
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARG_SIZE];
    char *token;

    while (1) {
        prompt();
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = 0;

        int i = 0;
        token = strtok(input, " ");
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        if (args[0] == NULL) continue;

        if (strcmp(args[0], "exit") == 0) {
            break;
        }

        int to_file = 0;
        char *filename = NULL;
        for (int j = 0; j < i; j++) {
            if (strcmp(args[j], ">") == 0) {
                if (j + 1 < i) {
                    to_file = 1;
                    filename = args[j + 1];
                    args[j] = NULL;
                    break;
                } else {
                    fprintf(stderr, "myshell: syntax error near unexpected token `newline`\n");
                    to_file = -1;
                    break;
                }
            }
        }

        if (to_file == -1) continue;

        if (strcmp(args[0], "cp") == 0) {
            if (args[1] && args[2]) cp(args[1], args[2]);
            else fprintf(stderr, "michell: cp: missing file operand\n");
        } else if (strcmp(args[0], "mv") == 0) {
            if (args[1] && args[2]) mv(args[1], args[2]);
            else fprintf(stderr, "michell: mv: missing file operand\n");
        } else if (strcmp(args[0], "touch") == 0) {
            if (args[1]) touch(args[1]);
            else fprintf(stderr, "michell: touch: missing file operand\n");
        } else if (strcmp(args[0], "cat") == 0) {
            if (args[1]) cat(args[1]);
            else fprintf(stderr, "michell: cat: missing file operand\n");
        } else if (strcmp(args[0], "echo") == 0) {
            echo(i, args, to_file, filename);
        } else if (strcmp(args[0], "ls") == 0) {
            ls();
        } else if (strcmp(args[0], "rm") == 0) {
            if (args[1]) rm(args[1]);
            else fprintf(stderr, "michell: rm: missing file operand\n");
        } else if (strcmp(args[0], "mkdir") == 0) {
            if (args[1]) mkdir_cmd(args[1]);
            else fprintf(stderr, "michell: mkdir: missing directory operand\n");
        } else if (strcmp(args[0], "rmdir") == 0) {
            if (args[1]) rmdir_cmd(args[1]);
            else fprintf(stderr, "michell: rmdir: missing directory operand\n");
        } else if (strcmp(args[0], "cd") == 0) {
            if (args[1]) cd(args[1]);
            else if (strcmp(args[0], "cd.") == 0) {
                cd_prev();
            } else fprintf(stderr, "michell: cd: missing directory operand\n");
        } else if (strcmp(args[0], "wc") == 0) {
            if (args[1]) wc(args[1]);
            else fprintf(stderr, "michell: wc: missing file operand\n");
        } else if (strcmp(args[0], "du") == 0) {
            if (args[1]) du(args[1]);
            else fprintf(stderr, "michell: du: missing file operand\n");
        } else if (strcmp(args[0], "grep") == 0) {
            if (args[1] && args[2]) grep(args[1], args[2]);
            else fprintf(stderr, "michell: grep: missing pattern or file operand\n");
        } else {
            pid_t pid = fork();
            int status;
            if (pid == 0) {
                if (execvp(args[0], args) == -1) {
                    perror("michell");
                }
                exit(EXIT_FAILURE);
            } else if (pid < 0) {
                perror("michell");
            } else {
                do {
                    waitpid(pid, &status, WUNTRACED);
                } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            }
        }
    }

    return 0;
}

