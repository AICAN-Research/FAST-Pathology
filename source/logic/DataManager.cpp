//
// Created by dbouget on 07.10.2021.
//

#include "DataManager.h"
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <source/utils/utilities.h>

namespace fast {

/**
 * Static methods should be defined outside the class.
 */

    DataManager* DataManager::_pinstance{nullptr};
    std::mutex DataManager::_mutex;

    DataManager::DataManager()
    {
        this->_visible_wsi_uid = "";
//        this->_project = std::make_unique<Project>();
        this->_project = std::make_shared<Project>();
    }

    DataManager::~DataManager()
    {
        // @TODO. The destructor is actually never called, might have to trigger the call to deletion from the closeEvent in MainWindow.
    }

    /**
     * The first time we call GetInstance we will lock the storage location
     *      and then we make sure again that the variable is null and then we
     *      set the value. RU:
     */
    DataManager *DataManager::GetInstance()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_pinstance == nullptr)
        {
            _pinstance = new DataManager();
        }
        return _pinstance;
    }

    void DataManager::doEmpty()
    {
        this->_project->emptyProject();
        _visible_wsi_uid = "";
    }

    std::shared_ptr<WholeSlideImage> DataManager::get_visible_image()
    {
        if(this->_visible_wsi_uid != "")
            return this->_project->getImage(this->_visible_wsi_uid);
        else
            return NULL;
    }

} //End of namespace fast
