#include "syscall_profiles.h"
#include <string.h>
#include <seccomp.h>
#include <sys/stat.h>
#include <fcntl.h>

// 注册一个用于 C 语言安全运行环境的系统调用白名单
int register_c_lang_safe(scmp_filter_ctx ctx, const judge_config_t *cfg) {
    // 允许的系统调用列表
    int allowed[] = {
        SCMP_SYS(access),
        SCMP_SYS(arch_prctl),
        SCMP_SYS(brk),
        SCMP_SYS(clock_gettime),
        SCMP_SYS(uname),
        SCMP_SYS(close),
        SCMP_SYS(exit_group),
        SCMP_SYS(faccessat),
        SCMP_SYS(fstat),
        SCMP_SYS(futex),
        SCMP_SYS(getrandom),
        SCMP_SYS(lseek),
        SCMP_SYS(mmap),
        SCMP_SYS(mprotect),
        SCMP_SYS(munmap),
        SCMP_SYS(newfstatat),
        SCMP_SYS(pread64),
        SCMP_SYS(prlimit64),
        SCMP_SYS(read),
        SCMP_SYS(readlink),
        SCMP_SYS(readlinkat),
        SCMP_SYS(readv),
        SCMP_SYS(rseq),
        SCMP_SYS(set_robust_list),
        SCMP_SYS(set_tid_address),
        SCMP_SYS(write),
        SCMP_SYS(writev)
    };

    // 为每个允许的系统调用添加一条 seccomp 规则
    for (size_t i = 0; i < sizeof(allowed)/sizeof(int); ++i) {
        if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, allowed[i], 0) != 0) {
            return -1;
        }
    }
    // extra rule for execve
    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(execve), 1, SCMP_A0(SCMP_CMP_EQ, (scmp_datum_t)(cfg->exe_path))) != 0) {
        return -1;
    }
    // do not allow "w" and "rw"
    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 1, SCMP_CMP(1, SCMP_CMP_MASKED_EQ, O_WRONLY | O_RDWR, 0)) != 0) {
        return -1;
    }
    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(openat), 1, SCMP_CMP(2, SCMP_CMP_MASKED_EQ, O_WRONLY | O_RDWR, 0)) != 0) {
        return -1;
    }
    return 0;
}

// 定义支持的系统调用配置文件及其注册函数
static syscall_profile_t profiles[] = {
    { "c_lang_safe", register_c_lang_safe }, // 适用于 C/C++ 程序的基本安全规则
    { NULL, NULL }                           // 结束标志
};

// 根据配置文件名获取对应的注册函数指针
int (*get_profile_handler(const char *name))(scmp_filter_ctx, const judge_config_t *) {
    for (int i = 0; profiles[i].name != NULL; ++i) {
        if (strcmp(profiles[i].name, name) == 0) {
            return profiles[i].register_rules;
        }
    }
    return NULL; // 未找到匹配项时返回 NULL
}
