#include "ripples_search.hpp"

ripples::ripples_search::ripples_search(
    MAT::Tree &tree, const std::vector<Missing_Sample> &samples,
    const server::ripples_parameters &params,
    const std::string &result_file_suffix)
    : runner_(tree, samples, params, result_file_suffix) {}
