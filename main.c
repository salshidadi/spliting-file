#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

struct pmd
{
    char st_file_name[1024];
    long block_size;
    int split_number;
    int split_block_size;
    int remainder;
};

int main(int argc, char *argv[])
{

    char *command_type = argv[1];
    char *filename = argv[2];

    printf("Command type: %s\n", command_type);
    printf("Opening file: %s\n", filename);

    ////////////////////////////////////////////////////////
    if (strcmp(command_type, "-p") == 0)
    {
        int split_number = atoi(argv[3]);
        printf("split number: %d\n", split_number);
        if (split_number < 2 || split_number > 16)
        {
            printf("We only accept dividing the file in the range of 2 - 16 times, please enter number within this range\n");
            return 1;
        }

        printf("Dividing it %d times...\n", split_number);

        struct stat st;

        if (stat(filename, &st) == 0)
        {
            printf("File size: %lld bytes\n", (long long)st.st_size);
            if (st.st_size > 1000000)
            {
                printf("please enter a file name with a size less than 1,000,000 bytes\n");
                return 1;
            }
        }

        void *base_ptr = malloc(st.st_size);
        unsigned char *moving_ptr = (unsigned char *)base_ptr;

        FILE *fh = fopen(filename, "rb");
        if (fh != NULL && base_ptr != NULL)
        {
            fread(base_ptr, st.st_size, 1, fh);
            fclose(fh);
        }

        int remainder = st.st_size % split_number;
        int block_size = (st.st_size - remainder) / split_number;

        for (int i = 0; i < split_number; i++)
        {

            char new_filename[1024];
            sprintf(new_filename, "%s.%d", filename, i);

            if (i == split_number - 1)
            {
                block_size += remainder;
            }

            FILE *fh = fopen(new_filename, "wb");
            if (fh != NULL)
            {
                fwrite(moving_ptr, block_size, 1, fh);
                fclose(fh);
            }

            moving_ptr += block_size;
        }

        block_size -= remainder;

        free(base_ptr);
        base_ptr = NULL;

        char pmd_file_name[1024];
        sprintf(pmd_file_name, "%s.%s", filename, "pmd");

        struct pmd files_data;
        files_data.block_size = st.st_size;
        files_data.st_file_name;
        sprintf(files_data.st_file_name, "%s", filename);
        files_data.remainder = remainder;
        files_data.split_block_size = block_size;
        files_data.split_number = split_number;

        FILE *fh_pmd = fopen(pmd_file_name, "wb");
        if (fh_pmd != NULL)
        {
            fwrite(&files_data, sizeof(files_data), 1, fh_pmd);

            fclose(fh_pmd);
        }
    }
    else
    {

        char *special_operation = argv[3];
        int rotation_number = atoi(argv[4]);
        char *result_file_name = argv[5];

        printf("special operation: %s\n", special_operation);
        printf("rotation number: %d\n", rotation_number);
        printf("result file name: %s\n", result_file_name);

        void *new_base_ptr = malloc(sizeof(struct pmd));
        struct pmd *struct_ptr = (struct pmd *)new_base_ptr;

        struct stat st;

        if (stat(filename, &st) == 0)
        {
            printf("File size: %lld bytes\n", (long long)st.st_size);
            if (st.st_size > 1000000)
            {
                printf("please enter a file name with a size less than 1,000,000 bytes\n");
                return 1;
            }
        }

        FILE *fh = fopen(filename, "rb");
        if (fh != NULL && new_base_ptr != NULL)
        {
            fread(new_base_ptr, st.st_size, 1, fh);
            fclose(fh);
        }

        printf("------------------------ below is the pmd date------------------------------\n");
        printf("file name: %s\n block size : % d\n number of splits : %  d\n split block size : %d\n remainder : %d\n",
               struct_ptr->st_file_name,
               struct_ptr->block_size,
               struct_ptr->split_number,
               struct_ptr->split_block_size,
               struct_ptr->remainder);

        void *merging_ptr = malloc(struct_ptr->block_size);
        unsigned char *moving_struct_ptr = (unsigned char *)merging_ptr;

        int RN;
        if (special_operation == NULL)
        {
            RN = 0;
        }
        else
        {
            RN = rotation_number;
        }

        for (int i = 0; i < struct_ptr->split_number; i++)
        {
            char new_filename[1024];
            sprintf(new_filename, "%s.%d", struct_ptr->st_file_name, RN);

            if (i == (struct_ptr->split_number - 1))
            {
                struct_ptr->split_block_size += struct_ptr->remainder;
            }

            fh = fopen(new_filename, "rb");
            if (fh != NULL && merging_ptr != NULL)
            {
                fread(moving_struct_ptr, struct_ptr->split_block_size, 1, fh);
                fclose(fh);
            }
            moving_struct_ptr += struct_ptr->split_block_size;

            RN++;
            if (RN == struct_ptr->split_number)
            {
                RN = 0;
            }
        }

        if (result_file_name == NULL)
        {
            sprintf(result_file_name, "%s", "result.webp");
        }
        fh = fopen(result_file_name, "wb");
        if (fh != NULL)
        {
            fwrite(merging_ptr, struct_ptr->block_size, 1, fh);
            fclose(fh);
        }
    }

    return 0;
}