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

int find_victim_fifo() {
    int oldest_time = current_time + 1;
    int victim_frame = -1;
    int victim_page = MAX_PAGES_PER_PROCESS + 1; // Start higher than max possible

    for (int i = 0; i < MAX_FRAMES; i++) {
        if (memory[i].valid) {
            // Prefer pages with lower numbers when times are equal
            if (memory[i].load_time < oldest_time ||
                (memory[i].load_time == oldest_time && memory[i].page_num < victim_page)) {
                oldest_time = memory[i].load_time;
                victim_page = memory[i].page_num;
                victim_frame = i;
                }
        }
    }

    return victim_frame;
}

int find_victim_lru() {
    int least_recent_time = current_time + 1;
    int victim_frame = -1;
    int victim_page = MAX_PAGES_PER_PROCESS + 1; // Start higher than max possible

    for (int i = 0; i < MAX_FRAMES; i++) {
        if (memory[i].valid) {
            // Prefer pages with lower numbers when times are equal
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
    if (process_idx < 0 || process_idx >= total_processes) {
        return -1; // Invalid process ID
    }

    // Check if address is valid for this process
    if (address >= processes[process_idx].memory_size) {
        processes[process_idx].segfault = true;
        processes[process_idx].active = false;
        processes[process_idx].last_fault_time = current_time;  // Record fault time
        free_process_pages(process_id);
        return -2; // SIGSEGV
    }

    const int page_num = get_page_number(address);

    // Check if page is already in memory
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (memory[i].valid && memory[i].process_id == process_id && memory[i].page_num == page_num) {
            memory[i].last_used_time = current_time;
            return i;
        }
    }

    // Page not in memory, need to load it
    const int free_frame = find_free_frame();
    if (free_frame != -1) {
        load_page(free_frame, process_id, page_num);
        return free_frame;
    }

    // No free frames, need to replace
    const int victim_frame = fifo ? find_victim_fifo() : find_victim_lru();
    if (victim_frame == -1) {
        return -1; // Shouldn't happen as we have MAX_FRAMES
    }

    load_page(victim_frame, process_id, page_num);
    return victim_frame;
}

void print_memory_state(FILE *output_file) {
    fprintf(output_file, "\n%-8d  ", current_time);  // Print time instant first

    for (int i = 0; i < total_processes; i++) {
        if (processes[i].segfault && processes[i].last_fault_time == current_time) {
            // Only print SIGSEGV at the exact time of the fault
            fprintf(output_file, "%-18s ", "SIGSEGV");
        }
        else if (!processes[i].active) {
            // Inactive processes (after fault) print empty
            fprintf(output_file, "%-18s ", "");
        }
        else {
            // Active processes: print their frames
            char buffer[256] = "";
            bool first = true;
            for (int j = 0; j < MAX_FRAMES; j++) {
                if (memory[j].valid && memory[j].process_id == processes[i].id) {
                    if (!first) strcat(buffer, ",");
                    sprintf(buffer + strlen(buffer), "F%d", j);
                    first = false;
                }
            }
            fprintf(output_file, "%-18s ", (strlen(buffer) > 0) ? buffer : "");
        }
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