#ifndef MODEL_HPP
#define MODEL_HPP
#include "glMesh.hpp"
#include "joint.hpp"
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "./xml/rapidxml.hpp"
#include "./shaders/shader.h"

using namespace rapidxml;

class Model : public glMesh 
{
private:
    GLuint weightBuffer;
    GLuint jointBuffer;

    GLuint transformsID;

    joint root;
    int jointCount;

    void loadWeights(std::string file);
    void loadJoints(std::string file, joint &current, xml_node<> *node, bool root = false);
    void loadJointIndeces(std::string file, joint &current);
    template <class T>
    void arrange(std::string file, std::vector<T> &v);
    std::vector<glm::mat4> transforms;  //like that or just each time pass it?

public:
    Model(std::string file);
    ~Model();
    void draw(float time, float *MVP = nullptr, float *V = nullptr, float *M = nullptr, glm::vec3 *lightPos = nullptr);
    void display(joint *j = nullptr);
};

#endif