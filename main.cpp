#include <FAST/Tools/CommandLineParser.hpp>
#include "MainWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
    Reporter::setGlobalReportMethod(Reporter::COUT);
    /*
    CommandLineParser parser("File to load");
    parser.addPositionVariable(1, "filename", true);
    parser.parse(argc, argv);
     */

    // Setup window
    auto window = MainWindow::New();
    window->start();
}

