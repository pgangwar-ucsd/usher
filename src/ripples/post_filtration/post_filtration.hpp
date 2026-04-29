#pragma once

#include "recombinant.hpp"

namespace ripples::filtration {

class post_filtration {
  public:
    using string = std::string;
    using Status = ripples::server::Status;

    post_filtration(MAT::Tree &tree, const string &recomb_file);
    Status write(const string &outfilepath);

  private:
    MAT::Tree &tree_;
    // Load recombination results file output from ripples-fast
    candidate_recombinants recombinants_;
};

}; // namespace ripples::filtration
