#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <net/if.h>
#include <ifaddrs.h>

#define MAX_BUFFER 1024
#define USAGE_FILE "usage.txt"
#define PROCESSES_FILE "processes.txt"

// Function to get CPU usage
float get_cpu_usage() {
    FILE *fp;
    float cpu_usage;
    char buffer[MAX_BUFFER];

    fp = popen("top -bn1 | grep \"Cpu(s)\" | sed \"s/.*, *\\([0-9.]*\\)%* id.*/\\1/\" | awk '{print 100 - $1}'", "r");
    if (fp == NULL) {
        perror("Failed to run top command");
        return -1;
    }

    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        cpu_usage = atof(buffer);
    }
    pclose(fp);

    return cpu_usage;
}

// Function to get memory usage
float get_memory_usage() {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        perror("sysinfo");
        return -1;
    }

    float total_memory = info.totalram;
    float free_memory = info.freeram;
    float used_memory = total_memory - free_memory;
    
    return (used_memory / total_memory) * 100.0;
}

// Function to get network usage
long get_network_usage() {
    struct ifaddrs *ifap, *ifa;
    long total_bytes = 0;

    if (getifaddrs(&ifa) == -1) {
        perror("getifaddrs");
        return -1;
    }

    for (ifap = ifa; ifap; ifap = ifap->ifa_next) {
        if (ifap->ifa_addr && ifap->ifa_addr->sa_family == AF_PACKET) {
            // Assuming eth0, modify as needed
            if (strcmp(ifap->ifa_name, "eth0") == 0) {
                // You'd typically read from /sys/class/net/interface/statistics/
                // This is a placeholder
                total_bytes += 1024;  // Dummy value
            }
        }
    }

    freeifaddrs(ifap);
    return total_bytes / 1024;  // Convert to KB
}

// Function to list processes
void list_processes() {
    DIR *dir;
    struct dirent *entry;
    char path[256];
    FILE *processes_file;

    processes_file = fopen(PROCESSES_FILE, "w");
    if (!processes_file) {
        perror("Error opening processes file");
        return;
    }

    dir = opendir("/proc");
    if (dir == NULL) {
        perror("Error opening /proc directory");
        fclose(processes_file);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (atoi(entry->d_name) > 0) {
                snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);
                
                FILE *comm_file = fopen(path, "r");
                if (comm_file) {
                    char process_name[256];
                    if (fgets(process_name, sizeof(process_name), comm_file)) {
                        process_name[strcspn(process_name, "\n")] = 0;
                        fprintf(processes_file, "PID: %s, Name: %s\n", entry->d_name, process_name);
                    }
                    fclose(comm_file);
                }
            }
        }
    }

    closedir(dir);
    fclose(processes_file);
}

int main() {
    FILE *usage_file;
    time_t now;
    struct tm *timeinfo;

    while(1) {
        usage_file = fopen(USAGE_FILE, "a");
        if (!usage_file) {
            perror("Error opening usage file");
            return 1;
        }

        // Get current time
        time(&now);
        timeinfo = localtime(&now);
        char time_str[10];
        strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);

        // Get resource metrics
        float cpu_usage = get_cpu_usage();
        float memory_usage = get_memory_usage();
        long network_usage = get_network_usage();

        // Write to file
        fprintf(usage_file, "Time: %s, CPU: %.2f%%, Memory: %.2f%%, Network: %ld KB\n", 
                time_str, cpu_usage, memory_usage, network_usage);

        fclose(usage_file);

        // List processes every iteration
        list_processes();

        // Sleep for 2 seconds
        sleep(2);
    }

    return 0;
}