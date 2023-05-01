#include "let.h"
#include "model.h"
#include "print.h"
#include "sched_analysis.h"

static int _cmp_inc_task_priority(const struct task &a, const struct task &b)
{
        return a.p < b.p;
}

static int _cmp_inc_task_id(const struct task &a, const struct task &b)
{
        return a.id < b.id;
}

static int _cmp_dec_itm_size(const struct item &a, const struct item &b)
{
        return a.size > b.size;
}

static int _cmp_inc_bin_load_rem(const struct bin &a, const struct bin &b)
{
        return a.load_rem < b.load_rem;
}

void sort_inc_task_priority(vector<struct task> &v_tasks)
{
        sort(v_tasks.begin(), v_tasks.end(), _cmp_inc_task_priority);
}

void sort_inc_task_id(vector<struct task> &v_tasks)
{
        sort(v_tasks.begin(), v_tasks.end(), _cmp_inc_task_id);
}

void sort_dec_itm_size(vector<struct item> &v_itms)
{
        sort(v_itms.begin(), v_itms.end(), _cmp_dec_itm_size);
}

void sort_inc_bin_load_rem(vector<struct bin> &v_bins)
{
        sort(v_bins.begin(), v_bins.end(), _cmp_inc_bin_load_rem);
}

void add_bin(vector<struct bin> &v_bins, struct context &ctx)
{
        struct bin tmp_bin;
        struct item let;

        /* create and insert bin */
        tmp_bin.id = ctx.bins_count;
        tmp_bin.flag = SCHED_OK;
        tmp_bin.load = 0;
        tmp_bin.load_rem = ctx.prm.phi;
        tmp_bin.phi = ctx.prm.phi;
        tmp_bin.memcost = 0;
        v_bins.push_back(tmp_bin);
        ctx.bins_count++;
        printf("Bin %d Created\n", ctx.bins_count - 1);

        /* create and insert let task */
        let = {0};
        init_let_task(let, ctx);
        ctx.itms_count++;
        add_itm_to_v_bins(v_bins, let, tmp_bin.id, ctx, let.size, let.v_tasks[0].t);
        let.is_allocated = YES;
}

void add_itm_to_v_bins(vector<struct bin> &v_bins, struct item &itm, int bin_id, 
                struct context &ctx, int load, int gcd)
{
        for (int i = 0; i < ctx.bins_count; i++) {
                if (v_bins[i].id == bin_id) {
                        if (v_bins[i].phi < load) {
                                printf("ERR Bin %d Overflow with itm.size %d\n", 
                                                v_bins[i].id, itm.size);
                                exit(0);
                        }
                        itm.is_allocated = YES;
                        v_bins[i].load = load;
                        v_bins[i].load_rem = v_bins[i].phi - load;
                        v_bins[i].v_itms.push_back(itm);
                        compute_bin_memcost(v_bins[i]);
                        update_let(v_bins[i], gcd);

                        if (itm.is_frag == NO) {
                                printf("Item %d added in Bin %d\n\n", 
                                                itm.id, v_bins[i].id);
                                return;

                        } else {
                                printf("Fragment %d added in Bin %d\n\n", 
                                                itm.id, v_bins[i].id);
                                return;
                        }
                }
        }
}

void copy_back_prio_to_tc(struct bin &b)
{
        int itm_idx;
        int task_idx;

        for (unsigned int i = 0; i < b.v_tasks.size(); i++) {
                itm_idx = b.v_tasks[i].idx.itm_idx;
                task_idx = b.v_tasks[i].idx.task_idx;
                b.v_itms[itm_idx].v_tasks[task_idx].p = b.v_tasks[i].p;
        }
}

void copy_back_resp_to_tc(struct bin &b)
{
        int itm_idx;
        int task_idx;

        for (unsigned int i = 0; i < b.v_tasks.size(); i++) {
                itm_idx = b.v_tasks[i].idx.itm_idx;
                task_idx = b.v_tasks[i].idx.task_idx;
                b.v_itms[itm_idx].v_tasks[task_idx].r = b.v_tasks[i].r;
        }
}

void copy_tc_to_v_tasks_with_pos(struct bin &b, int bin_idx, int itm_idx)
{
        for (unsigned int i = 0; i < b.v_itms[itm_idx].v_tasks.size(); i++) {
                b.v_tasks.push_back(b.v_itms[itm_idx].v_tasks[i]);
                b.v_tasks.back().idx.bin_idx = bin_idx;
                b.v_tasks.back().idx.itm_idx = itm_idx;
                b.v_tasks.back().idx.task_idx = i;
        }
        /* sort tasks by id */
        sort_inc_task_id(b.v_tasks);
}

void compute_itm_load(struct item &itm)
{
        for (unsigned int i = 0; i < itm.v_tasks.size(); i++)
                itm.size += itm.v_tasks[i].u;
}

void compute_bin_load(struct bin &b, int &load)
{
        for (unsigned int i = 0; i < b.v_itms.size(); i++)
                load += b.v_itms[i].size;
}

void compute_bin_load_rem(struct bin &b)
{
        b.load = 0;
        b.load_rem = 0;

        for (unsigned int i = 0; i < b.v_itms.size(); i++)
                b.load += b.v_itms[i].size;

        b.load_rem = b.phi - b.load;

        if (b.load_rem < 0) {
                printf("Core: %d load_rem: %d\n", b.id, b.load_rem);
                exit(0);
        }
}

void compute_bin_memcost(struct bin &b)
{
        b.memcost = 0;
        for (unsigned int i = 0; i < b.v_itms.size(); i++) {
                /* let task has no memcost */
                if (b.v_itms[i].is_let == YES)
                        continue;

                b.memcost += b.v_itms[i].memcost;
        }

        if (b.memcost < 0) {
                printf("Core: %d memcost: %d\n", b.id, b.memcost);
                exit(0);
        }
}

int compute_gcd(vector<struct task> &v_tasks)
{
        int gcd;

        gcd = 0;

        /* if there is only one task (cut) gcd = t */
        if (v_tasks.size() == 1) {
                gcd = v_tasks[0].t;
                return gcd;
        }

        /* if there is only 2 values return gcd */
        if (v_tasks.size() == 2) {
                gcd =__gcd(v_tasks[0].t, v_tasks[1].t);
                return gcd;
        }

        /* cmp gcd for array */
        gcd =__gcd(v_tasks[0].t, v_tasks[1].t);

        for (unsigned int i = 2; i < v_tasks.size(); i++)
                gcd = __gcd(gcd, v_tasks[i].t);

        return gcd;
}

void replace_bin_by_id(vector<struct bin> &v_bins, struct bin &b)
{
        for (unsigned int i = 0; i < v_bins.size(); i++) {
                if (v_bins[i].id == b.id) {
                        v_bins[i] = b;
                        return;
                }
        }
}

struct item retrieve_tc_by_id(vector<struct bin> &v_bins, int tc_id)
{
        for (unsigned int i = 0; i < v_bins.size(); i++) {
                for (unsigned int j = 0; j < v_bins[i].v_itms.size(); j++) {
                        if (v_bins[i].v_itms[j].id == tc_id) {
                                return v_bins[i].v_itms[j];
                        }
                }
        }
        printf("ERR! Could not retrieve tc %d by id!\n", tc_id);
        exit(0);
}

struct bin retrieve_core_by_id(vector<struct bin> &v_bins, int bin_id)
{
        for (unsigned int i = 0; i < v_bins.size(); i++) {
                if (v_bins[i].id == bin_id)
                        return v_bins[i];
        }
        printf("ERR! Could not retrieve core by id!\n");
        exit(0);
}

int get_bin_idx_by_id(vector<struct bin> &v_bins, int bin_id)
{
        for (unsigned int i = 0; i < v_bins.size(); i++) {
                if (v_bins[i].id == bin_id)
                        return i;
        }
        return -1;
}

void insert_itm_to_core(struct bin &b, struct item &itm)
{
        b.v_itms.push_back(itm);
        printf("Added TC %d to Core %d\n", itm.id, b.id);
        compute_bin_load_rem(b);
}

void delete_itm_by_id(struct bin &b, int itm_id)
{
        int flag = NO;
        for (unsigned int j = 0; j < b.v_itms.size(); j++) {
                if (b.v_itms[j].id == itm_id) {
                        b.v_itms.erase(b.v_itms.begin() + j);
                        printf("Removed TC %d from Core %d\n", itm_id, b.id);
                        compute_bin_load_rem(b);
                        flag = YES;
                }
        }
        if (flag == NO) {
                printf("ERR! TC %d not removed from Core %d\n", itm_id, b.id);
                print_core(b);
                exit(0);
        }
}

void add_tasks_to_v_tasks(vector<struct task> &dst_v_tasks, 
                vector<struct task> &src_v_tasks)
{
        for (unsigned int i = 0; i < src_v_tasks.size(); i++)
                dst_v_tasks.push_back(src_v_tasks[i]);
}
