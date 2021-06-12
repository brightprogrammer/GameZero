#ifndef GAMEZERO_UTILS_FILE_HPP
#define GAMEZERO_UTILS_FILE_HPP

#include <iterator>
#include <string>
#include <vector>
#include "log.hpp"
#include "assert.hpp"
#include <fstream>

namespace GameZero{

    /**
     * @brief Read file from system
     * 
     * @param filename : filepath / filename
     * @param readBinary : read in binary format ?
     * @return std::string containing read file
     */
    [[nodiscard]] inline bool ReadFile(std::string& data, const char* filename, bool readBinary = false){
        // file handle
        std::ifstream file;
        
        // read file
        if(readBinary) file.open(filename, std::ios::binary);
        else file.open(filename);

        // check if file is open
        if(!file.is_open()){
            LOG(CRITICAL, "Failed to read file [ %s ]", filename);
            return false;
        }

        // std::istreambuf_iterator iteratively reads from given std::basic_streambuf
        // empty constructor is treated as end of streambuf and hence signals end of file
        data = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        file.close();

        return true;
    }

    /**
     * @brief 
     * 
     * @tparam size_type : read failure
     * @param data vector to read data into
     * @param filename name of file / filepath
     * @param readBinary read in binary?
     * @return true : read success
     * @return false : read failure 
     */
    template<typename size_type>
    [[nodiscard]] inline bool  ReadFile(std::vector<size_type>& data, const char* filename, bool readBinary = false){
        // file handle
        std::ifstream file;
        
        // read file
        if(readBinary) file.open(filename, std::ios::binary | std::ios::ate);
        else file.open(filename, std::ios::ate);

        size_t fileSize = file.tellg();
        file.seekg(0);

        // check if file is open
        if(!file.is_open()){
            LOG(CRITICAL, "Failed to read file [ %s ]", filename);
            return false;
        }
        
        // read file
        data.clear();
        data.resize(fileSize / sizeof(size_type));
        file.read(reinterpret_cast<char*>(data.data()), fileSize);
        file.close();

        return true;
    };

}

#endif//GAMEZERO_UTILS_FILE_HPP