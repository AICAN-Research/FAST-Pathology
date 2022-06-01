#include "ProjectWidget.h"
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Reporter.hpp>
#include "source/gui/MainWindow.hpp"

namespace fast {
    ProjectWidget::ProjectWidget(MainWindow* mainWindow, QWidget *parent): QWidget(parent){
        m_mainWindow = mainWindow;
        setupInterface();
        setupConnections();
    }

    ProjectWidget::~ProjectWidget(){

    }

    void ProjectWidget::updateTitle() {
        m_projectLabel->setText(QString::fromStdString(
            "<h2>Project: " + m_mainWindow->getCurrentProject()->getName() + "</h2>"
             "Storage path: " + m_mainWindow->getCurrentProject()->getRootFolder()
         ));
    }

    void ProjectWidget::setupInterface()
    {
        m_projectLabel = new QLabel();
        m_projectLabel->setWordWrap(true);

        _selectFileButton = new QPushButton(this);
        _selectFileButton->setText("Import images");
        //selectFileButton->setFixedWidth(200);
        _selectFileButton->setFixedHeight(50);
        //selectFileButton->setStyleSheet("color: white; background-color: blue");

        _main_layout = new QVBoxLayout(this);
        _main_layout->addWidget(m_projectLabel);
        _main_layout->addWidget(_selectFileButton);
        createWSIScrollAreaWidget();
    }

    void ProjectWidget::resetInterface()
    {
        _projectFolderName = QString();
        _wsi_scroll_listwidget->clear();
        _thumbnail_qpushbutton_map.clear();
        emit resetDisplay();
        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
        // TODO m_mainWindow->doEmpty();
    }

    void ProjectWidget::setupConnections() {
        QObject::connect(_selectFileButton, &QPushButton::clicked, this, &ProjectWidget::selectFile);
    }

    void ProjectWidget::createWSIScrollAreaWidget() {
        //auto scrollAreaDialog = new QDialog();
        //scrollAreaDialog->setGeometry(100, 100, 260, 260);

        // TODO: Need to substitute this with QListWidget or similar as it's quite slow and memory expensive for
        //        larger number of elements
        _wsi_scroll_area = new QScrollArea(this);  //scrollAreaDialog);
        _wsi_scroll_area->setAlignment(Qt::AlignTop);
        //scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        _wsi_scroll_area->setWidgetResizable(true);
        _wsi_scroll_area->setGeometry(10, 10, 200, 200);

        _wsi_scroll_listwidget = new QListWidget(this);
        //scrollList->setStyleSheet("border: none, padding: 0, background: white, color: none");
        _wsi_scroll_listwidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        _wsi_scroll_listwidget->setItemAlignment(Qt::AlignTop);
        _wsi_scroll_listwidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        _wsi_scroll_listwidget->setResizeMode(QListView::Adjust);  // resizable adaptively
        _wsi_scroll_listwidget->setGeometry(10, 10, 200, 200);
        //scrollList->setStyleSheet("*:hover {background:blue;}");  // setting color of widget when hovering
        //scrollList->setStyleSheet("QListWidget:item:selected:active {background: blue;}; QListWidget:item:selected:!active {background: gray};");
        //scrollList->setFocus();
        //scrollList->setViewMode(QListWidget::IconMode);
        //scrollList->setIconSize(QSize(100, 100));
        //scrollList->setFlow(QListView::TopToBottom);
        //scrollList->setResizeMode(QListWidget::Adjust);
        //QObject::connect(scrollList, &QPushButton::clicked, std::bind(&MainWindow::selectFileInProject, this, 1));
        //QObject::connect(scrollList, &QListWidget::itemPressed, std::bind(&MainWindow::selectFileInProject, this, 1));  // this, SLOT(onListMailItemClicked(QListWidgetItem*)));
        //QObject::connect(scrollList,itemClicked(QListWidgetItem*), std::bind(&MainWindow::selectFileInProject, this, 1));
        //connect(ui->listMail, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onListMailItemClicked(QListWidgetItem*)));
        // QListWidget::itemPressed(QListWidgetItem *item)
        //QObject::connect(scrollList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(&MainWindow::selectFileInProject));

        //scrollList->setSelectionMode(QListWidgetItem::NoSelection);
        //QObject::connect(scrollList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(itemDoubleClicked(QListWidgetItem*)));

        _wsi_scroll_area->setWidget(_wsi_scroll_listwidget);

        _wsi_scroll_widget = new QWidget(this);
        //scrollArea->setWidget(scrollWidget);

        _wsi_scroll_layout = new QVBoxLayout;
        _wsi_scroll_widget->setLayout(_wsi_scroll_layout);

        //connect(scrollList, SIGNAL(activated(int)), scrollLayout, SLOT(setCurrentIndex(int)));

        //fileLayout->insertWidget(2, scrollArea);  //widget);
        _main_layout->addWidget(_wsi_scroll_area);

        //scrollAreaDialog->show();
    }

    void ProjectWidget::createProject() {
        // start by selecting where to create folder and give project name
        // should also handle if folder name already exist, prompt warning and option to change name
        QFileDialog dialog(this);
        dialog.setFileMode(QFileDialog::AnyFile);

        _projectFolderName = dialog.getExistingDirectory(this, tr("Set Project Directory"),
                QCoreApplication::applicationDirPath(), QFileDialog::DontUseNativeDialog);

        Reporter::info()<< "Project dir: " << _projectFolderName.toStdString() << Reporter::end();
        m_mainWindow->getCurrentProject()->setRootFolder(_projectFolderName.toStdString());

        // create file for saving which WSIs exist in folder
        QString projectFileName = "/project.txt";
        QFile file(_projectFolderName + projectFileName);
        if (file.open(QIODevice::ReadWrite)) {
            QTextStream stream(&file);
        }

        // now create folders for saving results and such (prompt warning if name already exists)
        QDir().mkdir(_projectFolderName + QString::fromStdString("/results"));
        QDir().mkdir(_projectFolderName + QString::fromStdString("/pipelines"));
        QDir().mkdir(_projectFolderName + QString::fromStdString("/thumbnails"));

        // check if any WSIs have been selected previously, and ask if you want to make a project and add these,
        // or make a new fresh one -> if no, need to clear all WSIs in the QListWidget
        if (!m_mainWindow->getCurrentProject()->isProjectEmpty())
        {
            // prompt
            QMessageBox mBox;
            mBox.setIcon(QMessageBox::Warning);
            mBox.setText("There are already WSIs that has been used.");
            mBox.setInformativeText("Do you wish to add them to the project?");
            mBox.setDefaultButton(QMessageBox::Yes);
            mBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            int ret = mBox.exec();

            switch (ret) {
                case QMessageBox::Yes:
                    std::cout << "Saved!" << std::endl;
                    saveProject();
                    break;
                case QMessageBox::No:
                    std::cout << "Removing WSIs from QListWidget!" << std::endl;
                    _wsi_scroll_listwidget->clear();
                    m_mainWindow->getCurrentProject()->emptyProject();
                    emit resetDisplay();
                    break;
                default:
                    break;
            }
        }
    }

    void ProjectWidget::saveProject()
    {
        if (m_mainWindow->getCurrentProject()->isProjectEmpty())
            return;

        m_mainWindow->getCurrentProject()->saveProject();
    }

    void ProjectWidget::openProject() {
        // If existing data, asks for confirmation to delete.
        if (!m_mainWindow->getCurrentProject()->isProjectEmpty())
        {
            QMessageBox mBox;
            mBox.setIcon(QMessageBox::Warning);
            mBox.setText("Opening project will erase current edits.");
            mBox.setInformativeText("Do you still wish to open?");
            mBox.setDefaultButton(QMessageBox::No);
            mBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            int ret = mBox.exec();

            switch (ret) {
                case QMessageBox::Yes:
                    _wsi_scroll_listwidget->clear();
                    _thumbnail_qpushbutton_map.clear();
                    emit resetDisplay();
                    QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
                    m_mainWindow->getCurrentProject()->emptyProject();
                    break;
                case QMessageBox::No:
                    break;
                default:
                    break;
            }
        }

        // select project file
        QFileDialog dialog(this);
        dialog.setFileMode(QFileDialog::ExistingFile);
        QString projectPath = dialog.getOpenFileName(
                this,
                tr("Select Project File"), nullptr,
                tr("Project (*project.txt)"),
                nullptr, QFileDialog::DontUseNativeDialog
        );

        std::cout << projectPath.toStdString() << std::endl;
        _projectFolderName = splitCustom(projectPath.toStdString(), "project.txt")[0].c_str();
        std::cout << _projectFolderName.toStdString() << std::endl;
        m_mainWindow->getCurrentProject()->setRootFolder(_projectFolderName.toStdString());

        auto progDialog = QProgressDialog(this);
        progDialog.setRange(0, m_mainWindow->getCurrentProject()->getWSICountInProject()-1);
        //progDialog.setContentsMargins(0, 0, 0, 0);
        progDialog.setVisible(true);
        progDialog.setModal(false);
        progDialog.setLabelText("Loading WSIs...");
        //QRect screenrect = mWidget->screen()[0].geometry();
        progDialog.move(width() - progDialog.width() * 1.1, progDialog.height() * 0.1);
        progDialog.show();
        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);

        m_mainWindow->getCurrentProject()->loadProject();

        for (std::string uid: m_mainWindow->getCurrentProject()->getAllWsiUids())
        {
            Reporter::info() << "Selected file: " << uid << Reporter::end();
            auto button = new ProjectThumbnailPushButton(m_mainWindow, uid, this);
            int width_val = 100;
            int height_val = 150;
            auto listItem = new QListWidgetItem;
            listItem->setSizeHint(QSize(width_val, height_val));
            QObject::connect(button, &ProjectThumbnailPushButton::clicked, this, &ProjectWidget::changeWSIDisplayReceived);
            QObject::connect(button, &ProjectThumbnailPushButton::rightClicked, this, &ProjectWidget::removeImage);
            _wsi_scroll_listwidget->addItem(listItem);
            _wsi_scroll_listwidget->setItemWidget(listItem, button);
            _wsi_thumbnails_listitem[uid] = listItem;
            _thumbnail_qpushbutton_map[uid] = button;
            QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
        }
    }

    void ProjectWidget::selectFile() {
        // TODO: Unable to read .zvi and .scn (Zeiss and Leica). I'm wondering if they are stored in some unexpected way (not image pyramids)
        auto fileNames = QFileDialog::getOpenFileNames(this, tr("Select File(s)"), nullptr,
                                                       tr("WSI Files (*.tiff *.tif *.svs *.ndpi *.bif *vms *.vsi);;All Files(*)"), //*.zvi *.scn)"),
                nullptr, QFileDialog::DontUseNativeDialog);

        auto progDialog = QProgressDialog(this);
        progDialog.setRange(0, fileNames.count()-1);
        //progDialog.setContentsMargins(0, 0, 0, 0);
        progDialog.setVisible(true);
        progDialog.setModal(false);
        progDialog.setLabelText("Loading WSIs...");
        //QRect screenrect = mWidget->screen()[0].geometry();
        progDialog.move(width() - progDialog.width() * 1.1, progDialog.height() * 0.1);
        progDialog.show();

        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
        loadSelectedWSIs(fileNames);
    }

    void ProjectWidget::loadSelectedWSIs(const QList<QString> &fileNames)
    {
        auto currentPosition = 0; // curr_pos <=?

        // need to handle scenario where a WSI is added, but there already exists N WSIs from before
        auto nb_wsis_in_list = 0; //wsiList.size();
        if (nb_wsis_in_list != 0)
            currentPosition = nb_wsis_in_list;

        int counter = 0;
        for (QString fileName : fileNames)
        {
            if (fileName == "")
                return;
            //filename = fileName.toStdString();
            auto currFileName = fileName.toStdString();
            Reporter::info() << "Selected file: " << currFileName << Reporter::end();
            const std::string id_name = m_mainWindow->getCurrentProject()->includeImage(currFileName);

            // @TODO. Shouldn't we keep a class attribute list with those buttons, so that it's easier to retrieve them
            // and delete them on the fly?
            auto button = new ProjectThumbnailPushButton(m_mainWindow, id_name, this);
            _thumbnail_qpushbutton_map[id_name] = button;
            int width_val = 100;
            int height_val = 150;
            auto listItem = new QListWidgetItem;
            listItem->setSizeHint(QSize(width_val, height_val));
            QObject::connect(button, &ProjectThumbnailPushButton::clicked, this, &ProjectWidget::changeWSIDisplayReceived);
            QObject::connect(button, &ProjectThumbnailPushButton::rightClicked, this, &ProjectWidget::removeImage);
            _wsi_scroll_listwidget->addItem(listItem);
            _wsi_scroll_listwidget->setItemWidget(listItem, button);
            _wsi_thumbnails_listitem[id_name] = listItem;

//            emit newImageFilename(id_name);
            // to render straight away (avoid waiting on all WSIs to be handled before rendering)
            QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
        }
    }

    void ProjectWidget::loadProject()
    {
        auto currentPosition = 0; // curr_pos <=?

        // need to handle scenario where a WSI is added, but there already exists N WSIs from before
        auto nb_wsis_in_list = 0; //wsiList.size();
        if (nb_wsis_in_list != 0)
            currentPosition = nb_wsis_in_list;

        int counter = 0;
        for (auto uid : m_mainWindow->getCurrentProject()->getAllWsiUids())
        {
            auto fileName = m_mainWindow->getCurrentProject()->getImage(uid)->get_filename();
            auto currFileName = fileName;
            Reporter::info() << "Selected file: " << currFileName << Reporter::end();
            //const std::string id_name = m_mainWindow->getCurrentProject()->includeImage(currFileName);

            // @TODO. Shouldn't we keep a class attribute list with those buttons, so that it's easier to retrieve them
            // and delete them on the fly?
            auto button = new ProjectThumbnailPushButton(m_mainWindow, uid, this);
            _thumbnail_qpushbutton_map[uid] = button;
            int width_val = 100;
            int height_val = 150;
            auto listItem = new QListWidgetItem;
            listItem->setSizeHint(QSize(width_val, height_val));
            QObject::connect(button, &ProjectThumbnailPushButton::clicked, this, &ProjectWidget::changeWSIDisplayReceived);
            QObject::connect(button, &ProjectThumbnailPushButton::rightClicked, this, &ProjectWidget::removeImage);
            _wsi_scroll_listwidget->addItem(listItem);
            _wsi_scroll_listwidget->setItemWidget(listItem, button);
            _wsi_thumbnails_listitem[uid] = listItem;

            //            emit newImageFilename(id_name);
            // to render straight away (avoid waiting on all WSIs to be handled before rendering)
            QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
        }
    }

    void ProjectWidget::removeImage(std::string uid)
    {
        _wsi_scroll_listwidget->removeItemWidget(_wsi_thumbnails_listitem[uid]);
        _wsi_thumbnails_listitem.erase(uid);
        _thumbnail_qpushbutton_map.erase(uid);
        _wsi_scroll_listwidget->update();
        emit changeWSIDisplayTriggered(uid, false);
        m_mainWindow->getCurrentProject()->removeImage(uid);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
    }

    void ProjectWidget::changeWSIDisplayReceived(std::string id_name, bool state)
    {
        if(state)
        {
            for(auto it = _thumbnail_qpushbutton_map.begin(); it != _thumbnail_qpushbutton_map.end(); it++)
            {
                if(it->first != id_name)
                    it->second->setCheckedState(false);
            }
        }
        emit changeWSIDisplayTriggered(id_name, state);
    }

    void ProjectWidget::downloadAndAddTestData() {
        auto mBox = new QMessageBox(this);
        mBox->setIcon(QMessageBox::Warning);
        mBox->setText("This will download the test data, add the models, and open two WSIs.");
        mBox->setInformativeText("Are you sure you want to continue?");
        mBox->setDefaultButton(QMessageBox::Yes);
        mBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        int ret = mBox->exec();

        switch (ret) {
        case QMessageBox::Yes:
            // toggle and update text on button to show current mode (also default)
            break;
        case QMessageBox::No:
            // if "No", do nothing
            return;
        case QMessageBox::Cancel:
            // if "Cancel", do nothing
            return;
        default:
            break;
        }

        // progress bar
        auto progDialog = new QProgressDialog(this);
        progDialog->setRange(0, 0);
        progDialog->setValue(0);
        progDialog->setVisible(true);
        progDialog->setModal(false);
        progDialog->setLabelText("Downloading test data...");
        progDialog->move(width() - progDialog->width() * 1.1, progDialog->height() * 0.1);
        //m_pBar.show();

        QUrl url{"http://folk.ntnu.no/andpeder/FastPathology/test_data.zip"};

        QNetworkAccessManager* m_NetworkMngr = new QNetworkAccessManager(this);
        QNetworkReply *reply = m_NetworkMngr->get(QNetworkRequest(QUrl(url)));
        QEventLoop loop;

        // @TODO. Download does NOT work...
        QObject::connect(reply, &QNetworkReply::downloadProgress, [=](qint64 ist, qint64 max_) { //[=](qint64 ist, qint64 max) {
            progDialog->setRange(0, max_);
            progDialog->setValue(ist);
            //if (max < 0) hideProgress();
        });
        QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

        loop.exec();
        QUrl aUrl(url);
        QFileInfo fileInfo = aUrl.path();

        QString downloadsFolder = QDir::homePath() + "/fastpathology/data";
        QFile file(downloadsFolder + "/" + fileInfo.fileName());
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();
        delete reply;

        // unzip TODO: Is there something wrong with the include/import of this function? Might be a problem later on.
        extractZipFile((downloadsFolder + "/" + fileInfo.fileName()).toStdString(), downloadsFolder.toStdString());

        // OPTIONAL: Add Models to test if inference is working
        QList<QString> fileNames;
        QDirIterator it2(downloadsFolder + "/test_data/Models/", QDir::Files);
        while (it2.hasNext()) {
            auto tmp = it2.next();
            fileNames.push_back(tmp);
        }
        // @TODO. To link to the PipelineManager to create.
//        addModelsDrag(fileNames);

        auto mBox2 = new QMessageBox(this);
        mBox2->setIcon(QMessageBox::Warning);
        mBox2->setText("Download is finished.");
        mBox2->setInformativeText("Do you wish to open the test WSIs?");
        mBox2->setDefaultButton(QMessageBox::Yes);
        mBox2->setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        int ret2 = mBox2->exec();

        switch (ret2) {
        case QMessageBox::Yes:
            // toggle and update text on button to show current mode
            break;
        case QMessageBox::No:
            // if "No", do nothing (also default)
            return;
        case QMessageBox::Cancel:
            // if "Cancel", do nothing
            return;
        default:
            break;
        }

        // OPTIONAL: Add WSIs to project for visualization
        fileNames.clear();
        QDirIterator it(downloadsFolder + "/test_data/WSI/", QDir::Files);
        while (it.hasNext()) {
            auto tmp = it.next();
            fileNames.push_back(tmp);
        }
        selectFileDrag(fileNames);
    }

    void ProjectWidget::selectFileDrag(const QList<QString> &fileNames)
    {
//        // check if view object list is empty, if not, prompt to save results or not, if not clear
//        if (!m_mainWindow->isEmpty()) {
//            QMessageBox mBox;
//            mBox.setIcon(QMessageBox::Warning);
//            mBox.setStyleSheet(styleSheet());
//            mBox.setText("There are unsaved results.");
//            mBox.setInformativeText("Do you wish to save them?");
//            mBox.setDefaultButton(QMessageBox::Save);
//            mBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
//            int ret = mBox.exec();

//            switch (ret) {
//                case QMessageBox::Save:
//                    std::cout << "Results not saved yet. Just cancelled the switch!" << std::endl;
//                    // Save was clicked
//                    return;
//                case QMessageBox::Discard:
//                    // Don't Save was clicked
//                    std::cout << "Discarded!" << std::endl;
//                    break;
//                case QMessageBox::Cancel:
//                    // Cancel was clicked
//                    std::cout << "Cancelled!" << std::endl;
//                    return;
//                default:
//                    // should never be reached
//                    break;
//            }
//        }
//        /*
//        if (pageComboBox->count() != 0) { // if not empty, clear
//            pageComboBox->clear();
//            exportComboBox->clear();
//        }
//         */


//        resetInterface();

        auto progDialog = QProgressDialog(this);
        progDialog.setRange(0, fileNames.count()-1);
        progDialog.setVisible(true);
        progDialog.setModal(false);
        progDialog.setLabelText("Loading WSIs...");
        QRect screenrect = screen()[0].geometry();
        progDialog.move(width() - progDialog.width() / 2, - width() / 2 - progDialog.width() / 2);
        progDialog.show();

        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
        loadSelectedWSIs(fileNames);
    }


} // End of namespace fast
