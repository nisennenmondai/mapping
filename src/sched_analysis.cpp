#include "sched_analysis.h"

static void _assign_unique_priorities(vector<struct bin> &v_bins)
{
        for (unsigned int i = 0; i < v_bins.size(); i++) {
                v_bins[i].v_tasks.clear();
                for (unsigned int j = 0; j < v_bins[i].v_itms.size(); j++) {
                        copy_tc_to_v_tasks_with_pos(v_bins[i], i, j);
                }
        }

        /* assign and copy back unique priorities to each tasks */
        for (unsigned int i = 0; i < v_bins.size(); i++) {
                assign_unique_priorities(v_bins[i]);
                copy_back_prio_to_tc(v_bins[i]);
        }
}

static int _search_unsched_task(vector<struct task> &v_tasks)
{
        int high_p;
        int flag;
        vector<int> v_p;

        flag = NO;
        high_p = -1;

        for (unsigned int i = 0; i < v_tasks.size(); i++) {
                if (v_tasks[i].r  > v_tasks[i].t) {
                        v_p.push_back(v_tasks[i].p);
                        flag = YES;
                }
        }

        if (flag == NO)
                return -1;

        else if (flag == YES) {
                /* look for unched task with highest priority */
                for (unsigned int i = 0; i < v_p.size(); i++) {
                        high_p = v_p[i];
                        if (high_p > v_p[i + 1])
                                high_p = v_p[i + 1];
                }
                return high_p;
        }
        return -1;
}

static void _find_hp_tasks(vector<struct task> &v_tasks, vector<struct task> &hp_tasks,  
                struct task &tau, int &r_prev)
{
        for (unsigned int i = 0; i < v_tasks.size(); i++) {
                if (v_tasks[i].p < tau.p)
                        hp_tasks.push_back(v_tasks[i]);
        }
}

static void _fixpoint(vector<struct task> &hp_tasks, struct task &tau, 
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
                //printf("Current Response Time of tau %d: %d > period %d exit loop\n\n", 
                //                tau.id, r_curr, tau.t);
                tau.r = r_curr;
                ret = SCHED_FAILED;
                return;
        }

        if (r_curr > r_prev) {
                r_prev = r_curr;
                _fixpoint(hp_tasks, tau, r_curr, r_prev, ret);
        }

        if (r_curr == r_prev) {
                //printf("WCRT of tau %d: %d\n", tau.id, r_curr);
                tau.r = r_curr;
                ret = SCHED_OK;
                if (tau.r > tau.t) {
                        printf("ERR! tau.r: %d > tau.t: %d\n", tau.r, tau.t);
                        exit(0);
                }
                return;
        }
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
                _fixpoint(hp_tasks, v_tasks[i], r_curr, r_prev, ret);

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

void wcrt_v_bins(vector<struct bin> &v_bins, struct context &ctx)
{
        _assign_unique_priorities(v_bins);

        for (unsigned int i = 0; i < v_bins.size(); i++) {
                for (unsigned int j = 0; j < v_bins[i].v_itms.size(); j++)
                        v_bins[i].flag = wcrt(v_bins[i].v_tasks);

                if (v_bins[i].flag == SCHED_FAILED) {
                        printf("Core %d SCHED_FAILED\n", v_bins[i].id);
                } else {
                        printf("Core %d SCHED_OK\n", v_bins[i].id);
                }
        }

        /* copy back new response time to original tasks in tc */
        for (unsigned int i = 0; i < v_bins.size(); i++)
                copy_back_resp_to_tc(v_bins[i]);
}

float sched_rate(vector<struct bin> &v_bins, struct context &ctx)
{
        float sched_rate;

        sched_rate = 0.0;
        ctx.sched_ok_count = 0;
        ctx.sched_failed_count = 0;

        for (unsigned int i = 0; i < v_bins.size(); i++) {
                if (v_bins[i].flag == SCHED_OK)
                        ctx.sched_ok_count++;
        }

        for (unsigned int i = 0; i < v_bins.size(); i++) {
                if (v_bins[i].flag == SCHED_FAILED)
                        ctx.sched_failed_count++;
        }

        sched_rate = (float)ctx.sched_ok_count / (float)ctx.bins_count;

        return sched_rate;
}

void assign_unique_priorities(struct bin &b)
{
        sort_inc_task_id(b.v_tasks);

        for (unsigned int i = 0; i < b.v_tasks.size(); i++)
                b.v_tasks[i].p = i + 1;
}

void assign_new_priorities(struct bin &b, int p, int itm_idx)
{
        sort_inc_task_id(b.v_tasks);
        /* starting new p */
        p = p + 1;
        //printf("Priorities Reassignment... with p: %d\n", p);

        /* assign new priorities */
        for (unsigned int i = 0; i < b.v_tasks.size(); i++) {
                if (b.v_tasks[i].idx.itm_idx == (int)itm_idx)
                        continue;

                if (b.v_tasks[i].p < p) {
                        for (unsigned int j = 0; j < b.v_itms[itm_idx].v_tasks.size(); j++) {
                                if (b.v_itms[itm_idx].v_tasks[j].p == p) {
                                        //printf("priority %d already assigned\n", p);
                                        p++;
                                }
                        }
                        b.v_tasks[i].p = p;
                        //printf("tau %d p: %d itm_idx: %d\n", b.v_tasks[i].id, b.v_tasks[i].p, b.v_tasks[i].idx.itm_idx);
                        p = p + 1;
                }
        }
}

void reassign(struct bin &b, int &p, int itm_idx)
{
        int flag;
        struct bin b_tmp;

        flag = -1;
        b_tmp = b;

        assign_new_priorities(b_tmp, p, itm_idx);

        /* test if schedulable */
        flag = wcrt(b_tmp.v_tasks);
        b_tmp.flag = flag;
        copy_back_resp_to_tc(b_tmp);
        copy_back_prio_to_tc(b_tmp);

        if (flag == SCHED_OK) {
                b = b_tmp;
                printf("Core %d SCHED_OK with new priority assignment\n", 
                                b_tmp.id);
                for (unsigned int i = 0; i < b_tmp.v_tasks.size(); i++) {
                        if (b_tmp.v_tasks[i].r > b_tmp.v_tasks[i].t) {
                                printf("p: %d tau.id: %d r: %d t: %d\n", 
                                                b_tmp.v_tasks[i].p, 
                                                b_tmp.v_tasks[i].id, 
                                                b_tmp.v_tasks[i].r, 
                                                b_tmp.v_tasks[i].t);
                                printf("ERR! wcrt should have failed\n");
                                exit(0);
                        }
                }
                return;

        } else if (flag == SCHED_FAILED) {
                //printf("Core %d SCHED_FAILED with new priority assignment\n\n", 
                //                b_tmp.id);
                return;

        } else {
                printf("ERR! priority reassignment\n");
                exit(0);
        }
}

void reassign_bin(struct bin &b)
{
        int p;

        p = -1;
        /* detect bin not schedulable */
        for (unsigned int j = 0; j < b.v_itms.size(); j++) {
                p = _search_unsched_task(b.v_itms[j].v_tasks);

                /* if no unscheduled task found go to next itm */
                if (p == -1) 
                        continue;
                else
                        reassign(b, p, j);
        }
}

void reassignment(vector<struct bin> &v_bins)
{
        int p;

        p = -1;
        /* detect bin not schedulable */
        for (unsigned int i = 0; i < v_bins.size(); i++) {
                if (v_bins[i].flag == SCHED_FAILED) {
                        for (unsigned int j = 0; j < v_bins[i].v_itms.size(); j++) {
                                p = _search_unsched_task(v_bins[i].v_itms[j].v_tasks);

                                /* if no unscheduled task found go to next itm */
                                if (p == -1) 
                                        continue;
                                else
                                        reassign(v_bins[i], p, j);
                        }
                }
        }
}
