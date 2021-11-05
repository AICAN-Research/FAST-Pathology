//
// Created by dbouget on 02.11.2021.
//

#ifndef FASTPATHOLOGY_VIEWWIDGET_H
#define FASTPATHOLOGY_VIEWWIDGET_H

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
#include "source/logic/DataManager.h"
#include "source/utils/utilities.h"
#include "source/utils/qutilities.h"
#include "source/gui/ViewTab/DynamicViewTabWidget.h"

namespace fast {
    class WholeSlideImageImporter;
    class ImagePyramid;
    class ImagePyramidRenderer;
    class Renderer;
    class SegmentationRenderer;

class ViewWidget: public QWidget {
Q_OBJECT
public:
    ViewWidget(QWidget* parent=0);
    ~ViewWidget();

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

    /**
     * @brief createDynamicViewWidget
     * @param someName
     * @param modelName
     */
    void createDynamicViewWidget(std::string someName, std::string modelName);

    bool hideChannel(const std::string& name);
    void toggleRenderer(std::string someName);
    bool opacityRenderer(int value, const std::string& name);

private:
    QVBoxLayout* _main_layout; /* Principal layout holder for the current custom QWidget */
    QStackedLayout* _stacked_layout; /* ? */
    QWidget* _stacked_widget; /* ? */
    QComboBox* _page_combobox; /* ? */
    std::map<std::string, DynamicViewTabWidget*> _dynamic_widget_list; /* */

public slots:
    void processTriggerUpdate(std::string process_name);
};

}
#endif //FASTPATHOLOGY_VIEWWIDGET_H
