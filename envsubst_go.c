#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc)
    {
        argc--;
        argv++;
    }

    char *input_file = NULL;
    char *output_file = NULL;

    while (argc > 0)
    {
        char *switc = *argv;
        argv++;
        argc--;

        int switch_len = strlen(switc);
        if (!switch_len)
            return 1;

        if (switc[0] != '-')
        {
            fprintf(stderr, "Expected switch, starting with -\n");
            return 1;
        }

        if (switch_len <= 1)
        {
            fprintf(stderr, "Missing switch after -\n");
            return 1;
        }

        if (argc <= 0)
        {
            fprintf(stderr, "Missing argument after switch -\n");
            return 1;
        }

        char *arg = *argv;
        argc--;
        argv++;

        switch (switc[1])
        {
        case 'i':
            if (input_file != NULL)
            {
                fprintf(stderr, "Input was already set (to %s)\n", input_file);
                return 1;
            }

            input_file = arg;
            break;
        case 'o':
            if (output_file != NULL)
            {
                fprintf(stderr, "Output was already set (to %s)\n", output_file);
                return 1;
            }

            output_file = arg;
            break;
        default:
            fprintf(stderr, "Unexpected switch %c\n", switc[1]);
            return 1;
        }
    }

    FILE *input_fd;
    if (input_file == NULL || !strcmp(input_file, "-"))
    {
        input_fd = stdin;
    }
    else
    {
        input_fd = fopen(input_file, "r");
        if (input_fd == NULL)
        {
            perror("fopen");
            return 1;
        }
    }

    FILE *output_fd;
    if (output_file == NULL || !strcmp(output_file, "-"))
    {
        output_fd = stdout;
    }
    else
    {
        output_fd = fopen(output_file, "wb");
        if (output_fd == NULL)
        {
            perror("fopen");
            return 1;
        }
    }

    bool is_dollar = false;
    bool just_dollar = false;
    bool is_par = false;

    char buf[4096];
    char name[257];
    int buf_ptr = 0;
    int buf_len = 0;

    int ptr = 0;
    int line = 1;
    int col = 1;

    for (;;)
    {
        if (buf_ptr >= buf_len)
        {
            buf_ptr = 0;
            buf_len = fread_unlocked(buf, 1, sizeof(buf), stdin);

            if (errno)
            {
                perror("fread");
                return 1;
            }

            if (buf_len == 0)
                break;
        }

        int c = buf[buf_ptr++];

        col++;
        if (c == '\n')
        {
            line++;
            col = 1;
        }

        if (is_dollar)
        {
            if (c == '$' && just_dollar) {
                is_dollar = false;
                putchar_unlocked('$');
                continue;
            }

            just_dollar = false;

            if (!is_par && c == '{')
            {
                is_par = true;
                continue;
            }

            if (!is_par && c == '}')
            {
                fprintf(stderr, "Unmatched } at %d:%d\n", line, col);
                return 1;
            }

            if (c == '}')
            {
                is_par = false;
            }

            if (isalnum(c) || c == '_')
            {
                if (ptr == sizeof(name) - 1)
                {
                    fprintf(stderr, "wat ze fuck do you think you are doing with your >256 env var name what ze fuck\n");
                    return 1;
                }

                name[ptr++] = c;
                continue;
            }
            else if (is_par)
            {
                fprintf(stderr, "Unmatched { at %d:%d\n", line, col);
                return 1;
            }

            if (ptr == 0)
            {
                fprintf(stderr, "Lone $ at %d:%d\n", line, col);
                return 1;
            }

            is_dollar = false;
            name[ptr] = '\0';

            char *env = getenv(name);
            if (env != NULL)
            {
                fputs(env, stdout);
            }
            else
            {
                fprintf(stderr, "Missing environment variable: %s\n", name);
                return 1;
            }

            ptr = 0;
            if (c == '}')
                continue;
        }

        if (c == '$')
        {
            is_dollar = true;
            just_dollar = true;
            continue;
        }

        putchar_unlocked(c);
    }

    if (is_dollar)
    {
        if (is_par)
        {
            fprintf(stderr, "Unmatched { at %d:%d\n", line, col);
            return 1;
        }

        if (ptr == 0)
        {
            fprintf(stderr, "Lone $ at %d:%d\n", line, col);
            return 1;
        }

        name[ptr] = '\0';

        char *env = getenv(name);
        if (env != NULL)
        {
            fputs(env, stdout);
        }
        else
        {
            fprintf(stderr, "Missing environment variable: %s\n", name);
            return 1;
        }

        ptr = 0;
    }

    return 0;
}