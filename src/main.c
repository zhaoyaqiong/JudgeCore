
#include "judgecore.h"
#include "judge_log.h"
#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CONFIG "config.json"

extern int load_config_from_json(const char *path, judge_config_t *cfg);
extern void fill_user_output(const judge_config_t *cfg, judge_result_t *res);
void print_config(const judge_config_t *cfg) {
    printf("Configuration:\n");
    printf("  exe_path         = %s\n", cfg->exe_path);
    printf("  input_path       = %s\n", cfg->input_path);
    printf("  output_path      = %s\n", cfg->output_path);
    printf("  error_path       = %s\n", cfg->error_path);
    printf("  max_cpu_time     = %d ms\n", cfg->max_cpu_time);
    printf("  max_real_time    = %d ms\n", cfg->max_real_time);
    printf("  max_memory       = %d KB\n", cfg->max_memory);
    printf("  max_output_size  = %d bytes\n", cfg->max_output_size);
    printf("  max_processes    = %d\n", cfg->max_processes);
    printf("  max_stack_kb     = %d KB\n", cfg->max_stack_kb);
    printf("  uid              = %d\n", cfg->uid);
    printf("  gid              = %d\n", cfg->gid);
    printf("  syscall_profile  = %s\n", cfg->syscall_profile);
    printf("  enable_debug     = %s\n", cfg->enable_debug ? "true" : "false");
    printf("  include_output   = %s\n", cfg->include_output ? "true" : "false");
    printf("  env_count        = %d\n", cfg->env_count);
    for (int i = 0; i < cfg->env_count; i++) {
        printf("    env[%d]         = %s\n", i, cfg->env[i]);
    }
}
int main(int argc, char *argv[]) {
    judge_config_t cfg;
    judge_result_t res;
    memset(&cfg, 0, sizeof(cfg));
    memset(&res, 0, sizeof(res));

    if (parse_arguments(argc, argv, &cfg) != 0) {
        return 1;
    }

    if (strlen(cfg.exe_path) == 0) {
        if (load_config_from_json(cfg.config_path, &cfg) != 0) {
            fprintf(stderr, "Failed to load config file: %s\n", cfg.config_path);
            return 1;
        }
    }
    print_config(&cfg);
    if (run_task(&cfg, &res) != 0) {
        fprintf(stderr, "Execution failed. Status: %s\n", res.status);
    } else {
        printf("Execution result:\n");
        printf("  Status: %s\n", res.status);
        printf("  Message: %s\n", res.message);
        printf("  Exit code: %d\n", res.exit_code);
        printf("  Signal: %d\n", res.signal);
        printf("  Time: %d ms\n", res.time_used_ms);
        printf("  Memory: %d KB\n", res.memory_used_kb);
        if (cfg.include_output && res.user_output) {
            printf("\n--- Program Output ---\n%s\n", res.user_output);
        }
    }

    if (res.user_output) {
        free(res.user_output);
    }
    return 0;
}
