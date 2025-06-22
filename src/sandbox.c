#include "judge_config.h"
#include "judge_result.h"
#include "judge_log.h"
#include "judge_seccomp.h"

#include <unistd.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>

#define EXIT_INPUT_ERROR  101
#define EXIT_OUTPUT_ERROR 102
#define EXIT_ERROR_ERROR  103
#define EXIT_SETGID_ERROR 104
#define EXIT_SETUID_ERROR 105
#define EXIT_DUP2_INPUT_ERROR  106
#define EXIT_DUP2_OUTPUT_ERROR 107
#define EXIT_DUP2_ERROR_ERROR  108

static void sandbox_fail(int code, FILE* in_fd, FILE* out_fd, FILE* err_fd) {
    if (in_fd) fclose(in_fd);
    if (out_fd) fclose(out_fd);
    if (err_fd) fclose(err_fd);
    raise(SIGUSR1);
    _exit(code);
}

static void set_resource_limits(const judge_config_t *cfg) {
    struct rlimit rl;

    if (cfg->max_cpu_time != -1) {
        // max_cpu_time 单位为毫秒，需转为秒（向上取整）
        rl.rlim_cur = rl.rlim_max = (cfg->max_cpu_time + 999) / 1000;
        setrlimit(RLIMIT_CPU, &rl);
        printf("CPUTIME: cur=%ld max=%ld\n", rl.rlim_cur, rl.rlim_max);
    }

    if (cfg->max_memory != -1) {
        rl.rlim_cur = rl.rlim_max = cfg->max_memory * 1024L;
        setrlimit(RLIMIT_AS, &rl);
        printf("MEMORY: cur=%ld max=%ld\n", rl.rlim_cur, rl.rlim_max);
    }

    if (cfg->max_output_size != -1) {
        rl.rlim_cur = rl.rlim_max = cfg->max_output_size;
        setrlimit(RLIMIT_FSIZE, &rl);
        printf("OUTPUT: cur=%ld max=%ld\n", rl.rlim_cur, rl.rlim_max);
    }

    if (cfg->max_stack_kb != -1) {
        rl.rlim_cur = rl.rlim_max = cfg->max_stack_kb * 1024L;
        setrlimit(RLIMIT_STACK, &rl);
        printf("STACK: cur=%ld max=%ld\n", rl.rlim_cur, rl.rlim_max);
    }

    if (cfg->max_processes != -1) {
        rl.rlim_cur = rl.rlim_max = cfg->max_processes;
        setrlimit(RLIMIT_NPROC, &rl);
        printf("PROCESS: cur=%ld max=%ld\n", rl.rlim_cur, rl.rlim_max);
    }
}

static void run_child_process(const judge_config_t *cfg) {
    FILE* in_fd = NULL;
    FILE* out_fd = NULL;
    FILE* error_fd = NULL;
    in_fd = fopen(cfg->input_path, "r");
    if (!in_fd) {
        sandbox_fail(EXIT_INPUT_ERROR, in_fd, out_fd, error_fd);
    }
    out_fd = fopen(cfg->output_path, "w");
    if (!out_fd) {
        sandbox_fail(EXIT_OUTPUT_ERROR, in_fd, out_fd, error_fd);
    }
    error_fd = fopen(cfg->error_path, "w");
    if (!error_fd) {
        sandbox_fail(EXIT_ERROR_ERROR, in_fd, out_fd, error_fd);
    }

    if (cfg->gid != -1 && setgid(cfg->gid) != 0) {
        sandbox_fail(EXIT_SETGID_ERROR, in_fd, out_fd, error_fd);
    }
    if (cfg->uid != -1 && setuid(cfg->uid) != 0) {
        sandbox_fail(EXIT_SETUID_ERROR, in_fd, out_fd, error_fd);
    }

    set_resource_limits(cfg);

    if (dup2(fileno(in_fd), fileno(stdin)) == -1) {
        sandbox_fail(EXIT_DUP2_INPUT_ERROR, in_fd, out_fd, error_fd);
    }
    if (dup2(fileno(out_fd), fileno(stdout)) == -1) {
        sandbox_fail(EXIT_DUP2_OUTPUT_ERROR, in_fd, out_fd, error_fd);
    }
    if (dup2(fileno(error_fd), fileno(stderr)) == -1) {
        sandbox_fail(EXIT_DUP2_ERROR_ERROR, in_fd, out_fd, error_fd);
    }

    apply_seccomp_filter(cfg->syscall_profile, cfg);

    char *envp[33] = {0};
    char *argv[] = {(char *)cfg->exe_path, NULL};
    for (int i = 0; i < cfg->env_count && i < 32; ++i) {
        envp[i] = cfg->env[i];
    }
    envp[cfg->env_count] = NULL;

    execve(cfg->exe_path, argv, envp);
    sandbox_fail(EXIT_ERROR_ERROR, in_fd, out_fd, error_fd);
}

int run_sandbox(const judge_config_t *cfg, judge_result_t *res) {
    // NOTE: 将未填写（为 0）的资源项统一视为无限制（-1），确保 setrlimit 逻辑一致。
    if (cfg->max_cpu_time == 0) ((judge_config_t *)cfg)->max_cpu_time = -1;
    if (cfg->max_real_time == 0) ((judge_config_t *)cfg)->max_real_time = -1;
    if (cfg->max_memory == 0) ((judge_config_t *)cfg)->max_memory = -1;
    if (cfg->max_output_size == 0) ((judge_config_t *)cfg)->max_output_size = -1;
    if (cfg->max_stack_kb == 0) ((judge_config_t *)cfg)->max_stack_kb = -1;
    if (cfg->max_processes == 0) ((judge_config_t *)cfg)->max_processes = -1;

    int pid = fork();
    if (pid < 0) {
        strcpy(res->status, "SystemError");
        return -1;
    }

    if (pid == 0) {
        run_child_process(cfg);
    } else {
        // 父进程
        int status;
        struct rusage usage;
        pid_t watchdog_pid = fork();
        if (watchdog_pid == 0) {
            sleep(cfg->max_real_time / 1000 + 1);  // 睡眠真实时间限制 + buffer
            kill(pid, SIGUSR2); // 若超时未退出，强制终止子进程
            _exit(0);
        }
        wait4(pid, &status, 0, &usage);

        res->exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        res->signal = WIFSIGNALED(status) ? WTERMSIG(status) : 0;
        res->time_used_ms = usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000;
        res->memory_used_kb = usage.ru_maxrss;

        res->timestamp = time(NULL);

        if (res->signal == SIGUSR1) {
            strcpy(res->status, "SandboxInternalError");
            log_debug("SandboxInternalError triggered by exit_code=%d", res->exit_code);
        }
        else if (res->signal != 0) {
            if (res->time_used_ms >= cfg->max_cpu_time) {
                strcpy(res->status, "TimeLimitExceeded");
            } else if (res->memory_used_kb >= cfg->max_memory) {
                strcpy(res->status, "MemoryLimitExceeded");
            } else {
                strcpy(res->status, "RuntimeError");
            }
        }
        else if (res->exit_code != 0) {
            strcpy(res->status, "NonZeroExit");
        } else {
            strcpy(res->status, "Accepted");
        }

        snprintf(res->message, sizeof(res->message), "Executed with code=%d signal=%d", res->exit_code, res->signal);

        log_debug("Judging finished. Status: %s, Time: %dms, Memory: %dKB", res->status, res->time_used_ms, res->memory_used_kb);
    }

    return 0;
}
