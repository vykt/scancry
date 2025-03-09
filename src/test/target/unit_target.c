//standard library
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

//system includes
#include <unistd.h>
#include <sys/types.h>


#define P_LEN 4


//preset values
char * names[P_LEN] = {
    "alice",
    "bob",
    "chiara",
    "david"
};

int value_changes[P_LEN] = {1, 2, 3, 4};
pid_t parent_pid;


//structures
typedef struct {

    int32_t health, armour;
    
} stats;

typedef struct {

    float x, y, z;

} pos;

typedef struct {

    char name[16];
    stats * s;
    pos * p;
    
} entity;

typedef struct {

    entity * players[4];

} game;


//constructors
entity * new_entity(int idx) {

    //allocate
    entity * e = malloc(sizeof(entity));
    e->s = malloc(sizeof(stats));
    e->p = malloc(sizeof(pos));

    //assign
    strncpy(e->name, names[idx], strlen(names[idx]));
    e->s->health = e->s->armour = 100;

    e->p->x = (float) (rand() % 100);
    e->p->y = (float) (rand() % 100);
    e->p->z = (float) (rand() % 100);

    return e;
}

game * new_game() {

    game * g = malloc(sizeof(game));

    for (int i = 0; i < 4; ++i) {
        g->players[i] = new_entity(i);
    }

    return g;
}


//globals
game * G;


//unit test signal handler
void sigusr1_handler() {

    //modify players
    for (int i = 0; i < 4; ++i) {

        G->players[i]->s->health -= value_changes[i];
        G->players[i]->s->armour -= value_changes[i] * 2;

        G->players[i]->p->x += (float) value_changes[i];
        G->players[i]->p->y += (float) value_changes[i];
        G->players[i]->p->z += (float) value_changes[i];
    }

    //notify parent that change was performed
    kill(parent_pid, SIGUSR1);

    return;
}


//main
int main(int argc, char ** argv) {

    //check correct number of args is provided (quiet -Wunused-parameter)
    if (argc != 2) return -1;

    //recover parent pid
    parent_pid = atoi(argv[1]);

    //initialise global game
    G = new_game();

    //register unit test handler
    signal(SIGUSR1, sigusr1_handler);

    //signal parent that initialisation is finished
    kill(parent_pid, SIGUSR1);

    for (int i = 0; ++i; ) {

        //sleep for 10ms to not hoard the CPU
        usleep(100000);
    }

    return 0;
}
