#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

// Disk Scheduling: C-SCAN and C-LOOK comparison
// Refactored to production quality — pure functions, no globals, testable.

struct SchedulingResult {
    std::vector<int> sequence;
    int totalSeek = 0;
    double averageSeek = 0.0;
};

int calculateSeek(const std::vector<int>& seq, int head) {
    int total = 0;
    int cur = head;
    for (int track : seq) {
        total += std::abs(track - cur);
        cur = track;
    }
    return total;
}

SchedulingResult cscan(const std::vector<int>& requests, int head, int diskSize, bool goingRight = true) {
    if (diskSize <= 0) throw std::invalid_argument("diskSize must be positive");
    if (head < 0 || head >= diskSize) throw std::invalid_argument("head out of range");
    if (requests.empty()) return {};

    std::vector<int> left, right;
    for (int r : requests) {
        if (r < 0 || r >= diskSize) throw std::invalid_argument("request out of range: " + std::to_string(r));
        if (r < head) left.push_back(r);
        else right.push_back(r);
    }
    std::sort(left.begin(), left.end());
    std::sort(right.begin(), right.end());

    SchedulingResult res;
    if (goingRight) {
        // Service right side, jump to 0, service left side
        for (int r : right) res.sequence.push_back(r);
        if (!left.empty() || !right.empty()) {
            // Go to end then wrap to 0 (counts as seek)
            if (res.sequence.empty() || res.sequence.back() != diskSize - 1) {
                res.sequence.push_back(diskSize - 1);
            }
            res.sequence.push_back(0);
        }
        for (int r : left) res.sequence.push_back(r);
    } else {
        for (auto it = left.rbegin(); it != left.rend(); ++it) res.sequence.push_back(*it);
        if (!left.empty() || !right.empty()) {
            if (res.sequence.empty() || res.sequence.back() != 0) res.sequence.push_back(0);
            res.sequence.push_back(diskSize - 1);
        }
        for (auto it = right.rbegin(); it != right.rend(); ++it) res.sequence.push_back(*it);
    }
    res.totalSeek = calculateSeek(res.sequence, head);
    res.averageSeek = requests.empty() ? 0.0 : static_cast<double>(res.totalSeek) / requests.size();
    return res;
}

SchedulingResult clook(const std::vector<int>& requests, int head, bool goingRight = true) {
    if (head < 0) throw std::invalid_argument("head must be non-negative");
    if (requests.empty()) return {};

    std::vector<int> left, right;
    for (int r : requests) {
        if (r < head) left.push_back(r);
        else right.push_back(r);
    }
    std::sort(left.begin(), left.end());
    std::sort(right.begin(), right.end());

    SchedulingResult res;
    if (goingRight) {
        for (int r : right) res.sequence.push_back(r);
        for (int r : left) res.sequence.push_back(r);
    } else {
        for (auto it = left.rbegin(); it != left.rend(); ++it) res.sequence.push_back(*it);
        for (auto it = right.rbegin(); it != right.rend(); ++it) res.sequence.push_back(*it);
    }
    res.totalSeek = calculateSeek(res.sequence, head);
    res.averageSeek = requests.empty() ? 0.0 : static_cast<double>(res.totalSeek) / requests.size();
    return res;
}

void printResult(const std::string& name, const SchedulingResult& r, int head) {
    std::cout << "\n--- " << name << " ---\n";
    std::cout << "Head start: " << head << "\n";
    std::cout << "Seek sequence: ";
    std::cout << head;
    for (int s : r.sequence) std::cout << " -> " << s;
    std::cout << "\n";
    std::cout << "Total seek distance: " << r.totalSeek << "\n";
    std::cout << "Average seek: " << r.averageSeek << "\n";
}

#ifdef DISK_SCHEDULING_TEST
void runTests() {
    // Deterministic self-test
    std::vector<int> req{82, 170, 43, 140, 24, 16, 190};
    int head = 50, diskSize = 200;
    auto a = cscan(req, head, diskSize);
    auto b = clook(req, head);
    std::cout << "[TEST] C-SCAN total=" << a.totalSeek << " C-LOOK total=" << b.totalSeek << "\n";
    // C-LOOK should be <= C-SCAN due to no edge travel
    if (b.totalSeek > a.totalSeek) std::cout << "[WARN] C-LOOK unexpectedly larger\n";
    else std::cout << "[PASS] Scheduling invariants hold\n";
}
#endif

int main(int argc, char* argv[]) {
#ifdef DISK_SCHEDULING_TEST
    if (argc > 1 && argv[1] && std::string(argv[1]) == "--test") {
        runTests();
        return 0;
    }
#else
    (void)argc;
    (void)argv;
#endif

    std::cout << "===== C-SCAN vs C-LOOK Disk Scheduling =====\n";
    int diskSize = 200;
    int head = 50;
    int n = 0;

    std::cout << "Enter disk size (default 200): ";
    std::string line;
    std::getline(std::cin, line);
    if (!line.empty()) {
        try { diskSize = std::stoi(line); } catch (...) {}
    }

    std::cout << "Enter initial head position (0-" << diskSize - 1 << ") [50]: ";
    std::getline(std::cin, line);
    if (!line.empty()) {
        try { head = std::stoi(line); } catch (...) {}
    }

    std::cout << "Enter number of requests: ";
    std::getline(std::cin, line);
    try { n = std::stoi(line); } catch (...) { n = 0; }

    std::vector<int> requests;
    requests.reserve(n);
    std::cout << "Enter " << n << " request positions (space-separated or one per line):\n";
    for (int i = 0; i < n; ++i) {
        int v;
        if (!(std::cin >> v)) {
            std::cerr << "Invalid input. Exiting.\n";
            return 1;
        }
        requests.push_back(v);
    }

    if (requests.empty()) {
        // Demo defaults if no input
        requests = {82, 170, 43, 140, 24, 16, 190};
        std::cout << "No requests given, using demo: 82 170 43 140 24 16 190\n";
    }

    try {
        auto r1 = cscan(requests, head, diskSize, true);
        auto r2 = clook(requests, head, true);
        printResult("C-SCAN (right)", r1, head);
        printResult("C-LOOK (right)", r2, head);

        std::cout << "\nComparison: ";
        if (r1.totalSeek < r2.totalSeek) std::cout << "C-SCAN is more efficient for this workload.\n";
        else if (r2.totalSeek < r1.totalSeek) std::cout << "C-LOOK is more efficient for this workload.\n";
        else std::cout << "Both have equal seek distance.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
