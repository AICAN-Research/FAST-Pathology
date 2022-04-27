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
#include <QStackedWidget>
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
    * Define the visual style for all elements inside the current global widget.
    */
    void setupStylesheets();

private:
    QVBoxLayout* _main_layout; /* Principal layout holder for the current custom QWidget */
    QStackedWidget* _layers_stacked_widget; /* Lower part holding as many stacks as results to display */
    QComboBox* _page_combobox; /* Combobox to easily move from one result to another and access their options */
    std::map<std::string, DynamicViewTabWidget*> _dynamic_widget_list; /** Container list for the dynamic widgets,
     not necessary for now as no extra-operations are needed, and the widgets can be iterated on from the stacked widget */

public slots:
    /**
     * Update to the View panel whenever the currently displayed WSI is changed, or changes state (i.e., is untoggled)
     * from a user click within the Project panel
     * @param wsi_uid Unique id in the DataManager for the clicked WSI
     * @param state true if the WSI has been toggled on and false if toggled off
     */
    void onChangeWSIDisplay(std::string wsi_uid, bool state);
    void processTriggerUpdate(std::string process_name);
    void deleteViewObjectReceived(std::string uid);

signals:
    void removeRendererFromViewRequested(const std::string&);
};

}
#endif //FASTPATHOLOGY_VIEWWIDGET_H
