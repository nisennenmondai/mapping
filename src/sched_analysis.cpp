#include "print.h"
#include "sched_analysis.h"

static void _find_hp_tasks(vector<struct task> &v_tasks, vector<struct task> &hp_tasks,  
                struct task &tau, int &r_prev)
{
        for (unsigned int i = 0; i < v_tasks.size(); i++) {
                if (v_tasks[i].p < tau.p)
                        hp_tasks.push_back(v_tasks[i]);
        }
}

static void _fixedpoint(vector<struct task> &hp_tasks, struct task &tau, 
                int &r_curr, int &r_prev, int &ret)
{
        int tmp;

        tmp = tau.c;

        if (tau.c > tau.t) {
                printf("ERROR Execution Time %d of tau %d > Period %d\n\n", tau.c, tau.id, tau.t);
                exit(0);
        }

        for (unsigned int i = 0; i < hp_tasks.size(); i++)
                tmp += (ceilf((float)r_prev/(float)hp_tasks[i].t) * hp_tasks[i].c);

        r_curr = tmp;

        if (r_curr > tau.t) {
                tau.r = r_curr;
                ret = SCHED_FAILED;
                return;
        }

        if (r_curr > r_prev) {
                r_prev = r_curr;
                _fixedpoint(hp_tasks, tau, r_curr, r_prev, ret);
        }

        if (r_curr == r_prev) {
                tau.r = r_curr;
                ret = SCHED_OK;
                if (tau.r > tau.t) {
                        printf("ERR! tau.r: %d > tau.t: %d\n", tau.r, tau.t);
                        exit(0);
                }
                return;
        }
}

static void _save_priorities(struct bin &b)
{
        int p;

        p = 2;

        sort_inc_task_priority(b.v_tasks);

        for (unsigned int i = 0; i < b.v_tasks.size(); i++) {
                if (b.v_tasks[i].is_let == YES) {
                        b.v_tasks[i].p = 1;
                        continue;
                }
                b.v_tasks[i].p = p;
                p++;
        }
}

static void _reassign(struct bin &b, int p, int itm_idx)
{
        /* starting new p */
        int newp;
        vector<int> v_p;

        newp = p + 1;

        /* store hp */
        for (unsigned int i = 0; i < b.v_tasks.size(); i++) {
                if (b.v_tasks[i].is_let == YES)
                        continue;
                if (b.v_tasks[i].idx.itm_idx == itm_idx)
                        continue;

                if (b.v_tasks[i].p < p) {
                        v_p.push_back(b.v_tasks[i].p);
                }
        }
        /* sort in decreasing order of priority to start with lowest hp */
        sort_dec_int(v_p);

        /* reassign */
        for (unsigned int z = 0; z < v_p.size(); z++) {
                for (unsigned int i = 0; i < b.v_tasks.size(); i++) {
                        if (b.v_tasks[i].is_let == YES)
                                continue;
                        if (b.v_tasks[i].idx.itm_idx == itm_idx)
                                continue;

                        if (b.v_tasks[i].p == v_p[z]) {
                                for (unsigned int j = i; j < b.v_tasks.size(); j++) {
                                        if (b.v_tasks[j].p == p)
                                                continue;
                                        if (b.v_tasks[j].idx.itm_idx == itm_idx && b.v_tasks[j].p < p)
                                                continue;
                                        b.v_tasks[j].p = newp;
                                        newp++;
                                }
                        }
                }
                _save_priorities(b);

                b.flag = wcrt(b.v_tasks);
                if (b.flag == SCHED_OK) {
                        copy_back_prio_to_tc(b);
                        copy_back_resp_to_tc(b);
                        printf("Core %d SCHED_OK with reassignment\n", b.id);
                        return;

                } else {
                        b.flag = SCHED_FAILED;
                        return;
                }
        }
}

static void _base_assignment(struct bin &b)
{
        int p;

        p = 2;

        sort_inc_task_id(b.v_tasks);

        for (unsigned int i = 0; i < b.v_tasks.size(); i++) {
                if (b.v_tasks[i].is_let == YES) {
                        b.v_tasks[i].p = 1;
                        continue;
                }
                b.v_tasks[i].p = p;
                p++;
        }
        b.flag = wcrt(b.v_tasks);
        copy_back_prio_to_tc(b);
        copy_back_resp_to_tc(b);
}

static void _reassignment(struct bin &b)
{
        struct bin tmp_b;

        sort_inc_task_priority(b.v_tasks);

        /* iterate in descending order */
        for (unsigned int i = b.v_tasks.size() - 1; i > 0; i--) {
                if (b.v_tasks[i].p == 0) {
                        printf("\nERR! Core %d tau %d p %d idx %d\n", 
                                        b.id, b.v_tasks[i].id, b.v_tasks[i].p, 
                                        b.v_tasks[i].idx.itm_idx);
                        exit(0);
                }
                if (b.v_tasks[i].r > b.v_tasks[i].t) {
                        tmp_b = {0};
                        tmp_b = b;
                        /* priority swapping */
                        _reassign(tmp_b, b.v_tasks[i].p, tmp_b.v_tasks[i].idx.itm_idx);
                        if (tmp_b.flag == SCHED_OK) {
                                b = tmp_b;
                                return;
                        }
                }
        }
        if (b.flag == SCHED_OK)
                printf("Core %d SCHED_OK\n", b.id);
}

int wcrt(vector<struct task> &v_tasks)
{
        int ret;
        int flag;
        int r_curr;
        int r_prev;
        vector<struct task> hp_tasks;

        ret = -1;
        flag = -1;
        r_curr = 0;
        r_prev = 0;
        flag = SCHED_OK;

        /* compute WCRT for each task */
        for (unsigned int i = 0; i < v_tasks.size(); i++) {
                r_curr = 0;
                r_prev = 0;
                hp_tasks.clear();
                _find_hp_tasks(v_tasks, hp_tasks, v_tasks[i], r_prev);
                /* recursive */
                _fixedpoint(hp_tasks, v_tasks[i], r_curr, r_prev, ret);

                if (ret == SCHED_FAILED)
                        flag = SCHED_FAILED;
        }

        if (flag == SCHED_OK)
                return SCHED_OK;

        else if (flag == SCHED_FAILED)
                return SCHED_FAILED;
        else {
                printf("ERR! wcrt!\n");
                exit(0);
        }
        return -1;
}

void wcrt_bin(struct bin &b, int bin_idx)
{
        int ret;

        ret = -1;
        b.v_tasks.clear();

        for (unsigned int i = 0; i < b.v_itms.size(); i++)
                copy_tc_to_v_tasks_with_pos(b, bin_idx, i);

        /* test wcrt */
        ret = wcrt(b.v_tasks);
        if (ret == SCHED_OK) {
                printf("WCRT Core: %d OK!\n", b.id);
                b.flag = SCHED_OK;

        }

        else if (ret == SCHED_FAILED) {
                printf("WCRT Core: %d FAILED!\n", b.id);
                b.flag = SCHED_FAILED;
        }
        copy_back_prio_to_tc(b);
        copy_back_resp_to_tc(b);
        compute_bin_load_rem(b);
}

void priority_assignment(struct bin &b)
{
        _base_assignment(b);
        if (b.flag == SCHED_FAILED) {
                printf("Core %d SCHED_FAILED\n", b.id);
                _reassignment(b);
        } else
                printf("Core %d SCHED_OK\n", b.id);
}

void sched_analysis(vector<struct bin> &v_bins, struct context &ctx)
{
        sort_inc_bin_load_rem(v_bins);
        copy_v_tc_to_v_tasks_with_pos(v_bins);

        for (unsigned int i = 0; i < v_bins.size(); i++)
                priority_assignment(v_bins[i]);

        ctx.p.sched_rate_allo = sched_rate(v_bins, ctx);
        ctx.p.sched_imp_allo = ctx.sched_ok_count;
}
