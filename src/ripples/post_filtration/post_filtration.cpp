#include "post_filtration.hpp"
#include "recomb_writer.hpp"
#include "src/ripples/util/text_parser.hpp"

ripples::filtration::post_filtration::post_filtration(
    MAT::Tree &tree, const std::string &recomb_file)
    : tree_(tree) {
    // Column indices in ripples-fast 'recombination.tsv' file
    static constexpr int RECOMB_ID_COL{0};
    static constexpr int BREAKPOINT_1_COL{1};
    static constexpr int BREAKPOINT_2_COL{2};
    static constexpr int DONOR_ID_COL{3};
    static constexpr int ACCEPTOR_ID_COL{6};
    static constexpr int ORIG_PARSIMONY_COL{9};
    static constexpr int RECOMB_PARSIMONY_COL{11};

    text_parser parser(recomb_file);
    // Skip over header
    parser.next_line();

    auto process_line = [&](const text_parser &parser) -> recombinant {
        auto recomb_id = parser.get_value(RECOMB_ID_COL);
        auto bp1 = parser.get_value(BREAKPOINT_1_COL);
        auto bp2 = parser.get_value(BREAKPOINT_2_COL);
        auto orig_score = parser.get_value(ORIG_PARSIMONY_COL);
        auto recomb_score = parser.get_value(RECOMB_PARSIMONY_COL);
        auto donor_id = parser.get_value(DONOR_ID_COL);
        auto acceptor_id = parser.get_value(ACCEPTOR_ID_COL);
        MAT::Node *recomb = tree_.get_node(string{recomb_id});
        MAT::Node *donor = tree_.get_node(string{donor_id});
        MAT::Node *acceptor = tree_.get_node(string{acceptor_id});

        recomb_results_row row{recomb, donor,      acceptor,    bp1,
                               bp2,    orig_score, recomb_score};
        return recombinant(row);
    };

    for (; !parser.done(); parser.next_line()) {
        recombinants_.emplace_back(process_line(parser));
    }
}

ripples::server::Status ripples::filtration::post_filtration::write(
    const std::string &outfilepath) {
    recomb_writer writer(tree_, outfilepath);
    return writer.write(recombinants_);
}

