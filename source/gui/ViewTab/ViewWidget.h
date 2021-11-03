//
// Created by dbouget on 02.11.2021.
//

#ifndef FASTPATHOLOGY_VIEWWIDGET_H
#define FASTPATHOLOGY_VIEWWIDGET_H

#include <string>
#include <iostream>
#include <fstream>
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
#include <iostream>
#include <FAST/Visualization/Renderer.hpp>
#include "source/logic/DataManager.h"
#include "source/utils/utilities.h"
#include "source/utils/qutilities.h"


namespace fast {
    class WholeSlideImageImporter;
    class ImagePyramid;
    class ImagePyramidRenderer;
    class Renderer;

class ViewWidget: public QWidget {
Q_OBJECT
public:
    ViewWidget(QWidget* parent=0);
    ~ViewWidget();

protected:
    /**
     * Define the interface for the current global widget.
     */
    void setupInterface();

    /**
     * Set the interface in its default state.
     */
    void resetInterface();

    /**
     * Define the connections for all elements inside the current global widget.
     */
    void setupConnections();

private:
    QVBoxLayout* _main_layout; /* Principal layout holder for the current custom QWidget */
    QStackedLayout* _stacked_layout; /* ? */
    QWidget* _stacked_widget; /* ? */
    QComboBox* _page_combobox; /* ? */

};

}
#endif //FASTPATHOLOGY_VIEWWIDGET_H
