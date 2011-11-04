
#ifndef JOHANN_EIGS_H
#define JOHANN_EIGS_H

#include "graph.h"

void compute_eigs (const SymmetricLinearForm & A,
                   float * eigs,
                   unsigned size,
                   unsigned num_eigs);


#endif // JOHANN_EIGS_H

