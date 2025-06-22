
#include "judge_config.h"
#include "judge_result.h"
#include <stdio.h>
#include <stdlib.h>

void fill_user_output(const judge_config_t *cfg, judge_result_t *res) {
    FILE *fp = fopen(cfg->output_path, "r");
    if (!fp) {
        res->user_output = NULL;
        return;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if(cfg->max_output_size != -1)
    {
        if (size > cfg->max_output_size) {
            res->output_truncated = true;
            size = cfg->max_output_size;
        } else {
            res->output_truncated = false;
        }
    }

    res->user_output = (char *)malloc(size + 1);
    if (res->user_output) {
        fread(res->user_output, 1, size, fp);
        res->user_output[size] = '\0';
    }

    fclose(fp);
}
