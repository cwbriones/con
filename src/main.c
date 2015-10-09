#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

int main(int argc, char** argv) {

    // Print version and exit information
    puts("con version 0.0.1");
    puts("Press Ctrl + C to Exit.\n");

    while (1) {
        char* input = readline("con> ");
        add_history(input);
        printf("%s\n", input);
        free(input);
    }

    return 0;
}
