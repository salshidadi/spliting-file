#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

int write_random_block(const char *filename, size_t block_size)
{
    FILE *file = fopen(filename, "wb");
    if (!file)
        return 1;

    unsigned char *buffer = malloc(block_size);
    if (!buffer)
    {
        fclose(file);
        return 1;
    }

    for (size_t i = 0; i < block_size; i++)
    {
        buffer[i] = rand() % 256;
    }

    fwrite(buffer, block_size, 1, file);

    free(buffer);
    fclose(file);
    return 0;
}

struct pmd
{
    char st_file_name[1024];
    long block_size;
    int split_number;
    int split_block_size;
    int remainder;
    unsigned short random_blocks;
};

unsigned short generate_random_mask(int total_bits, int bits_to_set)
{
    unsigned short val = 0;
    while (__builtin_popcount(val) < bits_to_set)
    {
        val |= (1 << (rand() % total_bits));
    }
    return val;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage error! Missing basic arguments.\n");
        return 1;
    }

    char *command_type = argv[1];
    char *filename = argv[2];

    srand((unsigned int)time(NULL));

    if (strcmp(command_type, "-p") == 0)
    {
        if (argc < 6) {
            printf("Error: Missing splitting arguments.\n");
            return 1;
        }

        int split_number = atoi(argv[3]);
        char *special_operation = argv[4];
        int number_of_random_blocks = atoi(argv[5]);
        unsigned short random_block_index = generate_random_mask(split_number + number_of_random_blocks, number_of_random_blocks);

        if (split_number < 2 || split_number > 16)
        {
            printf("Range error (2-16).\n");
            return 1;
        }

        struct stat st;
        if (stat(filename, &st) == 0)
        {
            if (st.st_size > 1000000)
            {
                printf("File too large.\n");
                return 1;
            }
        }

        int remainder = st.st_size % split_number;
        int block_size = (st.st_size - remainder) / split_number;

        void *base_ptr = malloc(st.st_size);
        unsigned char *moving_ptr = (unsigned char *)base_ptr;

        FILE *fh = fopen(filename, "rb");
        if (fh != NULL && base_ptr != NULL)
        {
            fread(base_ptr, st.st_size, 1, fh);
            fclose(fh);
        }

        int total_parts = split_number + number_of_random_blocks;
        for (int i = 0; i < total_parts; i++)
        {
            char new_filename[1024];
            sprintf(new_filename, "%s.%d", filename, i);

            if (random_block_index & (1 << i))
            {
                write_random_block(new_filename, block_size);
            }
            else
            {
                int current_block_size = block_size;
                if (moving_ptr == (unsigned char *)base_ptr + (block_size * (split_number - 1)))
                {
                    current_block_size += remainder;
                }

                FILE *fh_part = fopen(new_filename, "wb");
                if (fh_part != NULL)
                {
                    fwrite(moving_ptr, current_block_size, 1, fh_part);
                    fclose(fh_part);
                }
                moving_ptr += current_block_size;
            }
        }

        free(base_ptr);

        char pmd_file_name[1024];
        sprintf(pmd_file_name, "%s.pmd", filename);

        struct pmd files_data;
        files_data.block_size = st.st_size;#include <stdio.h>
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

}
        strcpy(files_data.st_file_name, filename); 
        files_data.remainder = remainder;
        files_data.split_block_size = block_size;
        files_data.split_number = split_number;
        files_data.random_blocks = random_block_index; 

        FILE *fh_pmd = fopen(pmd_file_name, "wb");
        if (fh_pmd != NULL)
        {
            fwrite(&files_data, sizeof(files_data), 1, fh_pmd);
            fclose(fh_pmd);
        }
        printf("Splitting done.\n");
    }
    else if (strcmp(command_type, "-m") == 0)
    {
        int include_all_files = 0;
        char *result_file_name = "result.webp";

        if (argc >= 5 && strcmp(argv[3], "-R") == 0)
        {
            include_all_files = 1;
            result_file_name = argv[4];
        }

        struct pmd meta;
        FILE *fh_meta = fopen(filename, "rb");
        if (!fh_meta) return 1;
        fread(&meta, sizeof(struct pmd), 1, fh_meta);
        fclose(fh_meta);

        long total_allocation_size = include_all_files ? (meta.split_block_size * 32 + meta.remainder) : meta.block_size;
        void *merging_ptr = malloc(total_allocation_size);
        unsigned char *moving_struct_ptr = (unsigned char *)merging_ptr;

        int part_index = 0;
        int recovered_count = 0;
        long total_written = 0;

        for (int i = 0; i < 32; i++)
        {
            if (!include_all_files && recovered_count == meta.split_number)
                break;

            char part_name[1024];
            sprintf(part_name, "%s.%d", meta.st_file_name, part_index);

            FILE *fh_part = fopen(part_name, "rb");
            if (fh_part == NULL) {
                if (include_all_files && part_index > 32) break;
                part_index++;
                continue;
            }

            if (!include_all_files && (meta.random_blocks & (1 << part_index)))
            {
                fclose(fh_part);
                part_index++;
                continue; 
            }

            int current_size = meta.split_block_size;
            if (!include_all_files && (recovered_count == meta.split_number - 1))
            {
                current_size += meta.remainder;
            }

            fread(moving_struct_ptr, current_size, 1, fh_part);
            fclose(fh_part);

            moving_struct_ptr += current_size;
            total_written += current_size;
            recovered_count++;
            part_index++;
        }

        FILE *fh_result = fopen(result_file_name, "wb");
        if (fh_result)
        {
            fwrite(merging_ptr, total_written, 1, fh_result);
            fclose(fh_result);
            printf("Saved to: %s (%ld bytes)\n", result_file_name, total_written);
        }

        free(merging_ptr);
    }

    return 0;
}
