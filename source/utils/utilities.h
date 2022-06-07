#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <time.h>
#include <QElapsedTimer>
#include <QStandardPaths>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <FAST/Utility.hpp>
#include <QProgressDialog>
#include <QEventLoop>


namespace fast {
    /**
     * Creates a string of random numbers of length n.
     * @param n
     * @return
     */
    static std::string createRandomNumbers_(int n) {
        srand(time(NULL));
        std::string out;
        for (int i = 0; i < n; i++) {
            out.append(std::to_string(rand() % 10));
        }
        return out;
    }

    // for string delimiter
    static std::vector<std::string> splitCustom(const std::string& s, const std::string& delimiter) {
        size_t pos_start = 0, pos_end, delim_len = delimiter.length();
        std::string token;
        std::vector<std::string> res;

        while ((pos_end = s.find (delimiter, pos_start)) != std::string::npos) {
            token = s.substr (pos_start, pos_end - pos_start);
            pos_start = pos_end + delim_len;
            res.push_back (token);
        }

        res.push_back (s.substr (pos_start));
        return res;
    }



    static void downloadZipFile(std::string URL, std::string destination, std::string title) {
        createDirectories(destination);
        std::cout << "Progress: " << std::endl;
        QNetworkAccessManager manager;
        QUrl url(QString::fromStdString(URL));
        QNetworkRequest request(url);
        auto timer = new QElapsedTimer;
        timer->start();
        auto reply = manager.get(request);
        int step = 5;
        int progress = step;
        auto progressDialog = new QProgressDialog("Downloading " + QString::fromStdString(title) +", please wait..", "Stop", 0, 101);
        progressDialog->setModal(Qt::ApplicationModal);
        QObject::connect(progressDialog, &QProgressDialog::canceled, reply, &QNetworkReply::abort);
        progressDialog->setWindowTitle("Dowmloading " + QString::fromStdString(title));
        progressDialog->setAutoClose(true);
        progressDialog->show();
        QObject::connect(reply, &QNetworkReply::downloadProgress, [&](quint64 current, quint64 max) {
            int percent = ((float)current / max) * 100;
            float speed = ((float)timer->elapsed() / 1000.0f)/percent;
            uint64_t remaining = speed * (100 - percent);
            if(percent >= progress) {
                std::cout << percent << "% - ETA ~" << (int)std::ceil((float)remaining / 60) << " minutes. " << std::endl;;
                progress += step;
            }
            progressDialog->setValue(percent);
        });
        auto tempLocation = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/data.zip";
        QFile file(tempLocation);
        if(!file.open(QIODevice::WriteOnly)) {
            throw Exception("Could not write to " + tempLocation.toStdString());
        }
        QObject::connect(reply, &QNetworkReply::readyRead, [&reply, &file]() {
            file.write(reply->read(reply->bytesAvailable()));
        });
        QObject::connect(&manager, &QNetworkAccessManager::finished, [&]() {
            std::cout << "Finished downloading file. Processing.." << std::endl;
            file.close();
            std::cout << "Unzipping the data file to: " << destination << std::endl;
            try {
                extractZipFile(file.fileName().toStdString(), destination);
            } catch(Exception & e) {
                std::cout << "ERROR: Zip extraction failed." << std::endl;
            }

            file.remove();
            std::cout << "Done." << std::endl;
            progressDialog->setValue(101);
        });

        auto eventLoop = new QEventLoop(&manager);

        // Make sure to quit the event loop when download is finished
        QObject::connect(&manager, &QNetworkAccessManager::finished, eventLoop, &QEventLoop::quit);

        // Wait for it to finish
        eventLoop->exec();
    }
}
