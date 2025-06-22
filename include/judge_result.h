#ifndef JUDGE_RESULT_H
#define JUDGE_RESULT_H

#include <stdbool.h>
#include <time.h>

typedef struct {
    char status[32];
    int exit_code;
    int signal;
    int time_used_ms;
    int memory_used_kb;
    bool output_truncated;
    time_t timestamp;
    char message[256];
    char *user_output;
} judge_result_t;

#endif
