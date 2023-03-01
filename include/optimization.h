#ifndef OPTIMIZATION_H 
#define OPTIMIZATION_H

#include "mapping.h"

void priority_optimization(vector<struct bin> &v_bins, struct context &ctx);

void displacement_optimization(vector<struct bin> &v_bins, struct context &ctx);

void swapping_optimization(vector<struct bin> &v_bins, struct context &ctx);

#endif /* OPTIMIZATION_H */
