#include "print.h"
#include "mapping.h"
#include "sched_analysis.h"

static int _count_pcu(vector<struct core> &v_cores)
{
        int count;

        count = 0;

        for (unsigned int i = 0; i < v_cores.size(); i++) {
                if (v_cores[i].is_empty == YES)
                        continue;
                if (v_cores[i].color == RED)
                        count++;
        }
        return count;
}

static int _count_zcu(vector<struct core> &v_cores)
{
        int count;

        count = 0;

        for (unsigned int i = 0; i < v_cores.size(); i++) {
                if (v_cores[i].is_empty == YES)
                        continue;
                if (v_cores[i].color != RED)
                        count++;
        }
        return count;
}

static void _rst_let_task(vector<struct core> &v_cores)
{
        for (unsigned int i = 0; i < v_cores.size(); i++) {
                for (unsigned int j = 0; j < v_cores[i].v_tcs.size(); j++) {
                        if (v_cores[i].v_tcs.size() == 1 && 
                                        v_cores[i].v_tcs[j].is_let == YES) {
                                v_cores[i].v_tcs[j].size = 0;
                                v_cores[i].v_tcs[j].gcd = 0;
                                v_cores[i].v_tcs[j].v_tasks[0] = {0};
                                v_cores[i].load = 0;
                                v_cores[i].load_rem = 0;
                                v_cores[i].is_empty = YES;
                        }
                }
        }
}

static void _execution_time(struct context &ctx)
{
        ctx.p.et = ctx.p.part_time+ ctx.p.allo_time + ctx.p.schd_time + 
                ctx.p.disp_time + ctx.p.swap_time;

        ctx.p.plac_time = ctx.p.disp_time + ctx.p.swap_time;
}

static void _schedulability_rate(struct context &ctx)
{
        ctx.p.sched_rate_allo = ctx.p.sched_rate_allo * PERCENT;
        ctx.p.swap_gain = (ctx.p.sched_rate_swap * PERCENT) - ctx.p.sched_rate_allo;
        ctx.p.disp_gain = (ctx.p.sched_rate_disp * PERCENT) - (ctx.p.sched_rate_swap * PERCENT);
        ctx.p.opti_gain = ctx.p.swap_gain + ctx.p.disp_gain;
}

static void _utilization_rate(vector<struct core> &v_cores, struct context &ctx)
{
        ctx.p.letu = 0.0;
        ctx.p.appu = 0.0;
        ctx.p.unuu = 0.0;
        ctx.p.maxu = 0.0;

        for (unsigned int i = 0; i < v_cores.size(); i++) {
                ctx.p.appu += v_cores[i].load;
                ctx.p.unuu += v_cores[i].load_rem;
                for (unsigned int j = 0; j < v_cores[i].v_tcs.size(); j++) {
                        if (v_cores[i].v_tcs.size() == 1 && 
                                        v_cores[i].v_tcs[j].is_let == YES)
                                continue;
                        if (v_cores[i].v_tcs[j].is_let == YES) 
                                ctx.p.letu += v_cores[i].v_tcs[j].size;
                }
        }
        /* substract let utilization to global sys */
        ctx.p.appu -= ctx.p.letu;
        ctx.p.letu /= PERMILL;
        ctx.p.appu /= PERMILL;
        ctx.p.unuu /= PERMILL;
        ctx.p.maxu = ctx.cores_count * ((float)PHI / (float)PERMILL);
}

float sched_rate(vector<struct core> &v_cores, struct context &ctx)
{
        float sched_rate;

        sched_rate = 0.0;
        ctx.sched_ok_count = 0;
        ctx.sched_failed_count = 0;

        for (unsigned int i = 0; i < v_cores.size(); i++) {
                if (v_cores[i].flag == SCHED_OK)
                        ctx.sched_ok_count++;
        }

        for (unsigned int i = 0; i < v_cores.size(); i++) {
                if (v_cores[i].flag == SCHED_FAILED)
                        ctx.sched_failed_count++;
        }

        sched_rate = (float)ctx.sched_ok_count / (float)(ctx.sched_ok_count + ctx.sched_failed_count);

        return sched_rate;
}

void cmp_stats(vector<struct core> &v_cores, vector<struct tc> &v_tcs, 
                struct context &ctx)
{
        for (unsigned int i = 0; i < v_cores.size(); i++) {
                for (unsigned int j = 0; j < v_cores[i].v_tasks.size(); j++) {
                        ctx.tasks_count++;
                        if (v_cores[i].v_tcs.size() == 1 && v_cores[i].v_tcs[j].is_let == YES)
                                ctx.cores_count--;
                }
        }
        _schedulability_rate(ctx);
        _execution_time(ctx);
        _utilization_rate(v_cores, ctx);
}

void print_task_chains(vector<struct tc> &v_tcs)
{
        int tasknbr;

        tasknbr = 0;

        sort_inc_tc_color(v_tcs);
        for (unsigned int i = 0; i < v_tcs.size(); i++) {
                if (v_tcs[i].color == RED)
                        printf("\033[0;31m");
                else if (v_tcs[i].color == BLUE)
                        printf("\033[0;34m");
                else if (v_tcs[i].color == YELLOW)
                        printf("\033[0;33m");
                else if (v_tcs[i].color == GREEN)
                        printf("\033[0;32m");
                else if (v_tcs[i].color == CYAN)
                        printf("\033[0;36m");
                else if (v_tcs[i].color == PURPLE)
                        printf("\033[0;35m");
                else
                        printf("\033[0;37m");
                printf("======================================================\n");
                printf("tc.id: %-3d tc.idx: %-3d u: %.3f color: %d\n", 
                                v_tcs[i].id, v_tcs[i].tc_idx, 
                                (float)v_tcs[i].size / PERMILL, 
                                v_tcs[i].color);
                printf("======================================================\n");
                for (unsigned int j = 0; j < v_tcs[i].v_tasks.size(); j++) {
                        printf("tau: %-2d u: %.3f c: %-6d t: %-6d uniq_id: %-3d\n",
                                        v_tcs[i].v_tasks[j].task_id, 
                                        v_tcs[i].v_tasks[j].u / PERMILL, 
                                        v_tcs[i].v_tasks[j].c, 
                                        v_tcs[i].v_tasks[j].t,
                                        v_tcs[i].v_tasks[j].uniq_id);
                        tasknbr++;
                }
                printf("------------------------------------------------------\n");
                printf("\033[0m");
                printf("\n\n");
        }
        printf("Total Number of Tasks: %d\n", tasknbr);
        printf("Total Number of Task-Chains: %lu\n\n", v_tcs.size());
}

void print_core(struct core &b)
{
        printf("Core: %d Load: %d Lrem: %d \n", b.id, b.load, b.load_rem);
        for (unsigned int i = 0; i < b.v_tcs.size(); i++) {
                for (unsigned int j = 0; j < b.v_tcs[i].v_tasks.size(); j++) {
                        printf("TC %-3d || tau %-3d p: %-3d c: %-6d t: %-6d u: %-3f\n", 
                                        b.v_tcs[i].id, 
                                        b.v_tcs[i].v_tasks[j].task_id, 
                                        b.v_tcs[i].v_tasks[j].p,
                                        b.v_tcs[i].v_tasks[j].c,
                                        b.v_tcs[i].v_tasks[j].t,
                                        b.v_tcs[i].v_tasks[j].u);
                }
        }
        printf("----------------------------------------------------------------------------\n");
        for (unsigned int i = 0; i < b.v_tasks.size(); i++)
                printf("tau %-3d uniq_id: %-3d u: %-3d p %-3d tc_idx %-3d tc_id: %-3d\n", 
                                b.v_tasks[i].task_id, b.v_tasks[i].uniq_id, (int)b.v_tasks[i].u, 
                                b.v_tasks[i].p, b.v_tasks[i].idx.tc_idx, 
                                b.v_tasks[i].tc_id);
        printf("\n");
}

void print_cores(vector<struct core> &v_cores, struct context &ctx)
{
        sort_inc_core_color(v_cores);
        _rst_let_task(v_cores);
        printf("+=====================================+\n");
        if (ctx.prm.a == BFDU)
                printf("| PRINT CORE BFDU                     |\n");
        if (ctx.prm.a == WFDU)
                printf("| PRINT CORE WFDU                     |\n");
        if (ctx.prm.a == FFDU)
                printf("| PRINT CORE FFDU                     |\n");
        printf("+=====================================+\n\n");

        for (unsigned int i = 0; i < v_cores.size(); i++)
                cmp_core_comcost(v_cores[i]);

        for (unsigned int i = 0; i < v_cores.size(); i++) {
                printf("+==============================================================+\n");
                printf("|Core: %d\n", v_cores[i].id);
                printf("|Capa: %.3f\n", (float)v_cores[i].phi / PERMILL);
                printf("|Load: %.3f\n", (float)v_cores[i].load / PERMILL);
                if (v_cores[i].is_empty == YES)
                        v_cores[i].load_rem = v_cores[i].phi;
                printf("|Lrem: %.3f\n", ((float)v_cores[i].load_rem / PERMILL));
                printf("|LETc: %d\n", v_cores[i].comcost);
                printf("|");
                if (v_cores[i].color == RED)
                        printf("\033[0;31m");
                else if (v_cores[i].color == BLUE)
                        printf("\033[0;34m");
                else if (v_cores[i].color == YELLOW)
                        printf("\033[0;33m");
                else if (v_cores[i].color == GREEN)
                        printf("\033[0;32m");
                else if (v_cores[i].color == CYAN)
                        printf("\033[0;36m");
                else 
                        printf("\033[0;35m"); /* purple */
                printf("Colr: %d\n", v_cores[i].color);
                printf("\033[0m");
                if (v_cores[i].is_empty == NO)
                        printf("|Empt:  NO\n");
                else
                        printf("|Empt:  YES\n");
                if (v_cores[i].flag == SCHED_OK)
                        printf("|Sched: OK\n");

                if (v_cores[i].flag == SCHED_FAILED)
                        printf("|Sched: FAILED\n");

                for (unsigned int j = 0; j < v_cores[i].v_tcs.size(); j++) {
                        if (v_cores[i].v_tcs[j].is_let == YES){
                                printf("|--------------------------------------------------------------|\n");
                                printf("|LET: %-3d u %.3f gcd %-6d\n", 
                                                v_cores[i].v_tcs[j].id, 
                                                (float)v_cores[i].v_tcs[j].size / PERMILL,
                                                v_cores[i].v_tcs[j].gcd);
                                printf("|--------------------------------------------------------------|\n");
                                for (unsigned int k = 0; k < v_cores[i].v_tcs[j].v_tasks.size(); k++) {
                                        printf("|tau: %-2d u: %-.3f p: %-2d r: %-8d c: %-8d t: %d", 
                                                        v_cores[i].v_tcs[j].v_tasks[k].task_id, 
                                                        v_cores[i].v_tcs[j].v_tasks[k].u / PERMILL,
                                                        v_cores[i].v_tcs[j].v_tasks[k].p,
                                                        v_cores[i].v_tcs[j].v_tasks[k].r,
                                                        v_cores[i].v_tcs[j].v_tasks[k].c,
                                                        v_cores[i].v_tcs[j].v_tasks[k].t);

                                        if (v_cores[i].v_tcs[j].v_tasks[k].r > v_cores[i].v_tcs[j].v_tasks[k].t)
                                                printf(" ---------------> deadline  missed!");
                                        if (v_cores[i].v_tcs[j].v_tasks[k].r == -1 && 
                                                        v_cores[i].flag == SCHED_OK) {
                                                printf("\nERR! \n");
                                                exit(0);

                                        } else
                                                printf("\n");
                                }
                                printf("\033[0m");

                        } else {
                                printf("|--------------------------------------------------------------|\n");
                                if (v_cores[i].v_tcs[j].color == RED)
                                        printf("\033[0;31m");
                                else if (v_cores[i].v_tcs[j].color == BLUE)
                                        printf("\033[0;34m");
                                else if (v_cores[i].v_tcs[j].color == YELLOW)
                                        printf("\033[0;33m");
                                else if (v_cores[i].v_tcs[j].color == GREEN)
                                        printf("\033[0;32m");
                                else if (v_cores[i].v_tcs[j].color == CYAN)
                                        printf("\033[0;36m");
                                else if (v_cores[i].v_tcs[j].color == PURPLE)
                                        printf("\033[0;35m");
                                else
                                        printf("\033[0;37m");
                                printf("|TC:  %-3d tc_idx %d u %.3f gcd %-6d color %d\n", 
                                                v_cores[i].v_tcs[j].id, 
                                                v_cores[i].v_tcs[j].tc_idx, 
                                                (float)v_cores[i].v_tcs[j].size,
                                                v_cores[i].v_tcs[j].gcd,
                                                v_cores[i].v_tcs[j].color);
                                printf("\033[0m");
                                printf("|--------------------------------------------------------------|\n");
                                if (v_cores[i].v_tcs[j].color == RED)
                                        printf("\033[0;31m");
                                else if (v_cores[i].v_tcs[j].color == BLUE)
                                        printf("\033[0;34m");
                                else if (v_cores[i].v_tcs[j].color == YELLOW)
                                        printf("\033[0;33m");
                                else if (v_cores[i].v_tcs[j].color == GREEN)
                                        printf("\033[0;32m");
                                else if (v_cores[i].v_tcs[j].color == CYAN)
                                        printf("\033[0;36m");
                                else if (v_cores[i].v_tcs[j].color == PURPLE)
                                        printf("\033[0;35m");
                                else
                                        printf("\033[0;37m");
                                for (unsigned int k = 0; k < v_cores[i].v_tcs[j].v_tasks.size(); k++) {
                                        printf("|tau: %-2d u: %-.3f p: %-2d r: %-8d c: %-8d t: %d", 
                                                        v_cores[i].v_tcs[j].v_tasks[k].task_id, 
                                                        v_cores[i].v_tcs[j].v_tasks[k].u / PERMILL,
                                                        v_cores[i].v_tcs[j].v_tasks[k].p,
                                                        v_cores[i].v_tcs[j].v_tasks[k].r,
                                                        v_cores[i].v_tcs[j].v_tasks[k].c,
                                                        v_cores[i].v_tcs[j].v_tasks[k].t);

                                        if (v_cores[i].v_tcs[j].v_tasks[k].r > v_cores[i].v_tcs[j].v_tasks[k].t)
                                                printf(" ---------------> deadline  missed!");
                                        if (v_cores[i].v_tcs[j].v_tasks[k].r == -1 && 
                                                        v_cores[i].flag == SCHED_OK) {
                                                printf("\nERR!\n");
                                                exit(0);

                                        } else
                                                printf("\n");
                                }
                                printf("\033[0m");
                        }
                }
                printf("+==============================================================+\n\n");
        }
}

void print_vectors(vector<struct core> &v_cores, vector<struct tc> &v_tcs, 
                struct context &ctx)
{
        printf("\n+=====================================+\n");
        if (ctx.prm.a == BFDU)
                printf("| PRINT VECTORS BFDU                  |\n");
        if (ctx.prm.a == WFDU)
                printf("| PRINT VECTORS WFDU                  |\n");
        if (ctx.prm.a == FFDU)
                printf("| PRINT VECTORS FFDU                  |\n");
        printf("+=====================================+\n");

        int sched_ok;
        int sched_failed;

        sched_ok = 0;
        sched_failed = 0;

        for (int i = 0; i < ctx.prm.n; i++) {
                if (v_tcs[i].is_allocated == NO) {
                        printf("ERR! some TC were not allocated!\n");
                        exit(0);
                }
        }

        printf("Vector:\n");
        for (unsigned int i = 0; i < v_tcs.size(); i++) {
                if (v_tcs[i].is_allocated == YES) {
                        printf(" %u ", v_tcs[i].size);
                        ctx.alloc_count++;
                }
        }
        printf("\n");
        printf("Number of Task-Chains allocated: %u\n", ctx.alloc_count);
        printf("\n");

        printf("Vector:\n");
        for (unsigned int i = 0; i < v_cores.size(); i++) {
                if (v_cores[i].flag == SCHED_OK) {
                        printf("%d  ", v_cores[i].id);
                        sched_ok++;
                }
        }
        printf("\n");
        printf("Number of Cores SCHED_OK: %u\n", sched_ok);
        printf("\n");

        printf("Vector:\n");
        for (unsigned int i = 0; i < v_cores.size(); i++) {
                if (v_cores[i].flag == SCHED_FAILED) {
                        printf("%d  ", v_cores[i].id);
                        sched_failed++;
                }
        }
        printf("\n");
        printf("Number of Cores SCHED_FAILED: %u\n", sched_failed);
        printf("\n");
}

void print_stats(vector<struct tc> &v_tcs, vector<struct core> &v_cores, 
                struct context &ctx)
{
        printf("\n+===========================================+\n");
        if (ctx.prm.a == BFDU)
                printf("| PRINT STATS BFDU                          |\n");
        if (ctx.prm.a == WFDU)
                printf("| PRINT STATS WFDU                          |\n");
        if (ctx.prm.a == FFDU)
                printf("| PRINT STATS FFDU                          |\n");
        printf("+===========================================+\n");

        cmp_stats(v_cores, v_tcs, ctx);

        printf("------------------------------------------------------------------------>\n");
        printf("n:      %u\n", ctx.prm.n);
        printf("phi:    %u\n", PHI);
        printf("sigma:  %u\n", ctx.prm.s);
        if (ctx.prm.a == BFDU)
                printf("alpha:  BFDU\n");
        if (ctx.prm.a == WFDU)
                printf("alpha:  WFDU\n");
        if (ctx.prm.a == FFDU)
                printf("alpha:  FFDU\n");
        printf("------------------------------------------------------------------------>\n");
        printf("Initial Number of Cores:       %-d\n", ctx.init_cores_count);
        printf("Number of Cores in Use   (M):  %-3d     PCU: %-3d ZCU: %-3d\n", 
                        ctx.cores_count, _count_pcu(v_cores), _count_zcu(v_cores));
        printf("Optimal Number of Cores (M*):  %-3d\n", ctx.cores_min);
        printf("------------------------------------------------------------------------>\n");
        printf("Initial Number of TC:          %-3d\n", ctx.prm.n);
        printf("Current Number of TC:          %-3ld\n", v_tcs.size());
        printf("Number of TC Partitioned:      %-3d\n", ctx.frags_count);
        printf("Number of TC Allocated:        %-3d\n", ctx.alloc_count);
        printf("Number of Tasks:               %-3d\n", ctx.tasks_count);
        printf("------------------------------------------------------------------------>\n");
        printf("Partitioning Time:                %-3.3f ms\n", ctx.p.part_time * PERMILL);
        printf("Allocation Time:                  %-3.3f ms\n", ctx.p.allo_time * PERMILL);
        printf("Schedulability Analysis Time:     %-3.3f ms\n", ctx.p.schd_time * PERMILL);
        printf("Swapping Time:                    %-3.3f ms\n", ctx.p.swap_time * PERMILL);
        printf("Displacement Time:                %-3.3f ms\n", ctx.p.disp_time * PERMILL);
        printf("Placement Time:                   %-3.3f ms\n", ctx.p.plac_time * PERMILL);
        printf("------------------------------------------------------------------------>\n");
        printf("Schedulability Rate (allo):       %-3.3f\n", ctx.p.sched_rate_allo);
        printf("Schedulability Rate (swap):       %-3.3f  +%-2d cores\n", 
                        ctx.p.sched_rate_swap * PERCENT, ctx.p.sched_imp_swap);
        printf("Schedulability Rate (disp):       %-3.3f  +%-2d cores\n", 
                        ctx.p.sched_rate_disp * PERCENT, ctx.p.sched_imp_disp);
        printf("------------------------------------------------------------------------>\n");
        printf("Swapping SR Gain:                +%-3.3f\n", ctx.p.swap_gain);
        printf("Displacement SR Gain:            +%-3.3f\n", ctx.p.disp_gain);
        printf("Total Optimization SR Gain:      +%-3.3f\n", ctx.p.opti_gain);
        printf("------------------------------------------------------------------------>\n");
        printf("Total APP Utilization (M)         %-3.3f\n", (ctx.p.appu / ctx.p.maxu) * PERCENT);
        printf("Total LET Utilization (M)         %-3.3f\n", (ctx.p.letu / ctx.p.maxu) * PERCENT);
        printf("------------------------------------------------------------------------>\n");
        printf("Total Execution Time:             %-3.3f ms\n", ctx.p.et * MSEC);
        printf("------------------------------------------------------------------------>\n");
}
