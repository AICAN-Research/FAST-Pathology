#pragma once

#include <QWidget>

namespace fast{

class ProjectSplashWidget : public QWidget {
    Q_OBJECT
    public:
        ProjectSplashWidget(std::string rootFolder, bool allowClose, QWidget* parent = nullptr);
    private slots:
        void newProjectNameDialog();
        /**
         * Opens the default browser and directs the user to the FastPathology GitHub "Issues" section, to easily
         * report an issue/bug or request a feature.
         */
        void reportIssueUrl();
        /**
         * Opens the default browser and directs the user to the FastPathology GitHub page to seek for assistance.
         */
        void helpUrl();
        /**
         * Opens a simple dialog which contains information about the software.
         */
        void aboutProgram();

        void downloadTestData();

        /**
         * Opens a data hub browser to download models and pipelines
         */
        void dataHub();
    signals:
        void quitSignal();
        void newProjectSignal(QString name);
        void openProjectSignal(QString name);
        void loadTestDataIntoProject();
        void refreshPipelines();
    private:
        std::string m_rootFolder;
};
}