#ifndef SCHED_ANALYSIS_H
#define SCHED_ANALYSIS_H

#include <random>

#include "model.h"

#define SCHED_OK     1
#define SCHED_FAILED 2

void priority_assignment(struct bin &b);

int wcrt(vector<struct task> &v_tasks);

void sched_analysis(vector<struct bin> &v_bins, struct context &ctx);

void verify_tc_schedulability(vector<vector<struct item>> &v_frag_itms, 
                vector<struct item> &v_non_frag_itms, struct context &ctx);

#endif /* SCHED_ANALYSIS_H */
