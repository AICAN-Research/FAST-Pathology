#pragma once

#include <QWidget>

namespace fast{

class ProjectSplashWidget : public QWidget {
    Q_OBJECT
    public:
        ProjectSplashWidget(QWidget* parent = nullptr);
    private slots:
        void newProjectNameDialog();
    signals:
        void quitSignal();
        void newProjectSignal(QString name);
    private:
};
}