#pragma once

#include "mat_copy_helper.hpp"

namespace ripples::server {

template <class T> struct is_tree_t {
    static_assert(std::is_same_v<std::decay_t<T>, MAT::Tree> ||
                      std::is_same_v<std::decay_t<T>, OptimizeMAT::Tree>,
                  "Type given to MATProxy is not a valid tree type.");
};

// Usage:
// MAT::Tree tree;
// MATProxy proxy_tree(tree);
// auto opt_copied_tree = proxy_tree.clone();
// if (!opt_copied_tree) {
// 	...handle error that occurred during tree copy
// }
// auto copied_tree = opt_copied_tree.value();

template <class T> class MATProxy final : is_tree_t<T> {
  public:
    MATProxy(const T &tree) : tree_(tree) {}

    // Getter for underlying tree held
    const auto &data() const noexcept { return tree_; }

    auto clone(std::vector<MAT::Node> &preallocated_nodes) const {
        using tree_t = std::decay_t<T>;
        // If given tree type is MatOptimize::MAT, convert to MAT
        if constexpr (std::is_same_v<tree_t, MatOptimize::MAT::Tree>) {
            std::cerr << "Copying MatOptimize::MAT::Tree to MAT::Tree\n";
            MATCopyHelper helper(tree_);
            auto opt_copied_tree = helper.copy_to_mat(preallocated_nodes);
            return opt_copied_tree;
        }
        // Otherwise underlying tree type if MAT, conver to MatOptimize::MAT
        else {
            std::cerr << "Copying MAT::Tree to MatOptimize::MAT::Tree\n";
            assert(false);
            return copy_mat_to_matoptimize();
        }
    }

  private:
    const T &tree_;

    std::optional<MatOptimize::MAT::Tree> copy_mat_to_matoptimize() const {
        // TODO: Currently unused by any workflow
        // TODO: implement copy_mat_to_matoptimize
        return std::nullopt;
    }
};

}; // namespace ripples::server

