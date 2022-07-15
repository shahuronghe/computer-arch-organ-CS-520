#include "project01.h"
#include <stdio.h>
#include <stdlib.h>

void stocking(char* req)
{
}
void cooling(char* req)
{
    stocking(req);
}
void baking(char* req)
{
    baking_limit++;
    if(baking_limit == 10){
        bakery_time++;
        baking_limit = 0;
    }
    cooling(req);
}
void proofing(char* req)
{
    baking(req);
}
void shaping(char* req)
{
    proofing(req);
}
void resting(char* req)
{
    shaping(req);
}
void rounding(char* req)
{
    resting(req);
}
void dividing(char* req)
{
    rounding(req);
}
void folding(char* req)
{
    dividing(req);
}
void fermentation(char* req)
{
    folding(req);
}
void mixing(char* req)
{
    fermentation(req);
}
void scaling(char* req)
{

    if (strcmp(req, "No-Request\n") == 0)
    {
        no_request++;
        //bakery_time++;     
    }
    
    if (strcmp(req, "Bake-Bagel\n") == 0)
    {
        bagel_baked++;
        bakery_time++;
        thousandCount++;
        mixing(req);

    }

    if (strcmp(req, "Bake-Baguette\n") == 0)
    {
        baguette_baked++;
        bakery_time+=2;
        thousandCount++;
        mixing(req);
    }
    if(thousandCount == 1000){
        thousandCount = 0;
        bakery_time +=10;
    }
}

void bake_it(char *req)
{
    scaling(req);
}

int main(int argc, char *argv[])
{
    // reading input file
    if (argc < 2)
    {
        fprintf(stdout, "no input file given\n");
        exit(1);
    }

    char *filename = argv[1];
    fprintf(stdout, "Baking INFO: Input Filename: %s \n", filename);

    char *line = NULL;
    size_t len = 0;
    size_t read;
    FILE *fp = fopen(filename, "r");
    while ((read = getline(&line, &len, fp)) != -1)
    {
        bake_it(line);
    }

    fclose(fp);

    // output formats
    baking_count = bagel_baked + baguette_baked;
    printf("Baking count: %d\n", baking_count);
    printf(" -Bagel baked: %d\n", bagel_baked);
    printf(" -Baguette baked: %d\n", baguette_baked);
    printf("No request: %d\n", no_request);
    printf("\nHow many minutes to bake: %d\n", bakery_time);
    performance = (double)baking_count / (double)bakery_time;
    printf("\nPerformance (bakes/minutes): %.2f\n", performance);
    return 0;
}