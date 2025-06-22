
#include "judgecore.h"
#include "judge_log.h"

extern int run_sandbox(const judge_config_t *cfg, judge_result_t *res);
extern void fill_user_output(const judge_config_t *cfg, judge_result_t *res);

int run_task(const judge_config_t *cfg, judge_result_t *res) {
    if (!cfg || !res) return -1;

    log_init("judge.log", cfg->enable_debug);
    log_debug("Judging started. Executable: %s", cfg->exe_path);

    int ret = run_sandbox(cfg, res);

    if (cfg->include_output) {
        fill_user_output(cfg, res);
    } else {
        res->user_output = NULL;
    }

    log_debug("Judging finished. Status: %s, Time: %dms, Memory: %dKB",
              res->status, res->time_used_ms, res->memory_used_kb);

    log_close();
    return ret;
}
