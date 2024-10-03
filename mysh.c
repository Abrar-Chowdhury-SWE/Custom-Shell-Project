#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <glob.h>
#include <stdbool.h>

#define MAX_ARG_SIZE 10000
#define MAX_INPUT_SIZE 1024
#define MAX_PATH_SIZE 4096

char *cmdArgs[MAX_ARG_SIZE];
int cmdArgCount;

int lastExitStatus;


// Function to check if a token is a control flow token (e.g., "then", "else", "&&", "||")
bool isControlFlowToken(const char *token);

// Function to copy a string and handle memory allocation
char *copyString(const char *s);

// Function to print the current working directory
void printWorkingDirectory();

// Function to print the executable path for a given program
void printExecutablePath(char *program);

// Function for interactive shell
void interactiveShell();

// Function for batch mode shell with a script file
void batchShell(char *scriptFileName);

// Function to find the executable path of a program in the system PATH
char *findExecutable(const char *program);

// Function to expand wildcard characters in command arguments
int expandWildcard(char *token, int index);

// Function to handle wildcard expansion in command arguments
int handleWildcards();

// Function to execute a command with optional input and output redirection
void executeCommand(int inputFd, int outputFd);

// Function to execute a sequence of piped commands
void executePipedCommands();

// Function to execute a command with input and output redirection
int executeCommandWithRedirection(int inputFd, int outputFd);

// Function to free memory allocated for cmdArgs
void freeCmdArgsMemory();

// Function to change the current working directory
void navigateDirectory(char *path);




bool isControlFlowToken(const char *token)
{
    return (strcmp(token, "then") == 0 ||
            strcmp(token, "else") == 0 ||
            strcmp(token, "&&") == 0 ||
            strcmp(token, "||") == 0);
}

char *copyString(const char *s) {
    char *copy = strdup(s);
    if (copy == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }
    return copy;
}


void printWorkingDirectory() {
    char *cwd = getcwd(NULL, 0); 
    if (cwd != NULL) {
        printf("%s\n", cwd);
        free(cwd); 
    } else {
        perror("getcwd() error");
    }
}

void printExecutablePath(char *program)
{
    if (program == NULL)
    {
        fprintf(stderr, "which: missing argument\n");
        lastExitStatus = 1;
        return;
    }

    char *path = findExecutable(program);
    if (path != NULL)
    {
        printf("%s\n", path);
        free(path);
    }
    else
    {
        fprintf(stderr, "which: %s not found in PATH\n", program);
        lastExitStatus = 1;
    }
}

void interactiveShell()
{
    printf("Welcome to custom shell!\n");
    char input[MAX_INPUT_SIZE];

    while (1)
    {
        printf("csh> ");
        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
        {
            printf("csh: exiting...\n");
            break;
        }
        else if (strncmp(input, "cd", 2) == 0)
        {
            char *path = strtok(input + 2, " ");
            if (path != NULL)
            {
                navigateDirectory(path);
            }
            else
            {
                fprintf(stderr, "cd: missing argument\n");
            }
            continue;
        }

        cmdArgCount = 0;
        char *token = strtok(input, " ");
        while (token != NULL)
        {
            cmdArgs[cmdArgCount++] = copyString(token);
            token = strtok(NULL, " ");
        }
        cmdArgs[cmdArgCount] = NULL;

        int inputFd = STDIN_FILENO;
        int outputFd = STDOUT_FILENO;

        for (int i = 0; i < cmdArgCount; ++i)
        {
            if (strcmp(cmdArgs[i], "<") == 0)
            {
                cmdArgs[i] = NULL;
                int fd = open(cmdArgs[i + 1], O_RDONLY);
                if (fd == -1)
                {
                    perror("Error opening input file");
                    break;
                }
                inputFd = fd;
                ++i;
            }
            else if (strcmp(cmdArgs[i], ">") == 0)
            {
                cmdArgs[i] = NULL;
                int fd = open(cmdArgs[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0640);
                if (fd == -1)
                {
                    perror("Error opening output file");
                    break;
                }
                outputFd = fd;
                ++i;
            }
            else if (strcmp(cmdArgs[i], "|") == 0)
            {
                executePipedCommands();
                break;
            }
        }

        executeCommand(inputFd, outputFd);

        for (int i = 0; i < cmdArgCount; ++i)
        {
            free(cmdArgs[i]);
        }
    }
}

void batchShell(char *scriptFileName)
{
    FILE *file = fopen(scriptFileName, "r");
    if (file == NULL)
    {
        perror("Error opening script file");
        exit(EXIT_FAILURE);
    }

    printf("Batch mode:\n");

    char line[MAX_INPUT_SIZE];
    while (fgets(line, sizeof(line), file) != NULL)
    {
        line[strcspn(line, "\n")] = '\0';

        printf("csh> %s\n", line);

        if (strcmp(line, "exit") == 0)
        {
            printf("csh: exiting...\n");
            break;
        }
        else if (strncmp(line, "cd", 2) == 0)
        {
            char *path = strtok(line + 2, " ");
            if (path != NULL)
            {
                navigateDirectory(path);
            }
            else
            {
                fprintf(stderr, "cd: missing argument\n");
            }
            continue;
        }

        cmdArgCount = 0;
        char *token = strtok(line, " ");
        while (token != NULL)
        {
            cmdArgs[cmdArgCount++] = copyString(token);
            token = strtok(NULL, " ");
        }
        cmdArgs[cmdArgCount] = NULL;

        int inputFd = STDIN_FILENO;
        int outputFd = STDOUT_FILENO;

        for (int i = 0; i < cmdArgCount; ++i)
        {
            if (strcmp(cmdArgs[i], "<") == 0)
            {
                cmdArgs[i] = NULL;
                int fd = open(cmdArgs[i + 1], O_RDONLY);
                if (fd == -1)
                {
                    perror("Error opening input file");
                    break;
                }
                inputFd = fd;
                ++i;
            }
            else if (strcmp(cmdArgs[i], ">") == 0)
            {
                cmdArgs[i] = NULL;
                int fd = open(cmdArgs[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0640);
                if (fd == -1)
                {
                    perror("Error opening output file");
                    break;
                }
                outputFd = fd;
                ++i;
            }
            else if (strcmp(cmdArgs[i], "|") == 0)
            {
                executePipedCommands();
                break;
            }
        }

        executeCommand(inputFd, outputFd);

        for (int i = 0; i < cmdArgCount; ++i)
        {
            free(cmdArgs[i]);
        }
    }

    fclose(file);
}

char *findExecutable(const char *program)
{
    char *path;
    char *directories[] = {"/usr/local/bin", "/usr/bin", "/bin", NULL};

    for (int i = 0; directories[i] != NULL; ++i)
    {
        path = malloc(MAX_PATH_SIZE);
        snprintf(path, MAX_PATH_SIZE, "%s/%s", directories[i], program);

        if (access(path, F_OK | X_OK) == 0)
        {
            return path;
        }

        free(path);
    }

    return NULL;
}

int expandWildcard(char *token, int index)
{
    glob_t globResult;
    int globStatus = glob(token, GLOB_NOCHECK, NULL, &globResult);
    int numFiles = globResult.gl_pathc;

    if (globStatus == 0)
    {
        for (int i = 0; i < numFiles; ++i)
        {
            cmdArgs[index++] = strdup(globResult.gl_pathv[i]);
        }

        globfree(&globResult);
        return 1;
    }
    else if (globStatus == GLOB_NOMATCH)
    {
        cmdArgs[index++] = strdup(token);
        return 0;
    }
    else
    {
        perror("Error expanding wildcard");
        exit(EXIT_FAILURE);
    }
}

int handleWildcards()
{
    int i = 0;
    int found = 0;

    while (cmdArgs[i] != NULL)
    {
        if (strchr(cmdArgs[i], '*') != NULL)
        {
            found = expandWildcard(cmdArgs[i], i);
        }
        ++i;
    }

    return found;
}

void executeCommand(int inputFd, int outputFd)
{
    int found = handleWildcards();

    if (cmdArgs[0] != NULL)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("Error forking process");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            if (inputFd != STDIN_FILENO)
            {
                dup2(inputFd, STDIN_FILENO);
                close(inputFd);
            }
            if (outputFd != STDOUT_FILENO)
            {
                dup2(outputFd, STDOUT_FILENO);
                close(outputFd);
            }

            int i = 0;
            while (cmdArgs[i] != NULL)
            {
                if (strcmp(cmdArgs[i], "|") == 0)
                {
                    int pipefd[2];
                    if (pipe(pipefd) == -1)
                    {
                        perror("Error creating pipe");
                        exit(EXIT_FAILURE);
                    }
                    dup2(pipefd[1], STDOUT_FILENO);
                    close(pipefd[0]);
                    close(pipefd[1]);
                    break;
                }
                ++i;
            }

            execvp(cmdArgs[0], cmdArgs);

            perror("Error executing command");
            exit(EXIT_FAILURE);
        }
        else
        {
            waitpid(pid, NULL, 0);

            if (found == 1)
            {
                for (int j = 0; j < cmdArgCount; ++j)
                {
                    free(cmdArgs[j]);
                }
            }
        }
    }
}

void executePipedCommands() {
    int numPipes = 0;
    for (int i = 0; i < cmdArgCount; ++i) {
        if (strcmp(cmdArgs[i], "|") == 0) {
            numPipes++;
        }
    }

    int pipefds[2 * numPipes];
    for (int i = 0; i < numPipes; ++i) {
        if (pipe(pipefds + i * 2) == -1) {
            perror("Error creating pipe");
            exit(EXIT_FAILURE);
        }
    }

    int j = 0; 
    int cmdIndex = 0; 
    for (int i = 0; i <= numPipes; ++i) {
        while (cmdArgs[j] != NULL && strcmp(cmdArgs[j], "|") != 0) {
            ++j;
        }
        cmdArgs[j] = NULL; 

        pid_t pid = fork();
        if (pid == 0) {
            // For all but the first command, get input from the previous pipe
            if (i != 0) {
                dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
            }

            // For all but the last command, output goes to the next pipe
            if (i != numPipes) {
                dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
            }

            for (int k = 0; k < 2 * numPipes; ++k) {
                close(pipefds[k]);
            }

            handleWildcards(cmdArgs + cmdIndex);
            execvp(cmdArgs[cmdIndex], cmdArgs + cmdIndex);
            perror("Error executing command");
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("Error forking process");
            exit(EXIT_FAILURE);
        }

        cmdIndex = ++j; 
    }

    for (int i = 0; i < 2 * numPipes; ++i) {
        close(pipefds[i]);
    }

    for (int i = 0; i <= numPipes; ++i) {
        wait(NULL);
    }
}

int executeCommandWithRedirection(int inputFd, int outputFd) {
    if (cmdArgs[0] == NULL) {
        return -1; 
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return -1;
    } else if (pid == 0) { 
        // Redirect standard input if necessary
        if (inputFd != STDIN_FILENO) {
            if (dup2(inputFd, STDIN_FILENO) == -1) {
                perror("dup2 inputFd");
                _exit(EXIT_FAILURE);
            }
            close(inputFd);
        }

        // Redirect standard output if necessary
        if (outputFd != STDOUT_FILENO) {
            if (dup2(outputFd, STDOUT_FILENO) == -1) {
                perror("dup2 outputFd");
                _exit(EXIT_FAILURE);
            }
            close(outputFd);
        }

        execvp(cmdArgs[0], cmdArgs);
        perror("execvp"); 
        _exit(EXIT_FAILURE);
    } else { // Parent process
        int status;
        waitpid(pid, &status, 0); 

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status); // Return the exit status of the child process
        } else {
            return -1;
        }
    }
}


void freeCmdArgsMemory()
{
    for (int i = 0; cmdArgs[i] != NULL; ++i)
    {
        free(cmdArgs[i]);
    }
}

void navigateDirectory(char *path)
{
    if (chdir(path) != 0)
    {
        perror("cd");
    }
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        if (isatty(STDIN_FILENO))
        {
            // Standard input is from a terminal - start in interactive mode
            interactiveShell();
        }
        else
        {
            // Standard input is not from a terminal - start in batch mode
            batchShell(NULL);
        }
    }
    else if (argc == 2)
    {
        // Read commands from the specified file in batch mode
        batchShell(argv[1]);
    }
    else
    {
        fprintf(stderr, "Usage: %s [script]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    freeCmdArgsMemory();

    return lastExitStatus;
}