#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <algorithm>
#include<bits/stdc++.h>

using namespace std;

/* TASK MODEL */
struct t_pos {
        int bin_idx;
        int itm_idx;
        int task_idx;
};

struct task {
        int c;
        int t;
        int d;
        int p;
        int r;
        int id;
        float u;
        struct t_pos idx;
};

struct cut {
        int id;
        pair<int, int> c_pair;
        vector<struct task> v_tasks_lf;
        vector<struct task> v_tasks_rf;
};

struct task_chain {
        float u;
        vector<struct task> v_tasks;
        vector<struct cut> v_cuts;
};

/* BIN-PACKING MODEL */
struct item {
        int id;
        int size;
        int nbr_cut;
        int frag_id;
        int disp_count;
        int swap_count;
        int is_let;
        int is_frag;
        int is_allocated;
        int is_fragmented;
        struct task_chain tc;
};

struct bin {
        int id;
        int phi;
        int flag;
        int cap_rem;
        vector<struct item> v_itms;
        vector<struct task> v_tasks;
};

/* OPERATIONS ON DATA STRUCTURES */

void sort_inc_task_priority(vector<struct task> &v_tasks);

void sort_inc_task_id(vector<struct task> &v_tasks);

void sort_dec_itm_size(vector<struct item> &v_itms);

void sort_inc_bin_cap_rem(vector<struct bin> &v_bins);

void copy_back_prio_to_tc(struct bin &b);

void copy_back_resp_to_tc(struct bin &b);

void copy_tc_to_v_tasks_with_pos(struct bin &b, int bin_idx, int itm_idx);

void compute_bin_cap_rem(struct bin &b);

void compute_tc_load(struct item &itm);

int compute_gcd(vector<struct task> &v_tasks);

void add_bin(vector<struct bin> &v_bins, struct context &ctx);

void add_itm_to_bin(vector<struct bin> &v_bins, struct item &itm, int bin_id, 
                struct context &ctx);

void replace_bin_by_id(vector<struct bin> &v_bins, struct bin &b);

struct item retrieve_tc_by_id(vector<struct bin> &v_bins, int tc_id);

struct bin retrieve_core_by_id(vector<struct bin> &v_bins, int bin_id);

int get_bin_idx_by_id(vector<struct bin> &v_bins, int bin_id);

void delete_itm_by_id(struct bin &b, int itm_id);

void insert_itm_to_core(struct bin &b, struct item &itm);

#endif /* MODEL_H */
