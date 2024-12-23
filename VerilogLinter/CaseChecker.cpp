#include "CaseChecker.h"

CaseChecker::CaseChecker(QObject *parent)
    : QObject{parent}
{}

unordered_set<string> CaseChecker::generateAllBinaryCases(int bit_width) {
    unordered_set<string> cases;
    int total_cases = pow(2, bit_width); // 2^bit_width

    for (int i = 0; i < total_cases; ++i) {
        // Generate binary string for the current case
        string binary = to_string(bit_width) + "'b" + bitset<32>(i).to_string().substr(32 - bit_width);
        cases.insert(binary);
    }
    return cases;
}

unordered_map<string, string> CaseChecker::parseSymbolicConstants(const string &code) {
    unordered_map<string, string> symbol_to_binary;
    regex param_regex(R"((parameter|localparam)\s+(\w+)\s*=\s*(\d+'b[01]+);)");
    smatch match;
    string::const_iterator search_start(code.cbegin());

    while (regex_search(search_start, code.cend(), match, param_regex)) {
        string symbol = match[2];
        string binary_value = match[3];
        symbol_to_binary[symbol] = binary_value;
        search_start = match.suffix().first;
    }

    return symbol_to_binary;
}

string CaseChecker::replaceSymbolicCases(const string &case_body, const unordered_map<string, string> &symbol_to_binary) {
    string replaced_body = case_body;
    for (const auto& [symbol, binary] : symbol_to_binary) {
        regex symbol_regex("\\b" + symbol + "\\b");
        replaced_body = regex_replace(replaced_body, symbol_regex, binary);
    }
    return replaced_body;
}

CaseBlockAnalysis CaseChecker::analyzeCaseBlock(const string &case_body) {
    CaseBlockAnalysis block_analysis;
    block_analysis.is_parallel_case = true;
    block_analysis.is_full_case = true;

    regex case_item_regex(R"((\d+'b[01]+|default)\s*:)");

    smatch case_item_match;
    unordered_map<string, int> case_counts;
    bool has_default = false;

    string::const_iterator case_search_start(case_body.cbegin());
    while (regex_search(case_search_start, case_body.cend(), case_item_match, case_item_regex)) {
        string detected_case = case_item_match[1];
        case_search_start = case_item_match.suffix().first;

        if (detected_case == "default") {
            has_default = true;
        } else {
            case_counts[detected_case]++;
            if (case_counts[detected_case] > 1) {
                block_analysis.is_parallel_case = false;
                block_analysis.non_parallel_cases[detected_case] = "Duplicate";
            }
            block_analysis.covered_cases.insert(detected_case);
        }
    }

    int bit_width = 0;
    regex bit_width_regex(R"(\d+'b[01]+)");
    smatch bit_width_match;
    string::const_iterator search_start(case_body.cbegin());
    while (regex_search(search_start, case_body.cend(), bit_width_match, bit_width_regex)) {
        int current_bit_width = stoi(bit_width_match[0].str().substr(0, bit_width_match[0].str().find("'")));
        bit_width = max(bit_width, current_bit_width);
        search_start = bit_width_match.suffix().first;
    }

    if (!block_analysis.covered_cases.empty()) {
        unordered_set<string> all_cases = generateAllBinaryCases(bit_width);
        for (const auto& expected : all_cases) {
            if (block_analysis.covered_cases.find(expected) == block_analysis.covered_cases.end() && !has_default) {
                block_analysis.is_full_case = false;
                block_analysis.missing_cases.insert(expected);
            }
        }
    } else {
        block_analysis.is_full_case = false;
    }

    return block_analysis;
}
