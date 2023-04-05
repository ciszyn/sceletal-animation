#include "glMesh.hpp"
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
#include "./shaders/shader.h"
#include "texture.hpp"

std::stringstream glMesh::search(std::string file, std::string path)
{
    std::ifstream fin(file);
    xml_document<> doc;
    std::stringstream buffer;
    buffer << fin.rdbuf();
    std::string content(buffer.str());
    doc.parse<0>(&content[0]);
    buffer = std::stringstream();

    xml_node<> *node = doc.first_node();

    std::stringstream path_stream(path);

    while(path_stream.peek() != EOF)
    {
        std::string title;
        path_stream >> title;
        node = node->first_node(title.c_str());
        
        std::string id;
        path_stream >> id;
        if(id == "-")
            continue;
        while(node->first_attribute("id")->value() != id)
            node = node->next_sibling(title.c_str());
    }

    buffer << node->value();

    fin.close();
    return buffer;
}

std::vector<glm::vec3> glMesh::read_v(std::string file, std::string source_id, bool size3)
{   
    std::stringstream buffer = search(file, "library_geometries - geometry - mesh - source " + source_id + " float_array -");

    std::vector<glm::vec3> ret;
    glm::vec3 vec;
    while(buffer >> vec.x)
    {
        buffer >> vec.y;
        if(size3)
            buffer >> vec.z;
        ret.push_back(vec);
    }
    return ret;
}

void glMesh::arrange(std::string file, std::vector<glm::vec3> &v, std::vector<glm::vec3> &vn, std::vector<glm::vec3> &uv)
{
    std::vector<glm::vec3> v_temp;
    std::vector<glm::vec3> vn_temp;
    std::vector<glm::vec3> uv_temp;

    std::stringstream buffer = search(file, "library_geometries - geometry - mesh - polylist - p -");

    int v_index, vn_index, uv_index, temp;
    while(buffer >> v_index)
    {
        buffer >> vn_index;
        buffer >> uv_index;
        buffer >> temp;

        v_temp.push_back(v[v_index]);
        vn_temp.push_back(vn[vn_index]);
        uv_temp.push_back(uv[uv_index]);
    }
    v = v_temp;
    vn = vn_temp;
    uv = uv_temp;
    return;
}

glMesh::glMesh(std::string file)
{
    glGenVertexArrays(1, &vertexID);
    glBindVertexArray(vertexID);

    std::vector<glm::vec3> v = read_v(file, "Cube-mesh-positions");
    std::vector<glm::vec3> vn = read_v(file, "Cube-mesh-normals");
    std::vector<glm::vec3> uv = read_v(file, "Cube-mesh-map-0", false);
    vn_size = vn.size();
    arrange(file, v, vn, uv);

    std::vector<glm::vec2> v_uv;
    for(int i = 0; i < uv.size(); i++)
    {
        v_uv.push_back(glm::vec2(uv[i]));
    }

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(glm::vec3), &v[0], GL_STATIC_DRAW);

    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, vn.size() * sizeof(glm::vec3), &vn[0], GL_STATIC_DRAW);

    glGenBuffers(1, &textureBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
    glBufferData(GL_ARRAY_BUFFER, vn.size() * sizeof(glm::vec2), &v_uv[0], GL_STATIC_DRAW);
    

    programID = LoadShaders("./shaders/vertexShader.glsl", "./shaders/fragmentShader.glsl");

    texture = loadBMP_custom("./res/diffuse.bmp");
    textureLocation = glGetUniformLocation(programID, "myTextureSampler");

    MatrixID = glGetUniformLocation(programID, "MVP");
    ViewMatrixID = glGetUniformLocation(programID, "V");
    ModelMatrixID = glGetUniformLocation(programID, "M");
    LightID = glGetUniformLocation(programID, "lightPos");

    v_size = v.size();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void glMesh::draw(float *MVP, float *V, float *M, glm::vec3 &lightPos)
{
    glUseProgram(programID);

    glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(textureLocation, 0);

    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, MVP);
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, V);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, M);

    glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glDrawArrays(GL_TRIANGLES, 0, v_size);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

glMesh::~glMesh()
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &normalBuffer);
    glDeleteBuffers(1, &textureBuffer);
    glDeleteTextures(1, &texture);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &vertexID);
}