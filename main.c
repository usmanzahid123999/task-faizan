#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <errno.h>
#include <string.h>

#define MAX_COMMAND_LENGTH 100
#define MAX_ADDRESS_LENGTH 20
#define MAX_VARIABLE_NAME_LENGTH 50

void print_help() {
    printf("Debugger commands:\n");
    printf("b [address] - Set a breakpoint at the specified address\n");
    printf("s - Perform a single step\n");
    printf("c - Continue execution until the next breakpoint\n");
    printf("vap [address] - Print the contents of the variable at the specified address\n");
    printf("vnp [variable_name] - Print the contents of the variable with the specified name\n");
    printf("vas [address] [value] - Set the value of the variable at the specified address\n");
    printf("vns [variable_name] [value] - Set the value of the variable with the specified name\n");
    printf("br - Remove breakpoint\n");
    printf("help - Display this help message\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <program_name>\n", argv[0]);
        return 1;
    }

    pid_t child_pid;
    struct user_regs_struct regs;
    char command[MAX_COMMAND_LENGTH];
    long address;
    long value;

    // Fork the process
    child_pid = fork();

    if (child_pid == 0) {
        // Child process
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execvp(argv[1], argv + 1);
    } else if (child_pid > 0) {
        // Parent process
        wait(NULL);
        printf("Debugger started. Type 'help' for commands.\n");

        while (1) {
            printf("> ");
            fgets(command, sizeof(command), stdin);

            if (strncmp(command, "b ", 2) == 0) {
                // Set breakpoint
                sscanf(command + 2, "%lx", &address);
                printf("Breakpoint set at address 0x%lx\n", address);
                // Implement breakpoint logic
            } else if (strncmp(command, "s", 1) == 0) {
                // Perform single step
                printf("Single step\n");
                // Implement single step logic
            } else if (strncmp(command, "c", 1) == 0) {
                // Continue execution
                printf("Continue execution\n");
                // Implement continue logic
            } else if (strncmp(command, "vap ", 4) == 0) {
                // Print variable at address
                sscanf(command + 4, "%lx", &address);
                printf("Print variable at address 0x%lx\n", address);
                // Implement variable printing logic
            } else if (strncmp(command, "vnp ", 4) == 0) {
                // Print variable by name
                printf("Print variable by name\n");
                // Implement variable printing logic
            } else if (strncmp(command, "vas ", 4) == 0) {
                // Set variable at address
                sscanf(command + 4, "%lx %lx", &address, &value);
                printf("Set variable at address 0x%lx to %ld\n", address, value);
                // Implement variable setting logic
            } else if (strncmp(command, "vns ", 4) == 0) {
                // Set variable by name
                printf("Set variable by name\n");
                // Implement variable setting logic
            } else if (strncmp(command, "br", 2) == 0) {
                // Remove breakpoint
                printf("Remove breakpoint\n");
                // Implement breakpoint removal logic
            } else if (strncmp(command, "help", 4) == 0) {
                // Print help message
                print_help();
            } else {
                printf("Unknown command. Type 'help' for commands.\n");
            }
        }
    } else {
        perror("fork");
        return 1;
    }

    return 0;
}
