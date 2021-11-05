//
// Created by dbouget on 03.11.2021.
//

#ifndef FASTPATHOLOGY_STATSWIDGET_H
#define FASTPATHOLOGY_STATSWIDGET_H

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
#include <QPainter>
#include <QPen>
#include <QTextEdit>
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

class StatsWidget: public QWidget {
Q_OBJECT
public:
    StatsWidget(QWidget* parent=0);
    ~StatsWidget();
    /**
     * Set the interface in its default state.
     */
    void resetInterface();

protected:
    /**
     * Define the interface for the current global widget.
     */
    void setupInterface();

    /**
     * Define the connections for all elements inside the current global widget.
     */
    void setupConnections();

     const bool calcTissueHist();

private:
    QVBoxLayout* _main_layout; /* Principal layout holder for the current custom QWidget */
    QPushButton* _calc_hist_pushbutton; /* */
};

}
#endif //FASTPATHOLOGY_STATSWIDGET_H
