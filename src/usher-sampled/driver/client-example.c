#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
//Connect to socket and return a stdlib file handle corresponding to the connection
static FILE*  make_f(char* path){
    int sock_fd=socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd<0) {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_un addr;
    addr.sun_family=AF_UNIX;
    strncpy(addr.sun_path, path, 108);
    int ret=connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret<0) {
        perror("cannot connect");
        exit(EXIT_FAILURE);
    }

    FILE* ret_f=fdopen(sock_fd, "a+");
    if (!ret_f) {
        perror("cannot create stdio file handle");
        exit(EXIT_FAILURE);
    }
    return ret_f;
}

//send arguments over the socket
static void send_args(FILE* f, int argc, char** argv){
    for (int idx=1; idx<argc; idx++) {
        fprintf(f, "%s\n",argv[idx]);
    }
    fputc('\n',f);
}

#define ArraySize(arr) ( sizeof(arr) / sizeof (*arr) )

int main (int argc, char** argv){
    if (argc < 5) {
        fprintf(stderr,
            "usage: client-example socket_file samples.vcf tree.pb out_dir"
            " [--run-ripples"
            " [--ripples-branch-len <N>]"
            " [--ripples-num-desc <N>]"
            " [--ripples-ancestor-radius <N>]]\n");
        exit(1);
    }

    int run_ripples = 0;
    char *branch_len_val   = NULL;
    char *num_desc_val     = NULL;
    char *ancestor_rad_val = NULL;

    for (int i = 5; i < argc; i++) {
        if (strcmp(argv[i], "--run-ripples") == 0) {
            run_ripples = 1;
        } else if ((strcmp(argv[i], "--ripples-branch-len") == 0 || strcmp(argv[i], "-b") == 0) && i + 1 < argc) {
            branch_len_val = argv[++i];
        } else if ((strcmp(argv[i], "--ripples-num-desc") == 0 || strcmp(argv[i], "-n") == 0) && i + 1 < argc) {
            num_desc_val = argv[++i];
        } else if ((strcmp(argv[i], "--ripples-ancestor-radius") == 0 || strcmp(argv[i], "-a") == 0) && i + 1 < argc) {
            ancestor_rad_val = argv[++i];
        } else {
            fprintf(stderr, "unknown option: %s\n", argv[i]);
            exit(1);
        }
    }

    FILE* fh=make_f(argv[1]);

    char *cmd_base[] = { "ignored", "-v", argv[2], "-i", argv[3], "-d", argv[4],
                "-k", "500", "-u","--no-ignore-prefix","user_"};

    if (!run_ripples) {
        send_args(fh, ArraySize(cmd_base), cmd_base);
    } else {
        /* Build ripples command dynamically so optional params are only
           included when the user actually supplied them. */
        char *cmd_ripples[32];
        int n = 0;
        /* copy base args */
        for (int i = 0; i < (int)ArraySize(cmd_base); i++)
            cmd_ripples[n++] = cmd_base[i];
        cmd_ripples[n++] = "--run-ripples";
        if (branch_len_val) {
            cmd_ripples[n++] = "--ripples-branch-len";
            cmd_ripples[n++] = branch_len_val;
        }
        if (num_desc_val) {
            cmd_ripples[n++] = "--ripples-num-desc";
            cmd_ripples[n++] = num_desc_val;
        }
        if (ancestor_rad_val) {
            cmd_ripples[n++] = "--ripples-ancestor-radius";
            cmd_ripples[n++] = ancestor_rad_val;
        }
        send_args(fh, n, cmd_ripples);
    }
    char* line=NULL;
    size_t capacity=0;
    while (getline(&line, &capacity, fh)>0) {
        if (line[0]==4) {
            break;
        }
        puts(line);
    }
    free(line);
}
