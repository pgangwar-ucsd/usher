#pragma once

#include "src/matOptimize/mutation_annotated_tree.hpp"
#include <iostream>
#include <optional>
#include <src/mutation_annotated_tree.hpp>
#include <stack>

namespace ripples::server {

// usher defined MAT Tree inferface
namespace MAT = Mutation_Annotated_Tree;
// matOptimize defined MAT Tree inferface
namespace OptimizeMAT = MatOptimize::Mutation_Annotated_Tree;

template <class T> struct is_tree_t {
    static_assert(std::is_same_v<std::decay_t<T>, MAT::Tree> ||
                      std::is_same_v<std::decay_t<T>, OptimizeMAT::Tree>,
                  "Type given to MATProxy is not a valid tree type");
};

// Usage:
// MAT::Tree tree;
// MATProxy proxy_tree(tree);
// std::optional<OptimizeMAT::Tree> opt_copied_tree = proxy_tree.clone();
// if (!opt_copied_tree) {
// 	...handle error that occurred during tree copy
// }
// OptimizeMAT::Tree copied_tree = opt_copied_tree.value();

template <class T> class MATProxy final : is_tree_t<T> {
  public:
    using OptionalTree = std::optional<T>;

    MATProxy(const T &tree) : tree_(tree) {}

    OptionalTree clone() const {
        using tree_t = std::decay_t<T>;
        if constexpr (std::is_same_v<tree_t, MAT::Tree>) {
            return copy_matoptimize_to_mat(tree_);
        } else {
            return copy_mat_to_matoptimize(tree_);
        }
    }

  private:
    static std::optional<MAT::Tree>
    copy_matoptimize_to_mat(const OptimizeMAT::Tree &tree);

    static std::optional<OptimizeMAT::Tree>
    copy_mat_to_matoptimize(const MAT::Tree &tree);

    const T &tree_;
};
}; // namespace ripples::server

