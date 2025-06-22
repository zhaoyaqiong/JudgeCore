#ifndef JUDGE_SECCOMP_H
#define JUDGE_SECCOMP_H
#include "judge_config.h"
int apply_seccomp_filter(const char *profile_name,  const judge_config_t * cfg);

#endif
