#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>


int v, s, E, b, S, B, lines, mask;
char t[1000];	

int hit_count, miss_count, evict_count, timestamp;

struct cache_line {
    int valid;
    int tag;
    int stamp;
};

struct cache_line *cache;

void print_usage() {
	printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n"
		"Options:\n"
		"  -h         Print this help message.\n"
		"  -v         Optional verbose flag.\n"
		"  -s <num>   Number of set index bits.\n"
		"  -E <num>   Number of lines per set.\n"
		"  -b <num>   Number of block offset bits.\n"
		"  -t <file>  Trace file.\n\n"
		"Examples:\n"
		"  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n"
		"  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n"
	);
}

void init_cache() {
    S = 1 << s;
    B = 1 << b;
    lines = S * E;
    mask = S - 1;

    cache = (struct cache_line *)malloc(sizeof(struct cache_line) * lines);
    memset(cache, 0, sizeof(struct cache_line) * lines);
}

void update(unsigned long address) {
    unsigned long set_index_start = ((address >> b) & mask) * E ;
    unsigned long tag = address >> (b + s);
    
    for (int i = 0; i < E; i++) {
        int j = set_index_start + i;
        if (cache[j].valid && cache[j].tag == tag) {
            cache[j].stamp = timestamp++;
            ++hit_count;
            if (v) {
                printf("hit\n");
            }
            return;
        }
    }
   
    int victim_stamp = INT_MAX; 
    int victim_index = -1;
    ++miss_count;
    if (v) {
        printf("miss\n");
    }

    for (int i = 0; i < E; i++) {
        int j = set_index_start + i;
        if (cache[j].valid == 0) {
            cache[j].valid = 1; 
            cache[j].tag = tag;
            cache[j].stamp = timestamp++;
            return;
        } else if (cache[j].stamp < victim_stamp) {
            victim_stamp = cache[j].stamp;
            victim_index = j;
        }
    }
    ++evict_count; 
    if (v) {
        printf("evict\n");
    }
    cache[victim_index].tag = tag;
    cache[victim_index].stamp = timestamp++;
}

void parse_trace() {
	FILE *fp = fopen(t, "r");
	char type;
	unsigned long addr;
	unsigned int size;
	if (fp == NULL) {
		printf("open %s failed\n", t);
		exit(EXIT_FAILURE);
	}
	while (fscanf(fp, "%c %lxu,%u\n", &type, &addr, &size) > 0) {
        switch (type) {
        case 'L':
            update(addr);
            break;
        case 'M':
            update(addr);
        case 'S':
            update(addr);
        }
	}
	fclose(fp);
}

int main(int argc, char *argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
		switch (opt) {
		case 'h':
			print_usage();
			exit(EXIT_SUCCESS);	
		case 'v':
			v = 1;
			break;
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
			strcpy(t, optarg);
			break;
		default:
			exit(EXIT_FAILURE);
		}	
	}
    init_cache();
    parse_trace();
    free(cache);
    printSummary(hit_count, miss_count, evict_count);
    return 0;
}
