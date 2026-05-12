#include <stdio.h>
#include <stdlib.h>

struct football_player{
    short number;
    char prefered_foot;
    char position;
    int height;   
};

int main() {

    // Write to disk

    // void *ptr = malloc(sizeof(struct football_player));
    // struct football_player *sptr = (struct football_player*)ptr;

    // sptr ->number = 10;
    // sptr ->prefered_foot  = 'L';
    // sptr ->position = 'F';
    // sptr ->height = 177;

    // FILE *fh = fopen("player.fcb", "wb");
    // if(fh != NULL){
    //     fwrite(ptr, sizeof(struct football_player), 1, fh);
    //     fclose(fh);
    // }
    // free(ptr);

    // ------------------------------------------------------------------
    
    // Read from disk

    void *ptr = malloc(sizeof(struct football_player));
    struct football_player *sptr = (struct football_player*)ptr;

    FILE *fh = fopen("player.fcb", "rb");
    if(fh != NULL){
        fread(ptr, sizeof(struct football_player), 1, fh);
        fclose(fh);
    }

    printf("number: %d\nprefered foot: %c\nposition: %c\nheight: %d\n",
            sptr ->number, sptr->prefered_foot, sptr->position, sptr->height);
        free(ptr);            
    

    return 0;
}