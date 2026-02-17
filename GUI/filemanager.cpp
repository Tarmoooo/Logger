#include "filemanager.h"

FileManager::FileManager() {
    openFile("log.bin");
}

void FileManager::saveToFile(const std::vector<unsigned char>& vec, const std::chrono::system_clock::time_point& timep)
{
    if (vec.size() < 4) return;

    int32_t packetSize = 0;
    std::memcpy(&packetSize, vec.data(), 4);

    file_.write(reinterpret_cast<const char*>(vec.data()), packetSize);

    auto duration = timep.time_since_epoch();
    double seconds = std::chrono::duration<double>(duration).count();
    file_.write(reinterpret_cast<const char*>(&seconds), sizeof(seconds));

    file_.flush();
}

void FileManager::readFromFile()
{
    file_.clear();
    file_.seekg(0, std::ios::beg);

    int32_t packetSize = 0;

    while(file_.read(reinterpret_cast<char*>(&packetSize), 4))
    {
        std::vector<unsigned char> data(packetSize);
        std::memcpy(data.data(), &packetSize, 4);

        file_.read(reinterpret_cast<char*>(data.data() + 4), packetSize - 4);

        double seconds = 0;
        file_.read(reinterpret_cast<char*>(&seconds), 8);

        auto duration = std::chrono::duration<double>(seconds);
        std::chrono::system_clock::time_point timep(
            std::chrono::duration_cast<std::chrono::system_clock::duration>(duration)
            );

        for(auto &handler : handlers_) {
            handler(data, timep);
        }
    }
}


void FileManager::openFile(const std::string &path)
{
    if(file_.is_open()) file_.close();

    if (std::filesystem::exists(path) && std::filesystem::file_size(path) != 0) {
        file_.open(path, std::ios::in | std::ios::out | std::ios::binary);
        readFromFile();
    }
    else

        file_.open(path,
                   std::ios::out | std::ios::binary | std::ios::app);

}

void FileManager::addHandler(std::function<void (const std::vector<unsigned char> &, const std::chrono::system_clock::time_point &)> handler)
{
    handlers_.push_back(handler);
}

