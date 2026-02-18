#pragma once

#include "src/mat_proxy.hpp"
#include <optional>

namespace ripples::server {

std::optional<std::vector<std::string>>
get_missing_sample_names(const MatOptimize::MAT::Tree &tree,
                         const std::vector<Sample_Muts> &missing_samples);

std::optional<std::vector<Missing_Sample>>
collect_missing_samples(const MAT::Tree &tree,
                        const std::vector<std::string> &missing_sample_names);

}; // namespace ripples::server

