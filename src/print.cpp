#include "print.h"
#include "mapping.h"
#include "sched_analysis.h"

#define MSEC 1000

static void approximation_ratio(vector<struct item> &v_itms, struct context &ctx)
{
        int min_nbr_cuts;

        min_nbr_cuts = 0;

        for (unsigned int i = 0; i < v_itms.size(); i++)  {
                if (v_itms[i].size > ctx.prm.phi)
                        min_nbr_cuts++;
        }
        ctx.p.opti_bins = (float)ctx.bins_count / (float)ctx.bins_min;
}

static void execution_time(struct context &ctx)
{
        ctx.p.redu_time = ctx.p.redu_time * MSEC;
        ctx.p.alloc_time = ctx.p.alloc_time * MSEC;
        ctx.p.e_time = ctx.p.redu_time + ctx.p.alloc_time + ctx.p.wca_time + 
                ctx.p.reass_time + ctx.p.disp_time;
}

void cmp_stats(vector<struct bin> &v_bins, vector<struct item> &v_itms, 
                struct context &ctx)
{
        vector<struct item> *v_frags_bfdu_f;
        vector<struct item> *v_frags_wfdu_f;

        v_frags_bfdu_f = get_frags_bfdu_f();
        v_frags_wfdu_f = get_frags_wfdu_f();

        for (int i = 0; i < ctx.prm.n; i++) {
                if (v_itms[i].is_fragmented == YES) 
                        continue;

                if (v_itms[i].is_allocated == YES)
                        ctx.alloc_count++;
        }

        if (ctx.prm.a == BFDU_F) {
                for (unsigned int i = 0; i < v_frags_bfdu_f->size(); i++) {
                        if (v_frags_bfdu_f->at(i).is_frag == YES) {
                                ctx.frags_count++;
                        }
                }
        }

        if (ctx.prm.a == WFDU_F) {
                for (unsigned int i = 0; i < v_frags_wfdu_f->size(); i++) {
                        if (v_frags_wfdu_f->at(i).is_frag == YES) {
                                ctx.frags_count++;
                        }
                }
        }

        for (int i = 0; i < ctx.prm.n; i++) {
                if (v_itms[i].is_fragmented == YES) 
                        ctx.cuts_count++;
        }

        for (unsigned int i = 0; i < v_bins.size(); i++) {
                for (unsigned int j = 0; j < v_bins[i].v_tasks.size(); j++) {
                        ctx.tasks_count++;
                }
        }
        execution_time(ctx);
        approximation_ratio(v_itms, ctx);
}

void print_task_chains(vector<struct item> &v_itms)
{
        int tasknbr;

        tasknbr = 0;

        for (unsigned int i = 0; i < v_itms.size(); i++) {
                printf("====================================\n");
                printf("tc.id: %d u: %d tasks_nbr: %lu\n", 
                                v_itms[i].id, v_itms[i].tc.u, v_itms[i].tc.v_tasks.size());
                printf("====================================\n");
                for (unsigned int j = 0; j < v_itms[i].tc.v_tasks.size(); j++) {
                        printf("tau %d: u: %02d c: %02d t: %d\n",
                                        j, v_itms[i].tc.v_tasks[j].u, 
                                        v_itms[i].tc.v_tasks[j].c, 
                                        v_itms[i].tc.v_tasks[j].t);
                        tasknbr++;
                }
                printf("------------------------------------\n");
                for (unsigned int j = 0; j < v_itms[i].tc.v_cuts.size(); j++) {
                        printf("cut: %d {%02d, %02d} ", v_itms[i].tc.v_cuts[j].id, 
                                        v_itms[i].tc.v_cuts[j].c_pair.first, 
                                        v_itms[i].tc.v_cuts[j].c_pair.second);
                        printf("lf: ");

                        for (unsigned int k = 0; k < v_itms[i].tc.v_cuts[j].v_tasks_lf.size(); k++)
                                printf("%d", v_itms[i].tc.v_cuts[j].v_tasks_lf[k].id);

                        printf(" ");
                        printf("rf: ");

                        for (unsigned int k = 0; k < v_itms[i].tc.v_cuts[j].v_tasks_rf.size(); k++)
                                printf("%d", v_itms[i].tc.v_cuts[j].v_tasks_rf[k].id);

                        printf("\n");
                }
                printf("------------------------------------\n");
                printf("\n\n");
        }
        printf("Total Number of Tasks: %d\n", tasknbr);
        printf("Total Number of Task-Chains: %lu\n\n", v_itms.size());
}

void print_v_tasks(struct bin &b)
{
        for (unsigned int i = 0; i < b.v_tasks.size(); i++) {
                printf("tau %d :p %d sched %d\n", b.v_tasks[i].id, b.v_tasks[i].p, b.flag);
        }
}

void print_core(struct bin &b)
{
        for (unsigned int i = 0; i < b.v_itms.size(); i++) {
                for (unsigned int j = 0; j < b.v_itms[i].tc.v_tasks.size(); j++) {
                        printf("TC %d tau %d p: %-2d idx: %d sched: %d\n", 
                                        b.v_itms[i].id, b.v_itms[i].tc.v_tasks[j].id, 
                                        b.v_itms[i].tc.v_tasks[j].p,
                                        b.v_tasks[i].idx.itm_idx, b.flag);
                }
        }
}

void print_cores(vector<struct bin> &v_bins, struct context &ctx)
{
        printf("+=====================================+\n");
        if (ctx.prm.a == BFDU_F)
                printf("| PRINT CORE BFDU_F                   |\n");
        if (ctx.prm.a == WFDU_F)
                printf("| PRINT CORE WFDU_F                   |\n");
        printf("+=====================================+\n\n");
        sort_inc_bin_cap_rem(v_bins);

        for (unsigned int i = 0; i < v_bins.size(); i++) {

                printf("+==========================================+\n");
                printf("|Core: %d\n", v_bins[i].id);
                printf("|Load: %u\n", ctx.prm.phi - v_bins[i].cap_rem);
                printf("|Lrem: %d\n", v_bins[i].cap_rem);
                if (v_bins[i].flag == SCHED_OK)
                        printf("|Sched: OK\n");

                if (v_bins[i].flag == SCHED_FAILED)
                        printf("|Sched: FAILED\n");

                for (unsigned int j = 0; j < v_bins[i].v_itms.size(); j++) {

                        if (v_bins[i].v_itms[j].is_frag == YES) {
                                printf("|------------------------------------------|\n");
                                printf("|Frag-task-chain: %u size %u\n", 
                                                v_bins[i].v_itms[j].id, 
                                                v_bins[i].v_itms[j].size);
                                printf("|------------------------------------------|\n");
                                for (unsigned int k = 0; k < v_bins[i].v_itms[j].tc.v_tasks.size(); k++) {
                                        printf("|tau: %-2d u: %-2d p: %-2d r: %-3d c: %-2d t: %d", 
                                                        v_bins[i].v_itms[j].tc.v_tasks[k].id, 
                                                        v_bins[i].v_itms[j].tc.v_tasks[k].u,
                                                        v_bins[i].v_itms[j].tc.v_tasks[k].p,
                                                        v_bins[i].v_itms[j].tc.v_tasks[k].r,
                                                        v_bins[i].v_itms[j].tc.v_tasks[k].c,
                                                        v_bins[i].v_itms[j].tc.v_tasks[k].t);

                                        if (v_bins[i].v_itms[j].tc.v_tasks[k].r == -1)
                                                printf(" ---------------> deadline  missed!");
                                        if (v_bins[i].v_itms[j].tc.v_tasks[k].r == -1 && 
                                                        v_bins[i].flag == SCHED_OK) {
                                                printf("\nERR! \n");
                                                exit(0);

                                        } else
                                                printf("\n");
                                }

                        } else {
                                printf("|------------------------------------------|\n");
                                printf("|task-chain: %u size %u\n", 
                                                v_bins[i].v_itms[j].id, 
                                                v_bins[i].v_itms[j].size);

                                printf("|------------------------------------------|\n");
                                for (unsigned int k = 0; k < v_bins[i].v_itms[j].tc.v_tasks.size(); k++) {
                                        printf("|tau: %-2d u: %-2d p: %-2d r: %-3d c: %-2d t: %d", 
                                                        v_bins[i].v_itms[j].tc.v_tasks[k].id, 
                                                        v_bins[i].v_itms[j].tc.v_tasks[k].u,
                                                        v_bins[i].v_itms[j].tc.v_tasks[k].p,
                                                        v_bins[i].v_itms[j].tc.v_tasks[k].r,
                                                        v_bins[i].v_itms[j].tc.v_tasks[k].c,
                                                        v_bins[i].v_itms[j].tc.v_tasks[k].t);

                                        if (v_bins[i].v_itms[j].tc.v_tasks[k].r == -1)
                                                printf(" ---------------> deadline  missed!");
                                        if (v_bins[i].v_itms[j].tc.v_tasks[k].r == -1 && 
                                                        v_bins[i].flag == SCHED_OK) {
                                                printf("\nERR!\n");
                                                exit(0);

                                        } else
                                                printf("\n");
                                }
                        }
                }
                printf("+==========================================+\n\n");
        }
}

void print_vectors(vector<struct bin> &v_bins, vector<struct item> &v_itms, 
                struct context &ctx)
{
        printf("\n+=====================================+\n");
        if (ctx.prm.a == BFDU_F)
                printf("| PRINT VECTORS BFDU_F                |\n");
        if (ctx.prm.a == WFDU_F)
                printf("| PRINT VECTORS WFDU_F                |\n");
        printf("+=====================================+\n");

        int count_not_alloc;
        int count_is_alloc;
        int count_frag;
        int count_cut;
        int sched_ok;
        int sched_failed;

        vector<struct item> *v_frags_bfdu_f;
        vector<struct item> *v_frags_wfdu_f;

        count_not_alloc = 0;
        count_is_alloc = 0;
        count_frag = 0;
        count_cut = 0;
        sched_ok = 0;
        sched_failed = 0;

        v_frags_bfdu_f = get_frags_bfdu_f();
        v_frags_wfdu_f = get_frags_wfdu_f();

        printf("Vector:\n");
        for (int i = 0; i < ctx.prm.n; i++) {
                if (v_itms[i].is_allocated == NO) {
                        printf(" %u ", v_itms[i].size);
                        count_not_alloc++;
                }
        }
        printf("\n");
        printf("Number of Task-Chains not allocated: %u\n", count_not_alloc);
        printf("\n");

        printf("Vector:\n");
        for (int i = 0; i < ctx.prm.n; i++) {
                if (v_itms[i].is_fragmented == YES) 
                        continue;

                if (v_itms[i].is_allocated == YES) {
                        printf(" %u ", v_itms[i].size);
                        count_is_alloc++;
                }
        }

        if (ctx.prm.a == BFDU_F) {
                for (unsigned int i = 0; i < v_frags_bfdu_f->size(); i++) {
                        printf(" %uF ", v_frags_bfdu_f->at(i).size);
                        count_frag++;
                }
                printf("\n");
                printf("Number of Task-Chains allocated: %d\n", 
                                count_is_alloc + count_frag);
                printf("\n");

                printf("Vector:\n");
                for (unsigned int i = 0; i < v_frags_bfdu_f->size(); i++) {
                        if (v_frags_bfdu_f->at(i).is_frag == YES) 
                                printf(" %u ", v_frags_bfdu_f->at(i).size);
                }
        }

        if (ctx.prm.a == WFDU_F) {
                for (unsigned int i = 0; i < v_frags_wfdu_f->size(); i++) {
                        printf(" %uF ", v_frags_wfdu_f->at(i).size);
                        count_frag++;
                }
                printf("\n");
                printf("Number of Task-Chains allocated: %d\n", 
                                count_is_alloc + count_frag);
                printf("\n");

                printf("Vector:\n");
                for (unsigned int i = 0; i < v_frags_wfdu_f->size(); i++) {
                        if (v_frags_wfdu_f->at(i).is_frag == YES) 
                                printf(" %u ", v_frags_wfdu_f->at(i).size);
                }
        }

        printf("\n");
        printf("Number of Fragments: %u\n", count_frag);
        printf("\n");

        printf("Vector:\n");
        for (int i = 0; i < ctx.prm.n; i++) {
                if (v_itms[i].is_fragmented == YES) {
                        printf(" %u ", v_itms[i].size);
                        count_cut++;
                }
        }
        printf("\n");
        printf("Number of Cuts: %u\n", count_cut);
        printf("\n");

        printf("Vector:\n");
        for (unsigned int i = 0; i < v_bins.size(); i++) {
                if (v_bins[i].flag == SCHED_OK) {
                        printf("%d  ", v_bins[i].id);
                        sched_ok++;
                }
        }
        printf("\n");
        printf("Number of Cores SCHED_OK: %u\n", sched_ok);
        printf("\n");

        printf("Vector:\n");
        for (unsigned int i = 0; i < v_bins.size(); i++) {
                if (v_bins[i].flag == SCHED_FAILED) {
                        printf("%d  ", v_bins[i].id);
                        sched_failed++;
                }
        }
        printf("\n");
        printf("Number of Cores SCHED_FAILED: %u\n", sched_failed);
        printf("\n");
}

void print_stats(vector<struct item> &v_itms, vector<struct bin> &v_bins, 
                struct context &ctx)
{
        printf("\n+===========================================+\n");
        if (ctx.prm.a == BFDU_F)
                printf("| PRINT STATS BFDU_F                        |\n");
        if (ctx.prm.a == WFDU_F)
                printf("| PRINT STATS WFDU_F                        |\n");
        printf("+===========================================+\n");

        cmp_stats(v_bins, v_itms, ctx);

        printf("------------------------------------------------------------------------>\n");
        printf("n:      %u\n", ctx.prm.n);
        printf("phi:    %u\n", ctx.prm.phi);
        if (ctx.prm.a == BFDU_F)
                printf("a:      BFDU_F\n");
        if (ctx.prm.a == WFDU_F)
                printf("a:      WFDU_F\n");
        printf("------------------------------------------------------------------------>\n");
        printf("Min Number of Cores:   %d\n", ctx.bins_min);
        printf("New Added Cores:       %d\n", ctx.cycl_count);
        printf("Actual Cores Count:    %d\n", ctx.bins_count);
        printf("------------------------------------------------------------------------>\n");
        printf("Task-Chains Allocated: %d\n", ctx.alloc_count + ctx.frags_count);
        printf("Total Number of Tasks: %d\n", ctx.tasks_count);
        printf("------------------------------------------------------------------------>\n");
        printf("Cuts Count:            %d\n", ctx.cuts_count);
        printf("Fragments Count:       %d\n", ctx.frags_count);
        printf("------------------------------------------------------------------------>\n");
        printf("Reduction Time:                     %f ms\n", ctx.p.redu_time);
        printf("Allocation Time:                    %f ms\n", ctx.p.alloc_time);
        printf("Schedulability Analysis Time:       %f ms\n", ctx.p.wca_time);
        printf("Priority Optimization Time:         %f ms\n", ctx.p.reass_time);
        printf("Displacement Optimization Time:     %f ms\n", ctx.p.disp_time);
        printf("------------------------------------------------------------------------>\n");
        printf("Total Execution Time:               %f ms\n", ctx.p.e_time);
        printf("------------------------------------------------------------------------>\n");
        printf("Schedulability Rate :               %.2f\n", 
                        ctx.p.sched_rate_bef * PERCENT);
        printf("Schedulability Rate (prio):         %.2f  +%-2d cores\n", 
                        ctx.p.sched_rate_prio * PERCENT, ctx.p.sched_imp_prio);
        printf("Schedulability Rate (disp):         %.2f  +%-2d cores\n", 
                        ctx.p.sched_rate_disp * PERCENT, ctx.p.sched_imp_disp);
        printf("------------------------------------------------------------------------>\n");
        printf("Optimization Improvement:           %.2f\n", 
                        (ctx.p.sched_rate_disp * PERCENT - ctx.p.sched_rate_bef * PERCENT));
        printf("------------------------------------------------------------------------>\n");
        printf("Approximation Ratio:                %f\n", ctx.p.opti_bins);
        printf("------------------------------------------------------------------------>\n");
}
