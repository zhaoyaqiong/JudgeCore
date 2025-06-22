
#include "judge_seccomp.h"
#include "syscall_profiles.h"
#include "judge_log.h"

#include <string.h>
#include <seccomp.h>

int apply_seccomp_filter(const char *profile_name, const judge_config_t* cfg) {
    if (!profile_name || strcmp(profile_name, "none") == 0) {
        log_debug("Seccomp: profile none, skipping.");
        return 0;
    }

    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);
    if (!ctx) return -1;

    int (*handler)(scmp_filter_ctx, const judge_config_t*) = get_profile_handler(profile_name);
    if (!handler) {
        seccomp_release(ctx);
        return -1;
    }

    if (handler(ctx, cfg) != 0) {
        seccomp_release(ctx);
        return -1;
    }
    if (seccomp_load(ctx) != 0) {
        seccomp_release(ctx);
        return -1;
    }

    seccomp_release(ctx);
    return 0;
}
