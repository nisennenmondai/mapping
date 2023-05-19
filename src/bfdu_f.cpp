#include "let.h"
#include "mapping.h"

static int _find_best_bin(vector<struct bin> &v_bins, struct item &itm,
                struct context &ctx, int &best_load, int &best_gcd)
{
        int bin_id;
        int tmp_rem;
        int tmp_load;
        int tmp_gcd;
        int is_found;
        int best_rem;

        bin_id = 0;
        tmp_load = 0;
        tmp_gcd = 0;
        tmp_rem = 0;
        is_found = NO;
        best_rem = ctx.prm.phi;

        for (unsigned int i = 0; i < v_bins.size(); i++) {
                tmp_load = check_if_fit_itm(v_bins[i], itm, tmp_gcd);
                if (tmp_load <= v_bins[i].phi) {
                        tmp_rem = v_bins[i].load_rem - tmp_load;

                        if (tmp_rem < best_rem) {
                                best_rem = tmp_rem;
                                best_load = tmp_load;
                                best_gcd = tmp_gcd;
                                bin_id = v_bins[i].id;
                        }
                        is_found = YES;
                        continue;
                }
        }
        if (is_found == YES) 
                return bin_id;

        return -1;
}

void bfdu_f(vector<struct item> &v_itms, vector<struct bin> &v_bins, 
                struct context &ctx)
{
        int ret;
        int load;
        int gcd;
        int bin_id;
        int alloc_count;

        /* STEP - 1, place all possible items in bins using BFDU */
        printf("\n<--------------------------------------->\n");
        printf("STEP 1, BFDU_F\n");
        printf("<--------------------------------------->\n");
        while (alloc_count != ctx.prm.n) {
                alloc_count = 0;
                for (int i = 0; i < ctx.prm.n; i++) {
                        ret = 0;
                        gcd = 0;
                        load = 0;
                        bin_id = 0;
                        if (v_itms[i].is_allocated == YES) 
                                continue;

                        /* find best bin to fit itm */
                        ret = _find_best_bin(v_bins, v_itms[i], ctx, load, gcd);

                        /* bin found add itm to it */
                        if (ret > -1) {
                                printf("Best Bin to accomodate Item %d is Bin %d\n", 
                                                v_itms[i].id, ret);
                                bin_id = ret;
                                add_itm_to_v_bins(v_bins, v_itms[i], bin_id, ctx, 
                                                load, gcd);
                                v_itms[i].is_allocated = YES;
                                continue;

                                /* no bin was found */
                        } else if (ret == -1) {
                                printf("No Bin was found to accomodate Item %d\n", 
                                                v_itms[i].id);
                                add_bin(v_bins, ctx);
                                ctx.cycl_count++;
                                continue;
                        }
                }
                /* count remaining item to be allocated */
                for (int i = 0; i < ctx.prm.n; i++) {
                        if (v_itms[i].is_allocated == YES)
                                alloc_count++;
                }
        }
}
