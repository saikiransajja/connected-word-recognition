#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using Feature = std::vector<double>;
using Pattern = std::vector<Feature>;
struct Word { std::string name; Pattern frames; };
struct State { int i = -1, k = -1, j = -1; };
struct Cell {
    double cost = std::numeric_limits<double>::infinity();
    State previous;
    bool starts_word = false;
};
struct Result { double cost; std::vector<std::string> words; };

double distance(const Feature& a, const Feature& b) {
    double result = 0;
    for (std::size_t p = 0; p < a.size(); ++p)
        result = std::hypot(result, a[p] - b[p]);
    return result;
}

// One-stage DP: vertical, diagonal and horizontal moves within a word;
// a completed template can start any next word at the next input frame.
Result recognize(const Pattern& input, const std::vector<Word>& vocabulary) {
    if (input.empty() || vocabulary.empty() || input.front().empty())
        throw std::invalid_argument("Input and vocabulary must be nonempty.");
    const auto dimensions = input.front().size();
    auto validate = [&](const Pattern& pattern) {
        if (pattern.empty()) throw std::invalid_argument("Empty template.");
        for (const auto& frame : pattern) {
            if (frame.size() != dimensions)
                throw std::invalid_argument("Feature dimensions must match.");
            for (double value : frame)
                if (!std::isfinite(value))
                    throw std::invalid_argument("Features must be finite.");
        }
    };
    validate(input);
    for (const auto& word : vocabulary) validate(word.frames);
    const int n = static_cast<int>(input.size());
    const int count = static_cast<int>(vocabulary.size());
    std::vector<std::vector<std::vector<Cell>>> dp(n);
    for (auto& row : dp) {
        row.resize(count);
        for (int k = 0; k < count; ++k)
            row[k].resize(vocabulary[k].frames.size());
    }
    for (int i = 0; i < n; ++i) {
        // Cache the best completed word to avoid a quadratic vocabulary scan.
        State boundary;
        double boundary_cost = std::numeric_limits<double>::infinity();
        if (i > 0) for (int k = 0; k < count; ++k) {
            int end = static_cast<int>(dp[i - 1][k].size()) - 1;
            if (dp[i - 1][k][end].cost < boundary_cost) {
                boundary_cost = dp[i - 1][k][end].cost;
                boundary = {i - 1, k, end};
            }
        }
        for (int k = 0; k < count; ++k) {
            for (int j = 0; j < static_cast<int>(dp[i][k].size()); ++j) {
                Cell& cell = dp[i][k][j];
                double best = std::numeric_limits<double>::infinity();
                auto consider = [&](double cost, State previous, bool start) {
                    if (cost < best) {
                        best = cost;
                        cell.previous = previous;
                        cell.starts_word = start;
                    }
                };
                if (i == 0 && j == 0) consider(0, {}, true);
                if (i > 0) {
                    // Stable ties prefer staying in the current word.
                    consider(dp[i - 1][k][j].cost, {i - 1, k, j}, false);
                    if (j > 0)
                        consider(dp[i - 1][k][j - 1].cost,
                                 {i - 1, k, j - 1}, false);
                    if (j == 0) consider(boundary_cost, boundary, true);
                }
                if (j > 0)
                    consider(dp[i][k][j - 1].cost, {i, k, j - 1}, false);
                if (std::isfinite(best))
                    cell.cost = best + distance(input[i], vocabulary[k].frames[j]);
            }
        }
    }
    State end;
    double best = std::numeric_limits<double>::infinity();
    for (int k = 0; k < count; ++k) {
        int j = static_cast<int>(dp[n - 1][k].size()) - 1;
        if (dp[n - 1][k][j].cost < best) {
            best = dp[n - 1][k][j].cost;
            end = {n - 1, k, j};
        }
    }
    if (!std::isfinite(best)) throw std::runtime_error("No finite alignment.");
    Result result{best, {}};
    for (State s = end; s.i >= 0; ) {
        const Cell& cell = dp[s.i][s.k][s.j];
        if (cell.starts_word) result.words.push_back(vocabulary[s.k].name);
        s = cell.previous;
    }
    std::reverse(result.words.begin(), result.words.end());
    return result;
}

void check(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error("Test failed: " + message);
}

int main() {
    try {
        // Synthetic features, not recordings: each frame has two coordinates.
        const std::vector<Word> vocabulary{
            {"ONE", {{0, 0}, {1, 0}, {2, 0}}},
            {"TWO", {{8, 0}, {9, 0}}}
        };
        const Pattern input{{0, 0}, {1, 0}, {1, 0}, {2, 0}, {8, 0}, {9, 0}};
        const Result result = recognize(input, vocabulary);
        check(result.words == std::vector<std::string>{"ONE", "TWO"},
              "time-stretched connected words");
        check(std::abs(result.cost) < 1e-9, "exact match cost");
        const auto repeat = recognize({{0,0},{1,0},{2,0},{0,0},{1,0},{2,0}}, vocabulary);
        check(repeat.words == std::vector<std::string>{"ONE", "ONE"},
              "repeated word boundaries");
        const auto horizontal = recognize({{0}, {2}}, {{"A", {{0}, {1}, {2}}}});
        check(std::abs(horizontal.cost - 1) < 1e-9, "horizontal transition");
        const auto fractional = recognize({{0.5}}, {{"A", {{0}}}});
        check(std::abs(fractional.cost - 0.5) < 1e-9, "fractional distance");
        bool rejected = false;
        try { recognize({{0, 1}}, {{"A", {{0}}}}); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected, "invalid dimensions");
        std::cout << "Recognized words:";
        for (const auto& word : result.words) std::cout << ' ' << word;
        std::cout << "\nTotal distance: " << result.cost
                  << "\nAll demonstration checks passed.\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
