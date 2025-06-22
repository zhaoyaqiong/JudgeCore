
#include "judgecore.h"
#include "judge_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CONFIG "config.json"

extern int load_config_from_json(const char *path, judge_config_t *cfg);
extern void fill_user_output(const judge_config_t *cfg, judge_result_t *res);

int main(int argc, char *argv[]) {
    const char *config_path = DEFAULT_CONFIG;
    if (argc >= 3 && strcmp(argv[1], "--config") == 0) {
        config_path = argv[2];
    }

    judge_config_t cfg;
    judge_result_t res;
    memset(&cfg, 0, sizeof(cfg));
    memset(&res, 0, sizeof(res));

    if (load_config_from_json(config_path, &cfg) != 0) {
        fprintf(stderr, "Failed to load config file: %s\n", config_path);
        return 1;
    }

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
