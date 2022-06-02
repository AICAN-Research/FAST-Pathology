#pragma once

#include <string>
#include <iostream>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QCoreApplication>
#include <QTextStream>
#include <QMessageBox>
#include <QProgressDialog>
#include <QListWidgetItem>
#include <QScrollArea>
#include <QScreen>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDirIterator>
#include <iostream>
#include <FAST/Visualization/Renderer.hpp>
#include "source/utils/utilities.h"
#include "source/gui/ProjectTab/ProjectThumbnailPushButton.h"


namespace fast {
    class WholeSlideImageImporter;
    class ImagePyramid;
    class ImagePyramidRenderer;
    class Renderer;
    class MainWindow;

class ProjectWidget: public QWidget {
Q_OBJECT
public:
    ProjectWidget(MainWindow* mainWindow, QWidget* parent=0);
    ~ProjectWidget();

    /**
     * Set the interface in its default state.
     */
    void resetInterface();

    /**
     * Imports WSIs to the program, where selections are made from a drag-and-drop event.
     * @param fileNames
     */
    void selectFileDrag(const QList<QString> &fileNames);

public slots:
    void selectFile();
    /**
     * @brief changeWSIDisplayReceived To toggle/untoggle the main view with the WSI represented by id_name
     * @param id_name Unique name for the WSI to consider
     * @param state True to display the WSI in the main central view and False to hide it.
     */
    void changeWSIDisplayReceived(std::string id_name, bool state);

    /**
     * @brief removeImage To remove one WSI from the list (and memory).
     * @param uid Unique name for the considered WSI.
     */
    void removeImage(std::string uid);
    void loadProject();
    void updateTitle();
signals:
    void changeWSIDisplayTriggered(std::string, bool);
    void resetDisplay();

protected:
    /**
     * Define the interface for the current global widget.
     */
    void setupInterface();

    /**
     * Creates the WSI scroll area for selecting which WSI to work with.
     */
    void createWSIScrollAreaWidget();

    /**
     * Define the connections for all elements inside the current global widget.
     */
    void setupConnections();

    void loadSelectedWSIs(const QList<QString> &fileNames);

private:
    QPushButton* _selectFileButton;
    QVBoxLayout* _main_layout;
    QScrollArea* _wsi_scroll_area;
    QListWidget* _wsi_scroll_listwidget;
    QWidget* _wsi_scroll_widget;
    QVBoxLayout* _wsi_scroll_layout;
    std::map<std::string, QListWidgetItem*> _wsi_thumbnails_listitem;
    std::map<std::string, ProjectThumbnailPushButton*> _thumbnail_qpushbutton_map;
    MainWindow* m_mainWindow;
    QLabel* m_projectLabel;
};

}