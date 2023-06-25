include(CMakeFindDependencyMacro)

find_dependency(Boost REQUIRED)
find_dependency(JPEG REQUIRED)
find_dependency(PNG REQUIRED)
find_dependency(glm REQUIRED)
find_dependency(Stb REQUIRED)
find_dependency(nlohmann_json REQUIRED)
find_dependency(OpenGL REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/mineolaTargets.cmake)