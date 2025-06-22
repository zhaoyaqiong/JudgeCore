#include "judge_config.h"
#include "judge_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

// 从 JSON 文件加载配置信息
int load_config_from_json(const char *path, judge_config_t *cfg) {
    json_error_t error;
    json_t *root = json_load_file(path, 0, &error);
    if (!root) {
        log_error("Failed to load JSON config: %s", error.text);
        return -1;
    }

    #define SET_STR_FIELD(json, key, dst) do { \
        json_t *tmp = json_object_get(json, key); \
        if (tmp && json_is_string(tmp)) { \
            strncpy(dst, json_string_value(tmp), sizeof(dst)); \
        } \
    } while (0)

    SET_STR_FIELD(root, "exe_path", cfg->exe_path);
    SET_STR_FIELD(root, "input_path", cfg->input_path);
    SET_STR_FIELD(root, "output_path", cfg->output_path);
    SET_STR_FIELD(root, "output_path", cfg->error_path);
    // syscall_profile: "none" means no syscall filter; otherwise use named profile
    SET_STR_FIELD(root, "syscall_profile", cfg->syscall_profile);

    cfg->max_cpu_time = json_integer_value(json_object_get(root, "max_cpu_time"));
    cfg->max_real_time = json_integer_value(json_object_get(root, "max_real_time"));
    cfg->max_memory = json_integer_value(json_object_get(root, "max_memory"));
    cfg->max_processes = json_integer_value(json_object_get(root, "max_processes"));
    cfg->max_stack_kb = json_integer_value(json_object_get(root, "max_stack_kb"));
    cfg->max_output_size = json_integer_value(json_object_get(root, "max_output_size"));
    cfg->uid = json_integer_value(json_object_get(root, "uid"));
    cfg->gid = json_integer_value(json_object_get(root, "gid"));
    // Removed enable_syscall_filter assignment as requested
    cfg->enable_debug = json_is_true(json_object_get(root, "enable_debug"));
    cfg->include_output = json_is_true(json_object_get(root, "include_output"));

    // 必填字段校验
    if (cfg->exe_path[0] == '\0') {
        log_error("Missing required field: exe_path");
        json_decref(root);
        return -1;
    }

    if (cfg->output_path[0] == '\0') {
        log_error("Missing required field: output_path");
        json_decref(root);
        return -1;
    }

    if (cfg->uid == 0 && !json_object_get(root, "uid")) {
        log_error("Missing required field: uid");
        json_decref(root);
        return -1;
    }

    if (cfg->gid == 0 && !json_object_get(root, "gid")) {
        log_error("Missing required field: gid");
        json_decref(root);
        return -1;
    }

    json_t *env = json_object_get(root, "env");
    if (json_is_array(env)) {
        cfg->env_count = (int)json_array_size(env);
        for (int i = 0; i < cfg->env_count && i < MAX_ENV_VARS; ++i) {
            cfg->env[i] = strdup(json_string_value(json_array_get(env, i)));
        }
    }

    // 统一将为 0 的资源项设置为 -1，表示不限制
    if (cfg->max_cpu_time == 0) cfg->max_cpu_time = -1;
    if (cfg->max_real_time == 0) cfg->max_real_time = -1;
    if (cfg->max_memory == 0) cfg->max_memory = -1;
    if (cfg->max_processes == 0) cfg->max_processes = -1;
    if (cfg->max_stack_kb == 0) cfg->max_stack_kb = -1;
    if (cfg->max_output_size == 0) cfg->max_output_size = -1;

    json_decref(root);
    return 0;
}
