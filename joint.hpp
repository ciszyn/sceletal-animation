#ifndef JOINT_HPP
#define JOINT_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <string>

class joint
{
public:
    int index;
    std::string name;
    std::vector<joint> children;
    std::vector<glm::vec3> pos;
    std::vector<glm::fquat> rot;
    std::vector<float> keyFrames;

    glm::mat4 worldTransform = glm::mat4(1.f);
    glm::mat4 localTransform = glm::mat4(1.f);

    int currentFrame = 0;
    glm::mat4 invWorld;
    
    glm::mat4 interpolate(float time);
    void update(glm::mat4 parentWorld, std::vector<glm::mat4> &output, float time);
};

#endif