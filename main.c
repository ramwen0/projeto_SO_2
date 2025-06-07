#include "memory_sim.h"

int main() {
    // Process all input sets with both FIFO and LRU algorithms

    // Input set 00
    run_simulation(inputP1Exec00, 0, true, "fifo00.out");  // exec_count will be determined internally
    run_simulation(inputP1Exec00, 0, false, "lru00.out");

    // Input set 01
    run_simulation(inputP1Exec01, 0, true, "fifo01.out");
    run_simulation(inputP1Exec01, 0, false, "lru01.out");

    // Input set 02
    run_simulation(inputP1Exec02, 0, true, "fifo02.out");
    run_simulation(inputP1Exec02, 0, false, "lru02.out");

    // Input set 03
    run_simulation(inputP1Exec03, 0, true, "fifo03.out");
    run_simulation(inputP1Exec03, 0, false, "lru03.out");

    // Input set 04
    run_simulation(inputP1Exec04, 0, true, "fifo04.out");
    run_simulation(inputP1Exec04, 0, false, "lru04.out");

    // Input set 05
    run_simulation(inputP1Exec05, 0, true, "fifo05.out");
    run_simulation(inputP1Exec05, 0, false, "lru05.out");

    return 0;
}