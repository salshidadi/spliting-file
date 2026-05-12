#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s <filename> <number>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];
    int count = atoi(argv[2]);

    printf("Opening file: %s\n", filename);
    printf("Dividing it %d times...\n", count);

    struct stat st;

    if (stat("photo.webp", &st) == 0)
    {
        printf("File size: %lld bytes\n", (long long)st.st_size);
    }

    void *ptr = malloc(st.st_size);
    FILE *fh = fopen("photo.webp", "rb");
    if (fh != NULL)
    {
        fread(ptr, st.st_size, 1, fh);
        fclose(fh);
    }

    int remainder = st.st_size % count;
    int block_size = (st.st_size-remainder) / count;

    for (int i = 0; i < count; i++)
    {

        char new_filename[1024];
        sprintf(new_filename, "%s.%d", filename, i);

        if (i == count - 1)
        {
            block_size += remainder;
        }

        FILE *fh = fopen(new_filename, "wb");
        if (fh != NULL)
        {
            fwrite(ptr, block_size, 1, fh);
            fclose(fh);
        }

        ptr += block_size;
    }

    return 0;
}