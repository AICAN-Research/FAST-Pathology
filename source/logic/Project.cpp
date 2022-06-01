#include "Project.h"
#include <FAST/Reporter.hpp>
#include <FAST/Utility.hpp>
#include <FAST/Pipeline.hpp>
#include <FAST/Importers/TIFFImagePyramidImporter.hpp>
#include <FAST/Exporters/TIFFImagePyramidExporter.hpp>
#include <FAST/Importers/MetaImageImporter.hpp>
#include <FAST/Exporters/MetaImageExporter.hpp>
#include <FAST/Importers/HDF5TensorImporter.hpp>
#include <FAST/Exporters/HDF5TensorExporter.hpp>
#include <FAST/Visualization/Renderer.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/HeatmapRenderer/HeatmapRenderer.hpp>
#include <FAST/Visualization/View.hpp>

namespace fast{
    Project::Project(std::string name, bool open)
    {
        m_name = name;
        // Default folder root from Qt temporary dir, automatically deleted.
        this->_root_folder = QDir::home().path().toStdString() + "/fastpathology/projects/" + name + "/";
        if(open) {
            std::vector<std::string> lines;
            std::ifstream file(_root_folder + "project.txt");
            std::string line;
            while (std::getline(file, line)) {
                lines.push_back(line);
            }
            for(int i = 0; i < lines.size(); i += 2) {
                _images[lines[i]] = std::make_shared<WholeSlideImage>(lines[i+1]);
            }
        } else {
            this->createFolderDirectoryArchitecture();
        }
    }

    Project::~Project()
    {
    }

    void Project::writeTimestmap() {
        std::ofstream timestampFile(_root_folder + "timestamp.txt");
        timestampFile << currentDateTime();
        timestampFile.close();
    }

    void Project::createFolderDirectoryArchitecture()
    {
        QDir().mkdir(QString::fromStdString(this->_root_folder));
        std::ofstream file(_root_folder + "project.txt", std::ios::out);
        file << "";
        file.close();
        writeTimestmap();
        // check if all relevant files and folders are in selected folder directory
        // if any of the folders does not exists, create them
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

        std::ofstream file(_root_folder + "project.txt", std::ios::app);
        file << img_name_short << "\n";
        file << image_filepath << "\n";
        file.close();
        writeTimestmap();

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

        std::vector<std::string> lines;
        {
            std::ifstream file(_root_folder + "project.txt");
            std::string line;
            while (std::getline(file, line)) {
                lines.push_back(line);
            }
        }

        {
            std::ofstream file(_root_folder + "project.txt", std::ios::out);
            for(int i = 0; i < lines.size(); i += 2) {
                if(lines[i] != uid) {
                    file << lines[i] << "\n";
                    file << lines[i+1] << "\n";
                }
            }
        }

        // TODO remove any results and thumbnails
        QDir().rmdir(QString::fromStdString(this->_root_folder + "/results/" + uid + "/"));

        writeTimestmap();
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

    void Project::saveResults(const std::string& wsi_uid, std::shared_ptr<Pipeline> pipeline, std::map<std::string, std::shared_ptr<DataObject>> pipelineData) {
        for(auto data : pipelineData) {
            const std::string dataTypeName = data.second->getNameOfClass();
            const std::string dataName = data.first;
            const std::string saveFolder = join(getRootFolder(), "results", wsi_uid, pipeline->getName(), dataName);
            createDirectories(saveFolder);
            std::cout << "Saving " << dataTypeName << " data to " << saveFolder << std::endl;
            if(dataTypeName == "ImagePyramid") {
                const std::string saveFilename = join(saveFolder, data.first + ".tiff");
                auto exporter = TIFFImagePyramidExporter::create(saveFilename)
                        ->connect(data.second);
                exporter->run();
            } else if(dataTypeName == "Image") {
                const std::string saveFilename = join(saveFolder, data.first + ".mhd");
                auto exporter = MetaImageExporter::create(saveFilename)
                        ->connect(data.second);
                exporter->run();
            } else if(dataTypeName == "Tensor") {
                const std::string saveFilename = join(saveFolder, data.first + ".hdf5");
                auto exporter = HDF5TensorExporter::create(saveFilename)
                        ->connect(data.second);
                exporter->run();
            } else {
                std::cout << "Unsupported data to export " << dataTypeName << std::endl;
            }

            // TODO handle multiple renderes somehow
            {
                std::ofstream file(join(saveFolder, "renderer.attributes.txt"), std::iostream::out);
                for(auto renderer : pipeline->getRenderers()) {
                    if(renderer->getNameOfClass() != "ImagePyramidRenderer")
                        file << renderer->attributesToString();
                }
                file.close();
            }
            {
                std::ofstream file(join(saveFolder, "pipeline.attributes.txt"), std::iostream::out);
                try {
                    file << pipeline->getPipelineAttribute("classes") << "\n";
                } catch(Exception& e) {

                }
                /*for(auto attribute : pipeline->getPipelineAttributes()) {
                    file << "Attribute " << attribute.first << " \"" << attribute.second << "\"\n";
                }*/
                file.close();
            }
        }
        writeTimestmap();
    }

    std::shared_ptr<WholeSlideImage> Project::getImage(int i) {
        if(i >= _images.size())
            throw Exception("Out of bounds in Project::getImage");
        auto it = _images.begin();
        std::advance(it, i);
        return it->second;
    }

    std::vector<Result> Project::loadResults(const std::string &wsi_uid) {
        std::vector<Result> results;
        // Load any results for current WSI
        const std::string saveFolder = join(_root_folder, "results", wsi_uid);
        if(!isDir(saveFolder))
            return {};
        for(auto pipelineName : getDirectoryList(saveFolder, false, true)) {
            const std::string folder = join(saveFolder, pipelineName);
            if(!isDir(folder))
                break;
            for(auto dataName : getDirectoryList(folder, false, true)) {
                const std::string folder = join(saveFolder, pipelineName, dataName);
                if(!isDir(folder))
                    break;
                for(auto filename : getDirectoryList(folder, true, false)) {
                    const std::string path = join(folder, filename);
                    const std::string extension = filename.substr(filename.rfind('.'));
                    Renderer::pointer renderer;
                    if(extension == ".tiff") {
                        auto importer = TIFFImagePyramidImporter::create(path);
                        renderer = SegmentationRenderer::create()->connect(importer);
                    } else if(extension == ".mhd") {
                        auto importer = MetaImageImporter::create(path);
                        renderer = SegmentationRenderer::create()->connect(importer);
                    } else if(extension == ".hdf5") {
                        auto importer = HDF5TensorImporter::create(path);
                        renderer = HeatmapRenderer::create()->connect(importer);
                    }
                    if(renderer) {
                        // Read attributes from txt file
                        std::ifstream file(join(folder, "renderer.attributes.txt"), std::iostream::in);
                        if(!file.is_open())
                            throw Exception("Error reading " + join(folder, "renderer.attributes.txt"));
                        do {
                            std::string line;
                            std::getline(file, line);
                            trim(line);
                            std::cout << line << std::endl;

                            std::vector<std::string> tokens = split(line);
                            if(tokens[0] != "Attribute")
                                break;

                            if(tokens.size() < 3)
                                throw Exception("Expecting at least 3 items on attribute line when parsing object " + renderer->getNameOfClass() + " but got " + line);

                            std::string name = tokens[1];

                            std::shared_ptr<Attribute> attribute = renderer->getAttribute(name);
                            std::string attributeValues = line.substr(line.find(name) + name.size());
                            trim(attributeValues);
                            attribute->parseInput(attributeValues);
                            std::cout << "Set attribute " << name << " to " << attributeValues  << " for object " << renderer->getNameOfClass() << std::endl;
                        } while(!file.eof());
                        renderer->loadAttributes();

                        Result result;

                        // Read pipeline attributes
                        {
                            std::ifstream file(join(folder, "pipeline.attributes.txt"), std::iostream::in);
                            if(file.is_open()) {
                                std::string line;
                                std::getline(file, line);
                                trim(line);
                                std::cout << line << std::endl;
                                result.classNames = split(line, ";");
                            }
                        }

                        result.WSI_uid = wsi_uid;
                        result.renderer = renderer;
                        result.name = dataName;
                        result.pipelineName = pipelineName;
                        results.push_back(result);
                    }
                }
            }
        }
        return results;
    }
} // End of namespace fast
