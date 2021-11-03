//
// Created by dbouget on 06.10.2021.
//

#include "ProjectWidget.h"

#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Data/ImagePyramid.hpp>

namespace fast {
    ProjectWidget::ProjectWidget(QWidget *parent): QWidget(parent){
        this->setupInterface();
        this->setupConnections();
    }

    ProjectWidget::~ProjectWidget(){

    }

    void ProjectWidget::setupInterface()
    {
        this->_createProjectButton = new QPushButton(this);
        this->_createProjectButton->setText("Create Project");
        this->_createProjectButton->setFixedHeight(50);
        this->_createProjectButton->setStyleSheet("color: white; background-color: blue");

        this->_openProjectButton = new QPushButton(this);
        this->_openProjectButton->setText("Open Project");
        this->_openProjectButton->setFixedHeight(50);
        //openProjectButton->setStyleSheet("color: white; background-color: blue");

        this->_selectFileButton = new QPushButton(this);
        this->_selectFileButton->setText("Import WSIs");
        //selectFileButton->setFixedWidth(200);
        this->_selectFileButton->setFixedHeight(50);
        //selectFileButton->setStyleSheet("color: white; background-color: blue");

        this->_main_layout = new QVBoxLayout(this);
        this->_main_layout->addWidget(this->_createProjectButton);
        this->_main_layout->addWidget(this->_openProjectButton);
        this->_main_layout->addWidget(this->_selectFileButton);
        this->createWSIScrollAreaWidget();
    }

    void ProjectWidget::resetInterface()
    {
        _wsi_scroll_listwidget->clear();
        _thumbnail_qpushbutton_map.clear();
        emit resetDisplay();
        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
        DataManager::GetInstance()->doEmpty();
    }

    void ProjectWidget::setupConnections() {
        QObject::connect(this->_createProjectButton, &QPushButton::clicked, this, &ProjectWidget::createProject);
//        QObject::connect(this->parentWidget(), SIGNAL(createProjectTriggered()), this, SLOT(createProject()));
        QObject::connect(this->_openProjectButton, &QPushButton::clicked, this, &ProjectWidget::openProject);
        QObject::connect(this->_selectFileButton, &QPushButton::clicked, this, &ProjectWidget::selectFile);
//        QObject::connect(this->parentWidget(), SIGNAL(selectFilesTriggered()), this, SLOT(selectFile()));
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

        this->_projectFolderName = dialog.getExistingDirectory(this, tr("Set Project Directory"),
                QCoreApplication::applicationDirPath(), QFileDialog::DontUseNativeDialog);

        std::cout << "Project dir: " << this->_projectFolderName.toStdString() << std::endl;

        // create file for saving which WSIs exist in folder
        QString projectFileName = "/project.txt";
        QFile file(this->_projectFolderName + projectFileName);
        if (file.open(QIODevice::ReadWrite)) {
            QTextStream stream(&file);
        }

        // now create folders for saving results and such (prompt warning if name already exists)
        QDir().mkdir(this->_projectFolderName + QString::fromStdString("/results"));
        QDir().mkdir(this->_projectFolderName + QString::fromStdString("/pipelines"));
        QDir().mkdir(this->_projectFolderName + QString::fromStdString("/thumbnails"));

        // check if any WSIs have been selected previously, and ask if you want to make a project and add these,
        // or make a new fresh one -> if no, need to clear all WSIs in the QListWidget
        if (not DataManager::GetInstance()->isEmpty()) {
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
                    DataManager::GetInstance()->doEmpty();
                    emit resetDisplay();
                    break;
                default:
                    break;
            }
        }
    }

    void ProjectWidget::saveProject() {
        // create file for saving which WSIs exist in folder
        QString projectFileName = "/project.txt";
        QFile file(_projectFolderName + projectFileName);
        file.resize(0);  // clear it and then write

        if (file.open(QIODevice::ReadWrite))
        {
            auto opened_images_fns = DataManager::GetInstance()->getProjectFilenames();
            foreach(std::string currPath, opened_images_fns) {
                    QTextStream stream(&file);
                    stream << currPath.c_str() << endl;
            }
        }
    }

    void ProjectWidget::openProject() {
        // If existing data, asks for confirmation to delete.
        if (not DataManager::GetInstance()->isEmpty()) {
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
                    DataManager::GetInstance()->doEmpty();
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

        // check if all relevant files and folders are in selected folder directory
        // qDebug << "hallo";
        // if any of the folders does not exists, create them
        if (!QDir(_projectFolderName + "/pipelines").exists()) {
            QDir().mkdir(_projectFolderName + "/pipelines");
        }
        if (!QDir(_projectFolderName + "/results").exists()) {
            QDir().mkdir(_projectFolderName + "/results");
        }
        if (!QDir(_projectFolderName + "/thumbnails").exists()) {
            QDir().mkdir(_projectFolderName + "/thumbnails");
        }

        // now, parse project.txt-file and add if there are any WSIs in the project
        // create file for saving which WSIs exist in folder
        QList<QString> fileNames;
        QString projectFileName = "project.txt";
        QFile file(_projectFolderName + projectFileName);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                fileNames.push_back(line);
            }
        }

        if (fileNames.empty()) {
            // prompt if no valid WSI was found in project-file
//            simpleInfoPrompt("There was found no valid WSI in the project file.");
            std::cout<<"There was found no valid WSI in the project file.\n";
        }

        auto progDialog = QProgressDialog(this);
        progDialog.setRange(0, fileNames.count()-1);
        //progDialog.setContentsMargins(0, 0, 0, 0);
        progDialog.setVisible(true);
        progDialog.setModal(false);
        progDialog.setLabelText("Loading WSIs...");
        //QRect screenrect = mWidget->screen()[0].geometry();
        progDialog.move(this->width() - progDialog.width() * 1.1, progDialog.height() * 0.1);
        progDialog.show();

        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);

        for (QString &fileName : fileNames) {
            if (fileName == "")
                return;
            auto currFileName = fileName.toStdString();
            std::cout << "Selected file: " << currFileName << std::endl;
            const std::string id_name = DataManager::GetInstance()->IncludeImage(currFileName);

            auto button = new ProjectThumbnailPushButton(id_name, this);
            int width_val = 100;
            int height_val = 150;
            auto listItem = new QListWidgetItem;
            listItem->setSizeHint(QSize(width_val, height_val));
            QObject::connect(button, &ProjectThumbnailPushButton::clicked, this, &ProjectWidget::selectNewDisplay);
            QObject::connect(button, &ProjectThumbnailPushButton::rightClicked, this, &ProjectWidget::removeImage);
            _wsi_scroll_listwidget->addItem(listItem);
            _wsi_scroll_listwidget->setItemWidget(listItem, button);
            _wsi_thumbnails_listitem[id_name] = listItem;
            _thumbnail_qpushbutton_map[id_name] = button;
            QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
        }
    }

    void ProjectWidget::selectFile() {
//        std::cout<<"Plop"<<std::endl;
        // check if view object list is empty, if not, prompt to save results or not, if not clear
//        if (pageComboBox->count() > 1) {
//            // prompt
//            QMessageBox mBox;
//            mBox.setIcon(QMessageBox::Warning);
//            mBox.setStyleSheet(mWidget->styleSheet());
//            mBox.setText("There are unsaved results.");
//            mBox.setInformativeText("Do you wish to save them?");
//            mBox.setDefaultButton(QMessageBox::Save);
//            mBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
//            int ret = mBox.exec();
//
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
//                    std::cout << "Cancelled!"  << std::endl;
//                    return;
//                default:
//                    // should never be reached
//                    break;
//            }
//        }

        // TODO: Unable to read .zvi and .scn (Zeiss and Leica). I'm wondering if they are stored in some unexpected way (not image pyramids)
        auto fileNames = QFileDialog::getOpenFileNames(this, tr("Select File(s)"), nullptr,
                                                       tr("WSI Files (*.tiff *.tif *.svs *.ndpi *.bif *vms)"), //*.zvi *.scn)"),
                nullptr, QFileDialog::DontUseNativeDialog);

        // return if the file dialog was cancelled without any files being selected
        if (fileNames.count() == 0) {
            return;
        }

//        // for a new selection of wsi(s), should reset and update these QWidgets
//        pageComboBox->clear();
//        exportComboBox->clear();

        auto progDialog = QProgressDialog(this);
        progDialog.setRange(0, fileNames.count()-1);
        //progDialog.setContentsMargins(0, 0, 0, 0);
        progDialog.setVisible(true);
        progDialog.setModal(false);
        progDialog.setLabelText("Loading WSIs...");
        //QRect screenrect = mWidget->screen()[0].geometry();
        progDialog.move(this->width() - progDialog.width() * 1.1, progDialog.height() * 0.1);
        progDialog.show();

        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
        auto currentPosition = 0; // curr_pos <=?

        // need to handle scenario where a WSI is added, but there already exists N WSIs from before
        auto nb_wsis_in_list = 0; //wsiList.size();
        if (nb_wsis_in_list != 0)
            currentPosition = nb_wsis_in_list;

        int counter = 0;
        for (QString& fileName : fileNames) {
            if (fileName == "")
                return;
            //filename = fileName.toStdString();
            auto currFileName = fileName.toStdString();
            std::cout << "Selected file: " << currFileName << std::endl;
            const std::string id_name = DataManager::GetInstance()->IncludeImage(currFileName);

            // @TODO. Shouldn't we keep a class attribute list with those buttons, so that it's easier to retrieve them
            // and delete them on the fly?
            auto button = new ProjectThumbnailPushButton(id_name, this);
//            auto button = new QPushButton(this);
//            auto thumbnail_image = DataManager::GetInstance()->get_image("0")->get_thumbnail();
//            auto m_NewPixMap = QPixmap::fromImage(thumbnail_image);
//            QIcon ButtonIcon(m_NewPixMap);
//            button->setIcon(ButtonIcon);
//            int width_val = 100;
//            int height_val = 150;
//            button->setIconSize(QSize((int) std::round(0.9 * (float) thumbnail_image.width() * (float) height_val /
//            (float) thumbnail_image.height()), (int) std::round(0.9f * (float) height_val)));
//            button->setToolTip(QString::fromStdString(splitCustom(currFileName, "/").back()));

            _thumbnail_qpushbutton_map[id_name] = button;
            int width_val = 100;
            int height_val = 150;
            auto listItem = new QListWidgetItem;
            listItem->setSizeHint(QSize(width_val, height_val));
            QObject::connect(button, &ProjectThumbnailPushButton::clicked, this, &ProjectWidget::selectNewDisplay);
            QObject::connect(button, &ProjectThumbnailPushButton::rightClicked, this, &ProjectWidget::removeImage);
            _wsi_scroll_listwidget->addItem(listItem);
            _wsi_scroll_listwidget->setItemWidget(listItem, button);
            _wsi_thumbnails_listitem[id_name] = listItem;

//            emit newImageFilename(id_name);
            // to render straight away (avoid waiting on all WSIs to be handled before rendering)
            QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
        }
    }

    void ProjectWidget::removeImage(std::string uid)
    {
        std::cout<<"Deleting current object\n";
        _wsi_scroll_listwidget->removeItemWidget(_wsi_thumbnails_listitem[uid]);
        _wsi_thumbnails_listitem.erase(uid);
        _thumbnail_qpushbutton_map.erase(uid);
        _wsi_scroll_listwidget->update();
        emit newImageDisplay(uid, false);
        DataManager::GetInstance()->RemoveImage(uid); //@TODO. To check, might be some memory leak here again.
        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
    }

    void ProjectWidget::selectNewDisplay(std::string id_name, bool state)
    {
        if(state)
        {
            for(auto it = _thumbnail_qpushbutton_map.begin(); it != _thumbnail_qpushbutton_map.end(); it++)
            {
                if(it->first != id_name)
                    it->second->setCheckedState(false);
            }
        }
        emit newImageDisplay(id_name, state);
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
        progDialog->move(this->width() - progDialog->width() * 1.1, progDialog->height() * 0.1);
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

    void ProjectWidget::selectFileDrag(const QList<QString> &fileNames) {

        // check if view object list is empty, if not, prompt to save results or not, if not clear
        if (!DataManager::GetInstance()->isEmpty()) {
            QMessageBox mBox;
            mBox.setIcon(QMessageBox::Warning);
            mBox.setStyleSheet(this->styleSheet());
            mBox.setText("There are unsaved results.");
            mBox.setInformativeText("Do you wish to save them?");
            mBox.setDefaultButton(QMessageBox::Save);
            mBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            int ret = mBox.exec();

            switch (ret) {
                case QMessageBox::Save:
                    std::cout << "Results not saved yet. Just cancelled the switch!" << std::endl;
                    // Save was clicked
                    return;
                case QMessageBox::Discard:
                    // Don't Save was clicked
                    std::cout << "Discarded!" << std::endl;
                    break;
                case QMessageBox::Cancel:
                    // Cancel was clicked
                    std::cout << "Cancelled!" << std::endl;
                    return;
                default:
                    // should never be reached
                    break;
            }
        }
        /*
        if (pageComboBox->count() != 0) { // if not empty, clear
            pageComboBox->clear();
            exportComboBox->clear();
        }
         */


        this->resetInterface();

        auto progDialog = QProgressDialog(this);
        progDialog.setRange(0, fileNames.count()-1);
        progDialog.setVisible(true);
        progDialog.setModal(false);
        progDialog.setLabelText("Loading WSIs...");
        QRect screenrect = this->screen()[0].geometry();
        progDialog.move(this->width() - progDialog.width() / 2, - this->width() / 2 - progDialog.width() / 2);
        progDialog.show();

        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
        // @TODO. Below is the same process as the selectFile, should make another submethod for both cases.

//        int counter = 0;
//        for (const QString& fileName : fileNames) {

//            if (fileName == "")
//                return;
//            //filename = fileName.toStdString();
//            auto currFileName = fileName.toStdString();
//            std::cout << "Selected file: " << currFileName << std::endl;
//            wsiList.push_back(currFileName);

//            // Import image from file using the ImageFileImporter
//            auto importer = WholeSlideImageImporter::New();
//            importer->setFilename(currFileName);
//            auto currImage = importer->updateAndGetOutputData<ImagePyramid>();

//            // for reading of multiple WSIs, only render last one
//            if (counter == 0) { //fileNames.count()-1) {

//                // current WSI (global)
//                filename = currFileName;

//                m_image = currImage;
//                std::cout << "count:" << counter << std::endl;
//                metadata = m_image->getMetadata(); // get metadata

//                auto renderer = ImagePyramidRenderer::New();
//                renderer->setSharpening(m_wsiSharpening);
//                renderer->setInputData(m_image);

//                // TODO: Something here results in me not being able to run analysis on new images (after the first)
//                removeAllRenderers();
//                m_rendererTypeList["WSI"] = "ImagePyramidRenderer";
//                insertRenderer("WSI", renderer);
//                getView(0)->reinitialize(); // Must call this after removing all renderers

//                wsiFormat = metadata["openslide.vendor"]; // get WSI format
//                magn_lvl = getMagnificationLevel(); // get magnification level of current WSI

//                // now make it possible to edit image in the View Widget
//                createDynamicViewWidget("WSI", modelName);

//                // update application name to contain current WSI
//                //setTitle(applicationName + " - " + splitCustom(filename, "/").back());
//                if (advancedMode) {
//                    setTitle(applicationName + " (Research mode)" + " - " + splitCustom(currFileName, "/").back());
//                } else {
//                    setTitle(applicationName + " - " + splitCustom(currFileName, "/").back());
//                }
//            }
//            counter ++;

//            // TODO: This is a little bit slow. Possible to speed it up? Bottleneck is probably the creation of thumbnails
//            auto access = currImage->getAccess(ACCESS_READ);
//            auto input = access->getLevelAsImage(currImage->getNrOfLevels() - 1);

//            // try to convert to FAST Image -> QImage
//            QImage image(input->getWidth(), input->getHeight(), QImage::Format_RGB32);

//            // TODO have to do some type conversion here, assuming float for now
//            unsigned char *pixelData = image.bits();

//            ImageAccess::pointer new_access = input->getImageAccess(ACCESS_READ);
//            void *inputData = new_access->get();
//            uint nrOfComponents = input->getNrOfChannels();

//            for (uint x = 0; x < (uint)input->getWidth(); x++) {
//                for (uint y = 0; y < (uint)input->getHeight(); y++) {
//                    uint i = x + y * input->getWidth();
//                    for (uint c = 0; c < (uint)input->getNrOfChannels(); c++) {
//                        float data;
//                        data = ((uchar *) inputData)[i * nrOfComponents + c]; // assumes TYPE_UINT8
//                        pixelData[i * 4 + (2-c)] = (unsigned char) data;  // TODO: NOTE (2-c) fixed BGR->RGB, but maybe there is a smarter solution?
//                        pixelData[i * 4 + 3] = 255; // Alpha
//                    }
//                }
//            }

//            auto button = new QPushButton(mWidget);
//            auto m_NewPixMap = QPixmap::fromImage(image);
//            QIcon ButtonIcon(m_NewPixMap);
//            button->setIcon(ButtonIcon);
//            int width_val = 100;
//            int height_val = 150;
//            button->setIconSize(QSize((int) std::round(0.9 * (float) image.width() * (float) height_val / (float) image.height()), (int) std::round(0.9f * (float) height_val)));

//            auto listItem = new QListWidgetItem;
//            listItem->setSizeHint(QSize(width_val, height_val));
//            QObject::connect(button, &QPushButton::clicked, std::bind(&MainWindow::selectFileInProject, this, curr_pos));
//            scrollList->addItem(listItem);
//            scrollList->setItemWidget(listItem, button);

//            curr_pos++; // this should change if we render the first WSI when loading

//            progDialog.setValue(counter);
//            QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
//        }
    }


} // End of namespace fast
