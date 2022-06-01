#include <QLabel>
#include <QVBoxLayout>
#include "SplashWidget.hpp"
#include <QApplication>
#include <QDesktopWidget>
#include <QPushButton>
#include <QInputDialog>

namespace fast{

ProjectSplashWidget::ProjectSplashWidget(QWidget* parent) : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint) {
    setWindowModality(Qt::ApplicationModal); // Lock on top
    auto layout = new QVBoxLayout();
    setLayout(layout);

    auto title = new QLabel();
    title->setText("<h1>FastPathology</h1>");
    layout->addWidget(title);

    auto horizontalLayout = new QHBoxLayout();
    layout->addLayout(horizontalLayout);

    auto leftLayout = new QVBoxLayout();
    horizontalLayout->addLayout(leftLayout);

    auto line = new QFrame;
    line->setFrameShape(QFrame::VLine);
    horizontalLayout->addWidget(line);

    auto rightLayout = new QVBoxLayout();
    horizontalLayout->addLayout(rightLayout);

    auto recentLabel = new QLabel();
    recentLabel->setText("<h2>Recent projects</h2>");
    leftLayout->addWidget(recentLabel);

    auto newProjectButton = new QPushButton();
    newProjectButton->setText("Start new project");
    rightLayout->addWidget(newProjectButton);
    connect(newProjectButton, &QPushButton::clicked, this, &ProjectSplashWidget::newProjectNameDialog);

    auto quitButton = new QPushButton();
    quitButton->setText("Quit");
    rightLayout->addWidget(quitButton);
    connect(quitButton, &QPushButton::clicked, this, &ProjectSplashWidget::quitSignal);

    // Move to the center
    adjustSize();
    move(QApplication::desktop()->screen()->rect().center() - rect().center());
}

void ProjectSplashWidget::newProjectNameDialog() {
    QString date = "20222";
    bool ok;
    QString text = QInputDialog::getText(this, "Start new project",
                                         "Project name:", QLineEdit::Normal,
                                         date, &ok);
    if (ok && !text.isEmpty()) {
        emit newProjectSignal(text);
        close();
    }
}

}