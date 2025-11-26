#define BOOST_GRAPH_USE_SPIRIT_PARSER // for header only

#include "automaton_parser.hh"
#include "symon_parser.hh"

#include <boost/program_options.hpp>

#include "boolean_monitor.hh"
#include "data_parametric_monitor.hh"
#include "parametric_monitor.hh"
#include "ppl_rational.hh"
#include "printer.hh"

using namespace boost::program_options;
using namespace boost;
using namespace NonSymbolic;
using namespace std;

using boost::operator>>;
using NonSymbolic::operator>>;
using std::operator>>;
using ::operator>>;

using boost::operator<<;
using NonSymbolic::operator<<;
using std::operator<<;
using ::operator<<;

/*!
 * @brief Execute the monitoring procedure
 *
 * @param [in] timedAutomatonFileName filename of the timed automaton
 * @param [in] signatureFileName filename of the sugnature
 * @param [in] timedWordFileName filename of the timed word. When it is "stdin", the monitor reads from standard input.
 */
template <typename TAType, typename BoostTAType, typename Number, typename Timestamp, typename Monitor,
          typename Printer, typename StringConstraint, typename NumberConstraint, typename TimingConstraintType,
          typename UpdateType>
int execute(const std::string &timedAutomatonFileName, const std::string &signatureFileName,
            const std::string &timedWordFileName, bool useNewSyntax = false) {
  TAType TA;
  Signature signature;

  // Open the automaton file
  std::ifstream taStream(timedAutomatonFileName);
  if (taStream.fail()) {
    std::cerr << "Error: " << strerror(errno) << " " << timedAutomatonFileName.c_str() << std::endl;
    return 1;
  }

  if (useNewSyntax) {
    // Use the new syntax parser
    SymonParser<StringConstraint, NumberConstraint, TimingConstraintType, UpdateType> parser;
    try {
      parser.parse(taStream);
    } catch (const std::runtime_error &e) {
      std::cerr << "Error during parsing " << timedAutomatonFileName.c_str() << "\n" << e.what() << std::endl;
      return 1;
    }

    TA = parser.getAutomaton();
    parser.setGlobalData(TA);

    signature = parser.makeSignature();
  } else {
    // Use the old syntax parser
    BoostTAType BoostTA;
    parseBoostTA(taStream, BoostTA);
    convBoostTA(BoostTA, TA);

    // read signature file
    std::fstream signatureStream(signatureFileName);
    if (signatureStream.fail()) {
      std::cerr << "Error: " << strerror(errno) << " " << signatureFileName.c_str() << std::endl;
      return 1;
    }
    signature = Signature(signatureStream);
  }

  // construct BooleanPrinter
  const auto printer = std::make_shared<Printer>();

  // construct Monitor
  const auto monitor = std::make_shared<Monitor>(TA);
  monitor->addObserver(printer);

  // construct TimedWordParser
  std::unique_ptr<TimedWordParser<Number, Timestamp>> timedWordParser;
  std::fstream timedWordFileStream;
  if (timedWordFileName == "stdin") {
    timedWordParser = std::make_unique<TimedWordParser<Number, Timestamp>>(std::cin, signature);
  } else {
    timedWordFileStream.open(timedWordFileName);
    if (timedWordFileStream.fail()) {
      std::cerr << "Error: " << strerror(errno) << " " << timedWordFileName.c_str() << std::endl;
      return 1;
    }
    timedWordParser = std::make_unique<TimedWordParser<Number, Timestamp>>(timedWordFileStream, signature);
  }

  // construct TimedWordSubject
  TimedWordSubject<Number, Timestamp> timedWordSubject(std::move(timedWordParser));
  timedWordSubject.addObserver(monitor);

  // monitor all
  timedWordSubject.parseAndSubjectAll();
  return 0;
}

int main(int argc, char *argv[]) {
  using Number = double;
  using Timestamp = double;
#ifdef NDEBUG
  const auto programName = "SyMon (relase build)";
#else
#ifdef DEBUG
  const auto programName = "SyMon (debug build)";
#else
  const auto programName = "SyMon (unknown)";
#endif
#endif
  std::cin.tie(0);
  std::ios::sync_with_stdio(false);
  const auto errorHeader = "SyMon: ";

  const auto die = [&errorHeader](const char *message, int status) {
    std::cerr << errorHeader << message << std::endl;
    exit(status);
  };

  // visible options
  options_description visible("description of options");
  std::string signatureFileName;
  std::string timedWordFileName;
  std::string timedAutomatonFileName;
  visible.add_options()("help,h", "help")("boolean,b", "non-parametric and  boolean mode")("dataparametric,d",
                                                                                           "data-parametric mode")(
      "parametric,p", "parametric mode")("new,n", "use the experimental syntax of SyMon")("version,V", "version")(
      "input,i", value<std::string>(&timedWordFileName)->default_value("stdin"), "input file of Timed Words")(
      "automaton,f", value<std::string>(&timedAutomatonFileName)->default_value(""), "input file of Timed Automaton")(
      "signature,s", value<std::string>(&signatureFileName)->default_value(""), "input file of signature");

  command_line_parser parser(argc, argv);
  parser.options(visible);
  variables_map vm;
  const auto parseResult = parser.run();
  store(parseResult, vm);
  notify(vm);

  if (timedAutomatonFileName.empty() || (signatureFileName.empty() && !vm.count("new")) || vm.count("help")) {
    std::cout << programName << " [OPTIONS] -f <automaton_file> -s <signature_file> (-i <timedword_file>)\n"
              << visible << std::endl;
    return 0;
  }
  if (vm.count("version")) {
    std::cout << "SyMon 0.0.2\n" << visible << std::endl;
    return 0;
  }

  if (vm.count("boolean") + vm.count("dataparametric") + vm.count("parametric") > 1) {
    die("only one mode can be specified!!", 1);
  }

  if (vm.count("new")) {
    // Use the new syntax parser
    if (vm.count("parametric")) {
      // parametric with new syntax
      return execute<ParametricTA, BoostPTA, PPLRational, PPLRational, ParametricMonitor, ParametricPrinter,
                     Symbolic::StringConstraint, Symbolic::NumberConstraint, ParametricTimingConstraint,
                     Symbolic::Update>(timedAutomatonFileName, signatureFileName, timedWordFileName, true);
    } else if (vm.count("dataparametric")) {
      // data parametric with new syntax
      return execute<DataParametricTA<Timestamp>, DataParametricBoostTA<Number>, PPLRational, Timestamp, DataParametricMonitor<Timestamp>,
                     DataParametricPrinter<Timestamp>, Symbolic::StringConstraint, Symbolic::NumberConstraint,
                     std::vector<TimingConstraint<Timestamp>>, Symbolic::Update>(timedAutomatonFileName, signatureFileName,
                                                                      timedWordFileName, true);
    } else {
      // boolean with new syntax
      return execute<NonParametricTA<Number, Timestamp>, NonParametricBoostTA<Number, Timestamp>, Number, Timestamp, BooleanMonitor<Number, Timestamp>,
                     BooleanPrinter<Number, Timestamp>, NonSymbolic::StringConstraint, NonSymbolic::NumberConstraint<Number>,
                     std::vector<TimingConstraint<Timestamp>>, NonSymbolic::Update<Number>>(timedAutomatonFileName, signatureFileName,
                                                                         timedWordFileName, true);
    }
  } else if (vm.count("parametric")) {
    // parametric
    return execute<ParametricTA, BoostPTA, PPLRational, PPLRational, ParametricMonitor, ParametricPrinter,
                   Symbolic::StringConstraint, Symbolic::NumberConstraint, ParametricTimingConstraint,
                   Symbolic::Update>(timedAutomatonFileName, signatureFileName, timedWordFileName, false);
  } else if (vm.count("dataparametric")) {
    // data parametric
    return execute<DataParametricTA<Timestamp>, DataParametricBoostTA<Number>, PPLRational, Timestamp, DataParametricMonitor<Timestamp>,
                   DataParametricPrinter<Timestamp>, Symbolic::StringConstraint, Symbolic::NumberConstraint,
                   std::vector<TimingConstraint<Timestamp>>, Symbolic::Update>(timedAutomatonFileName, signatureFileName,
                                                                    timedWordFileName, false);
  } else {
    // boolean
    return execute<NonParametricTA<Number, Timestamp>, NonParametricBoostTA<Number, Timestamp>, Number, Timestamp, BooleanMonitor<Number, Timestamp>,
                   BooleanPrinter<Number, Timestamp>, NonSymbolic::StringConstraint, NonSymbolic::NumberConstraint<Number>,
                   std::vector<TimingConstraint<Timestamp>>, NonSymbolic::Update<Number>>(timedAutomatonFileName, signatureFileName,
                                                                       timedWordFileName, false);
  }
  return 0;
}
