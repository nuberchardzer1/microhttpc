#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "header.h"

void run_tests() {
    printf("\n--- Running tests ---\n");

    header_map_t map = {0};
    char *val;
    
    // TEST 1
    add_header(&map, "Host", "localhost:8080");
    val = get_header(&map, "Host");
    assert(val && strcmp(val, "localhost:8080") == 0);
    printf("✅ Test 1 passed\n");

    // TEST 2
    add_header(&map, "Content-Type", "text/plain");
    assert(get_header(&map, "Host"));
    assert(get_header(&map, "Content-Type"));
    printf("✅ Test 2 passed\n");

    // TEST 3
    add_header(&map, "Host", "example.com");
    val = get_header(&map, "Host");
    assert(strcmp(val, "example.com") == 0);
    printf("✅ Test 3 passed\n");

    // TEST 4
    assert(get_header(&map, "Authorization") == NULL);
    printf("✅ Test 4 passed\n");


    printf("\nAll tests passed! 🎉\n");
}

void collision_test(){
    printf("--- Collision test ---\n");

    header_map_t map = {0};
    
    add_header(&map, "Key-A", "A");
    add_header(&map, "Key-B", "B");
    add_header(&map, "Key-C", "C");

    char *a = get_header(&map, "Key-A");
    char *b = get_header(&map, "Key-B");
    char *c = get_header(&map, "Key-C");
    assert(a && b && c);
    assert(strcmp(a, "A") == 0);
    assert(strcmp(b, "B") == 0);
    assert(strcmp(c, "C") == 0);
    printf("✅ Test passed\n");
}

header_t *linear_search(header_t *arr, int n, const char *key) {
    for (int i = 0; i < n; i++)
        if (strcmp(arr[i].key, key) == 0)
            return &arr[i];
    return NULL;
}

double now_sec() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}


void benchmark_map(){
    srand(42);
    int h_header = 1024;
    int n_lookups = 1024;
    header_t arr[h_header];
    header_map_t map = {0};

    // Test data
    for (int i = 0; i < h_header; i++) {
        snprintf(arr[i].key, sizeof(arr[i].key), "Header-%d", i);
        snprintf(arr[i].val, sizeof(arr[i].val), "Value-%d", i);
        add_header(&map, arr[i].key, arr[i].val);
    }

    char keys[n_lookups][64];
    for (int i = 0; i < n_lookups; i++) {
        int id = rand() % h_header;
        snprintf(keys[i], sizeof(keys[i]), "Header-%d", id);
    }

    double start = now_sec();
    for (int i = 0; i < n_lookups; i++)
        linear_search(arr, h_header, keys[i]);
    double linear_time = now_sec() - start;

    start = now_sec();
    for (int i = 0; i < n_lookups; i++)
        get_header(&map, keys[i]);
    double hashmap_time = now_sec() - start;

    printf("\n===== Benchmark results =====\n");
    printf("Headers total:  %d\n", h_header);
    printf("Lookups:        %d\n", n_lookups);
    printf("Linear search:  %.6f sec\n", linear_time);
    printf("Hash map:       %.6f sec\n", hashmap_time);
    printf("Speedup:        %.1fx\n", linear_time / hashmap_time);
}