#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

// ./e -p photo.webp 3 (Normal split and generate the .pmd file)
// ./e -p photo.webp 3 -R 2 (Split them with added 2 random block in random indexes)
// ./e -m photo.webp.pmd (merbe the original blocks only, the final result is the original file)
// ./e -m photo.webp.pmd -r 2 (merge them but after applying rotation)
// ./e -m photo.webp.pmd -R result.webp (merge the file with both original and fake block, name the result file as the input)

void write_random_block(const char *filename, size_t block_size)
{
    FILE *file = fopen(filename, "wb");
    if (!file)
        return;

    unsigned char *buffer = malloc(block_size);
    if (!buffer)
    {
        fclose(file);
        return;
    }

    for (size_t i = 0; i < block_size; i++)
    {
        buffer[i] = rand() % 256;
    }

    fwrite(buffer, block_size, 1, file);

    free(buffer);
    fclose(file);
}

unsigned short generate_random_mask(int total_bits, int bits_to_set)
{
    unsigned short val = 0;

    while (__builtin_popcount(val) < bits_to_set)
    {
        val |= (1 << (rand() % total_bits)); // val = val | (1<< random_number)
    }

    return val;
}

void read_file(char *filename, unsigned char *ptr, size_t block_size)
{
    FILE *fh = fopen(filename, "rb");
    if (fh != NULL && ptr != NULL)
    {
        fread(ptr, block_size, 1, fh);
        fclose(fh);
    }
}

void write_file(char *filename, unsigned char *ptr, size_t block_size)
{
    FILE *fh = fopen(filename, "wb");
    if (fh != NULL && ptr != NULL)
    {
        fwrite(ptr, block_size, 1, fh);
        fclose(fh);
    }
}

struct pmd
{
    char filename[1024];
    long file_size;
    int split_number;
    int block_size;
    int remainder;
    unsigned short random_blocks_index;
    int number_of_random_blooks;
};

long long get_file_size(char *filename)
{
    struct stat st;
    if (stat(filename, &st) == 0)
    {
        return st.st_size;
    }
    return -1;
}

int main(int argc, char *argv[])
{

    srand((unsigned int)time(NULL));

    char *operation_type = argv[1];
    char *filename = argv[2];

    printf("Operation type: %s\n", operation_type);
    printf("Opening file: %s\n", filename);

    if (strcmp(operation_type, "-p") == 0)
    {
        int split_number = atoi(argv[3]);

        char *special_operation = "";
        int number_of_random_blocks = 0;
        unsigned short random_block_index = 0;

        if (argc > 5)
        {
            special_operation = argv[4];
            number_of_random_blocks = atoi(argv[5]);
        }

        if ((split_number + number_of_random_blocks) > 16)
        {
            printf("We only accept dividing the file in the range of 2 - 16 times, please enter a number within this range\n");
            return 1;
        }

        if (number_of_random_blocks > 0)
        {
            random_block_index = generate_random_mask(split_number + number_of_random_blocks, number_of_random_blocks);
        }

        long long file_size = get_file_size(filename);
        int remainder = file_size % split_number;
        int block_size = (file_size - remainder) / split_number;

        printf("split number: %d\n", split_number);
        printf("special operation: %s\n", special_operation);
        printf("number of random blocks: %d\n", number_of_random_blocks);
        printf("random block index: %d\n", random_block_index);
        printf("Dividing it %d times...\n", split_number);
        printf("file size %lld\n", file_size);
        printf("block size %d\n", block_size);
        printf("remainder %d\n", remainder);

        void *base_ptr = malloc(file_size);
        unsigned char *moving_ptr = (unsigned char *)base_ptr;

        read_file(filename, base_ptr, file_size);

        int original_blocks_counter = split_number;
        for (int i = 0; i < (split_number + number_of_random_blocks); i++)
        {
            char new_filename[1024];
            sprintf(new_filename, "%s.%d", filename, i);

            if (random_block_index & (1 << i))
            {
                write_random_block(new_filename, block_size);
            }
            else
            {
                if (original_blocks_counter == 1)
                {
                    block_size += remainder;
                }
                original_blocks_counter--;
                write_file(new_filename, moving_ptr, block_size);
                moving_ptr += block_size;
                if (original_blocks_counter == 0)
                {
                    block_size -= remainder;
                }
            }
        }

        free(base_ptr);
        base_ptr = NULL;
        moving_ptr = NULL;

        char pmd_filename[1024];
        sprintf(pmd_filename, "%s.%s", filename, "pmd");

        struct pmd files_data;

        files_data.file_size = file_size;
        sprintf(files_data.filename, "%s", filename);
        files_data.remainder = remainder;
        files_data.block_size = block_size;
        files_data.split_number = split_number;
        files_data.random_blocks_index = random_block_index;
        files_data.number_of_random_blooks = number_of_random_blocks;

        write_file(pmd_filename, &files_data, sizeof(files_data));
    }
    else
    {
        long long pmd_file_size = get_file_size(filename);

        void *base_ptr_pmd = malloc(pmd_file_size);
        struct pmd *struct_ptr = (struct pmd *)base_ptr_pmd;

        read_file(filename, base_ptr_pmd, pmd_file_size);

        char *special_operation = "";
        int rotation_number = 0;
        char *output_filename = struct_ptr->filename;

        if (argc > 3)
        {
            special_operation = argv[3];
            if (strcmp(special_operation, "-r") == 0 && argc > 4)
            {
                rotation_number = atoi(argv[4]);
            }
            else if (strcmp(special_operation, "-R") == 0 && argc > 4)
            {
                output_filename = argv[4];
            }
        }

        printf("special operation: %s\n", special_operation);
        printf("rotation number: %d\n", rotation_number);
        printf("output file name: %s\n", output_filename);

        printf("------------------------ below is the pmd date------------------------------\n");
        printf("file name: %s\n file size : % d\n number of splits : %  d\n  block size : %d\n remainder : %d\n random index: %d\n number of random block: %d\n",
               struct_ptr->filename,
               struct_ptr->file_size,
               struct_ptr->split_number,
               struct_ptr->block_size,
               struct_ptr->remainder,
               struct_ptr->random_blocks_index,
               struct_ptr->number_of_random_blooks);

        void *base_ptr_merging = malloc(struct_ptr->file_size + (struct_ptr->block_size * struct_ptr->number_of_random_blooks));
        unsigned char *moving_ptr_merging = (unsigned char *)base_ptr_merging;

        int heavy_id = -1;
        int total_blocks = struct_ptr->split_number + struct_ptr->number_of_random_blooks;
        for (int i = total_blocks - 1; i >= 0; i--)
        {
            if (!((struct_ptr->random_blocks_index) & (1 << i)))
            {
                heavy_id = i;
                break;
            }
        }

        if (strcmp(special_operation, "-R") == 0)
        {
            for (int i = 0; i < total_blocks; i++)
            {
                char new_filename[1024];
                sprintf(new_filename, "%s.%d", struct_ptr->filename, i);

                if (i == heavy_id)
                {
                    struct_ptr->block_size += struct_ptr->remainder;
                }

                read_file(new_filename, moving_ptr_merging, struct_ptr->block_size);
                moving_ptr_merging += struct_ptr->block_size;

                if (i == heavy_id)
                {
                    struct_ptr->block_size -= struct_ptr->remainder;
                }
            }
        }
        else if (strcmp(special_operation, "-r") == 0)
        {
            for (int i = 0; i < total_blocks; i++)
            {
                char new_filename[1024];
                sprintf(new_filename, "%s.%d", struct_ptr->filename, rotation_number);

                if (rotation_number == heavy_id)
                {
                    struct_ptr->block_size += struct_ptr->remainder;
                }

                read_file(new_filename, moving_ptr_merging, struct_ptr->block_size);
                moving_ptr_merging += struct_ptr->block_size;

                if (rotation_number == heavy_id)
                {
                    struct_ptr->block_size -= struct_ptr->remainder;
                }
                if (rotation_number == (total_blocks - 1))
                {
                    rotation_number = -1;
                }
                rotation_number++;
            }
        }
        else
        {
            int original_blocks_counter = struct_ptr->split_number;
            for (int i = 0; i < (struct_ptr->split_number + struct_ptr->number_of_random_blooks); i++)
            {
                char new_filename[1024];
                sprintf(new_filename, "%s.%d", struct_ptr->filename, i);

                if ((struct_ptr->random_blocks_index) & (1 << i))
                {
                    continue;
                }
                else
                {
                    if (original_blocks_counter == 1)
                    {
                        struct_ptr->block_size += struct_ptr->remainder;
                    }
                    original_blocks_counter--;
                    read_file(new_filename, moving_ptr_merging, struct_ptr->block_size);
                    moving_ptr_merging += struct_ptr->block_size;
                }
            }
        }

        write_file(output_filename, base_ptr_merging, struct_ptr->file_size + (struct_ptr->block_size * struct_ptr->number_of_random_blooks));

        free(base_ptr_merging);
        free(base_ptr_pmd);
    }

    return 0;
}
