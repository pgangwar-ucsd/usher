#pragma once

#include "input.hpp"
#include "params.hpp"
#include "src/ripples/util/text_parser.hpp"
#include <functional>
#include <string_view>

namespace ripples {

using SampleFilter = std::function<bool(std::string_view)>;
using Samples = std::vector<std::string>;

inline static constexpr int MONTH_LEN{7};

SampleFilter getFilter(std::string_view month, std::string_view start_month,
                       std::string_view end_month);

class sample_collection {
  public:
    using Month = std::string;
    using SamplesByMonth = std::map<Month, Samples>;

    sample_collection(const ripples::sample_params &params);

    auto begin() const { return samples_by_month_.begin(); }
    auto end() const { return samples_by_month_.end(); }
    size_t num_months() const { return samples_by_month_.size(); };

  private:
    void determine_mode(const sample_params &params);

    void get_collection(const sample_params &params);
    void get_collection_by_month(const sample_params &params);

    SamplesByMonth samples_by_month_;
};

}; // namespace ripples
