#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

int main(void)
{
    bool is_dollar = false;
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