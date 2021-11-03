//
// Created by dbouget on 03.11.2021.
//

#include "StatsWidget.h"

#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Data/ImagePyramid.hpp>

namespace fast {
    StatsWidget::StatsWidget(QWidget *parent): QWidget(parent){
    }

    StatsWidget::~StatsWidget(){

    }

    void StatsWidget::setupInterface()
    {
        this->_main_layout = new QVBoxLayout(this);
        this->_main_layout->setAlignment(Qt::AlignTop);

        // make button that prints distribution of pixels of each class -> for histogram
        this->_calc_hist_pushbutton = new QPushButton(this);
        this->_calc_hist_pushbutton->setText("Calculate tissue histogram");
        this->_calc_hist_pushbutton->setFixedHeight(50);
        this->_main_layout->insertWidget(0, this->_calc_hist_pushbutton);
    }

    void StatsWidget::resetInterface()
    {
        return;
    }

    void StatsWidget::setupConnections()
    {
        QObject::connect(this->_calc_hist_pushbutton, &QPushButton::clicked, this, &StatsWidget::calcTissueHist);
    }

    const bool StatsWidget::calcTissueHist() {
        std::cout << "Calculating histogram..."<<std::endl;

        int barWidth = 9;
        int boxWidth = 250;
        int boxHeight = 250;
        QPixmap pm(boxWidth, boxHeight);
        pm.fill();

        int len = 5;

        //int x_vec[] = {1, 2, 3, 4, 5};
        std::string x_vec[] = {"a", "b", "c", "d", "e"};
        int y_vec[] = {12, 20, 43, 30, 5};

        double drawMinHeight = 0.1 * (double)(boxHeight);
        double drawMinWidth = 0.1 * (double)(boxWidth);

        double newWidth = (double)(boxWidth - drawMinWidth);
        double newHeight = (double)(boxHeight - drawMinHeight);

        double maxHeight = *std::max_element(y_vec,y_vec+len);
        double drawMaxHeight = (double)(maxHeight + maxHeight*0.1);

        auto painters = new QPainter(&pm);
        painters->setPen(QColor(140, 140, 210));

        for (int i = 0; i < len; i++) {
            std::cout << std::to_string(i) << std::endl;
            qreal h = y_vec[i] * maxHeight;
            // draw level
            painters->fillRect(drawMinWidth / 2 + (double)(i + 1) * (double)(newWidth) / (double)(len + 1) - (double)((double)(barWidth)/(double)(2)),
                               newHeight,
                               barWidth,
                               - (double)(y_vec[i]) / (double)(drawMaxHeight) * (double)(newHeight),
                               Qt::blue);
            // clear the rest of the control
            //painters->fillRect(barWidth * i, 0, barWidth * (i + 1), maxHeight - h, Qt::black);
        }

        int lineWidth = 3;
        int space = drawMinHeight;

        //auto linePainter = new QPainter(&pm);
        painters->setPen(QPen(QColor(0, 0, 0), lineWidth));
        painters->drawLine(space, boxHeight - space, boxWidth - space, boxHeight - space);
        painters->drawLine(space - 1, boxHeight - space, space - 1, space);

        // add ticks on lines
        painters->setPen(QPen(QColor(0, 0, 0), 2));
        painters->setFont(QFont("times", 10));
        std::string stringIter;
        int xTextSpace = 18;
        int numTicks = 5;
        double tickSize = 10;
        for (int j = 0; j < numTicks; j++) {
            painters->drawLine(drawMinWidth / 2 + (double)(j + 1) * (double)(newWidth) / (double)(len + 1),
                               newHeight,
                               drawMinWidth / 2 + (double)(j + 1) * (double)(newWidth) / (double)(len + 1),
                               newHeight + tickSize / 2);
            painters->drawText(drawMinWidth / 2 + (double)(j + 1) * (double)(newWidth) / (double)(len + 1) - 4, newHeight + xTextSpace, QString::number(j + 1));

        }

        int yTextSpace = 22;
        for (int j = 0; j < numTicks; j++) {
            painters->drawLine(space - tickSize / 2,
                               drawMinHeight - drawMinHeight*0.5 + (double)(j + 1) * (double)(newHeight) / (double)(len + 1),
                               space,
                               drawMinHeight - drawMinHeight*0.5 + (double)(j + 1) * (double)(newHeight) / (double)(len + 1));

            painters->drawText(drawMinWidth - yTextSpace, drawMinHeight - drawMinHeight*0.5 + (double)(j + 1) * (double)(newHeight) / (double)(len + 1) + 4,
                               QString::number((int)(std::round((double)(numTicks - j) * (double)(maxHeight) / (double)(len + 1) + 4))));
            //painters->drawText(drawMinWidth - yTextSpace, drawMinHeight - drawMinHeight*0.5 + (double)(j + 1) * (double)(newHeight) / (double)(len + 1) + 4,
            //        QString::number((int)(y_vec[numTicks - j - 1])));

        }

        // draw arrow heads on end of axes
        int xArrowSize = 8;
        int yArrowSize = 16;
        QRectF rect = QRectF(space - xArrowSize + lineWidth, space - yArrowSize, xArrowSize, yArrowSize);

        QPainterPath path;
        path.moveTo(rect.left() + (rect.width() / 2), rect.top());
        path.lineTo(rect.bottomLeft());
        path.lineTo(rect.bottomRight());
        path.lineTo(rect.left() + (rect.width() / 2), rect.top());

        painters->fillPath(path, QBrush(QColor ("black")));

        QTransform t;
        t.translate(boxWidth, boxHeight - space - yArrowSize - xArrowSize);
        t.rotate(90);
        QPainterPath path2 = t.map(path);

        // draw title of histogram
        painters->setPen(QPen(QColor(0, 0, 0), 2));
        painters->setFont(QFont("times", 10));

        painters->fillPath(path2, QBrush(QColor ("black")));

        auto histBox = new QLabel;
        histBox->setPixmap(pm);
        histBox->setAlignment(Qt::AlignHCenter);

        this->_main_layout->insertWidget(2, histBox); // finally, add histogram to Widget

        // add some text box that explains in words the result from the analysis or something similar...
        auto smallTextWindowStats = new QTextEdit;
        smallTextWindowStats->setPlainText(tr("This is some window with text that displays some relevant "
                                              "information regarding the inference or analysis you just did."));
        smallTextWindowStats->setReadOnly(true);
        smallTextWindowStats->show();
        smallTextWindowStats->setFixedHeight(smallTextWindowStats->document()->size().toSize().height() + 3);

        this->_main_layout->insertWidget(1, smallTextWindowStats);

        return true;
    }


} // End of namespace fast
