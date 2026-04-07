#pragma once

#include "src/ripples/util/copy/mat_proxy.hpp"
#include <optional>

namespace ripples::server {

std::pair<std::optional<std::vector<std::string>>, std::string>
get_missing_sample_names(const MatOptimize::MAT::Tree &tree,
                         const std::vector<Sample_Muts> &missing_samples);

std::pair<std::optional<std::vector<Missing_Sample>>, std::string>
collect_missing_samples(const MAT::Tree &tree,
                        const std::vector<std::string> &missing_sample_names);

}; // namespace ripples::server

