#include <cstring>
#include <cerrno>
#include <iostream>
#include <sstream>
#include "../src/data_parametric_monitor.hh"
#include "../src/automaton.hh"
#include "../src/automaton_parser.hh"


template<typename TAType, typename BoostTAType>
static TAType parseDotTA(const std::string& timedAutomatonFileName) {
    std::istringstream taStream(timedAutomatonFileName);
    if (taStream.fail()) {
        std::cerr << "Error: " << strerror(errno) << " " << timedAutomatonFileName.c_str() << std::endl;
        throw std::runtime_error("Failed to read automaton from string.");
    }

    TAType TA;
    BoostTAType BoostTA;
    parseBoostTA(taStream, BoostTA);
    convBoostTA(BoostTA, TA);
    return TA;
}

class EpsilonTransitionAutomatonFixture {
    const std::string eps_dot = R"DOT(digraph G {
        graph [
            clock_variable_size = 1
            string_variable_size = 1
            number_variable_size = 0
            parameter_size = 0
        ]
        0 [init=1][match=0]
        1 [init=0][match=0]
        2 [init=0][match=0]
        3 [init=0][match=0]
        4 [init=0][match=1]
        0 -> 0 [label=0]
        0 -> 1 [label=0][s_constraints="{x1 == 'a'}"][reset="{0}"]
        1 -> 2 [label=127][s_constraints="{x0 == 'z'}"][guard="{x0 == 2}"]
        2 -> 3 [label=127][s_constraints="{x0 == 'z'}"][guard="{x0 == 3}"]
        3 -> 4 [label=0][s_constraints="{x1 == 'b'}"][guard="{x0 > 4}"]

    })DOT";

    public:
        auto makeDataParametricTA() {
            using TAType = DataParametricTA;
            using BoostTAType = DataParametricBoostTA;
            return parseDotTA<TAType, BoostTAType>(eps_dot);
        }

};

class EpsilonTransitionToAcceptStateAutomatonFixture {
    const std::string eps_dot = R"DOT(digraph G {
        graph [
            clock_variable_size = 1
            string_variable_size = 1
            number_variable_size = 0
            parameter_size = 0
        ]
        0 [init=1][match=0]
        1 [init=0][match=0]
        2 [init=0][match=1]
        0 -> 0 [label=0]
        0 -> 1 [label=0][s_constraints="{x1 == 'b'}"][reset="{0}"]
        1 -> 2 [label=127][guard="{x0 == 3}"]

    })DOT";

    public:
        auto makeDataParametricTA() {
            using TAType = DataParametricTA;
            using BoostTAType = DataParametricBoostTA;
            return parseDotTA<TAType, BoostTAType>(eps_dot);;
        }

};
