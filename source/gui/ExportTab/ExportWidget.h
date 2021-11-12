//
// Created by dbouget on 12.11.2021.
//

#ifndef FASTPATHOLOGY_EXPORTWIDGET_H
#define FASTPATHOLOGY_EXPORTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>

namespace fast
{
    class ExportWidget: public QWidget
    {
        Q_OBJECT
        public:
            ExportWidget(QWidget* parent=0);
            ~ExportWidget();
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

        private:
            QVBoxLayout* _main_layout; /* */
    };
} // End of namespace fast
#endif //FASTPATHOLOGY_EXPORTWIDGET_H
