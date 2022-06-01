#include "ProjectThumbnailPushButton.h"
#include "source/gui/MainWindow.hpp"

namespace fast{
    ProjectThumbnailPushButton::ProjectThumbnailPushButton(MainWindow* mainWindow, std::string name, QWidget *parent):_name(name), QPushButton(parent)
    {
        m_mainWindow = mainWindow;
        this->SetupInterface();
        _checked = false;
        /*QObject::connect(this, &QPushButton::clicked, this, &ProjectThumbnailPushButton::custom_clicked);
        QObject::connect(this, &ProjectThumbnailPushButton::rightClicked, this, &ProjectThumbnailPushButton::right_clicked);*/
    }

    ProjectThumbnailPushButton::~ProjectThumbnailPushButton()
    {

    }

    void ProjectThumbnailPushButton::SetupInterface()
    {
        auto thumbnail_image = m_mainWindow->getCurrentProject()->getImage(_name)->get_thumbnail();
        auto m_NewPixMap = QPixmap::fromImage(thumbnail_image);
        QIcon ButtonIcon(m_NewPixMap);
        this->setIcon(ButtonIcon);
        int width_val = 100;
        int height_val = 150;
        this->setIconSize(QSize((int) std::round(0.9 * (float) thumbnail_image.width() * (float) height_val /
                                                   (float) thumbnail_image.height()), (int) std::round(0.9f * (float) height_val)));
        this->setToolTip(QString::fromStdString(_name));
    }

    void ProjectThumbnailPushButton::custom_clicked()
    {
        emit clicked(_name, _checked);
    }

    void ProjectThumbnailPushButton::right_clicked() {
        _checked = !_checked;
//            this->setChecked(_checked);
//            this->setDown(_checked);
//            custom_clicked();
        emit clicked(_name, _checked);
    }

    // @TODO. Redundant with the default QPushButton QConnect redirection, will have to remove the other.
    void ProjectThumbnailPushButton::mousePressEvent(QMouseEvent *e)
    {
        if(e->button()==Qt::RightButton) {
            if (_checked)
                emit rightClicked(_name);
        }
        else if(e->button()==Qt::LeftButton) {
            _checked = !_checked;
//            this->setChecked(_checked);
//            this->setDown(_checked);
//            custom_clicked();
            emit clicked(_name, _checked);
        }
    }
}
