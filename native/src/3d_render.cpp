//https://www.youtube.com/watch?v=HQYsFshbkYw
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dmsdk/sdk.h>
#include <math.h>


/* Define window size */
#define W 608
#define H 480
/* Define various vision related constants */
#define EyeHeight  6    // Camera height from floor when standing
#define DuckHeight 2.5  // And when crouching
#define HeadMargin 1    // How much room there is above camera before the head hits the ceiling
#define KneeHeight 2    // How tall obstacles the player can simply walk over without jumping
#define hfov (0.73f*H)  // Affects the horizontal field of vision
#define vfov (.2f*H)    // Affects the vertical field of vision

struct xy {float x,y;};

/* Sectors: Floor and ceiling height; list of edge vertices and neighbors */
static struct sector
{
    float floor, ceil;
    struct xy *vertex; //vertex have x,y coords
    signed char *neighbors; //Each edge may have a corresponding neighboring sector
    unsigned int npoints; //vertexes number
} *sectors = NULL;
static unsigned NumSectors = 0;

/*Player: location */
static struct player{
    struct xyz {float x,y,z;} where, velocity;
    float angle, anglesin, anglecos, yaw;//looking towards(sin() and cos() thereof) yaw is rotation of player?
    unsigned sector; //current sector
} player;

void UnloadLevel()
{
    for(unsigned a=0; a<NumSectors; ++a) free(sectors[a].vertex);
    for(unsigned a=0; a<NumSectors; ++a) free(sectors[a].neighbors);
    free(sectors);
    sectors    = NULL;
    NumSectors = 0;
}

void LoadLevel(char *data, uint32_t datasize){
    UnloadLevel();
    printf("load level");
    char *pch;
    char word[256], *ptr;
    struct xy *vert = NULL, v;
    int n, m, NumVertices = 0;
    pch = strtok (data,"\n");
     while (pch != NULL){
        ptr = pch;
        sscanf(ptr, "%32s%n", word, &n);
        switch(sscanf(ptr, "%32s%n", word, &n) == 1 ? word[0] : '\0'){
            case 'v': {// vertex
                for(sscanf(ptr += n, "%f%n", &v.y, &n); sscanf(ptr += n, "%f%n", &v.x, &n) == 1; )
                    { vert = (xy *)realloc(vert, ++NumVertices * sizeof(*vert)); vert[NumVertices-1] = v; }
                    break;}
            case 's': {// sector
                sectors = (sector *)realloc(sectors, ++NumSectors * sizeof(*sectors));
                 struct sector* sect = &sectors[NumSectors-1];
                 int* num = NULL;
                sscanf(ptr += n, "%f%f%n", &sect->floor,&sect->ceil, &n);
                for(m=0; sscanf(ptr += n, "%32s%n", word, &n) == 1 && word[0] != '#'; ){
                    num = (int *)realloc(num, ++m * sizeof(*num)); num[m-1] = word[0]=='x' ? -1 : atoi(word); }
                sect->npoints   = m /= 2;
                sect->neighbors = (signed char *)malloc( (m  ) * sizeof(*sect->neighbors) );
                sect->vertex    = (xy *) malloc( (m+1) * sizeof(*sect->vertex)    );
                for(n=0; n<m; ++n) sect->neighbors[n] = num[m + n];
                for(n=0; n<m; ++n) sect->vertex[n+1]  = vert[num[n]]; // TODO: Range checking
                sect->vertex[0] = sect->vertex[m]; // Ensure the vertexes form a loop
                free(num);
                break;}
            case 'p':{ // player
                float angle;
                sscanf(ptr += n, "%f %f %f %d", &v.x, &v.y, &angle,&n);
                player = (struct player) { {v.x, v.y, 0}, {0,0,0}, angle,0,0,0, n }; // TODO: Range checking
                player.where.z = sectors[player.sector].floor + EyeHeight;}
        }
        pch = strtok (NULL, "\n");
      }


    free(vert);
    printf("\nload done\n");
    printf("player: %f", player.where.z);
}

