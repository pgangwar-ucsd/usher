#include "arg_parse.hpp"
#include <filesystem>
#include <iostream>
#include <tbb/global_control.h>
#include <tbb/info.h>

namespace po = boost::program_options;

ripples::variables_map ripples::parse_argv(int argc, char *argv[]) {
    uint32_t num_cores = tbb::info::default_concurrency();
    std::string num_threads_message = "Number of threads to use when possible "
                                      "[DEFAULT uses all available cores, " +
                                      std::to_string(num_cores) +
                                      " detected on this machine]";

    po::options_description desc("ripples sim options");
    desc.add_options()(
        "input-mat,i", po::value<std::string>()->required(),
        "Input mutation-annotated tree file to optimize [REQUIRED].")(
        "input-metadata,M", po::value<std::string>()->required(),
        "Input metadata file [REQUIRED]")
								(
        "num-descendants-search,n", po::value<uint32_t>()->default_value(5),
        "Minimum number of leaves that node should have to be considered "
        "for recombination.")

								(
        "num-descendants-recomb", po::value<uint32_t>()->default_value(0),
        "Minimum number of leaves that node should have to be considered "
        "for recombination.")
								
								(
        "branch-length,l", po::value<uint32_t>()->default_value(2),
        "Minimum length of the branch to consider to recombination "
        "events.")

				("ancestor-radius,r",
                   po::value<uint32_t>()->default_value(std::numeric_limits<uint32_t>::max()),
                   "Number of ancestors to search when considering a node as "
                   "a recombinant."
			  )

				(
        "outdir,d", po::value<std::string>()->default_value("."),
        "The output directory to write results to."
				)
				(
        "outfile,o", po::value<std::string>()->default_value("simulation_results.tsv"),
        "Output log file with simulation results."
				)
				(
        "output-mat,o", po::value<std::string>()->default_value(""),
        "Output the MAT as a .pb file containing the simulated recombinants that were placed onto the tree."
				)
				(
        "log-mutations", po::value<std::string>()->default_value("simulation_mutations"),
        "Output log file with simulation recombinant mutation results."
				)
									 
				(
        "month,m", po::value<std::string>()->default_value("all"),
        "Samples during a given month to sample recombinant donor/acceptor "
        "pairs from. [DEFAULT all]"
				)
			  (
        "start-month,s", po::value<std::string>()->default_value(""),
        "Starting month to consider for sampling recombinant donor/acceptor "
        "pairs from"
				)

				(
        "end-month,e", po::value<std::string>()->default_value(""),
        "Ending month to consider for sampling recombinant donor/acceptor pairs from."
				)

				(
        "num-recombinants,N", po::value<uint32_t>()->default_value(1000),
        "Number of recombinants to simulate for each month."
				)

				(
        "num-replicates,R", po::value<uint32_t>()->default_value(5),
        "Number of recombinant replicates to place for each simulated recombinant (num-recombinants)."
				)
			 (
        "threads,T", po::value<uint32_t>()->default_value(num_cores),
        num_threads_message.c_str())("help,h", "Print help messages");

    po::options_description all_options;
    all_options.add(desc);
    po::positional_options_description p;
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv)
                      .options(all_options)
                      .positional(p)
                      .run(),
                  vm);
        po::notify(vm);
    } catch (std::exception &e) {
        std::cerr << desc << std::endl;
        // Return with error code 1 unless
        // the user specifies help
        if (vm.count("help"))
            exit(0);
        else
            exit(1);
    }
    return vm;
}

