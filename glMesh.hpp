#ifndef GLMESH_HPP
#define GLMESH_HPP
#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>
#include <fstream>
#include <sstream>
#include "./xml/rapidxml.hpp"
#include <iostream>

using namespace rapidxml;

class glMesh
{
protected:
    GLuint vertexID;
    GLuint vertexBuffer;
    GLuint normalBuffer;
    GLuint textureBuffer;

    GLuint texture;
    GLuint textureLocation;
    
    GLuint programID;
    GLuint MatrixID;
    GLuint ViewMatrixID;
    GLuint ModelMatrixID;
    GLuint LightID;

    int v_size;
    int vn_size;

    //reads vertices from .dae
    std::vector<glm::vec3> read_v(std::string file, std::string source_id, bool size3 = true);
    void arrange(std::string file, std::vector<glm::vec3> &v, std::vector<glm::vec3> &vn, std::vector<glm::vec3> &uv);
    std::stringstream search(std::string file, std::string path);

public:

    glMesh(std::string file);
    void draw(float *MVP, float *V, float *M, glm::vec3 &lightPos);
    ~glMesh();
};

#endif