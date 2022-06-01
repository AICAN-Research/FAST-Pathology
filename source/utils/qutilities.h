#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <QString>
#include <QTimer>
#include <QMessageBox>
#include <QWidget>
#include <QRect>
#include <QScreen>

namespace fast {
    /**
     * Shows simple QMessageBox informing the usage about something.
     * @param str Message to display
     * @param parent Parent widget providing the Qt context
     */
    static void simpleInfoPrompt(const QString& str, QWidget *parent=0) {
        auto mBox = new QMessageBox(parent);
        mBox->setText(str);
        mBox->setIcon(QMessageBox::Information);
        mBox->setModal(false);
        QRect screenrect = parent->screen()[0].geometry();
        mBox->move(parent->width() - mBox->width() / 2, -parent->width() / 2 - mBox->width() / 2);
        mBox->show();
        QTimer::singleShot(3000, mBox, SLOT(accept()));
    }

    static void clearLayout(QLayout *layout) {
        QLayoutItem *item;
        while((item = layout->takeAt(0))) {
            if (item->layout()) {
                clearLayout(item->layout());
                delete item->layout();
            }
            if (item->widget()) {
                delete item->widget();
            }
            delete item;
        }
    }
}