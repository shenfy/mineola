#ifndef MINEOLA_GLMHELPER_H
#define MINEOLA_GLMHELPER_H

#include <string>
#include <glm/glm.hpp>

namespace mineola {

glm::vec3 ParseVec3(const std::string &str);
glm::vec4 ParseVec4(const std::string &str);
glm::mat4 ParseMat4(const std::string &str);

}
#endif /* MINEOLA_GLMHELPER_H */
