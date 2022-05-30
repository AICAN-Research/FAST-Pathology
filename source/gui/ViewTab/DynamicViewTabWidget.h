//
// Created by dbouget on 04.11.2021.
//

#ifndef FASTPATHOLOGY_DYNAMICVIEWTABWIDGET_H
#define FASTPATHOLOGY_DYNAMICVIEWTABWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QGroupBox>
#include <QColorDialog>
#include <QComboBox>
#include <QList>
#include <QString>
#include "source/logic/DataManager.h"
#include "source/utils/qutilities.h"
#include "source/utils/utilities.h"

namespace fast {
    class Renderer;
    class HeatmapRenderer;
    class SegmentationRenderer;
    class BoundingBoxRenderer;
    class ImagePyramidRenderer;

class DynamicViewTabWidget: public QWidget {
Q_OBJECT
public:
    DynamicViewTabWidget(const std::string& renderer_name, QWidget* parent=0);
    ~DynamicViewTabWidget();

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
    const std::string _renderer_name; /* Unique id/name for the current object, linked to the renderer name (model_name in model metadata)*/
    QList<QString> _classes; /* */
    uint _current_class; /* */

    QVBoxLayout* _main_layout; /* */
    QPushButton* _toggle_image_pushbutton; /* */
    QSlider* _opacity_slider; /* */
    QLabel* _tissue_label; /* */
    QHBoxLayout* _tissue_textbox_layout; /* */
    QWidget* _tissue_textbox_widget; /* */
    QPushButton* _set_color_pushbutton; /* */
    QPushButton* _toggle_class_pushbutton; /* */
    QPushButton* _delete_tab_pushbutton; /* */
    QGroupBox* _renderer_groupbox; /* */
    QGroupBox* _class_groupbox; /* */
    QComboBox* _class_list_combobox; /* */
    QLabel* _class_selector_label; /* */
    QHBoxLayout* _class_selector_layout; /* */
    QHBoxLayout* _toggle_color_layout; /* */

    QVBoxLayout* _all_view_layout; /* */
    QVBoxLayout* _class_view_layout; /* */

public slots:
    void toggleRendererReceived();
    bool opacityRendererReceived(int value);
    bool hideChannelReceived();

private slots:
    void updateCurrentClassIndexReceived(int index);

signals:
    void deleteViewObjectTriggered(std::string&);
};

} // End of namespace fast

#endif //FASTPATHOLOGY_DYNAMICVIEWTABWIDGET_H
