#pragma once

#include <iostream>
#include <string>
#include <QString>
#include <QTemporaryDir>
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include "source/utils/utilities.h"
#include "source/logic/WholeSlideImage.h"

namespace fast{
    class DataObject;
    class Pipeline;
    class Renderer;

    class Result {
        public:
            std::string name;
            std::string pipelineName;
            std::string WSI_uid;
            std::vector<std::string> classNames;
            std::shared_ptr<Renderer> renderer;
    };

    class Project {
        public:
            Project(std::string name, bool open = false);
            ~Project();

            const std::string getRootFolder(){return this->_root_folder;}
            bool isProjectEmpty() const{return _images.empty();}
            int getWSICountInProject() const{return this->_images.size();}
            std::vector<std::string> getAllWsiUids() const;
            std::shared_ptr<WholeSlideImage> getImage(const std::string& name);
            std::shared_ptr<WholeSlideImage> getImage(int i);
            std::string getName() const { return m_name; };

            void emptyProject();
            void saveResults(const std::string& wsi_uid, std::shared_ptr<Pipeline> pipeline, std::map<std::string, std::shared_ptr<DataObject>> data);

            std::vector<Result> loadResults(const std::string& wsi_uid);

            /**
             * @brief includeImage Include image to the current project.
             * @param image_filepath Disk location of the WSI to include.
             * @return
             */
            const std::string includeImage(const std::string& image_filepath);
            /**
             * @brief includeImageFromProject Reload a WSI from a previously saved project.
             * @param uid_name Unique identifier for the WSI.
             * @param image_filepath Disk location of the WSI.
             */
            void includeImageFromProject(const std::string& uid_name, const std::string& image_filepath);
            /**
             * @brief removeImage Remove a WSI from the current project.
             * @param uid Unique identifier for the WSI to remove.
             */
            void removeImage(const std::string& uid);

            void writeTimestmap();
       protected:
            /**
             * @brief createFolderDirectoryArchitecture Prepare the folder structure with sub-folders
             * such as thumbnails or results.
             */
            void createFolderDirectoryArchitecture();
            /**
             * @brief saveThumbnails Iteratively saving on disk the thumbnail of each opened WSI.
             */
            void saveThumbnails();
            /**
             * @brief saveThumbnail Saving only the thumbnail of a specific WSI.
             * @param wsi_uid unique id of the WSI whose thumbnail should be saved.
             */
            void saveThumbnail(const std::string& wsi_uid);
       private:
            std::string m_name;
            std::string _root_folder;  /* Location on disk where to save all data for the current project. */
            std::map<std::string, std::shared_ptr<WholeSlideImage>> _images; /* Loaded image objects. */
    };
} // End of namespace fast
