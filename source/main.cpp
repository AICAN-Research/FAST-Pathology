#include <FAST/Tools/CommandLineParser.hpp>
#include "MainWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("FastPathology", "An open-source platform for deep learning-based research and decision support in digital pathology");
    parser.parse(argc, argv);
        
    // Setup window
    auto window = MainWindow::New();
    window->start();
}
