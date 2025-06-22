#include "judge_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

static FILE *fp_log = NULL;
static int debug_enabled = 0;

static const char *timestamp_str() {
    static char buf[32];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    return buf;
}

void log_init(const char *path, int enable_debug) {
    if (path && path[0] != '\0') {
        fp_log = fopen(path, "a");
        if (!fp_log) {
            fprintf(stderr, "[LOG] Cannot open log file: %s. Falling back to stdout.\n", path);
            fp_log = stdout;
        }
    } else {
        fp_log = stdout;
    }
    debug_enabled = enable_debug;
}

void log_debug(const char *fmt, ...) {
    if (!debug_enabled || !fp_log) return;
    va_list args;
    va_start(args, fmt);
    fprintf(fp_log, "[%s] DEBUG: ", timestamp_str());
    vfprintf(fp_log, fmt, args);
    fprintf(fp_log, "\n");
    fflush(fp_log);
    va_end(args);
}

void log_error(const char *fmt, ...) {
    if (!fp_log) return;
    va_list args;
    va_start(args, fmt);
    fprintf(fp_log, "[%s] ERROR: ", timestamp_str());
    vfprintf(fp_log, fmt, args);
    fprintf(fp_log, "\n");
    fflush(fp_log);
    va_end(args);
}

void log_info(const char *fmt, ...) {
    if (!fp_log) return;
    va_list args;
    va_start(args, fmt);
    fprintf(fp_log, "[%s] INFO: ", timestamp_str());
    vfprintf(fp_log, fmt, args);
    fprintf(fp_log, "\n");
    fflush(fp_log);
    va_end(args);
}

void log_close(void) {
    if (fp_log && fp_log != stderr && fp_log != stdout) {
        fclose(fp_log);
    }
    fp_log = NULL;
}
