#include "recomb_simulator.hpp"
#include <algorithm>
#include <filesystem>
#include <random>

ripples::simulator::simulator(MAT::Tree &tree,
                              const ripples::simulation_params &params)
    : tree_(tree), params_(params), recomb_counter_(0) {
    // create user specified output directory to write results
    if (std::filesystem::create_directories(params_.outdir)) {
        std::cout << "Created output directory: " << params_.outdir << '\n';
    } else {
        std::cout << "Output directory " << params_.outdir
                  << " already exists. Writing results there.\n";
    }
}

void ripples::simulator::search(const MissingSamples &samples,
                                const server::ripples_parameters &params,
                                const std::string &month) {
    // Perform ripples search of all placed simulated recombinants
    ripples_search s(tree_, samples, params, month);
    std::cout << "Finished with recombination search.\n";
    server::Status status = s();
    if (!status) {
        std::cout << "ERROR OCCURRED WHEN RUNNING RIPPLES\n";
        exit(1);
        // TODO: Handle error
    }
    std::cout << "Outfile to write simulation results: " << params_.outfile
              << '\n';

    // Read recombination.tsv results file
    constexpr bool header{true};
    constexpr int NODE_ID_COL{0};
    const std::string result_file = "recombination_" + month + ".tsv";
    std::cout << "Ripples results file to load: " << result_file << '\n';
    auto recomb_results_filepath =
        std::filesystem::path(params_.outdir) / result_file;
    std::cout << "Ripples results file path to load: "
              << recomb_results_filepath << '\n';
    text_parser parser(recomb_results_filepath);
    if (header) {
        parser.next_line();
    }
    auto sim_summary_path =
        std::filesystem::path(params_.outdir) / params_.outfile;
    std::ofstream output(sim_summary_path);
    output << "Month\tRecombinantsDetectedOutofN=" << samples.size() << '\n';

    result r(month, output);
    auto &recombs = r.detected_recombs;
    for (; !parser.done(); parser.next_line()) {
        recombs.insert(std::string{parser.get_value(NODE_ID_COL)});
    }
    r.log();
    std::cout << "Simulation summary results file written: " << sim_summary_path
              << '\n';
    output.flush();
    output.close();
}

void ripples::simulator::result::log() const {
    os << month + '\t' + std::to_string(detected_recombs.size()) << '\n';
}

ripples::simulator::OptionalBranchLengths
ripples::simulator::get_branch_lengths(const ripples::MissingSamples &recombs) {
    std::vector<float> bl_vec;
    bl_vec.reserve(recombs.size());
    size_t idx{};
    for (const auto &r : recombs) {
        auto *node = tree_.get_node(r.name);
        if (!node) {
            std::cout << "Node lookup in tree: " << idx << " not successful!\n";
            return std::nullopt;
        }
        bl_vec.push_back(node->mutations.size());
        ++idx;
    }
    return bl_vec;
}

std::vector<std::string> ripples::simulator::create_months() {
    std::array<std::string, 4> YEARS = {"2020", "2021", "2022", "2023"};
    std::array<std::string, 12> MONTHS = {"01", "02", "03", "04", "05", "06",
                                          "07", "08", "09", "10", "11", "12"};
    std::vector<std::string> months;
    for (size_t i = 0; i < YEARS.size(); i++) {
        for (size_t j = 0; j < MONTHS.size(); j++) {
            months.emplace_back(YEARS[i] + "-" + MONTHS[j]);
        }
    }
    return months;
};

ripples::recomb_collection
ripples::simulator::make_recombinants(const Samples &samples,
                                      uint32_t num_recombinants) {
    auto log_path = std::filesystem::path(params_.outdir) /
                    std::string{params_.mutations_logfile + ".tsv"};
    std::ofstream log(log_path);
    // Write mutation file header
    log << "DonorID\tDonorNumMutations\tDonorMutations\tAcceptorID\tAcceptorNum"
           "Mutations\tAcceptorMutations\tRecombID\tRecombNumMutations\tRecomb"
           "Mutations\tBreakpoint\n";

    std::vector<recomb> recombs;
    recombs.reserve(num_recombinants);
    for (uint32_t i = 0; i < num_recombinants; ++i) {
        size_t breakpoint = select_random_breakpoint();
        DonorAcceptorPair parents = select_random_pair(samples);
        recomb &r =
            recombs.emplace_back(recomb_counter_++, breakpoint, parents, tree_);
        log << r;
    }
    log.flush();
    log.close();
    std::cout << "Mutation logfile written: " << log_path << '\n';
    return recomb_collection(tree_, recombs, params_.outdir);
}

ripples::recomb ripples::simulator::make_recombinant(const Samples &samples) {
    size_t breakpoint = select_random_breakpoint();
    DonorAcceptorPair parents = select_random_pair(samples);
    return recomb(recomb_counter_++, breakpoint, parents, tree_);
}

// TODO: take seed as a parameter
size_t ripples::simulator::select_random_breakpoint() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, GENOME_SIZE - 1);
    return dist(gen);
}

ripples::DonorAcceptorPair
ripples::simulator::select_random_pair(const Samples &samples) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> dist(0, samples.size() - 1);
    int donor = dist(gen);
    int acceptor = dist(gen);
    while (donor == acceptor) {
        acceptor = dist(gen);
    }
    return std::make_pair(tree_.get_node(samples[donor]),
                          tree_.get_node(samples[acceptor]));
}

ripples::DonorAcceptorPair
ripples::simulator::select_random_pair(const NodePtrVec &nodes) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> dist(0, nodes.size() - 1);
    int donor = dist(gen);
    int acceptor = dist(gen);
    while (donor == acceptor) {
        acceptor = dist(gen);
    }
    return std::make_pair(nodes[donor], nodes[acceptor]);
}

void ripples::simulator::write_tree() {
    if (!params_.mat_outfile.empty()) {
        auto filepath =
            std::filesystem::path(params_.outdir) / params_.mat_outfile;
        std::cout << "Saving MAT to: " << filepath << '\n';
        MAT::save_mutation_annotated_tree(tree_, filepath);
    }
}

