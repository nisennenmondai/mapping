#ifndef BENCH_H
#define BENCH_H

#define B_CR 0
#define B_ET 1
#define B_SR 2

#define BUFFSIZE 256

#include "mapping.h"

struct b_stats {
        int phi;
        float mean_cr;
        float mean_sr;
        float mean_et;
};

void concatenate(char p[], char q[]);

void write_data_to_file(FILE *filename, vector<struct b_stats> &data,
                int type, int size);

void plot_data(FILE *gnuplot_pipe, char const *commands_gnuplot[], int cmd_nbr);

void print_b_stats(vector<struct b_stats> &v_stts_bfdu_f, int iter);

#endif /* BENCH.H */