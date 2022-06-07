#include <FAST/Tools/CommandLineParser.hpp>
#include "source/gui/MainWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
#if defined(__APPLE__) || defined(__MACOSX)
    // Mac hack for runInThread https://github.com/AICAN-Research/FAST-Pathology/issues/26
    std::cout << "DISABLE AFFINITY GL CHECK" << std::endl;
    QCoreApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity);
#endif
    CommandLineParser parser("FastPathology", "An open-source platform for deep learning-based research and decision support in digital pathology");
    parser.parse(argc, argv);

    // Setup window
    auto window = MainWindow::New();
    window->start();
}

