#include "arg_parse.hpp"
#include "params.hpp"
#include "recomb_simulator.hpp"
#include "sample_collection.hpp"
#include "src/ripples/util/logger.hpp"
#include <iostream>

void ripples_sim_main(int argc, char *argv[]) {
    using namespace ripples;
    auto vm = parse_argv(argc, argv);

    const std::string mat_filepath = vm["input-mat"].as<std::string>();
    const std::string metadata_filepath =
        vm["input-metadata"].as<std::string>();

    // Simulation parameters
    auto sim_params = get_simulation_parameters(vm);
    // Parameters for selecting samples to generate simulated recombinants from
    auto sample_params = get_sample_parameters(vm);
    // Ripples search parameters
    auto ripples_params = get_ripples_parameters(vm);

    // Load input MAT
    Timer timer;
    timer.Start();
    std::cout << "Loading input MAT file: " << mat_filepath << '\n';
    MAT::Tree tree = MAT::load_mutation_annotated_tree(mat_filepath);
    tree.uncondense_leaves();
    std::cout << "Finished loading MAT tree in " << timer.Stop() << " msec\n";

    // Load input metadata and bin samples by month
    sample_collection collection(sample_params);
    simulator sim(tree, sim_params);

    Timer sim_timer;
    sim_timer.Start();
    std::cout << "Samples loaded, running recombination simulation\n";

    std::cout << "Number of months in collection: " << collection.num_months()
              << '\n';
    for (const auto &[month, samples] : collection) {
        std::cout << "Month: " << month << '\n';
        std::cout << "Num samples: " << samples.size() << '\n';
        const MissingSamples recombs =
            sim.make_recombinants(samples, sim_params.num_recombinants)
                .place_all(sim_params.num_replicates);
        sim.write_tree();
        sim.search(recombs, ripples_params, month);
        std::cout << "Reached end of simulation for month: " << month << '\n';
    }
    std::cout << "Completed simulation in " << sim_timer.Stop() << " msec\n";
}

int main(int argc, char *argv[]) {
    try {
        ripples_sim_main(argc, argv);
    } catch (const std::exception &e) {
        std::cerr << "[Error]: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
