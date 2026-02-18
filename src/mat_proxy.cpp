#include "mat_proxy.hpp"
#include <iostream>

ripples::server::MAT::Mutation ripples::server::MATCopyHelper::copy_mutation(
    const OptimizeMAT::Mutation &mutation) const {
    // NOTE: Chromosome field left as empty string.
    MAT::Mutation m;
    m.position = mutation.get_position();
    // MatOptimize mutation has conversion operators to unpack uint8_t
    m.ref_nuc = mutation.get_ref_one_hot();
    m.par_nuc = mutation.get_par_one_hot();
    m.mut_nuc = mutation.get_mut_one_hot();
    return m;
}

