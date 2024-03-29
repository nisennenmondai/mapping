#include "let.h"
#include "print.h"
#include "mapping.h"
#include "generator.h"

static int _find_best_core(vector<struct core> &v_cores, struct tc &tc,
                struct context &ctx, int &best_load, int &best_gcd, int color)
{
        int core_id;
        int tmp_rem;
        int tmp_load;
        int tmp_gcd;
        int is_found;
        int best_rem;

        core_id = 0;
        tmp_load = 0;
        tmp_gcd = 0;
        tmp_rem = 0;
        is_found = NO;
        best_rem = PHI;

        for (unsigned int i = 0; i < v_cores.size(); i++) {
                if (v_cores[i].color != color && color != WHITE) 
                        continue;

                tmp_load = check_if_fit_tc(v_cores[i], tc, tmp_gcd);
                if (tmp_load <= v_cores[i].phi) {
                        tmp_rem = v_cores[i].phi - tmp_load;
                        if (tmp_rem < best_rem) {
                                best_rem = tmp_rem;
                                best_load = tmp_load;
                                best_gcd = tmp_gcd;
                                core_id = v_cores[i].id;
                                is_found = YES;
                        }
                        continue;
                }
        }
        if (is_found == YES) 
                return core_id;

        return -1;
}

void bfdu(vector<struct tc> &v_tcs, vector<struct core> &v_cores, 
                struct context &ctx)
{
        int m;
        int ret;
        int load;
        int gcd;
        int core_id;
        int assign_count;
        int cycl_count;

        cycl_count = 0;
        m = v_tcs.size();

        /* STEP - 1, place all possible tcs in cores using BFDU */
        printf("\n<--------------------------------------->\n");
        printf("STEP 1, BFDU\n");
        printf("<--------------------------------------->\n");
        while (assign_count != m) {
                assign_count = 0;
                for (int i = 0; i < m; i++) {
                        ret = 0;
                        gcd = 0;
                        load = 0;
                        core_id = 0;
                        if (v_tcs[i].is_assign == YES) 
                                continue;

                        /* find best core to fit tc */
                        ret = _find_best_core(v_cores, v_tcs[i], ctx, load, gcd, v_tcs[i].color);

                        /* core found add tc to it */
                        if (ret != -1) {
                                printf("Best Core to accomodate TC %d idx: %d is core %d\n", 
                                                v_tcs[i].id, v_tcs[i].tc_idx, ret);
                                core_id = ret;
                                add_tc_to_v_cores(v_cores, v_tcs[i], core_id, 
                                                load, gcd);
                                v_tcs[i].is_assign = YES;
                                cycl_count = 0;
                                continue;

                                /* no core was found */
                        } else {
                                printf("No Core was found to accomodate TC %d idx: %d size: %d\n", 
                                                v_tcs[i].id, v_tcs[i].tc_idx, v_tcs[i].u);

                                if (v_tcs[i].color == WHITE) {
                                        add_core(v_cores, gen_rand(0, 5), 1, ctx);
                                        cycl_count++;

                                } else {
                                        add_core(v_cores, v_tcs[i].color, 1, ctx);
                                        cycl_count++;
                                } 

                                if (cycl_count > 1) {
                                        printf("ERR!: Impossible to assign all TC, System Unfeasible\n");
                                        STATE = FAILED;
                                        return;
                                }
                                continue;
                        }
                }
                /* count remaining tc to be allocated */
                for (int i = 0; i < m; i++) {
                        if (v_tcs[i].is_assign == YES)
                                assign_count++;
                }
        }
        verify_cores_load(v_cores);
}
