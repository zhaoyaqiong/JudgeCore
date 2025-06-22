#ifndef JUDGE_CONFIG_H
#define JUDGE_CONFIG_H

#include <stdbool.h>

#define MAX_ENV_VARS 32
#define MAX_PATH_LEN 512

typedef struct {
    char config_path[MAX_PATH_LEN];
    char exe_path[MAX_PATH_LEN];
    char input_path[MAX_PATH_LEN];
    char output_path[MAX_PATH_LEN];
    char error_path[MAX_PATH_LEN];
    int max_cpu_time;
    int max_real_time;
    int max_memory;
    int max_output_size;
    int max_processes;
    int max_stack_kb;
    int uid;
    int gid;
    char syscall_profile[MAX_PATH_LEN];
    bool enable_debug;
    bool include_output;
    char *env[MAX_ENV_VARS];
    int env_count;
} judge_config_t;

#endif
