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
#include <QDesktopServices>
#include <QTextEdit>
#include <QScreen>
#include "source/utils/utilities.h"
#include <FAST/DataHub.hpp>

namespace fast{

ProjectSplashWidget::ProjectSplashWidget(std::string rootFolder, bool allowClose, QWidget* parent) : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint) {
    m_rootFolder = rootFolder;
    setWindowModality(Qt::ApplicationModal); // Lock on top
    if(allowClose)
        setWindowFlags(Qt::Popup);
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

    horizontalLayout->addSpacing(20);

    auto line = new QFrame;
    line->setLineWidth(3);
    line->setStyleSheet("border: 10px solid rgb(150, 150, 150);");
    line->setFrameShape(QFrame::VLine);
    horizontalLayout->addWidget(line);

    horizontalLayout->addSpacing(20);

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
    QObject::connect(recentList, &QListWidget::itemDoubleClicked, [=](QListWidgetItem* item) {
        close();
        emit openProjectSignal(item->text());
    });

    auto deleteProjectButton = new QPushButton();
    deleteProjectButton->setText("Delete selected projects");
    leftLayout->addWidget(deleteProjectButton);
    QObject::connect(deleteProjectButton, &QPushButton::clicked, [=]() {
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
    newProjectButton->setStyleSheet("background-color: #ADD8E6;");
    rightLayout->addWidget(newProjectButton);
    QObject::connect(newProjectButton, &QPushButton::clicked, this, &ProjectSplashWidget::newProjectNameDialog);

    if(allowClose) {
        auto closeButton = new QPushButton();
        closeButton->setText("Close menu");
        rightLayout->addWidget(closeButton);
        QObject::connect(closeButton, &QPushButton::clicked, this, &ProjectSplashWidget::close);
    }

    auto dataHubButton = new QPushButton();
    dataHubButton->setText("Download models && pipelines");
    rightLayout->addWidget(dataHubButton);
    QObject::connect(dataHubButton, &QPushButton::clicked, this, &ProjectSplashWidget::dataHub);

    if(!fileExists(rootFolder + "/../images/LICENSE.md")) {
        auto downloadButton = new QPushButton();
        downloadButton->setText("Download and open test images");
        rightLayout->addWidget(downloadButton);
        QObject::connect(downloadButton, &QPushButton::clicked, this, &ProjectSplashWidget::downloadTestData);
    }

    auto quitButton = new QPushButton();
    quitButton->setText("Quit");
    rightLayout->addWidget(quitButton);
    QObject::connect(quitButton, &QPushButton::clicked, this, &ProjectSplashWidget::quitSignal);

    rightLayout->addSpacing(20);
    rightLayout->addStretch(20);

    auto helpButton = new QPushButton();
    helpButton->setText("Help");
    rightLayout->addWidget(helpButton);
    QObject::connect(helpButton, &QPushButton::clicked, this, &ProjectSplashWidget::helpUrl);

    auto reportButton = new QPushButton();
    reportButton->setText("Report an issue");
    rightLayout->addWidget(reportButton);
    QObject::connect(reportButton, &QPushButton::clicked, this, &ProjectSplashWidget::reportIssueUrl);

    auto aboutButton = new QPushButton();
    aboutButton->setText("About");
    rightLayout->addWidget(aboutButton);
    QObject::connect(aboutButton, &QPushButton::clicked, this, &ProjectSplashWidget::aboutProgram);


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

void ProjectSplashWidget::reportIssueUrl() {
    QDesktopServices::openUrl(QUrl("https://github.com/AICAN-Research/FAST-Pathology/issues", QUrl::TolerantMode));
}

void ProjectSplashWidget::helpUrl() {
    QDesktopServices::openUrl(QUrl("https://github.com/AICAN-Research/FAST-Pathology", QUrl::TolerantMode));
}

void ProjectSplashWidget::aboutProgram() {
    auto currLayout = new QVBoxLayout;

    QPixmap image(QString::fromStdString(":/data/Icons/fastpathology_logo_large.png"));

    auto label = new QLabel;
    label->setPixmap(image);
    label->setAlignment(Qt::AlignCenter);

    auto textBox = new QTextEdit;
    textBox->setEnabled(false);
    textBox->setText("<html><b>FastPathology " + QString(FAST_PATHOLOGY_VERSION) + "</b</html>");
    textBox->append("");
    textBox->setAlignment(Qt::AlignCenter);
    textBox->append("Open-source platform for deep learning-based research and decision support in digital pathology.");
    textBox->append("");
    textBox->append("");
    textBox->setAlignment(Qt::AlignCenter);
    textBox->append("Created by SINTEF Medical Technology and Norwegian University of Science and Technology (NTNU)");
    textBox->setAlignment(Qt::AlignCenter);
    textBox->setStyleSheet("QTextEdit { border: none }");
    textBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //textBox->setBaseSize(150, 200);

    currLayout->addWidget(label);
    currLayout->addWidget(textBox);

    auto dialog = new QDialog(this);
    dialog->setWindowTitle("About");
    dialog->setLayout(currLayout);
    dialog->setFixedSize(QSize(800, 800));

    dialog->show();
}

void ProjectSplashWidget::downloadTestData() {
    downloadZipFile("http://fast.eriksmistad.no/download/fastpathology-test-images-v1.0.0.zip", join(m_rootFolder, "..", "images"), "test dataset");
    // Create new project
    emit newProjectSignal("Test project");
    emit loadTestDataIntoProject();
    close();
}

void ProjectSplashWidget::dataHub() {
    try {
        auto browser = new DataHubBrowser("fast-pathology", "https://datahub.eriksmistad.no/", join(m_rootFolder, "..", "datahub"));
        QObject::connect(&browser->getDataHub(), &DataHub::finished, this, &ProjectSplashWidget::refreshPipelines);
        browser->setWindowModality(Qt::ApplicationModal);
        browser->show();
        browser->move(QGuiApplication::primaryScreen()->geometry().center() - rect().center());
    } catch(Exception &e) {
        QMessageBox msgBox;
        msgBox.setText("An error occured while trying to connect to the DataHub. Make sure you are connected to the internet and try again later.");
        msgBox.setWindowTitle("Error connecting");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return;
    }
}

}