#include <FAST/Tools/CommandLineParser.hpp>
#include "MainWindow.hpp"
#include <QApplication>
#include <QSplashScreen>

using namespace fast;

int main(int argc, char** argv) {
    Reporter::setGlobalReportMethod(Reporter::COUT);

    // Setup window
    auto window = MainWindow::New();
    window->start();
}

