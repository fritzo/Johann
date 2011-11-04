
#include <stdio.h>

int main()
{
    char c;

    do {
        c = getchar();
        printf (" = %i\n", c);
    } while (c != '\377'); //EOF = ctrl-d
}

