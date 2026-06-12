#pragma once

#include "src/ripples/server/ripples_runner.hpp"

namespace ripples {

class ripples_search {
  public:
    ripples_search(MAT::Tree &tree, const std::vector<Missing_Sample> &samples,
                   const server::ripples_parameters &params,
                   const std::string &result_file_suffix);

    server::Status operator()() { return runner_(); }

  private:
    server::ripples_runner runner_;
};

}; // namespace ripples

