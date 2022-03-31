#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

int main(int argc, char *argv[])
{
    int lc, td;
    char *host_id, *depth, *lucky_number;
    for (int i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-m") == 0)
            host_id = argv[i + 1];
        else if (strcmp(argv[i], "-d") == 0)
            depth = argv[i + 1];
        else if (strcmp(argv[i], "-l") == 0)
            lucky_number = argv[i + 1];
    }

    lc = atoi(lucky_number);
    td = atoi(depth);

    if (td == 0)
    {
        char read_fifo[20];
        char *write_fifo = "fifo_0.tmp";

        int fd0, fd1;

        sprintf(read_fifo, "fifo_%s.tmp", host_id);

        fd0 = open(read_fifo, O_RDONLY);
        fd1 = open(write_fifo, O_WRONLY);

        dup2(fd0, STDIN_FILENO);
        dup2(fd1, STDOUT_FILENO);
    }

    int pipe_rc[4][2];

    if (td < 2) //  host
    {
        for (int i = 0; i < 4; i++)
            if (pipe(pipe_rc[i]) == -1)
                fprintf(stderr, "fail to create pipe\n");

        if (fork() == 0) // write to 0, read from 1
        {
            dup2(pipe_rc[0][1], STDOUT_FILENO);
            dup2(pipe_rc[1][0], STDIN_FILENO);

            for (int i = 0; i < 4; i++)
                for (int j = 0; j <= 1; j++)
                    close(pipe_rc[i][j]);

            depth[0]++;
            execl("host", "./host", "-m", host_id, "-d", depth, "-l", lucky_number, 0);
        }
        else if (fork() == 0) // write to 2, read from 3
        {
            dup2(pipe_rc[2][1], STDOUT_FILENO);
            dup2(pipe_rc[3][0], STDIN_FILENO);
            for (int i = 0; i < 4; i++)
                for (int j = 0; j <= 1; j++)
                    close(pipe_rc[i][j]);

            depth[0]++;
            execl("host", "./host", "-m", host_id, "-d", depth, "-l", lucky_number, 0);
        }
        else // read from 0, 2 write to 1, 3
        {
            for (int i = 0; i < 4; i++)
                close(pipe_rc[i][!(i % 2)]);

            while (1)
            {
                int a, b, c, d, e, f, g, h;
                int cnt = 0, j = 0, k = 0, l = 0;
                char left_list[1024], right_list[1024], buf[1] = "0";

                if (td == 0)
                {
                    fscanf(stdin, "%d %d %d %d %d %d %d %d", &a, &b, &c, &d, &e, &f, &g, &h);
                    scanf("%*[^\n]%*c");

                    j = sprintf(left_list, "%d %d %d %d\n", a, b, c, d);
                    k = sprintf(right_list, "%d %d %d %d\n", e, f, g, h);
                }
                else if (td == 1)
                {
                    fscanf(stdin, "%d %d %d %d", &a, &b, &c, &d);
                    j = sprintf(left_list, "%d %d\n", a, b);
                    k = sprintf(right_list, "%d %d\n", c, d);
                }

                write(pipe_rc[1][1], left_list, j * sizeof(char));
                write(pipe_rc[3][1], right_list, k * sizeof(char));

                if (a == -1)
                {
                    wait(NULL);
                    wait(NULL);
                    exit(0);
                }

                FILE *fp0 = fdopen(pipe_rc[0][0], "r");
                FILE *fp2 = fdopen(pipe_rc[2][0], "r");

                int times[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

                for (int i = 0; i < 10; i++)
                {
                    int l_id, l_num, l_d, r_id, r_num, r_d;
                    fscanf(fp0, "%d %d", &l_id, &l_num);
                    fscanf(fp2, "%d %d", &r_id, &r_num);

                    l_d = abs(lc - l_num);
                    r_d = abs(lc - r_num);

                    if (td == 1)
                    {
                        if (l_d <= r_d)
                            fprintf(stdout, "%d %d\n", l_id, l_num);
                        else
                            fprintf(stdout, "%d %d\n", r_id, r_num);
                    }
                    else if (l_d <= r_d)
                        times[l_id] += 10;
                    else
                        times[r_id] += 10;

                    fflush(stdout);
                }

                if (td == 0)
                {
                    char tmp[1024];
                    int len = sprintf(tmp, "%s\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n",
                                      host_id, a, times[a], b, times[b], c, times[c], d, times[d], e, times[e], f, times[f], g, times[g], h, times[h]);
                    // fprintf(stderr, "%s\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n",
                    // host_id, a, times[a], b, times[b], c, times[c], d, times[d], e, times[e], f, times[f], g, times[g], h, times[h]);
                    write(1, tmp, len * sizeof(char));
                }
            }
        }
    }
    else // left node
    {
        while (1)
        {
            int pipe_lp[2][2];

            pipe(pipe_lp[0]);
            pipe(pipe_lp[1]);

            int num = 1, cnt = 0, j = 0, k = 0;
            char left_list[1024], right_list[1024];

            fscanf(stdin, "%s", &left_list);
            fscanf(stdin, "%s", &right_list);

            int e = atoi(left_list);
            if (e == -1)
                _exit(0);

            if (fork() == 0)
            {
                dup2(pipe_lp[0][1], STDOUT_FILENO);
                for (int i = 0; i < 2; i++)
                    for (int j = 0; j < 2; j++)
                        close(pipe_lp[i][j]);
                execl("player", "./player", "-n", left_list, 0);
            }
            else if (fork() == 0)
            {
                dup2(pipe_lp[1][1], STDOUT_FILENO);
                for (int i = 0; i < 2; i++)
                    for (int j = 0; j < 2; j++)
                        close(pipe_lp[i][j]);
                execl("player", "./player", "-n", right_list, 0);
            }
            else
            {
                close(pipe_lp[0][1]);
                close(pipe_lp[1][1]);

                FILE *fp0 = fdopen(pipe_lp[0][0], "r");
                FILE *fp1 = fdopen(pipe_lp[1][0], "r");

                for (int i = 0; i < 10; i++)
                {
                    int l_id, l_num, l_d, r_id, r_num, r_d;
                    fscanf(fp0, "%d %d", &l_id, &l_num);
                    fscanf(fp1, "%d %d", &r_id, &r_num);

                    l_d = abs(lc - l_num);
                    r_d = abs(lc - r_num);

                    if (l_d <= r_d)
                        fprintf(stdout, "%d %d\n", l_id, l_num);
                    else
                        fprintf(stdout, "%d %d\n", r_id, r_num);

                    fflush(stdout);
                }
            }
        }
    }

    return 0;
}