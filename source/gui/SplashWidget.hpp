#pragma once

#include <QWidget>

namespace fast{

class ProjectSplashWidget : public QWidget {
    Q_OBJECT
    public:
        ProjectSplashWidget(std::string rootFolder, QWidget* parent = nullptr);
    private slots:
        void newProjectNameDialog();
    signals:
        void quitSignal();
        void newProjectSignal(QString name);
        void openProjectSignal(QString name);
    private:
};
}