//
// Created by dbouget on 08.10.2021.
//

#ifndef FASTPATHOLOGY_PROJECTTHUMBNAILPUSHBUTTON_H
#define FASTPATHOLOGY_PROJECTTHUMBNAILPUSHBUTTON_H

#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>
#include <cmath>
#include "source/logic/DataManager.h"

/** @TODO. The button should be clickable, and the image is displayed only when button is clicked.
 * For the right click event, it can't occur when button is clicked, to delete the current widget.*/
namespace fast {
    class ProjectThumbnailPushButton: public QPushButton {
        Q_OBJECT
        public:
            /**
             * @brief Custom QPushButton holding a thumbnail image.
             * @param name Unique identifier for the current widget.
             * @param parent QWidget used as parent.
             */
            ProjectThumbnailPushButton(const std::string name, QWidget *parent=0);
            ~ProjectThumbnailPushButton();

            inline const std::string getName() const {return _name;}
            inline void setCheckedState(bool state){_checked = state;}

        private:
            void SetupInterface();

        public slots:
            void custom_clicked();
            void right_clicked();

        private slots:
            void mousePressEvent(QMouseEvent *e);

        signals:
            void clicked(std::string, bool state);
            void rightClicked(std::string);

        private:
            const std::string _name;
            bool _checked;
    };
}

#endif //FASTPATHOLOGY_PROJECTTHUMBNAILPUSHBUTTON_H
