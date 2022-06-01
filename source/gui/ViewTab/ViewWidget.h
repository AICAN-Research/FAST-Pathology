#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QFileDialog>
#include <QCoreApplication>
#include <QTextStream>
#include <QMessageBox>
#include <QProgressDialog>
#include <QListWidgetItem>
#include <QScrollArea>
#include <QComboBox>
#include <QLabel>
#include <QFileDialog>
#include <QColorDialog>
#include <QGroupBox>
#include <iostream>
#include <FAST/Visualization/Renderer.hpp>
#include "source/utils/utilities.h"
#include "source/utils/qutilities.h"
#include "source/logic/Project.h"

namespace fast {

class MainWindow;

class ViewWidget: public QWidget {
Q_OBJECT
public:
    ViewWidget(MainWindow* mainWindow, QWidget* parent=0);
    ~ViewWidget();

    /**
     * Set the interface in its default state.
     */
    void resetInterface();

    void setResults(std::vector<Result> results);

protected:
    /**
     * Define the interface for the current global widget.
     */
    void setupInterface();

    /**
     * Define the connections for all elements inside the current global widget.
     */
    void setupConnections();

    void writeRendererAttributes(Result result);

private:
    MainWindow* m_mainWindow;
    QVBoxLayout* _main_layout; /* Principal layout holder for the current custom QWidget */
    QStackedLayout* _stacked_layout;
    QWidget* _stacked_widget;
    QComboBox* _page_combobox;
};

}
