#include "sample_collection.hpp"
#include <iostream>

ripples::SampleFilter ripples::getFilter(std::string_view month,
                                         std::string_view start_month,
                                         std::string_view end_month) {
    if (month == "all") {
        if (start_month.empty() || end_month.empty()) {
            throw std::runtime_error("[Error] If running in 'all' mode, need "
                                     "to provide start_month and end_month");
        }
        std::cout << "Getting all samples for months between: " << start_month
                  << " and " << end_month << '\n';
        return [=](std::string_view date) {
            if (date.front() == '?' || date.size() != 10 ||
                date < start_month || date > end_month) {
                return false;
            }
            return true;
        };
    } else {
        std::cout << "Getting samples for month: " << month << '\n';
        return [=](std::string_view date) {
            if (date.front() == '?' || date.size() != 10 ||
                date.substr(0, ripples::MONTH_LEN) != month) {
                return false;
            }
            return true;
        };
    }
}

void ripples::sample_collection::determine_mode(const sample_params &params) {
    auto month_range_mode = [](const sample_params &params) -> bool {
        return !params.start_month.empty() && !params.end_month.empty();
    };
    auto one_months_mode = [](const sample_params &params) -> bool {
        return params.month != "all";
    };

    if (one_months_mode(params) || month_range_mode(params)) {
        std::cout << "Getting sample collection by month\n";
        get_collection_by_month(params);
    } else {
        std::cout << "Getting all samples\n";
        get_collection(params);
    }
}

ripples::sample_collection::sample_collection(const sample_params &params) {
    determine_mode(params);
}

void ripples::sample_collection::get_collection(const sample_params &params) {
    text_parser parser(params.metadata_filepath);
    constexpr bool header{true};
    constexpr int key_col{0};
    const std::string SINGLE_BIN = "all";
    // If file has header, skip over first header line
    // Assuming header is just a single first line in TSV file
    if (header) {
        parser.next_line();
    }
    // SamplesByMonth samples_by_month;
    //  No filtering, just get all samples listed in the file into a single bin
    for (; !parser.done(); parser.next_line()) {
        auto name = parser.get_value(key_col);
        // Only add samples that pass filter criteria
        samples_by_month_[SINGLE_BIN].emplace_back(name);
    }
}

void ripples::sample_collection::get_collection_by_month(
    const sample_params &params) {
    text_parser parser(params.metadata_filepath);
    constexpr bool header{true};
    constexpr int key_col{0};
    constexpr int date_col{2};
    // If file has header, skip over first header line
    // Assuming header is just a single first line in TSV file
    if (header) {
        parser.next_line();
    }
    auto filter = getFilter(params.month, params.start_month, params.end_month);
    for (; !parser.done(); parser.next_line()) {
        auto name = parser.get_value(key_col);
        auto date = std::string{parser.get_value(date_col)};
        // Only add samples that pass filter criteria
        if (filter(date)) {
            // Bin sample by month
            samples_by_month_[date.substr(0, ripples::MONTH_LEN)].emplace_back(
                name);
        }
    }
}

