#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include "../src/termbox.h"

#define GRAPH_WIDTH 50
#define GRAPH_HEIGHT 10
#define HISTORY_SIZE GRAPH_WIDTH

// Structure to store historical values
typedef struct {
    double values[HISTORY_SIZE];
    int current_index;
} History;

// Initialize history with zeros
void init_history(History* history) {
    for (int i = 0; i < HISTORY_SIZE; i++) {
        history->values[i] = 0.0;
    }
    history->current_index = 0;
}

// Add new value to history
void add_to_history(History* history, double value) {
    history->values[history->current_index] = value;
    history->current_index = (history->current_index + 1) % HISTORY_SIZE;
}

// Get CPU usage percentage
double get_cpu_usage() {
    static unsigned long long prev_total = 0, prev_idle = 0;

    FILE* fp = fopen("/proc/stat", "r");
    if (!fp) return 0.0;

    char buffer[1024];
    char * res = fgets(buffer, sizeof(buffer), fp);
    (void) res;
    fclose(fp);

    unsigned long long user, nice, system, idle, iowait, irq, softirq;
    sscanf(buffer, "cpu %llu %llu %llu %llu %llu %llu %llu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq);

    unsigned long long total = user + nice + system + idle + iowait + irq + softirq;
    unsigned long long current_idle = idle + iowait;

    unsigned long long total_diff = total - prev_total;
    unsigned long long idle_diff = current_idle - prev_idle;

    prev_total = total;
    prev_idle = current_idle;

    if (total_diff == 0) return 0.0;
    return 100.0 * (1.0 - ((double)idle_diff / total_diff));
}

// Get memory usage percentage
double get_memory_usage() {
    struct sysinfo si;
    if (sysinfo(&si) < 0) return 0.0;

    unsigned long total_ram = si.totalram;
    unsigned long free_ram = si.freeram;
    unsigned long used_ram = total_ram - free_ram;

    return 100.0 * ((double)used_ram / total_ram);
}

// Draw graph
void draw_graph(int x, int y, History* history, const char* title) {
    tb_string(x, y - 1, TB_WHITE | TB_BOLD, 0, title);

    // Draw Y axis labels
    tb_string(x - 4, y, TB_WHITE, 0, "100%");
    tb_string(x - 4, y + GRAPH_HEIGHT - 1, TB_WHITE, 0, "  0%");

    // Draw graph border
    for (int i = 0; i <= GRAPH_WIDTH + 1; i++) {
        tb_string(x + i, y - 1, TB_WHITE, 0, "─");
        tb_string(x + i, y + GRAPH_HEIGHT, TB_WHITE, 0, "─");
    }
    for (int i = 0; i < GRAPH_HEIGHT; i++) {
        tb_string(x - 1, y + i, TB_WHITE, 0, "│");
        tb_string(x + GRAPH_WIDTH + 1, y + i, TB_WHITE, 0, "│");
    }

    // Draw corners
    tb_string(x - 1, y - 1, TB_WHITE, 0, "┌");
    tb_string(x - 1, y + GRAPH_HEIGHT, TB_WHITE, 0, "└");
    tb_string(x + GRAPH_WIDTH + 1, y - 1, TB_WHITE, 0, "┐");
    tb_string(x + GRAPH_WIDTH + 1, y + GRAPH_HEIGHT, TB_WHITE, 0, "┘");

    for (int i = 0; i < GRAPH_WIDTH; i++) {
        int index = (history->current_index - GRAPH_WIDTH + i + HISTORY_SIZE) % HISTORY_SIZE;
        double value = history->values[index];
        int height = (int)((value / 100.0) * (GRAPH_HEIGHT - 1));

        for (int j = 0; j < height; j++) {
            tb_string(x + i, y + GRAPH_HEIGHT - 1 - j, TB_GREEN, 0, "█");
        }
    }
}

int main() {
    if (tb_init() < 0) {
        fprintf(stderr, "Failed to initialize termbox\n");
        return 1;
    }

    tb_select_output_mode(TB_OUTPUT_256);

    History cpu_history, mem_history;
    init_history(&cpu_history);
    init_history(&mem_history);

    int running = 1;
    while (running) {
        struct tb_event ev;
        if (tb_peek_event(&ev, 1000) > 0) {
            if (ev.type == TB_EVENT_KEY && ev.key == TB_KEY_ESC) {
                running = 0;
            }
        }

        tb_clear_buffer();

        // Get current usage values
        double cpu = get_cpu_usage();
        double mem = get_memory_usage();

        // Add values to history
        add_to_history(&cpu_history, cpu);
        add_to_history(&mem_history, mem);

        // Draw graphs
        tb_stringf(10, 3, TB_WHITE | TB_BOLD, 0, "Current CPU: %.1f%%", cpu);
        draw_graph(10, 5, &cpu_history, "CPU Usage");

        tb_stringf(10, 18, TB_WHITE | TB_BOLD, 0, "Memory: %.1f%%", mem);
        draw_graph(10, 20, &mem_history, "Memory Usage");

        // Draw instructions
        tb_string(10, tb_height() - 2, TB_WHITE, 0, "Press ESC to exit");

        tb_render();
        usleep(250000);  // Update every 250ms
    }

    tb_shutdown();
    return 0;
}