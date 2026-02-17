#pragma once
#include <vector>
#include <ios>
#include <chrono>
#include <fstream>
#include <string>
#include <filesystem>
#include <functional>
#include <iostream>

class FileManager
{
public:
    FileManager();
    void saveToFile(const std::vector<unsigned char>& vec, const std::chrono::system_clock::time_point& timep);
    void readFromFile();
    void openFile(const std::string& path);
    void addHandler(std::function<void(const std::vector<unsigned char>& vec, const std::chrono::system_clock::time_point& timep)> handler);

private:
    std::fstream file_;
    std::vector<std::function<void(const std::vector<unsigned char>& vec, const std::chrono::system_clock::time_point& timep)>> handlers_;

};
