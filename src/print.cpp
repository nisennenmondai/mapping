#include "print.h"
#include "bench.h"

static void standard_deviation(vector<struct bin> &v_bins, struct context &ctx)
{
        float sum = 0.0;
        float mean = 0.0;
        float sumsqr = 0.0;
        float variance = 0.0;
        unsigned int n = v_bins.size();

        for (unsigned int i = 0; i < n; i++) {
                sum += (ctx.prm.phi - v_bins[i].cap_rem);
        }

        mean = sum / n;

        for (unsigned int i = 0; i < n; i++) {
                ctx.standard_dev = (ctx.prm.phi - v_bins[i].cap_rem) - mean;
                sumsqr += ctx.standard_dev * ctx.standard_dev;
        }
        variance = sumsqr / n;
        ctx.standard_dev = sqrt(variance) ;
}

static void approximation_ratio(vector<struct item> &v_itms, struct context &ctx)
{
        int min_nbr_cuts = 0;

        for (unsigned int i = 0; i < v_itms.size(); i++)  {
                if (v_itms[i].size > ctx.prm.phi)
                        min_nbr_cuts++;
        }
        ctx.opti_bins = (float)ctx.bins_count / (float)ctx.bins_min;
}

static void comp_time(struct context &ctx)
{
        ctx.redu_time = ctx.redu_time * MSEC;
        ctx.alloc_time = ctx.alloc_time * MSEC;
        ctx.frag_time = ctx.frag_time * MSEC;
        ctx.e_time = ctx.redu_time + ctx.alloc_time + ctx.frag_time;
}

void comp_stats(vector<struct bin> &v_bins, vector<struct item> &v_itms, 
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

        comp_time(ctx);
        standard_deviation(v_bins, ctx);
        approximation_ratio(v_itms, ctx);
}

void print_not_allocated(vector<struct item> &v_itms, struct context &ctx)
{
        for (int i = 0; i < ctx.prm.n; i++) {
                if (v_itms[i].is_allocated == NO)
                        printf("Item %d could not be allocated\n", 
                                        v_itms[i].id);
        }
}

void print_not_fragmented(vector<struct item> &v_itms, struct context &ctx)
{
        for (int i = 0; i < ctx.prm.n; i++) {
                if (v_itms[i].is_allocated == NO)
                        printf("Item %d could not be fragmented\n", 
                                        v_itms[i].id);
        }
}

void print_v_itms(vector<struct item> &v_itms, struct context &ctx)
{
        printf("+=====================================+\n");
        printf("| PRINT ITEMS                         |\n");
        printf("+=====================================+\n");

        for (int i = 0; i < ctx.prm.n; i++) {
                v_itms[i].id = i;
                printf("item.id: %u\n", v_itms[i].id);
                printf("item.size: %u\n", v_itms[i].size);
                printf("item.nbr_cut: %u\n", v_itms[i].nbr_cut);
                printf("set of cuts: ");

                for (int j = 0; j < v_itms[i].nbr_cut; j++) {
                        printf("{%u,%u} ", v_itms[i].tc.v_cuts[j].c_pair.first, 
                                        v_itms[i].tc.v_cuts[j].c_pair.second);
                }
                printf("\n\n");
        }
}

void print_v_bins(vector<struct bin> &v_bins, struct context &ctx)
{
        printf("+=====================================+\n");
        if (ctx.prm.a == BFDU_F)
                printf("| PRINT BINS BFDU_F                   |\n");
        if (ctx.prm.a == WFDU_F)
                printf("| PRINT BINS WFDU_F                   |\n");
        printf("+=====================================+\n\n");

        for (unsigned int i = 0; i < v_bins.size(); i++) {

                printf("+====================+\n");
                printf("|Bin %d:       \n", v_bins[i].id);
                printf("|Load:    %u\n", ctx.prm.phi - v_bins[i].cap_rem);
                printf("|--------------------|\n");

                for (unsigned int j = 0; j < v_bins[i].vc_itms.size(); j++) {

                        if (v_bins[i].vc_itms[j].is_frag == YES) {
                                printf("|Frag: %u size %u\n", 
                                                v_bins[i].vc_itms[j].id, 
                                                v_bins[i].vc_itms[j].size);
                        } else {
                                printf("|Item: %u size %u\n", 
                                                v_bins[i].vc_itms[j].id, 
                                                v_bins[i].vc_itms[j].size);
                        }


                }
                printf("+====================+\n\n");
        }
}

void print_vectors(vector<struct item> &v_itms, struct context &ctx)
{
        printf("\n+=====================================+\n");
        if (ctx.prm.a == BFDU_F)
                printf("| PRINT VECTORS BFDU_F                |\n");
        if (ctx.prm.a == WFDU_F)
                printf("| PRINT VECTORS WFDU_F                |\n");
        printf("+=====================================+\n");

        int count_not_alloc = 0;
        int count_is_alloc = 0;
        int count_frag = 0;
        int count_cut = 0;

        vector<struct item> *v_frags_bfdu_f;
        vector<struct item> *v_frags_wfdu_f;

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
        printf("Number of Items not allocated: %u\n", count_not_alloc);
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
                printf("Number of Items allocated: %d\n", 
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
                printf("Number of Items allocated: %d\n", 
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
}

void print_stats(vector<struct item> &v_itms, vector<struct bin> &v_bins, 
                struct context &ctx)
{
        printf("\n+=====================================+\n");
        if (ctx.prm.a == BFDU_F)
                printf("| PRINT STATS BFDU_F                  |\n");
        if (ctx.prm.a == WFDU_F)
                printf("| PRINT STATS WFDU_F                  |\n");
        printf("+=====================================+\n");

        comp_stats(v_bins, v_itms, ctx);

        printf("------------------------------------------->\n");
        printf("N:    %u\n", ctx.prm.n);
        printf("S:    %u\n", ctx.prm.s);
        printf("C:    %u\n", ctx.prm.c);
        printf("phi:  %u\n", ctx.prm.phi);
        if (ctx.prm.a == BFDU_F)
                printf("A:    BFDU_F\n");
        if (ctx.prm.a == WFDU_F)
                printf("A:    WFDU_F\n");
        printf("------------------------------------------->\n");
        printf("Min Number of Bins:   %d\n", ctx.bins_min);
        printf("Cycles Count:         %d\n", ctx.cycl_count);
        printf("Actual Bins Count:    %d\n", ctx.bins_count);
        printf("------------------------------------------->\n");
        printf("Items Allocated:      %d\n", ctx.alloc_count + ctx.frags_count);
        printf("------------------------------------------->\n");
        printf("Cuts Count:           %d\n", ctx.cuts_count);
        printf("Fragments Count:      %d\n", ctx.frags_count);
        printf("------------------------------------------->\n");
        printf("Reduction Time:       %f ms\n", ctx.redu_time);
        if (ctx.prm.a == BFDU_F) {
                printf("BFDU Time:            %f ms\n", ctx.alloc_time);
                printf("BFF Time:             %f ms\n", ctx.frag_time);
                printf("------------------------------------------->\n");
        }
        if (ctx.prm.a == WFDU_F) {
                printf("WFDU Time:            %f ms\n", ctx.alloc_time);
                printf("WFF Time:             %f ms\n", ctx.frag_time);
                printf("------------------------------------------->\n");

        }
        printf("Execution Time:       %f ms\n", ctx.e_time);
        printf("Load Distribution:    %f\n", ctx.standard_dev);
        printf("------------------------------------------->\n");
        printf("Approximation Ratio:  %f\n", ctx.opti_bins);
        printf("------------------------------------------->\n");
}


void print_task_chains(vector<struct item> &v_itms)
{
        int tasknbr = 0;

        for (unsigned int i = 0; i < v_itms.size(); i++) {
                printf("===========================\n");
                printf("tc.id: %d u: %d size: %lu\n", 
                                i, v_itms[i].tc.u, v_itms[i].tc.v_tasks.size());
                printf("===========================\n");
                for (unsigned int j = 0; j < v_itms[i].tc.v_tasks.size(); j++) {
                        printf("tau %d: u: %d  c: %d  t: %d\n",
                                        j, v_itms[i].tc.v_tasks[j].u, 
                                        v_itms[i].tc.v_tasks[j].c, 
                                        v_itms[i].tc.v_tasks[j].t);
                        tasknbr++;
                }
                printf("---------------------------\n");
                for (unsigned int j = 0; j < v_itms[i].tc.v_cuts.size(); j++) {
                        printf("{%d, %d} ", v_itms[i].tc.v_cuts[j].c_pair.first, 
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
                printf("---------------------------\n");
                printf("\n\n");
        }
        printf("Total Number of Tasks: %d\n", tasknbr);
        printf("Total Number of Task-Chains: %lu\n\n", v_itms.size());
}
