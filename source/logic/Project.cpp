//
// Created by dbouget on 09.11.2021.
//

#include "Project.h"
#include <FAST/Reporter.hpp>
#include <FAST/Utility.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Exporters/ImageExporter.hpp>

namespace fast{
    Project::Project()
    {
        // Default folder root from Qt temporary dir, automatically deleted.
        auto default_dir = QTemporaryDir();
        this->_root_folder = QDir::tempPath().toStdString() + "/project_" + createRandomNumbers_(8);
        QDir().mkpath(QString::fromStdString(this->_root_folder));
        Reporter::info()<<"Temporary project folder is set to: "<<this->_root_folder<<Reporter::end();
        this->_temporary_dir_flag = true;
        this->createFolderDirectoryArchitecture();
    }

    Project::~Project()
    {
        std::cout<<"Deleting temporary project folder: "<<this->_root_folder<<std::endl;
        if(this->_temporary_dir_flag)
            QDir().rmdir(QString::fromStdString(this->_root_folder));
    }

    void Project::createFolderDirectoryArchitecture()
    {
        // check if all relevant files and folders are in selected folder directory
        // if any of the folders does not exists, create them
        if (!QDir(QString::fromStdString(this->_root_folder + "/pipelines")).exists())
        {
            QDir().mkdir(QString::fromStdString(this->_root_folder + "/pipelines"));
        }
        if (!QDir(QString::fromStdString(this->_root_folder + "/results")).exists())
        {
            QDir().mkdir(QString::fromStdString(this->_root_folder + "/results"));
        }
        if (!QDir(QString::fromStdString(this->_root_folder + "/thumbnails")).exists())
        {
            QDir().mkdir(QString::fromStdString(this->_root_folder + "/thumbnails"));
        }
    }

    std::shared_ptr<WholeSlideImage> Project::getImage(const std::string& name)
    {
        if (this->_images.find(name) != this->_images.end())
            return this->_images[name];
        // else?
    }

    std::vector<std::string> Project::getAllWsiUids() const
    {
        std::vector<std::string> uids;
        for(auto it = this->_images.begin(); it != this->_images.end(); ++it) {
          uids.push_back(it->first);
        }

        return uids;
    }

    void Project::setRootFolder(const std::string& root_folder)
    {
        this->_root_folder = root_folder;
        this->_temporary_dir_flag = false;
        this->createFolderDirectoryArchitecture();
    }

    void Project::emptyProject()
    {
        this->_images.clear();
    }

    const std::string Project::includeImage(const std::string& image_filepath)
    {
        auto image(std::make_shared<WholeSlideImage>(image_filepath));
        std::string img_name_short = splitCustom(splitCustom(image_filepath, "/").back(), ".").front();
        bool in_use = this->_images.find(img_name_short) != _images.end();
        if(in_use)
        {
            bool new_in_use = true;
            while(new_in_use)
            {
                int rand_int = rand() % 10000;
                std::string new_name = img_name_short + "#" + std::to_string(rand_int);
                if(this->_images.find(new_name) == this->_images.end())
                {
                    new_in_use = false;
                    img_name_short = new_name;
                }
            }
        }
        this->_images[img_name_short] = image;

        return img_name_short;
    }

    void Project::includeImageFromProject(const std::string& uid_name, const std::string& image_filepath)
    {
        std::string thumbnail_filename = this->_root_folder + "/thumbnails" + uid_name + ".png";
        if (QFile(QString::fromStdString(thumbnail_filename)).exists())
        {
            QImage thumbnail = QImage(QString::fromStdString(thumbnail_filename));
            auto image(std::make_shared<WholeSlideImage>(image_filepath, thumbnail));
            this->_images[uid_name] = image;
        }
        else
        {
            auto image(std::make_shared<WholeSlideImage>(image_filepath));
            this->_images[uid_name] = image;
        }
    }

    void Project::removeImage(const std::string& uid)
    {
        this->_images.erase(uid);
    }

    void Project::loadProject()
    {
        this->emptyProject();

        // Parse project.txt-file to identify all WSIs to be added to the project
        std::map<std::string, std::string> fileNames;
        std::string projectFileName = "project.txt";
        QFile file(QString::fromStdString(this->_root_folder + projectFileName));
        if (file.open(QIODevice::ReadOnly))
        {
            QTextStream in(&file);
            while (!in.atEnd())
            {
                QString line = in.readLine();
                std::string uid = splitCustom(line.toStdString(), ",").front();
                std::string filename = splitCustom(line.toStdString(), ",").back();

                fileNames[uid] = filename;
            }
        }

        // Create WSI instances for each file
        for (const auto wsi_pair : fileNames)
        {
            auto thumbnail_filename = this->_root_folder + "/thumnbails/" + wsi_pair.first + ".png";
            this->includeImageFromProject(wsi_pair.first, wsi_pair.second);
        }

        //@TODO. Should then attempt to load all results linked to each WSI
    }

    void Project::saveProject()
    {
        // create file for saving which WSIs exist in folder
        std::string overall_filename = this->_root_folder + "/project.txt";
        QFile file(QString::fromStdString(overall_filename));
        file.resize(0);  // clear it and then write

        if (file.open(QIODevice::ReadWrite))
        {
            foreach(const auto wsi_pair, this->_images)
            {
                    QTextStream stream(&file);
                    stream << (wsi_pair.first + "," + wsi_pair.second->get_filename()).c_str() << endl;
            }
        }
        this->saveThumbnails();
    }

    void Project::saveThumbnails()
    {
        for (const auto currWSI : this->_images)
        {
            QImage thumbnail = currWSI.second->get_thumbnail();
            std::string dump_filename = this->_root_folder + "/thumbnails/" + splitCustom(splitCustom(currWSI.second->get_filename(), "/").back(), ".")[0] + ".png";
            thumbnail.save(QString::fromStdString(dump_filename));
        }
    }

    void Project::saveThumbnail(const std::string& wsi_uid)
    {
        if (this->_images.find(wsi_uid) != this->_images.end())
        {
            QImage thumbnail = this->_images[wsi_uid]->get_thumbnail();
            std::string dump_filename = this->_root_folder + "/thumbnails/" + splitCustom(splitCustom(this->_images[wsi_uid]->get_filename(), "/").back(), ".")[0] + ".png";
            thumbnail.save(QString::fromStdString(dump_filename));
        }
        else
            std::cout<<"Requested saving thumbnail for WSI named: "<<wsi_uid<<", which is not in the project..."<<std::endl;
    }

    void Project::saveResults(const std::string& wsi_uid)
    {
    }

    std::shared_ptr<WholeSlideImage> Project::getImage(int i) {
        if(i >= _images.size())
            throw Exception("Out of bounds in Project::getImage");
        auto it = _images.begin();
        std::advance(it, i);
        return it->second;
    }

} // End of namespace fast
