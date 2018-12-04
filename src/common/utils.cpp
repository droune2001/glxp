#include "utils.h"
#include <fstream>

namespace utils
{

std::vector<char> read_file_content(const std::string &file_path)
{
    std::vector<char> content;

    std::ifstream file(file_path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        printf("!!!!!!!!! Failed to open shader file.\n");
        return std::vector<char>();
    }
    else
    {
        size_t file_size = (size_t)file.tellg();
        std::vector<char> buffer(file_size);

        file.seekg(0);
        file.read(buffer.data(), file_size);

        file.close();
        return buffer;
    }
}

} // namespace utils
