//when doing compile, gcc -O3 -o merge merge.c


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#define BUFF_SIZE 100
int readaline_and_out(FILE *fin, FILE *fout);
const char* reverseString(char *);

int
main(int argc, char *argv[])
{
    FILE *file1, *file2, *fout;
    int eof1 = 0, eof2 = 0;
    long line1 = 0, line2 = 0, lineout = 0;
    struct timeval before, after;
    int duration;
    int ret = 1;

    if (argc != 4) {
        fprintf(stderr, "usage: %s file1 file2 fout\n", argv[0]);
        goto leave0;
    }
    if ((file1 = fopen(argv[1], "rt")) == NULL) {
        perror(argv[1]);
        goto leave0;
    }
    if ((file2 = fopen(argv[2], "rt")) == NULL) {
        perror(argv[2]);
        goto leave1;
    }
    if ((fout = fopen(argv[3], "wt")) == NULL) {
        perror(argv[3]);
        goto leave2;
    }
    setvbuf(file1,NULL,_IOFBF,BUFF_SIZE);
    setvbuf(file2,NULL,_IOFBF,BUFF_SIZE);
    setvbuf(fout,NULL,_IOFBF,BUFF_SIZE);
    gettimeofday(&before, NULL);
    do {
        if (!eof1) {
            if (!readaline_and_out(file1, fout)) {
                line1++; lineout++;
            } else
                eof1 = 1;
        }
        if (!eof2) {
            if (!readaline_and_out(file2, fout)) {
                line2++; lineout++;
            } else
                eof2 = 1;
        }
    } while (!eof1 || !eof2);
    gettimeofday(&after, NULL);
    
    duration = (after.tv_sec - before.tv_sec) * 1000000 + (after.tv_usec - before.tv_usec);
    printf("Processing time = %d.%06d sec\n", duration / 1000000, duration % 1000000);
    printf("File1 = %ld, File2= %ld, Total = %ld Lines\n", line1, line2, lineout);
    ret = 0;
    
leave3:
    fclose(fout);
leave2:
    fclose(file2);
leave1:
    fclose(file1);
leave0:
    return ret; 
}

int
readaline_and_out(FILE *fin, FILE *fout)
{
    char* ptr;
    char str[BUFF_SIZE];
    size_t location;

    location = fread(str, 1, BUFF_SIZE, fin);
    
    if (location < BUFF_SIZE){
        fwrite(reverseString(str), 1, location, fout);
        return 1;
    }

    else {
        ptr = strchr(str, 0x0a);
        location = ptr - str+1;
        fwrite(reverseString(str), 1, location, fout);
        fseek(fin, -(BUFF_SIZE - location), SEEK_CUR);
        return 0;
    }
}

const char* reverseString(char* str)
{
    int i, j;
    char temp;
    i=j=0;

    j=strlen(str)-2;
    for (i=0; i<j; i++, j--)
    {
        temp=str[i];
        str[i]=str[j];
        str[j]=temp;
    }

    return str;
}



