#ifndef MEMORY_SIM_H
#define MEMORY_SIM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MEMORY_SIZE (21 * 1024)     // 21KB
#define PAGE_SIZE (3 * 1024)        // 3KB
#define MAX_FRAMES (MEMORY_SIZE / PAGE_SIZE)  // 7 frames
#define MAX_PROCESSES 20
#define MAX_PAGES_PER_PROCESS 4   // 11KB / 3KB = 3.666 → 4 pages max

// Declare the input arrays without sizes
extern int inputP1Mem00[];
extern int inputP1Exec00[];
extern int inputP1Mem01[];
extern int inputP1Exec01[];
extern int inputP1Mem02[];
extern int inputP1Exec02[];
extern int inputP1Mem03[];
extern int inputP1Exec03[];
extern int inputP1Mem04[];
extern int inputP1Exec04[];
extern int inputP1Mem05[];
extern int inputP1Exec05[];

// Structure to represent a page
typedef struct {
    int process_id;
    int page_num;
    bool valid;
    int load_time;      // For FIFO
    int last_used_time; // For LRU
} PageFrame;

// Structure to represent a process
typedef struct {
    int id;
    int memory_size;
    int page_count;
    bool active;
    bool segfault;
    int last_fault_time;  // Track when the fault occurred
} Process;

// -- Function prototypes --
// > Helpers < //
int get_exec_count(const int exec_order[]); // Get execution count
int get_mem_input_size(const int mem_input[]); // Get memory input size
int get_page_number(int address);
// > Initialization < //
void initialize_memory();
void initialize_processes(const int mem_sizes[], int count);
// > Paging and frame loading < //
int find_free_frame();
int find_victim_fifo_for_process(int process_id);
int find_victim_lru_for_process(int process_id);
void load_page(const int frame, const int process_id, const int page_num);
void free_process_pages(const int process_id);
int allocate_page(const int process_id, const int address, const bool fifo);
// > Output < //
void print_memory_state(FILE *output_file);
// > Main loop < //
void run_simulation(int exec_order[], int exec_count, const bool fifo, const char *output_filename);

#endif // MEMORY_SIM_H