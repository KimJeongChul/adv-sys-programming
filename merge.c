#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

char
*strrev(char *str);

int
main(int argc, char *argv[])
{
        int file1, file2;
        FILE *fout;
        long line1 = 0, line2 = 0, lineout = 0;
        struct timeval before, after;
        struct stat file1_stat, file2_stat;
        char *file1_buf = 0, *file2_buf = 0;
        char *file1_token, *file2_token, *file1_ptr, *file2_ptr;
        char *file1_str, *file2_str;
        int duration;
        int ret = 1;

        if (argc != 4) {
                fprintf(stderr, "usage: %s file1 file2 fout\n", argv[0]);
                goto leave0;
        }
        // file open
        if((file1 = open(argv[1], O_RDONLY)) < 0) {
                perror(argv[1]);
                goto leave0;
        }

        if((file2 = open(argv[2], O_RDONLY)) < 0) {
                perror(argv[2]);
                goto leave1;
        }

        if((fout = fopen(argv[3], "wt")) == NULL) {
                perror(argv[3]);
                goto leave2;
        }

        // fstat
        if(fstat(file1, &file1_stat) < 0 ) {
                fprintf(stderr, "Error : read file system information\n");
                goto leave3;
        }

        if(fstat(file2, &file2_stat) < 0 ) {
                fprintf(stderr, "Error : read file system information\n");
                goto leave3;
        }

        // buffer
        if((file1_buf = (char *) malloc(file1_stat.st_size)) == NULL) {
                fprintf(stderr, "Error : allocate file1 buffer\n");
                goto leave3;
        }

        if((file2_buf = (char *) malloc(file2_stat.st_size)) == NULL) {
                fprintf(stderr, "Error : allocate file2 buffer\n");
                goto leave4;
        }

        // read
        if(read(file1, file1_buf, malloc_usable_size(file1_buf)) < 1) {
                fprintf(stderr, "Error : read file 1\n");
                goto leave5;
        }

        if(read(file2, file2_buf, malloc_usable_size(file2_buf)) < 1) {
                fprintf(stderr, "Error : read file 1\n");
                goto leave3;
        }

        gettimeofday(&before, NULL);

        // write
        for(file1_str = file1_buf, file2_str = file2_buf; ; file1_str = NULL, file2_str = NULL) {
                file1_token = strtok_r(file1_str, "\n", &file1_ptr);
                file2_token = strtok_r(file2_str, "\n", &file2_ptr);

                if(file1_token != NULL) {
                        fprintf(fout,"%s\n",strrev(file1_token));
                        ++line1;
                }

                if(file2_token != NULL) {
                        fprintf(fout,"%s\n", strrev(file2_token));
                        ++line2;
                }

                if(file1_token == NULL && file2_token == NULL)
                        break;
        }

        gettimeofday(&after, NULL);
        lineout = line1 + line2;
        duration = (after.tv_sec - before.tv_sec) * 1000000 + (after.tv_usec - before.tv_usec);
        printf("Processing time = %d.%06d sec\n", duration / 1000000, duration % 1000000);
        printf("File1 = %ld, File2= %ld, Total = %ld Lines\n", line1, line2, lineout);
        ret = 0;

leave5:
        free(file1_buf);
leave4:
        free(file2_buf);
leave3:
        fclose(fout);
leave2:
        close(file2);
leave1:
        close(file1);
leave0:
        return ret;
}

char
*strrev(char *str)
{
        char tmp;
        char *start = str;
        char *end = str + strlen(str) - 1;
        while(start < end) {
                tmp = *start;
                *start = *end;
                *end = tmp;
                ++start;
                --end;
        }
        return str;
}