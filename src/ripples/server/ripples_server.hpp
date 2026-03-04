#pragma once

#include "ripples_runner.hpp"
#include "ripples_server_util.hpp"

namespace ripples::server {

class runner {
  public:
    using InputTree = MatOptimize::MAT::Tree;
    using MissingSamples = std::vector<Sample_Muts>;

    runner(InputTree &tree, const MissingSamples &missing_samples, std::vector<MAT::Node> &preallocated_nodes);

    bool operator()(const ripples_parameters &params);

  private:
    MATProxy<InputTree> proxy_tree_;
    const MissingSamples &missing_samples_;
    std::vector<MAT::Node> &preallocated_nodes_;
};
}; // namespace ripples::server

