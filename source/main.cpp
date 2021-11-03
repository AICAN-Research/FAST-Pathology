#include <FAST/Tools/CommandLineParser.hpp>
#include "source/gui/MainWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("FastPathology", "An open-source platform for deep learning-based research and decision support in digital pathology");
    //parser.addOption("--verbose", "Print info messages");
    //parser.addOption("--disable-sharpening", "Disable sharpening filter when rendering WSIs");
    //parser.addOption("--opencl-platform", "Set which opencl platform to use");
    parser.parse(argc, argv);

    //if (parser.getOption("--disable-sharpening"))
    //    MainWindow::m_disableSharpening = true;
        
    // Setup window
    auto window = MainWindow::New();
    window->start();
}

