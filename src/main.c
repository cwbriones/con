#include <stdio.h>

static char input[2048];

int main(int argc, char** argv) {

    // Print version and exit information
    puts("con version 0.0.1");
    puts("Press Ctrl + C to Exit.\n");

    while (1) {
        fputs("con> ", stdout);
        fgets(input, 2048, stdin);
        printf("%s", input);
    }

    return 0;
}
