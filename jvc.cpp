#include <stdio.h>
#include "docopt.cpp/docopt.h"
#include <fstream>
#include <cassert>
#include <stdint.h>

static const char USAGE[] = R"(Usage:
    jvc diff <a> <b>
)";

struct Range {
    size_t start;
    size_t end;
};

enum class DiffType {
    Ins,
    Mod,
    Del,
};

struct Diff {
    DiffType type;
    std::string change;
    std::tuple<Range, Range> ranges;
};

std::vector<std::string> get_lines(std::string path) {
    std::ifstream file(path);
    std::vector<std::string> lines;
    std::string line;
    do {
        getline(file, line);
        lines.push_back(std::move(line));
    } while (!file.eof());
    return lines;
}

void diff(std::string base, std::string changed) {
    // TODO: optimize via some kind of buffer, maybe with mmap
    std::vector<std::string> lines_base = get_lines(base);
    std::vector<std::string> lines_changed = get_lines(changed);

    auto compare_base = lines_base.begin();
    auto compare_changed = lines_changed.begin();

    while (compare_base != lines_base.end() && compare_changed != lines_changed.end()) {
        // if we have a match, just continue
        if (*compare_base == *compare_changed) {
            compare_base++;
            compare_changed++;
        }
        // if we don't: need to find next matching line
        else {
            auto ahead_base = compare_base + 1;
            auto ahead_changed = compare_changed + 1;

            uint8_t HAVE_MATCH = 0b100;
            uint8_t AHEAD_BASE_ENDED = 0b01;
            uint8_t AHEAD_CHANGED_ENDED = 0b10;
            uint8_t BOTH_ENDED = AHEAD_BASE_ENDED | AHEAD_CHANGED_ENDED;
            assert(HAVE_MATCH > BOTH_ENDED);

            uint8_t end = 0;
            while (end < BOTH_ENDED) {
                if (ahead_base < lines_base.end()) {
                    if (*ahead_base == *compare_changed) {
                        for (; compare_base < ahead_base; compare_base++) {
                            printf("<%s\n", compare_base->c_str());
                        }
                        compare_base = ahead_base;
                        end |= HAVE_MATCH;
                    } else {
                        ahead_base++;
                    }
                } else {
                    end |= AHEAD_BASE_ENDED;
                }

                if (ahead_changed < lines_changed.end()) {
                    if (*ahead_changed == *compare_base) {
                        for (; compare_changed < ahead_changed; compare_changed++) {
                            printf(">%s\n", compare_changed->c_str());
                        }
                        compare_changed = ahead_changed;
                        end |= HAVE_MATCH;
                    } else {
                        ahead_changed++;
                    }
                } else {
                    end |= AHEAD_CHANGED_ENDED;
                }
            }

            // BUG: can only detect and insertions and deletions
            // for same length modifitations we can compare *ahead_base==*ahead_changed
            // for different length modifications, we need to compare ahead with all intermediate steps

            if (!(end & HAVE_MATCH)) {
                break;
            }
        }
    }

    for (; compare_base < lines_base.end(); compare_base++) {
        printf("<%s\n", compare_base->c_str());
    }
    for (; compare_changed < lines_changed.end(); compare_changed++) {
        printf(">%s\n", compare_changed->c_str());
    }
}

int main(int argc, char** argv){
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, {argv+1, argv+argc});
    std::string a = args["<a>"].asString();
    std::string b = args["<b>"].asString();

    if (args["diff"].asBool()) {
        diff(a,b);
    }
}
