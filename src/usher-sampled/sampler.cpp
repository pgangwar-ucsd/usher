#include "src/matOptimize/mutation_annotated_tree.hpp"
#include "usher.hpp"
#include <algorithm>
#include <climits>
#include <signal.h>
#include <unordered_map>
#include <vector>
#include <taskflow/taskflow.hpp>
#include <memory>

namespace MAT = MatOptimize::Mutation_Annotated_Tree;

static void add_mut(std::unordered_map<int, uint8_t> &output,
                    const MAT::Mutation &mut) {
    auto result = output.emplace(mut.get_position(), mut.get_mut_one_hot());
    if (!result.second) {
        result.first->second |= mut.get_mut_one_hot();
    }
}

struct Assign_Descendant_Possible_Muts_Cont {
    std::unordered_map<int, uint8_t> &output;
    std::vector<std::unordered_map<int, uint8_t>> children_out;
    MAT::Node *root;
    Assign_Descendant_Possible_Muts_Cont(
        std::unordered_map<int, uint8_t> &output, size_t child_count,
        MAT::Node *root)
        : output(output), children_out(child_count), root(root) {}

    void execute() {
        auto reserve_size = root->mutations.size();
        for (const auto &child_mut : children_out) {
            reserve_size += child_mut.size();
        }
        output.reserve(reserve_size);
        for (const auto &child_mut_vec : children_out) {
            for (const auto &mut : child_mut_vec) {
                auto result = output.emplace(mut.first, mut.second);
                if (!result.second) {
                    result.first->second |= mut.second;
                }
            }
        }
        for (auto &mut : root->mutations) {
            auto result =
                output.emplace(mut.get_position(), mut.get_mut_one_hot());
            if (!result.second) {
                result.first->second |= mut.get_mut_one_hot();
            }
            mut.set_descendant_mut(result.first->second);
        }
    }
};

void build_task_graph(MAT::Node* node, std::unordered_map<int, uint8_t>& output, tf::Subflow& subflow) {
    // 1. Allocate Context on the HEAP (Safety Fix)
    auto cont_ptr = std::make_shared<Assign_Descendant_Possible_Muts_Cont>(output, node->children.size(), node);
    
    // 2. Create Continuation Task
    tf::Task continuation = subflow.emplace([cont_ptr]() {
        cont_ptr->execute();
    });

    // 3. Spawn Children
    for (size_t i = 0; i < node->children.size(); ++i) {
        auto child = node->children[i];

        if (child->children.empty()) {
            // Leaf optimization (Serial execution)
            cont_ptr->children_out[i].reserve(child->mutations.size());
            for (auto &mut : child->mutations) {
                mut.set_descendant_mut(mut.get_mut_one_hot());
                cont_ptr->children_out[i].emplace(mut.get_position(), mut.get_mut_one_hot());
            }
        } else {
            // Recursion: Create a subflow for the child
            // Capture 'cont_ptr' and 'i' by value to be safe
            tf::Task child_task = subflow.emplace([child, cont_ptr, i](tf::Subflow& sf) {
                build_task_graph(child, cont_ptr->children_out[i], sf);
            });

            // The continuation (execute) must wait for the child to finish
            child_task.precede(continuation);
        }
    }
}

// Entry Point
void assign_descendant_muts(MAT::Tree &in, uint32_t num_threads) {
    std::unordered_map<int, uint8_t> ignore;
    tf::Executor executor(num_threads);
    tf::Taskflow taskflow;
    
    taskflow.emplace([&](tf::Subflow& subflow) {
        build_task_graph(in.root, ignore, subflow);
    });
    
    executor.run(taskflow).wait(); 
}
