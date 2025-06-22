
#include "judgecore.h"
#include "judge_log.h"
#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern int load_config_from_json(const char *path, judge_config_t *cfg);
extern int load_global_config_from_json(const char *path, global_config_t *cfg);
extern void fill_user_output(const judge_config_t *cfg, judge_result_t *res);
void print_log_config(const judge_config_t *cfg) {
    log_debug("Configuration:");
    log_debug("  exe_path         = %s", cfg->exe_path);
    log_debug("  input_path       = %s", cfg->input_path);
    log_debug("  output_path      = %s", cfg->output_path);
    log_debug("  error_path       = %s", cfg->error_path);
    log_debug("  max_cpu_time     = %d ms", cfg->max_cpu_time);
    log_debug("  max_real_time    = %d ms", cfg->max_real_time);
    log_debug("  max_memory       = %d KB", cfg->max_memory);
    log_debug("  max_output_size  = %d bytes", cfg->max_output_size);
    log_debug("  max_processes    = %d", cfg->max_processes);
    log_debug("  max_stack_kb     = %d KB", cfg->max_stack_kb);
    log_debug("  uid              = %d", cfg->uid);
    log_debug("  gid              = %d", cfg->gid);
    log_debug("  syscall_profile  = %s", cfg->syscall_profile);
    log_debug("  enable_debug     = %s", cfg->enable_debug ? "true" : "false");
    log_debug("  include_output   = %s", cfg->include_output ? "true" : "false");
    log_debug("  env_count        = %d", cfg->env_count);
    for (int i = 0; i < cfg->env_count; i++) {
        log_debug("    env[%d]         = %s", i, cfg->env[i]);
    }
}
int main(int argc, char *argv[]) {
    global_config_t gcfg;
    judge_config_t cfg;
    judge_result_t res;
    memset(&cfg, 0, sizeof(cfg));
    memset(&res, 0, sizeof(res));
    load_global_config_from_json("global_config.json", &gcfg);
    log_init(gcfg.log_file_path, gcfg.debug);
    if (parse_arguments(argc, argv, &cfg) != 0) {
        return 1;
    }

    if (strlen(cfg.exe_path) == 0) {
        if (load_config_from_json(cfg.config_path, &cfg) != 0) {
            log_error("Failed to load config file: %s\n", cfg.config_path);
            return 1;
        }
    }

    print_log_config(&cfg);
    if (run_task(&cfg, &res) != 0) {
        log_debug( "Execution failed. Status: %s\n", res.status);
    } else {
        log_debug( "Execution result:");
        printf("Execution result:\n");
        log_debug( "  Status: %s", res.status);
        printf("  Status: %s\n", res.status);
        log_debug("  Message: %s", res.message);
        printf("  Message: %s\n", res.message);
        log_debug("  Exit code: %d", res.exit_code);
        printf("  Exit code: %d\n", res.exit_code);
        log_debug("  Signal: %d", res.signal);
        printf("  Signal: %d\n", res.signal);
        log_debug("  Time: %d ms", res.time_used_ms);
        printf("  Time: %d ms\n", res.time_used_ms);
        log_debug("  Memory: %d KB", res.memory_used_kb);
        printf("  Memory: %d KB\n", res.memory_used_kb);
        if (cfg.include_output && res.user_output) {
            log_debug("\n--- Program Output ---\n%s", res.user_output);
            printf("\n--- Program Output ---\n%s\n", res.user_output);
        }
    }
    if (res.user_output) {
        free(res.user_output);
    }
    return 0;
}
