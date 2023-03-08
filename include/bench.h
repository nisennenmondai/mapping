#ifndef BENCH_H
#define BENCH_H

#define B_AR 0
#define B_ET 1
#define B_FR 2
#define B_LD 3
#define B_SCHED 4

#define BUFFSIZE 256

#include "mapping.h"

struct b_stats {
        int phi;
        float mean_ar;
        float mean_et;
        float mean_fr;
        float mean_op;
        float mean_ld;
        float mean_sched;
};

void concatenate(char p[], char q[]);

void write_data_to_file(FILE *filename, vector<struct b_stats> &data,
                int type, int size);

void plot_data(FILE *gnuplot_pipe, char const *commands_gnuplot[], int cmd_nbr);

void print_b_stats(vector<struct b_stats> &v_stts_bfdu_f, 
                vector<struct b_stats> &v_stts_wfdu_f, int iter);

#endif /* BENCH.H */
