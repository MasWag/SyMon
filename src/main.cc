#define BOOST_GRAPH_USE_SPIRIT_PARSER // for header only

#include "automaton_parser.hh"

#include <boost/program_options.hpp>

#include "boolean_monitor.hh"
#include "data_parametric_monitor.hh"
#include "parametric_monitor.hh"
#include "printer.hh"
#include "ppl_rational.hh"

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
template<typename TAType, typename BoostTAType, typename Number, typename Timestamp, typename Monitor, typename Printer>
int execute(const std::string &timedAutomatonFileName,
            const std::string &signatureFileName,
            const std::string &timedWordFileName) {
  TAType TA;

  // parse TA
  std::ifstream taStream(timedAutomatonFileName);
  if (taStream.fail()) {
    std::cerr << "Error: " << strerror(errno) << " " << timedAutomatonFileName.c_str() << std::endl;
    return 1;
  }
  BoostTAType BoostTA;
  parseBoostTA(taStream, BoostTA);
  convBoostTA(BoostTA, TA);

  // read signature file
  std::fstream signatureStream(signatureFileName);
  if (signatureStream.fail()) {
    std::cerr << "Error: " << strerror(errno) << " " << signatureFileName.c_str() << std::endl;
    return 1;
  }
  Signature signature(signatureStream);

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
  const auto programName = "dataMonitor";
  std::cin.tie(0);
  std::ios::sync_with_stdio(false);
  const auto errorHeader = "dataMonitor: ";

  const auto die = [&errorHeader](const char *message, int status) {
    std::cerr << errorHeader << message << std::endl;
    exit(status);
  };

  // visible options
  options_description visible("description of options");
  std::string signatureFileName;
  std::string timedWordFileName;
  std::string timedAutomatonFileName;
  visible.add_options()
          ("help,h", "help")
          ("boolean,b", "non-parametric and  boolean mode")
          ("dataparametric,d", "data-parametric mode")
          ("parametric,p", "parametric mode")
          ("version,V", "version")
          ("input,i", value<std::string>(&timedWordFileName)->default_value("stdin"), "input file of Timed Words")
          ("automaton,f", value<std::string>(&timedAutomatonFileName)->default_value(""),
           "input file of Timed Automaton")
          ("signature,s", value<std::string>(&signatureFileName)->default_value(""), "input file of signature");

  command_line_parser parser(argc, argv);
  parser.options(visible);
  variables_map vm;
  const auto parseResult = parser.run();
  store(parseResult, vm);
  notify(vm);

  if (timedAutomatonFileName.empty() || signatureFileName.empty() || vm.count("help")) {
    std::cout << programName << " [OPTIONS] -f <automaton_file> -s <signature_file> (-i <timedword_file>)\n"
              << visible << std::endl;
    return 0;
  }
  if (vm.count("version")) {
    std::cout << "dataMonitor 0.0.2\n"
              << visible << std::endl;
    return 0;
  }

  if (vm.count("boolean") && vm.count("dataparametric") && vm.count("parametric")) {
    die("only one mode can be specified!!", 1);
  }

  if (vm.count("parametric")) {
    // parametric
    return execute<ParametricTA, BoostPTA, PPLRational, PPLRational, ParametricMonitor, ParametricPrinter>(
            timedAutomatonFileName, signatureFileName, timedWordFileName);
  } else if (vm.count("dataparametric")) {
    // data parametric
    return execute<DataParametricTA, DataParametricBoostTA, Parma_Polyhedra_Library::Coefficient, double, DataParametricMonitor, DataParametricPrinter>(
            timedAutomatonFileName, signatureFileName, timedWordFileName);
  } else {
    // boolean
    return execute<NonParametricTA<Number>, NonParametricBoostTA<Number>, Number, double, BooleanMonitor<Number>, BooleanPrinter>(
            timedAutomatonFileName, signatureFileName, timedWordFileName);
  }
  return 0;
}
