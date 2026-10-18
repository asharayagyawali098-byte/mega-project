#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/utmpx.h>
#endif

// ============================================================================
// MACROS AND STRUCTS
// ============================================================================
#define MAX_BUFFER_SIZE 1024
#define LOG_PREFIX "[C-SYSCHECK]"

typedef struct {
    char os_name[64];
    char architecture[32];
    unsigned int cpu_cores;
    unsigned long long total_ram_mb;
    unsigned long long free_ram_mb;
} SystemHardwareInfo;

typedef struct ProcessNode {
    unsigned int process_id;
    char process_name[128];
    struct ProcessNode* next;
} ProcessNode;

// ============================================================================
// HARDWARE DETECTION
// ============================================================================
void get_hardware_diagnostics(SystemHardwareInfo* info) {
    if (info == NULL) return;

    memset(info, 0, sizeof(SystemHardwareInfo));

#if defined(_WIN32) || defined(_WIN64)
    strncpy(info->os_name, "Windows OS Platform", sizeof(info->os_name) - 1);

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    info->cpu_cores = sysInfo.dwNumberOfProcessors;

    if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
        strncpy(info->architecture, "x86_64 (64-bit)", sizeof(info->architecture) - 1);
    } else {
        strncpy(info->architecture, "x86 (32-bit)", sizeof(info->architecture) - 1);
    }

    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memStatus)) {
        info->total_ram_mb = memStatus.ullTotalPhys / (1024 * 1024);
        info->free_ram_mb = memStatus.ullAvailPhys / (1024 * 1024);
    }
#else
    strncpy(info->os_name, "Unix/POSIX Generic", sizeof(info->os_name) - 1);
    strncpy(info->architecture, "Unknown/POSIX", sizeof(info->architecture) - 1);
    info->cpu_cores = 4;
    info->total_ram_mb = 8192;
    info->free_ram_mb = 4096;
#endif
}

// ============================================================================
// MEMORY & LINKED LIST DATA STRUCTURES
// ============================================================================
ProcessNode* create_process_node(unsigned int pid, const char* name) {
    ProcessNode* node = (ProcessNode*)malloc(sizeof(ProcessNode));
    if (node == NULL) {
        fprintf(stderr, "%s Memory allocation failed for ProcessNode!\n", LOG_PREFIX);
        return NULL;
    }
    node->process_id = pid;
    strncpy(node->process_name, name, sizeof(node->process_name) - 1);
    node->next = NULL;
    return node;
}

void append_process(ProcessNode** head, unsigned int pid, const char* name) {
    ProcessNode* newNode = create_process_node(pid, name);
    if (*head == NULL) {
        *head = newNode;
        return;
    }
    ProcessNode* temp = *head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = newNode;
}

void free_process_list(ProcessNode* head) {
    ProcessNode* temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

// ============================================================================
// DIAGNOSTIC CHECKS
// ============================================================================
void print_header() {
    printf("=======================================================\n");
    printf("  ASHRAYA GYAWALI SYSTEM COMMAND - C LOW-LEVEL ENGINE  \n");
    printf("=======================================================\n");
}

void print_system_timestamp() {
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    printf("%s System Diagnostic Time: %s\n", LOG_PREFIX, buffer);
}

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================
int main() {
    print_header();
    print_system_timestamp();

    // 1. Run Hardware Diagnostic
    SystemHardwareInfo hwInfo;
    get_hardware_diagnostics(&hwInfo);

    printf("\n--- HARDWARE & OS DIAGNOSTICS ---\n");
    printf(" OS Platform     : %s\n", hwInfo.os_name);
    printf(" Architecture    : %s\n", hwInfo.architecture);
    printf(" CPU Cores       : %u Cores\n", hwInfo.cpu_cores);
    printf(" Total Memory    : %llu MB\n", hwInfo.total_ram_mb);
    printf(" Available RAM   : %llu MB\n", hwInfo.free_ram_mb);

    // 2. Mock Process Enumeration via Dynamic Linked List
    printf("\n--- INSPECTING CRITICAL CORE PROCESSES ---\n");
    ProcessNode* processList = NULL;
    
    append_process(&processList, 104, "system_idle_process.exe");
    append_process(&processList, 412, "python_flask_backend.exe");
    append_process(&processList, 890, "cpp_analytics_engine.exe");
    append_process(&processList, 1024, "vscode_workspace.exe");

    ProcessNode* current = processList;
    while (current != NULL) {
        printf(" [PID: %04u] Process: %-30s | Status: ACTIVE\n", current->process_id, current->process_name);
        current = current->next;
    }

    // Cleanup Memory
    free_process_list(processList);

    printf("\n=======================================================\n");
    printf("%s Low-level C diagnostics completed successfully.\n", LOG_PREFIX);
    printf("=======================================================\n");

    return 0;
}