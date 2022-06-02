#include <QLabel>
#include <QVBoxLayout>
#include "SplashWidget.hpp"
#include <QApplication>
#include <QDesktopWidget>
#include <QPushButton>
#include <QInputDialog>
#include <QListWidget>
#include <FAST/Utility.hpp>
#include <QDir>
#include <QMessageBox>
#include <fstream>
#include "source/utils/utilities.h"

namespace fast{

ProjectSplashWidget::ProjectSplashWidget(std::string rootFolder, QWidget* parent) : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint) {
    m_rootFolder = rootFolder;
    setWindowModality(Qt::ApplicationModal); // Lock on top
    auto layout = new QVBoxLayout();
    setLayout(layout);

    QPixmap image(QString::fromStdString(":/data/Icons/fastpathology_logo_large.png"));

    auto logo = new QLabel;
    logo->setPixmap(image);
    logo->setAlignment(Qt::AlignCenter);
    layout->addWidget(logo);

    auto title = new QLabel();
    title->setText("<h1>FastPathology </h1><h2>v" + QString(FAST_PATHOLOGY_VERSION) + "</h2><br>");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    auto horizontalLayout = new QHBoxLayout();
    layout->addLayout(horizontalLayout);

    auto leftLayout = new QVBoxLayout();
    horizontalLayout->addLayout(leftLayout);

    auto line = new QFrame;
    line->setLineWidth(3);
    line->setStyleSheet("border: 10px solid rgb(150, 150, 150);");
    line->setFrameShape(QFrame::VLine);
    horizontalLayout->addWidget(line);

    auto rightLayout = new QVBoxLayout();
    horizontalLayout->addLayout(rightLayout);

    auto recentLabel = new QLabel();
    recentLabel->setText("<h2>Recent projects</h2><br>Double click to open.");
    leftLayout->addWidget(recentLabel);

    auto recentList = new QListWidget();
    std::map<std::string, std::string> sortedFolders;
    for(auto folder : getDirectoryList(rootFolder, false, true)) {
        std::ifstream file(join(rootFolder, folder, "/timestamp.txt"));
        if(file.is_open()) {
            std::string timestamp;
            std::getline(file, timestamp);
            file.close();
            sortedFolders[timestamp] = folder;
        } else {
            std::cout << "Project " << folder << " was missing timestmap.txt" << std::endl;
        }
    }
    // Create a map reverse iterator
    decltype(sortedFolders)::reverse_iterator it;

    for (it = sortedFolders.rbegin(); it != sortedFolders.rend(); it++) {
        auto item = new QListWidgetItem(
                QString::fromStdString(it->second),
                recentList);
        //"Last modified: " + QString::fromStdString(it->first),
    }
    leftLayout->addWidget(recentList);
    connect(recentList, &QListWidget::itemDoubleClicked, [=](QListWidgetItem* item) {
        close();
        emit openProjectSignal(item->text());
    });

    auto deleteProjectButton = new QPushButton();
    deleteProjectButton->setText("Delete selected projects");
    leftLayout->addWidget(deleteProjectButton);
    connect(deleteProjectButton, &QPushButton::clicked, [=]() {
        auto reply = QMessageBox::question(this,
                   "Delete projects",
                   "Are you sure you whish to delete these projects?"
                        "<br>All results will be deleted, but whole-slide images will be left untouched.",
                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            for(auto item : recentList->selectedItems()) {
                std::cout << "Deleting " << (QString::fromStdString(rootFolder) + item->text() + "/").toStdString() << std::endl;
                auto dir = QDir(QString::fromStdString(rootFolder) + item->text() + "/");
                dir.removeRecursively();
                recentList->removeItemWidget(item);
                delete item;
            }
        }
    });

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
    QString text = QString::fromStdString(currentDateTime());
    bool ok;
    text = QInputDialog::getText(this, "Start new project",
                                 "Project name:", QLineEdit::Normal,
                                 text, &ok);
    if (ok && !text.isEmpty()) {
        // Check if already exists
        if(isDir(join(m_rootFolder, text.toStdString()))) {
            QMessageBox msgBox;
            msgBox.setText("A project with that name already exists, please select another name or delete the other project.");
            msgBox.setWindowTitle("Invalid project name");
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return;
        }
        close();
        emit newProjectSignal(text);
    }
}

}