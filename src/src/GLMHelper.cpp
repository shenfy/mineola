#include <mineola/GLMHelper.h>
#include <vector>
#include <boost/algorithm/string.hpp>

namespace mineola {

glm::vec3 ParseVec3(const std::string &str) {
    glm::vec3 result;
    std::vector<std::string> str_vec;
    boost::algorithm::split(str_vec, str, boost::algorithm::is_any_of(","));
    for (int idx = 0; idx < std::min(3, (int)str_vec.size()); ++idx) {
        result[idx] = std::stof(str_vec[idx]);
    }
    return result;
}

glm::vec4 ParseVec4(const std::string &str) {
    glm::vec4 result;
    std::vector<std::string> str_vec;
    boost::algorithm::split(str_vec, str, boost::algorithm::is_any_of(","));
    for (int idx = 0; idx < std::min(4, (int)str_vec.size()); ++idx) {
        result[idx] = std::stof(str_vec[idx]);
    }
    return result;
}

glm::mat4 ParseMat4(const std::string &str) {
    glm::mat4 result;
    std::vector<std::string> str_vec;
    boost::algorithm::split(str_vec, str, boost::algorithm::is_any_of(","));
    int index = 0;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (index < (int)str_vec.size()) {
                result[col][row] = std::stof(str_vec[index]);
                ++index;
            }
        }
    }
    return result;
}

}
