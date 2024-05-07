#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <errno.h>
#include <string.h>
#include <elf.h>
#include <libelf.h>
#include <fcntl.h> // Added to resolve 'open' related issues
#include <libelf.h>
#include <gelf.h>


#define MAX_COMMAND_LENGTH 100
#define MAX_ADDRESS_LENGTH 20
#define MAX_VARIABLE_NAME_LENGTH 50

void print_help() {
    printf("Debugger commands:\n");
    printf("b [address] - Set a breakpoint at the specified address\n");
    printf("q - Exit from debugger\n");
    printf("s - Perform a single step\n");
    printf("c - Continue execution until the next breakpoint\n");
    printf("vap [address] - Print the contents of the variable at the specified address\n");
    printf("vnp [variable_name] - Print the contents of the variable with the specified name\n");
    printf("vas [address] [value] - Set the value of the variable at the specified address\n");
    printf("vns [variable_name] [value] - Set the value of the variable with the specified name\n");
    printf("br - Remove breakpoint\n");
    printf("help - Display this help message\n");
}

void set_breakpoint(pid_t child_pid, long address) {
    // Set breakpoint at the specified address
    ptrace(PTRACE_POKETEXT, child_pid, (void *)address, (void *)((ptrace(PTRACE_PEEKTEXT, child_pid, (void *)address, 0) & ~0xFF) | 0xCC));
}

void remove_breakpoint(pid_t child_pid, long address) {
    // Remove breakpoint at the specified address
    ptrace(PTRACE_POKETEXT, child_pid, (void *)address, (void *)(ptrace(PTRACE_PEEKTEXT, child_pid, (void *)address, 0) & ~0xFF));
}

void single_step(pid_t child_pid) {
    // Perform single step
    ptrace(PTRACE_SINGLESTEP, child_pid, NULL, NULL);
    wait(NULL); // Wait for child to finish single step
}

void continue_execution(pid_t child_pid) {
    // Continue execution
    ptrace(PTRACE_CONT, child_pid, NULL, NULL);
    wait(NULL); // Wait for child to reach next breakpoint
}

long get_variable_value(pid_t child_pid, long address) {
    // Get the value of variable at the specified address
    return ptrace(PTRACE_PEEKDATA, child_pid, (void *)address, NULL);
}

long find_variable_address(const char *variable_name, const char *executable_path) {
    int fd;
    Elf *elf;
    Elf_Scn *scn = NULL;
    GElf_Shdr shdr;
    Elf_Data *data;

    if (elf_version(EV_CURRENT) == EV_NONE) {
        fprintf(stderr, "libelf initialization failed: %s\n", elf_errmsg(-1));
        return 0;
    }

    if ((fd = open(executable_path, O_RDONLY, 0)) < 0) {
        perror("open");
        return 0;
    }

    if ((elf = elf_begin(fd, ELF_C_READ, NULL)) == NULL) {
        fprintf(stderr, "elf_begin() failed: %s\n", elf_errmsg(-1));
        close(fd);
        return 0;
    }

    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        if (gelf_getshdr(scn, &shdr) != &shdr) {
            fprintf(stderr, "getshdr() failed: %s\n", elf_errmsg(-1));
            elf_end(elf);
            close(fd);
            return 0;
        }

        if (shdr.sh_type == SHT_SYMTAB || shdr.sh_type == SHT_DYNSYM) {
            data = elf_getdata(scn, NULL);
            if (data == NULL) {
                fprintf(stderr, "elf_getdata() failed: %s\n", elf_errmsg(-1));
                elf_end(elf);
                close(fd);
                return 0;
            }

            size_t symcount = shdr.sh_size / shdr.sh_entsize;
            for (size_t i = 0; i < symcount; ++i) {
                GElf_Sym sym;
                if (gelf_getsym(data, i, &sym) != &sym) {
                    fprintf(stderr, "gelf_getsym() failed: %s\n", elf_errmsg(-1));
                    elf_end(elf);
                    close(fd);
                    return 0;
                }

                char *name = elf_strptr(elf, shdr.sh_link, sym.st_name);
                if (name != NULL && strcmp(name, variable_name) == 0) {
                    elf_end(elf);
                    close(fd);
                    return sym.st_value;
                }
            }
        }
    }

    elf_end(elf);
    close(fd);
    return 0;
}

void print_variable_at_address(pid_t child_pid, long address) {
    // Print the value of variable at the specified address
    printf("Value at address 0x%lx: %ld\n", address, get_variable_value(child_pid, address));
}

void print_variable_by_name(pid_t child_pid, const char *variable_name) {
    // Print the value of variable by name
    long address = find_variable_address(variable_name, "/home/usman/ftask/program.c");
    if (address != 0) {
        printf("Value of variable '%s' at address 0x%lx: %ld\n", variable_name, address, get_variable_value(child_pid, address));
    } else {
        printf("Variable '%s' not found.\n", variable_name);
    }
}

void set_variable_at_address(pid_t child_pid, long address, long value) {
    // Set the value of variable at the specified address
    ptrace(PTRACE_POKEDATA, child_pid, (void *)address, (void *)value);
}

void set_variable_by_name(pid_t child_pid, const char *variable_name, long value) {
    // Set the value of variable by name
    long address = find_variable_address(variable_name, "program");
    if (address != 0) {
        set_variable_at_address(child_pid, address, value);
        printf("Value of variable '%s' at address 0x%lx set to %ld\n", variable_name, address, value);
    } else {
        printf("Variable '%s' not found.\n", variable_name);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <program_name>\n", argv[0]);
        return 1;
    }

    pid_t child_pid;
    char command[MAX_COMMAND_LENGTH];
    long address, value;
    
    // Fork the process
    child_pid = fork();

    if (child_pid == 0) {
        // Child process
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0) {
            perror("ptrace TRACEME");
            return 1;
        }
        if (execvp(argv[1], argv + 1) < 0) {
            perror("execvp");
            return 1;
        }
    } else if (child_pid > 0) {
        // Parent process
        wait(NULL);
        printf("Debugger started. Type 'help' for commands.\n");

        while (1) {
            printf("> ");
            fgets(command, sizeof(command), stdin);
            
            // Tokenize command
            char *token = strtok(command, " \n");
            if (token == NULL) {
                printf("Unknown command. Type 'help' for commands.\n");
                continue;
            }
          if (strcmp(token, "q") == 0) {
    // Quit the debugger
    printf("Exiting debugger.\n");
    break; // Exit the while loop
            } 

            if (strcmp(token, "b") == 0) {
                // Set breakpoint
                token = strtok(NULL, " \n");
                if (token == NULL) {
                    printf("Missing address argument for breakpoint command.\n");
                    continue;
                }
                address = strtol(token, NULL, 16);
                set_breakpoint(child_pid, address);
            } else if (strcmp(token, "s") == 0) {
                // Perform single step
                single_step(child_pid);
            } else if (strcmp(token, "c") == 0) {
                // Continue execution
                continue_execution(child_pid);
            } else if (strcmp(token, "vap") == 0) {
                // Print variable at address
                token = strtok(NULL, " \n");
                if (token == NULL) {
                    printf("Missing address argument for print variable at address command.\n");
                    continue;
                }
                address = strtol(token, NULL, 16);
                print_variable_at_address(child_pid, address);
            } else if (strcmp(token, "vnp") == 0) {
                // Print variable by name
                token = strtok(NULL, " \n");
                if (token == NULL) {
                    printf("Missing variable name argument for print variable by name command.\n");
                    continue;
                }
                print_variable_by_name(child_pid, token);
            } else if (strcmp(token, "vas") == 0) {
                // Set variable at address
                token = strtok(NULL, " \n");
                if (token == NULL) {
                    printf("Missing address argument for set variable at address command.\n");
                    continue;
                }
                address = strtol(token, NULL, 16);
                token = strtok(NULL, " \n");
                if (token == NULL) {
                    printf("Missing value argument for set variable at address command.\n");
                    continue;
                }
                value = strtol(token, NULL, 10);
                set_variable_at_address(child_pid, address, value);
            } else if (strcmp(token, "vns") == 0) {
                // Set variable by name
                token = strtok(NULL, " \n");
                if (token == NULL) {
                    printf("Missing variable name argument for set variable by name command.\n");
                    continue;
                }
                char variable_name[MAX_VARIABLE_NAME_LENGTH];
                strcpy(variable_name, token);
                token = strtok(NULL, " \n");
                if (token == NULL) {
                    printf("Missing value argument for set variable by name command.\n");
                    continue;
                }
                value = strtol(token, NULL, 10);
                set_variable_by_name(child_pid, variable_name, value);
            } else if (strcmp(token, "br") == 0) {
                // Remove breakpoint
                token = strtok(NULL, " \n");
                if (token == NULL) {
                    printf("Missing address argument for remove breakpoint command.\n");
                    continue;
                }
                address = strtol(token, NULL, 16);
                remove_breakpoint(child_pid, address);
            } else if (strcmp(token, "help") == 0) {
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
