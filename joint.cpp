#include "joint.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

glm::mat4 joint::interpolate(float time)
{
    if(time > keyFrames.back())
        time -= int(time / keyFrames.back()) * keyFrames.back();
    for(int i = 0; i < keyFrames.size(); i++)
        if(time > keyFrames[i])
            currentFrame = i;

    float w = (time - keyFrames[currentFrame]) / (keyFrames[currentFrame+1] - keyFrames[currentFrame]);
    
    glm::vec3 translation = w * pos[currentFrame] + (1-w) * pos[currentFrame + 1];
    glm::fquat rotation = glm::slerp(rot[currentFrame], rot[currentFrame+1], w);
    glm::mat4 transf = glm::translate(glm::mat4(1.f), translation) * glm::mat4_cast(rotation);
    return transf;
}

void joint::update(glm::mat4 parentWorld, std::vector<glm::mat4> &output, float time)
{
    worldTransform = parentWorld * interpolate(time);
    localTransform = worldTransform * transpose(invWorld);
    output[index] = localTransform;
    for(int i = 0; i < children.size(); i++)
        children[i].update(worldTransform, output, time);
    return;
}