//
// Created by Outis on 25-6-22.
//
// cli.c
#include "judge_config.h"
#include "cli.h"
#include "argtable3.h"
#include <string.h>

int parse_arguments(int argc, char *argv[], judge_config_t *cfg) {
    struct arg_lit  *help     = arg_lit0(NULL, "help", "Show help");
    struct arg_file *config   = arg_file0(NULL, "config", "<file>", "Load config from file");
    struct arg_file *exec     = arg_file0(NULL, "exec", "<path>", "Executable path");
    struct arg_file *input    = arg_file0(NULL, "input", "<infile>", "Input file (default: /dev/null)");
    struct arg_file *output   = arg_file0(NULL, "output", "<outfile>", "Output file (default: ./output.txt)");
    struct arg_file *error    = arg_file0(NULL, "error", "<outfile>", "Error file (default: ./error.txt)");
    struct arg_int  *cpu      = arg_int0(NULL, "max-cpu", "<ms>", "Max CPU time in ms");
    struct arg_int  *real     = arg_int0(NULL, "max-real", "<ms>", "Max real time in ms");
    struct arg_int  *mem      = arg_int0(NULL, "max-mem", "<kb>", "Max memory in KB");
    struct arg_int  *outsize  = arg_int0(NULL, "max-output", "<bytes>", "Max output size in bytes");
    struct arg_int  *stack    = arg_int0(NULL, "max-stack", "<kb>", "Max stack size in KB");
    struct arg_int  *proc     = arg_int0(NULL, "max-proc", "<count>", "Max number of processes");
    struct arg_int  *uid      = arg_int0(NULL, "uid", "<uid>", "Run as user UID");
    struct arg_int  *gid      = arg_int0(NULL, "gid", "<gid>", "Run as group GID");
    struct arg_str  *profile  = arg_str0(NULL, "syscall-profile", "<name>", "Syscall profile name");
    struct arg_lit  *debug    = arg_lit0(NULL, "enable-debug", "Enable debug mode");
    struct arg_lit  *outflag  = arg_lit0(NULL, "include-output", "Include user output");
    struct arg_str  *env      = arg_strn(NULL, "env", "<ENV=VAL>", 0, MAX_ENV_VARS, "Environment variables");
    struct arg_end  *end      = arg_end(20);

    void *argtable[] = {
            help, config, exec, input, output, error,
            cpu, real, mem, outsize, stack, proc,
            uid, gid, profile, debug, outflag,
            env, end
    };

    int nerrors = arg_parse(argc, argv, argtable);
    if (help->count > 0) {
        arg_print_syntax(stdout, argtable, "\n");
        arg_print_glossary(stdout, argtable, "  %-25s %s\n");
        arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
        return -1;
    }

    if (nerrors > 0) {
        arg_print_errors(stderr, end, argv[0]);
        arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
        return -1;
    }

    // Clear config and set defaults
    memset(cfg, 0, sizeof(*cfg));
    strncpy(cfg->input_path, "/dev/null", MAX_PATH_LEN);
    strncpy(cfg->output_path, "./output.txt", MAX_PATH_LEN);
    strncpy(cfg->error_path, "./error.txt", MAX_PATH_LEN);
    cfg->max_cpu_time = -1;
    cfg->max_real_time = -1;
    cfg->max_memory = -1;
    cfg->max_output_size = -1;
    cfg->max_stack_kb = -1;
    cfg->max_processes = -1;
    cfg->uid = 1000;
    cfg->gid = 1000;
    strncpy(cfg->syscall_profile, "c_lang_safe", MAX_PATH_LEN);
    cfg->enable_debug = false;
    cfg->include_output = true;
    cfg->env_count = 0;

    if (config->count > 0 && exec->count > 0) {
        fprintf(stderr, "--config and --exec are mutually exclusive\n");
        arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
        return -1;
    }

    if (config->count > 0) {
        strncpy(cfg->config_path, config->filename[0], MAX_PATH_LEN);
        strncpy(cfg->exe_path, "", MAX_PATH_LEN);
        arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
        return 0;
    } else if (exec->count > 0) {
        strncpy(cfg->exe_path, exec->filename[0], MAX_PATH_LEN);
        if (input->count > 0) strncpy(cfg->input_path, input->filename[0], MAX_PATH_LEN);
        if (output->count > 0) strncpy(cfg->output_path, output->filename[0], MAX_PATH_LEN);
        if (error->count > 0) strncpy(cfg->error_path, error->filename[0], MAX_PATH_LEN);
    } else {
        fprintf(stderr, "One of --config or --exec must be specified\n");
        arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
        return -1;
    }

    if (cpu->count > 0) cfg->max_cpu_time = cpu->ival[0];
    if (real->count > 0) cfg->max_real_time = real->ival[0];
    if (mem->count > 0) cfg->max_memory = mem->ival[0];
    if (outsize->count > 0) cfg->max_output_size = outsize->ival[0];
    if (stack->count > 0) cfg->max_stack_kb = stack->ival[0];
    if (proc->count > 0) cfg->max_processes = proc->ival[0];
    if (uid->count > 0) cfg->uid = uid->ival[0];
    if (gid->count > 0) cfg->gid = gid->ival[0];
    if (profile->count > 0) strncpy(cfg->syscall_profile, profile->sval[0], MAX_PATH_LEN);
    if (debug->count > 0) cfg->enable_debug = true;
    if (outflag->count > 0) cfg->include_output = true;
    for (int i = 0; i < env->count && i < MAX_ENV_VARS; i++) {
        cfg->env[cfg->env_count++] = strdup(env->sval[i]);
    }

    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}