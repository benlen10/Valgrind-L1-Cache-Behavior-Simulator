#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "cachelab.h"

#define ADDRESS_LENGTH 64


int verbosity = 0; /* print trace if set */
int s = 0;         /* set index bits */
int b = 0;         /* block offset bits */
int E = 0;         /* associativity */
char *trace_file = NULL;

int S; /* number of sets S = 2^s In C, you can use "pow" function*/
int B; /* block size (bytes) B = 2^b In C, you can use "pow" function*/

/* Counters used to record cache statistics */
int miss_count = 0;
int hit_count = 0;
int eviction_count = 0;
/*****************************************************************************/


typedef unsigned long long int mem_addr_t;


typedef struct cache_line
{
    char valid;
    mem_addr_t tag;
    int timestamp;
} cache_line_t;

typedef cache_line_t *cache_set_t;
typedef cache_set_t *cache_t;

/* The cache we are simulating */
cache_t cache;

//Custom global var
static int curTimestamp = 0;

void initCache()
{
    S = pow(2.0, s);
    B = pow(2.0, b);
    cache = malloc(sizeof(cache_t));

    //Allocate cache memory
    cache = malloc((S * sizeof(cache_line_t)));

    //Allocate 2D Matrix
    for (int set = 0; set < S; set++)
    {
        cache[set] = malloc(E * sizeof(cache_line_t));
        for(int block = 0; block<E; block++){
            cache[set][block].timestamp = 0;
            cache[set][block].tag = 0;
            cache[set][block].valid = '0';
        }
    }
}

void freeCache()
{
    //Free each set
    for (int set = 0; set < S; set++)
    {
        free(cache[set]);
    }

    //Free the original cache struct
    free(cache);
}


void accessData(mem_addr_t addr)
{

    //Parse tag and set values from the address
    int tagSize = (64 - (s + b));
    unsigned long long temp = addr << (tagSize);
    unsigned long long set = temp >> (tagSize + b);
    mem_addr_t tag = addr >> (s + b);

    //Search for existing block
    for (int block = 0; block < E; block++)
    {
        if ((cache[set][block].tag == tag) && (cache[set][block].valid == '1'))
        {
            hit_count++;
            return;
        }
    }

    //IF not found, attempt to find an open block in the set
    miss_count++;
    for (int bl = 0; bl < E; bl++)
    {
        if (cache[set][bl].valid != '1')
        {
            cache[set][bl].tag = tag;
            cache[set][bl].valid = '1';
            cache[set][bl].timestamp = curTimestamp++;
            return;
        }
    }
    eviction_count++;
    int minTimestamp = cache[set][0].timestamp;
    int minBlock = 0;

    //Locate the oldest cache block
    for (int x = 0; x < E; x++)
    {
        if (cache[set][x].timestamp < minTimestamp)
        {
            minTimestamp = cache[set][x].timestamp;
            minBlock = x;
        }
    }

    //Replace the oldest cache block
    cache[set][minBlock].timestamp = curTimestamp++;
    cache[set][minBlock].tag = tag;
}


void replayTrace(char *trace_fn)
{
    //Custom vars for verbose mode
    int curHit = 0;
    int curMiss = 0;
    int curEvict = 0;

    char buf[1000];
    mem_addr_t addr = 0;
    unsigned int len = 0;
    FILE *trace_fp = fopen(trace_fn, "r");

    if (!trace_fp)
    {
        fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
        exit(1);
    }

    while (fgets(buf, 1000, trace_fp) != NULL)
    {
        if (buf[1] == 'S' || buf[1] == 'L' || buf[1] == 'M')
        {
            sscanf(buf + 3, "%llx,%u", &addr, &len);

            if (verbosity)
                printf("%c %llx,%u ", buf[1], addr, len);

            if (buf[1] == 'M')
            {
                accessData(addr);
                accessData(addr);
            }
            else
            {
                accessData(addr);
            }

            if (verbosity)
                //Implement verbose feature
                if (miss_count > curMiss && eviction_count > curEvict)
                {
                    printf("miss eviction ");
                    curMiss = miss_count;
                    curEvict = eviction_count;
                }
            if (miss_count > curMiss)
            {
                printf("miss ");
                curMiss = miss_count;
            }
            if (hit_count == (curHit + 2))
            {
                printf("hit hit ");
                curHit = hit_count;
            }
            if (hit_count > curHit)
            {
                printf("hit ");
                curHit = hit_count;
            }
            printf("\n");
        }
    }

    fclose(trace_fp);
}

/*
 * printUsage - Print usage info
 */
void printUsage(char *argv[])
{
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}

/*
 * main - Main routine 
 */
int main(int argc, char *argv[])
{
    char c;

    // Parse the command line arguments: -h, -v, -s, -E, -b, -t
    while ((c = getopt(argc, argv, "s:E:b:t:vh")) != -1)
    {
        switch (c)
        {
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            trace_file = optarg;
            break;
        case 'v':
            verbosity = 1;
            break;
        case 'h':
            printUsage(argv);
            exit(0);
        default:
            printUsage(argv);
            exit(1);
        }
    }

    /* Make sure that all required command line args were specified */
    if (s == 0 || E == 0 || b == 0 || trace_file == NULL)
    {
        printf("%s: Missing required command line argument\n", argv[0]);
        printUsage(argv);
        exit(1);
    }

    /* Initialize cache */
    initCache();

#ifdef DEBUG_ON
    printf("DEBUG: S:%u E:%u B:%u trace:%s\n", S, E, B, trace_file);
#endif

    replayTrace(trace_file);

    /* Free allocated memory */
    freeCache();

    /* Output the hit and miss statistics for the autograder */
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}
