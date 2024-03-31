#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<getopt.h>

#define DEFAULT_BLOCK_SIZE 4096

void print_help_and_exit(int exit_code) {
    printf("Usage:\n");
    printf("./sparse-gen <output_file> [ ( -b | --block ) <size> ]\n");
    printf("./sparse-gen <input_file> <output_file> [ ( -b | --block ) <block_size> ]\n");
    printf("Print help message:\n");
    printf("./sparse-gen (--help | - h)\n");
    exit(exit_code);
}

char * duplicateStr(char * s) {
    size_t len = strlen(s);
    char * result = (char * ) malloc(len + 1);

    if (result == NULL) {
        fprintf(stderr, "Not enough memory\n");
        exit(EXIT_FAILURE);
    }

    strcpy(result, s);
    return result;
}

int open_file(char * filename, int flags, mode_t mode) {
    int fd = open(filename, flags, mode);
    if (fd == -1) {
        fprintf(stderr, "Cannot open '%s'\n", filename);
        exit(EXIT_FAILURE);
    }
    return fd;
}

void close_file(int fd, char * filename) {
    if (close(fd) == -1) {
        fprintf(stderr, "Cannot close '%s'\n", filename);
        exit(EXIT_FAILURE);
    }
}

void write_block(int fd, char * buffer, ssize_t count) {
    if (write(fd, buffer, count) != count) {
        fprintf(stderr, "I/O error while writing to output\n");
        exit(EXIT_FAILURE);
    }
}

void process_file(int fd_in, int fd_out, int block_size) {
    char * buffer = (char *)malloc(block_size);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory\n");
        exit(EXIT_FAILURE);
    }

    int n = 0;
    ssize_t count;

    while ((count = read(fd_in, buffer, block_size)) > 0) {
        n++;
        int nonZeroCharacter = 0;
        for (int i = 0; i < count; i++) {
            if (buffer[i]) {
                nonZeroCharacter = 1;
                break;
            }
        }

        if (nonZeroCharacter) {
            if (lseek(fd_out, block_size * (n - 1), SEEK_SET) == -1) {
                fprintf(stderr, "Cannot lseek in output file\n");
                exit(EXIT_FAILURE);
            }
            write_block(fd_out, buffer, count);
        }
    }

    if (count == -1) {
        fprintf(stderr, "I/O error while reading input file\n");
        exit(EXIT_FAILURE);
    }

    free(buffer);
}

int main(int argc, char * argv[]) {
    const struct option optarg_options[] = {
        {"block", 1, NULL, 'b'},
        {"help", 0, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    const char * const short_options = "-b:h";
    int next_option;
    int block_size = DEFAULT_BLOCK_SIZE;
    char * input_filename = NULL;
    char * output_filename = NULL;

    while ((next_option = getopt_long(argc, argv, short_options, optarg_options, NULL)) != -1) {
        switch (next_option) {
          case 'b':
              if (sscanf(optarg, "%d", &block_size) != 1 || block_size <= 0) {
                  fprintf(stderr, "Block size must be a positive integer\n");
                  exit(EXIT_FAILURE);
              }
              break;

          case 'h':
              print_help_and_exit(EXIT_SUCCESS);
              break;

          case 1:
              if (output_filename == NULL) {
                  output_filename = duplicateStr(optarg);
              } else if (input_filename == NULL) {
                  input_filename = output_filename;
                  output_filename = duplicateStr(optarg);
              } else {
                  fprintf(stderr, "Bad command line\n");
                  print_help_and_exit(EXIT_FAILURE);
              }
              break;

          case '?':
              fprintf(stderr, "Bad command line\n");
              print_help_and_exit(EXIT_FAILURE);
              break;
        }
    }

    if (output_filename == NULL) {
        fprintf(stderr, "Bad command line\n");
        print_help_and_exit(EXIT_FAILURE);
    }

    int fd_in = input_filename ? open_file(input_filename, O_RDONLY, 0) : STDIN_FILENO;
    int fd_out = open_file(output_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    process_file(fd_in, fd_out, block_size);

    if (input_filename) {
        close_file(fd_in, input_filename);
        free(input_filename);
    }

    close_file(fd_out, output_filename);
    free(output_filename);

    return 0;
}
