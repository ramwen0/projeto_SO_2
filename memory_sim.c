#include "memory_sim.h"

// Global variables
PageFrame memory[MAX_FRAMES];
Process processes[MAX_PROCESSES];
int current_time = 0;
int total_processes = 0;

// Helper function to get execution count without using sizeof
int get_exec_count(const int exec_order[]) {
    if (exec_order == inputP1Exec00) return 24;
    if (exec_order == inputP1Exec01) return 12;
    if (exec_order == inputP1Exec02) return 18;
    if (exec_order == inputP1Exec03) return 26;
    if (exec_order == inputP1Exec04) return 100;
    if (exec_order == inputP1Exec05) return 60;
    return 0;
}

// Helper function to get memory input size
int get_mem_input_size(const int mem_input[]) {
    if (mem_input == inputP1Mem00) return 5;
    if (mem_input == inputP1Mem01) return 5;
    if (mem_input == inputP1Mem02) return 5;
    if (mem_input == inputP1Mem03) return 10;
    if (mem_input == inputP1Mem04) return 20;
    if (mem_input == inputP1Mem05) return 3;
    return 0;
}

int get_page_number(int address) {
    return address / PAGE_SIZE;
}

void initialize_memory() {
    for (int i = 0; i < MAX_FRAMES; i++) {
        memory[i].valid = false;
        memory[i].process_id = -1;
        memory[i].page_num = -1;
        memory[i].load_time = -1;
        memory[i].last_used_time = -1;
    }
}

void initialize_processes(const int mem_sizes[], int count) {
    total_processes = count;
    for (int i = 0; i < count; i++) {
        processes[i].id = i + 1;
        processes[i].memory_size = mem_sizes[i];
        processes[i].page_count = (mem_sizes[i] + PAGE_SIZE - 1) / PAGE_SIZE;
        processes[i].active = true;
        processes[i].segfault = false;
    }
}

int find_free_frame() {
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (!memory[i].valid) {
            return i;
        }
    }
    return -1;
}

int find_victim_fifo_for_process(int process_id) {
    int oldest_time = current_time + 1;
    int victim_frame = -1;

    for (int i = 0; i < MAX_FRAMES; i++) {
        if (memory[i].valid && memory[i].process_id == process_id) {
            if (memory[i].load_time < oldest_time) {
                oldest_time = memory[i].load_time;
                victim_frame = i;
            }
        }
    }
    return victim_frame;
}

int find_victim_lru_for_process(int process_id) {
    int least_recent_time = current_time + 1;
    int victim_frame = -1;
    int victim_page = MAX_PAGES_PER_PROCESS + 1;
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (memory[i].valid && memory[i].process_id == process_id) {
            if (memory[i].last_used_time < least_recent_time ||
                (memory[i].last_used_time == least_recent_time && memory[i].page_num < victim_page)) {
                least_recent_time = memory[i].last_used_time;
                victim_page = memory[i].page_num;
                victim_frame = i;
                }
        }
    }
    return victim_frame;
}

void load_page(const int frame, const int process_id, const int page_num) {
    memory[frame].valid = true;
    memory[frame].process_id = process_id;
    memory[frame].page_num = page_num;
    memory[frame].load_time = current_time;
    memory[frame].last_used_time = current_time;
}

void free_process_pages(const int process_id) {
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (memory[i].valid && memory[i].process_id == process_id) {
            memory[i].valid = false;
        }
    }
}

int allocate_page(const int process_id, const int address, const bool fifo) {
    const int process_idx = process_id - 1;
    if (process_idx < 0 || process_idx >= total_processes) return -1;
    if (address >= processes[process_idx].memory_size) {
        processes[process_idx].segfault = true;
        processes[process_idx].active = false;
        processes[process_idx].last_fault_time = current_time;
        free_process_pages(process_id);
        return -2;
    }
    const int page_num = get_page_number(address);

    // Check if page is already loaded
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (memory[i].valid && memory[i].process_id == process_id &&
            memory[i].page_num == page_num) {
            memory[i].last_used_time = current_time;  // For LRU
            return i;
            }
    }

    // Try to find a free frame first
    int free_frame = find_free_frame();
    if (free_frame != -1) {
        load_page(free_frame, process_id, page_num);
        return free_frame;
    }

    // If no free frames, check if process has reached its page limit
    int pages_in_mem = 0;
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (memory[i].valid && memory[i].process_id == process_id) {
            pages_in_mem++;
        }
    }

    if (pages_in_mem >= processes[process_idx].page_count) {
        // Find victim using FIFO policy
        int victim_frame = find_victim_fifo_for_process(process_id);
        if (victim_frame != -1) {
            load_page(victim_frame, process_id, page_num);
            return victim_frame;
        }
    }

    return -1;
}

void print_memory_state(FILE *output_file) {
    if (!output_file) return;

    fprintf(output_file, "\n%-8d  ", current_time);

    for (int i = 0; i < total_processes; i++) {
        if (processes[i].segfault && processes[i].last_fault_time == current_time) {
            fprintf(output_file, "%-18s ", "SIGSEGV");
            continue;
        }
        if (!processes[i].active) {
            fprintf(output_file, "%-18s ", "");  // Process is inactive, show blank
            continue;
        }

        char buffer[128] = "";
        int idx = 0;

        // Collect valid frames for this process
        for (int j = 0; j < MAX_FRAMES; j++) {
            if (memory[j].valid && memory[j].process_id == processes[i].id) {
                if (idx > 0) strcat(buffer, ",");
                char frame_str[10];
                snprintf(frame_str, sizeof(frame_str), "F%d", j);
                strcat(buffer, frame_str);
                idx++;
            }
        }

        fprintf(output_file, "%-18s ", (idx > 0) ? buffer : "");
    }
}

void run_simulation(int exec_order[], int exec_count, const bool fifo, const char *output_filename) {
    // Reset global state
    initialize_memory();
    current_time = 0;

    // Determine which inputP1MemXX to use based on the exec_order
    const int* mem_input = NULL;
    if (exec_order == inputP1Exec00) mem_input = inputP1Mem00;
    else if (exec_order == inputP1Exec01) mem_input = inputP1Mem01;
    else if (exec_order == inputP1Exec02) mem_input = inputP1Mem02;
    else if (exec_order == inputP1Exec03) mem_input = inputP1Mem03;
    else if (exec_order == inputP1Exec04) mem_input = inputP1Mem04;
    else if (exec_order == inputP1Exec05) mem_input = inputP1Mem05;
    else {
        fprintf(stderr, "Error: Unknown execution order array\n");
        return;
    }

    const int mem_input_size = get_mem_input_size(mem_input);
    initialize_processes(mem_input, mem_input_size);

    FILE *output_file = fopen(output_filename, "w");
    if (!output_file) {
        perror("Failed to open output file");
        return;
    }

    // Print header
    fprintf(output_file, "time inst proc1              proc2              proc3              proc4              proc5");

    // Process each memory access in order
    const int actual_exec_count = get_exec_count(exec_order);
    for (int i = 0; i < actual_exec_count; i += 2) {
        const int process_id = exec_order[i];
        const int address = exec_order[i+1];

        const int process_idx = process_id - 1;
        if (process_idx < 0 || process_idx >= total_processes || !processes[process_idx].active) {
            // Invalid or terminated process, skip but still increment time
            print_memory_state(output_file);
            current_time++;
            continue;
        }

        // Perform the memory access at current time
        const int result = allocate_page(process_id, address, fifo);
        print_memory_state(output_file);

        if (result == -2) {
            // SIGSEGV occurred
            current_time++;
            continue;
        }

        current_time++;  // Increment time after processing
    }

    fclose(output_file);
}