#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    char *player_id;
    for (int i = 1; i < argc; i += 2)
        if (strcmp(argv[i], "-n") == 0)
            player_id = argv[i + 1];

    int id = atoi(player_id);
    if (id != -1)
        for (int i = 1; i <= 10; i++)
        {
            int guess;
            srand((id + i) * 323);
            guess = rand() % 1001;

            char tmp[20];
            sprintf(tmp, "%d %d\n", id, guess);

            write(STDOUT_FILENO, tmp, strlen(tmp) * sizeof(char));
        }
    _exit(0);
}