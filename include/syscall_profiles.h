#ifndef SYSCALL_PROFILES_H
#define SYSCALL_PROFILES_H
#include "judge_config.h"
#include <seccomp.h>

typedef struct {
    const char *name;
    int (*register_rules)(scmp_filter_ctx ctx, const judge_config_t *cfg);
} syscall_profile_t;

int (*get_profile_handler(const char *name))(scmp_filter_ctx, const judge_config_t *);

#endif
