//
// Created by dbouget on 02.11.2021.
//

#include "ProcessManager.h"
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <source/utils/utilities.h>

namespace fast {

/**
 * Static methods should be defined outside the class.
 */

    ProcessManager* ProcessManager::_pinstance{nullptr};
    std::mutex ProcessManager::_mutex;

/**
 * The first time we call GetInstance we will lock the storage location
 *      and then we make sure again that the variable is null and then we
 *      set the value. RU:
 */
    ProcessManager *ProcessManager::GetInstance()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_pinstance == nullptr)
        {
            _pinstance = new ProcessManager();
        }
        return _pinstance;
    }

    void ProcessManager::set_advanced_mode_status(bool status)
    {
        this->_advanced_mode = status;
    }

} // End of namespace fast
