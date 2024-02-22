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
        return a.task_id < b.task_id;
}

static int _cmp_dec_tc_size(const struct tc &a, const struct tc &b)
{
        return a.u > b.u;
}

static int _cmp_inc_tc_id(const struct tc &a, const struct tc &b)
{
        return a.id < b.id;
}

static int _cmp_inc_tc_tc_idx(const struct tc &a, const struct tc &b)
{
        return a.tc_idx < b.tc_idx;
}

static int _cmp_inc_tc_color(const struct tc &a, const struct tc &b)
{
        return a.color < b.color;
}

static int _cmp_inc_core_load_rem(const struct core &a, const struct core &b)
{
        return a.load_rem < b.load_rem;
}

static int _cmp_inc_core_color(const struct core &a, const struct core &b)
{
        return a.color < b.color;
}

static int _cmp_dec_int(const int &a, const int &b)
{
        return a > b;
}

void sort_dec_int(vector<int> &v_int)
{
        sort(v_int.begin(), v_int.end(), _cmp_dec_int);
}

void sort_inc_task_priority(vector<struct task> &v_tasks)
{
        sort(v_tasks.begin(), v_tasks.end(), _cmp_inc_task_priority);
}

void sort_inc_task_id(vector<struct task> &v_tasks)
{
        sort(v_tasks.begin(), v_tasks.end(), _cmp_inc_task_id);
}

void sort_dec_tc_size(vector<struct tc> &v_tcs)
{
        sort(v_tcs.begin(), v_tcs.end(), _cmp_dec_tc_size);
}

void sort_inc_tc_id(vector<struct tc> &v_tcs)
{
        sort(v_tcs.begin(), v_tcs.end(), _cmp_inc_tc_id);
}

void sort_inc_tc_tc_idx(vector<struct tc> &v_tcs)
{
        sort(v_tcs.begin(), v_tcs.end(), _cmp_inc_tc_tc_idx);
}

void sort_inc_tc_color(vector<struct tc> &v_tcs)
{
        sort(v_tcs.begin(), v_tcs.end(), _cmp_inc_tc_color);
}

void sort_inc_core_load_rem(vector<struct core> &v_cores)
{
        sort(v_cores.begin(), v_cores.end(), _cmp_inc_core_load_rem);
}

void sort_inc_core_color(vector<struct core> &v_cores)
{
        sort(v_cores.begin(), v_cores.end(), _cmp_inc_core_color);
}

void add_core(vector<struct core> &v_cores, int color, int speed_factor, 
                struct context &ctx)
{
        struct core tmp_core;
        struct tc let;

        /* create and insert core */
        tmp_core.id = ctx.cores_count;
        tmp_core.flag = SCHED_OK;
        tmp_core.load = 0;
        tmp_core.load_rem = PHI;
        tmp_core.phi = PHI / speed_factor;
        tmp_core.color = color;
        tmp_core.comcost = 0;
        tmp_core.is_empty = NO;
        v_cores.push_back(tmp_core);
        ctx.cores_count++;
        printf("Core %d created with Color %d\n", ctx.cores_count - 1, color);

        /* create and insert let task */
        let = {0};
        init_let_task(let, ctx);
        add_tc_to_v_cores(v_cores, let, tmp_core.id, ctx, let.u, let.v_tasks[0].t);
        ctx.tcs_count++;
        let.is_alloc = YES;
}

void add_tc_to_core(struct core &b, struct tc &tc, int load, int gcd)
{
        if (b.phi < load) {
                printf("ERR Core %d Overflow with tc.size %d\n", 
                                b.id, tc.u);
                exit(0);
        }
        tc.is_alloc = YES;
        b.load = load;
        b.load_rem = b.phi - load;
        b.v_tcs.push_back(tc);
        cmp_core_comcost(b);
        update_let(b, gcd);
        b.v_tasks.clear();

        for (unsigned int i = 0; i < b.v_tcs.size(); i++) {
                for (unsigned int j = 0; j < b.v_tcs[i].v_tasks.size(); j++) {
                        b.v_tcs[i].v_tasks[j].idx.tc_idx = i;
                        b.v_tcs[i].v_tasks[j].idx.task_idx = j;
                }
        }

        for (unsigned int i = 0; i < b.v_tcs.size(); i++)
                add_tasks_to_v_tasks(b.v_tasks, b.v_tcs[i].v_tasks);
}

void add_tc_to_v_cores(vector<struct core> &v_cores, struct tc &tc, int core_id, 
                struct context &ctx, int load, int gcd)
{
        for (int i = 0; i < ctx.cores_count; i++) {
                if (v_cores[i].id == core_id) {
                        if (v_cores[i].phi < load) {
                                printf("ERR Core %d Overflow with tc.size %d\n", 
                                                v_cores[i].id, tc.u);
                                exit(0);
                        }
                        tc.is_alloc = YES;
                        v_cores[i].load = load;
                        v_cores[i].load_rem = v_cores[i].phi - load;
                        v_cores[i].v_tcs.push_back(tc);
                        cmp_core_comcost(v_cores[i]);
                        update_let(v_cores[i], gcd);
                        v_cores[i].v_tasks.clear();
                        printf("TC %d added in Core %d\n\n", tc.id, v_cores[i].id);
                        return;
                }
        }
}

void copy_back_prio_to_tc(struct core &b)
{
        int tc_idx;
        int task_idx;

        tc_idx = 0;
        task_idx = 0;

        for (unsigned int i = 0; i < b.v_tasks.size(); i++) {
                tc_idx = b.v_tasks[i].idx.tc_idx;
                task_idx = b.v_tasks[i].idx.task_idx;
                b.v_tcs[tc_idx].v_tasks[task_idx].p = b.v_tasks[i].p;
        }
}

void copy_back_resp_to_tc(struct core &b)
{
        int tc_idx;
        int task_idx;

        tc_idx = 0;
        task_idx = 0;

        for (unsigned int i = 0; i < b.v_tasks.size(); i++) {
                tc_idx = b.v_tasks[i].idx.tc_idx;
                task_idx = b.v_tasks[i].idx.task_idx;
                b.v_tcs[tc_idx].v_tasks[task_idx].r = b.v_tasks[i].r;
        }
}

void copy_v_tc_to_v_tasks_with_pos(vector<struct core> &v_cores)
{
        for (unsigned int i = 0; i < v_cores.size(); i++) {
                v_cores[i].v_tasks.clear();
                for (unsigned int j = 0; j < v_cores[i].v_tcs.size(); j++)
                        copy_tc_to_v_tasks_with_pos(v_cores[i], i, j);
        }
}

void copy_tc_to_v_tasks_with_pos(struct core &b, int core_idx, int tc_idx)
{
        for (unsigned int i = 0; i < b.v_tcs[tc_idx].v_tasks.size(); i++) {
                b.v_tcs[tc_idx].v_tasks[i].idx.core_idx = core_idx;
                b.v_tcs[tc_idx].v_tasks[i].idx.tc_idx = tc_idx;
                b.v_tcs[tc_idx].v_tasks[i].idx.task_idx = i;
                b.v_tasks.push_back(b.v_tcs[tc_idx].v_tasks[i]);
        }
        /* sort tasks by id */
        sort_inc_task_id(b.v_tasks);
}

void cmp_tc_load(struct tc &tc)
{
        for (unsigned int i = 0; i < tc.v_tasks.size(); i++)
                tc.u += tc.v_tasks[i].u;
}

void cmp_core_load(struct core &b, int &load)
{
        for (unsigned int i = 0; i < b.v_tcs.size(); i++)
                load += b.v_tcs[i].u;
}

void cmp_core_comcost(struct core &b)
{
        b.comcost = 0;
        for (unsigned int i = 0; i < b.v_tcs.size(); i++) {
                /* let tc has no comcost */
                if (b.v_tcs[i].is_let == YES)
                        continue;

                b.comcost += b.v_tcs[i].comcost;
        }

        if (b.comcost < 0) {
                printf("Core: %d comcost: %d\n", b.id, b.comcost);
                exit(0);
        }
}

int cmp_gcd(vector<struct task> &v_tasks)
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

void rplc_core_by_id(vector<struct core> &v_cores, struct core &b)
{
        for (unsigned int i = 0; i < v_cores.size(); i++) {
                if (v_cores[i].id == b.id) {
                        v_cores[i] = b;
                        return;
                }
        }
}

struct tc get_tc_by_id(vector<struct core> &v_cores, int tc_id)
{
        for (unsigned int i = 0; i < v_cores.size(); i++) {
                for (unsigned int j = 0; j < v_cores[i].v_tcs.size(); j++) {
                        if (v_cores[i].v_tcs[j].id == tc_id) {
                                return v_cores[i].v_tcs[j];
                        }
                }
        }
        printf("ERR! Could not retrieve TC %d by id!\n", tc_id);
        exit(0);
}

struct core get_core_by_id(vector<struct core> &v_cores, int core_id)
{
        for (unsigned int i = 0; i < v_cores.size(); i++) {
                if (v_cores[i].id == core_id)
                        return v_cores[i];
        }
        printf("ERR! Could not retrieve Core %d by id!\n", core_id);
        exit(0);
}

int get_wcrt(struct core &b, int tc_id, int tc_idx)
{
        for (unsigned int i = 0; i < b.v_tcs.size(); i++) {
                if (b.v_tcs[i].id == tc_id && b.v_tcs[i].tc_idx == tc_idx) {
                        for (unsigned int j = 0; j < b.v_tcs[i].v_tasks.size(); j++) {
                                if (j == b.v_tcs[i].v_tasks.size() - 1)
                                        return b.v_tcs[i].v_tasks[j].r;
                        }
                }
        }
        return -1;
}

int get_color_by_id(vector<struct core> &v_cores, int core_id)
{
        for (unsigned int i = 0; i < v_cores.size(); i++) {
                if (v_cores[i].id == core_id)
                        return v_cores[i].color;
        }
        return -1;
}

int get_core_idx_by_id(vector<struct core> &v_cores, int core_id)
{
        for (unsigned int i = 0; i < v_cores.size(); i++) {
                if (v_cores[i].id == core_id)
                        return i;
        }
        return -1;
}

void del_tc_by_id(struct core &b, int tc_id, int tc_idx)
{
        int gcd;
        int flag;
        vector<struct task> v_tasks;

        gcd = 0;
        flag = NO;

        for (unsigned int i = 0; i < b.v_tcs.size(); i++) {
                if (b.v_tcs[i].id == tc_id && b.v_tcs[i].tc_idx == tc_idx) {
                        b.v_tcs.erase(b.v_tcs.begin() + i);
                        b.v_tasks.clear();
                        flag = YES;
                }
        }

        if (flag == NO) {
                printf("tc_idx: %d\n", tc_idx);
                printf("ERR! TC %d idx: %d not removed from Core %d\n", tc_id, tc_idx, b.id);
                print_core(b);
                exit(0);
        }

        /* check if there is only LET tc */
        if (b.v_tcs.size() == 1)
                gcd = b.v_tcs[0].v_tasks[0].t;

        else {
                for (unsigned int j = 0; j < b.v_tcs.size(); j++) {
                        if (b.v_tcs[j].is_let == YES)
                                continue;
                        add_tasks_to_v_tasks(v_tasks, b.v_tcs[j].v_tasks);
                }
                gcd = cmp_gcd(v_tasks);
        }
        update_let(b, gcd);
        cmp_core_comcost(b);
}

void add_tasks_to_v_tasks(vector<struct task> &dst_v_tasks, 
                vector<struct task> &src_v_tasks)
{
        for (unsigned int i = 0; i < src_v_tasks.size(); i++)
                dst_v_tasks.push_back(src_v_tasks[i]);
}
