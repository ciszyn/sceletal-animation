#include "Model.hpp"
#include "glMesh.hpp"
#include "joint.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>
#include "texture.hpp"

glm::fquat CalculateRotation( glm::mat4 a )
{
    glm::fquat q;
    float trace = a[0][0] + a[1][1] + a[2][2];
    if( trace > 0 ) 
    {
        float s = 0.5f / sqrtf(trace+ 1.0f);
        q.w = 0.25f / s;
        q.x = ( a[2][1] - a[1][2] ) * s;
        q.y = ( a[0][2] - a[2][0] ) * s;
        q.z = ( a[1][0] - a[0][1] ) * s;
    } 
    else 
    {
        if ( a[0][0] > a[1][1] && a[0][0] > a[2][2] ) 
        {
            float s = 2.0f * sqrtf( 1.0f + a[0][0] - a[1][1] - a[2][2]);
            q.w = (a[2][1] - a[1][2] ) / s;
            q.x = 0.25f * s;
            q.y = (a[0][1] + a[1][0] ) / s;
            q.z = (a[0][2] + a[2][0] ) / s;
        } 
        else if (a[1][1] > a[2][2]) 
        {
            float s = 2.0f * sqrtf( 1.0f + a[1][1] - a[0][0] - a[2][2]);
            q.w = (a[0][2] - a[2][0] ) / s;
            q.x = (a[0][1] + a[1][0] ) / s;
            q.y = 0.25f * s;
            q.z = (a[1][2] + a[2][1] ) / s;
        } 
        else 
        {
            float s = 2.0f * sqrtf( 1.0f + a[2][2] - a[0][0] - a[1][1] );
            q.w = (a[1][0] - a[0][1] ) / s;
            q.x = (a[0][2] + a[2][0] ) / s;
            q.y = (a[1][2] + a[2][1] ) / s;
            q.z = 0.25f * s;
        }
    }
    return q;
}

bool mycmpr(std::pair<float, int> first, std::pair<float, int> second)
{
    return first.first > second.first;
}

void Model::loadWeights(std::string file)
{
    std::vector<float> weights;
    std::stringstream buffer = search(file, "library_controllers - controller - skin - source Armature_Cube-skin-weights float_array -");
    float weight;
    while(buffer.peek() != EOF)
    {
        buffer >> weight;
        weights.push_back(weight);
    }

    buffer = search(file, "library_controllers - controller - skin - vertex_weights - vcount -");
    std::vector<int> weight_num;
    int num;
    while(buffer >> num)
    {
        weight_num.push_back(num);
    }

    buffer = search(file, "library_controllers - controller - skin - vertex_weights - v -");
    std::vector<glm::vec3> vert_weights;
    std::vector<glm::ivec3> vert_joints;
    int tmp;
    for(int i = 0; i < 740; i++)
    {
        float sum = 0;
        std::vector<std::pair<float, int>> attrib_temp;
        for(int j = 0; j < weight_num[i]; j++)
        {
            int joint_index, weight_index;
            buffer >> joint_index;
            buffer >> weight_index;
            std::pair<float, int> data(weights[weight_index], joint_index);
            attrib_temp.push_back(data);
        }
        sort(attrib_temp.begin(), attrib_temp.end(), mycmpr);
        glm::vec3 inf;
        glm::ivec3 joint;

        for(int j = 0; j < 3; j++)
        {
            if(attrib_temp.size() > j)
            {
                inf[j] = attrib_temp[j].first;
                joint[j] = attrib_temp[j].second;
            }
            else
            {
                inf[j] = 0;
                joint[j] = 0;
            }
            sum += inf[j];
        }
        inf /= sum;
        vert_weights.push_back(inf);
        vert_joints.push_back(joint);
        sum = 0;
    }
    std::vector<glm::vec3> t(vn_size);
    arrange(file, vert_weights);
    arrange(file, vert_joints);

    glBindVertexArray(vertexID);

    glGenBuffers(1, &weightBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, weightBuffer);
    glBufferData(GL_ARRAY_BUFFER, vert_weights.size() * sizeof(glm::vec3), &vert_weights[0], GL_STATIC_DRAW);

    glGenBuffers(1, &jointBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, jointBuffer);
    glBufferData(GL_ARRAY_BUFFER, vert_joints.size() * sizeof(glm::vec3), &vert_joints[0], GL_STATIC_DRAW);
}

void Model::loadJoints(std::string file, joint &current, xml_node<> *node, bool root)
{
    if(root)
    {
        current.name = node->first_attribute("id")->value();
        std::stringstream buffer;
        buffer << node->first_node("matrix")->value();
        for(int i = 0; i < 4; i++)
            for(int j = 0; j < 4; j++)
                buffer >> current.invWorld[i][j];
        current.invWorld = glm::inverse(current.invWorld);

        buffer = search(file, "library_animations - animation Armature_" + current.name + "_pose_matrix source - float_array -");
        float time_stamp;
        while(buffer >> time_stamp)
            current.keyFrames.push_back(time_stamp);

        buffer = search(file, "library_animations - animation Armature_" + current.name + "_pose_matrix source Armature_" + current.name + "_pose_matrix-output float_array -");
        for(int i = 0; i < current.keyFrames.size(); i++)
        {
            glm::mat4 transf;
            for(int j = 0; j < 4; j++)
                for(int k = 0; k < 4; k++)
                {
                    buffer >> transf[j][k];
                }
            glm::vec3 translation;
            for(int i = 0; i < 3; i++)
                translation[i] = transf[i][3];
            glm::fquat rotation = CalculateRotation(transf);
            current.pos.push_back(translation);
            current.rot.push_back(rotation);
        }
    }
    node = node->first_node("node");
    while(node != nullptr)
    {
        joint tmp;
        tmp.name = node->first_attribute("id")->value();
        std::stringstream buffer;
        buffer << node->first_node("matrix")->value();
        for(int i = 0; i < 4; i++)
            for(int j = 0; j < 4; j++)
                buffer >> tmp.invWorld[i][j];
        tmp.invWorld = current.invWorld * glm::inverse(tmp.invWorld);

        buffer = search(file, "library_animations - animation Armature_" + tmp.name + "_pose_matrix source - float_array -");
        float time_stamp;
        while(buffer >> time_stamp)
            tmp.keyFrames.push_back(time_stamp);

        buffer = search(file, "library_animations - animation Armature_" + tmp.name + "_pose_matrix source Armature_" + tmp.name + "_pose_matrix-output float_array -");
        for(int i = 0; i < tmp.keyFrames.size(); i++)
        {
            glm::mat4 transf;
            for(int j = 0; j < 4; j++)
                for(int k = 0; k < 4; k++)
                {
                    buffer >> transf[j][k];
                }
            glm::vec3 translation;
            for(int i = 0; i < 3; i++)
                translation[i] = transf[i][3];
            glm::fquat rotation = CalculateRotation(transf);
            tmp.pos.push_back(translation);
            tmp.rot.push_back(rotation);
        }

        loadJoints(file, tmp, node);
        current.children.push_back(tmp);
        node = node->next_sibling("node");
    }

}

void Model::loadJointIndeces(std::string file, joint &current)
{
    std::stringstream buffer = search(file, "library_controllers - controller - skin - source - Name_array -");
    int i = 0;
    std::string name;
    while(buffer >> name && name != current.name)
        i++;
    current.index = i;
    for(int i = 0; i < current.children.size(); i++)
        loadJointIndeces(file, current.children[i]);
}

Model::Model(std::string file) : glMesh(file)
{
    glDeleteProgram(programID);
    programID = LoadShaders("./shaders/animationVertex.glsl", "./shaders/animationFragment.glsl");

    textureLocation = glGetUniformLocation(programID, "myTextureSampler");
    transformsID = glGetUniformLocation(programID, "Transforms");
    MatrixID = glGetUniformLocation(programID, "MVP");
    ViewMatrixID = glGetUniformLocation(programID, "V");
    ModelMatrixID = glGetUniformLocation(programID, "M");
    LightID = glGetUniformLocation(programID, "lightPos");

    loadWeights(file);

    std::ifstream fin(file);
    xml_document<> doc;
    std::stringstream buffer;
    buffer << fin.rdbuf();
    std::string content(buffer.str());
    doc.parse<0>(&content[0]);
    buffer = std::stringstream();
    fin.close();
    
    xml_node<> *node = doc.first_node();
    node = node->first_node("library_visual_scenes")->first_node("visual_scene")->first_node("node");
    while(strcmp(node->first_attribute("id")->value(), "Armature") != 0)
        node = node->next_sibling("node");
    node = node->first_node("node");

    root.invWorld = glm::mat4(1.f);
    loadJoints(file, root, node, true);
    loadJointIndeces(file, root);

    node = doc.first_node()->first_node("library_controllers")->first_node("controller")->first_node("skin")->first_node("source")->first_node("Name_array");
    std::stringstream b;
    b << node->first_attribute("count")->value();
    b >> jointCount;
}

Model::~Model()
{
    glDeleteBuffers(1, &weightBuffer);
    glDeleteBuffers(1, &jointBuffer);
}

void Model::display(joint *j)
{
    std::cout << jointCount << std::endl;
    if(j == nullptr)
        display(&root);
    else
    {
        std::cout << j->index << "  " << j->name << std::endl;
        std::cout << to_string(j->invWorld) << std::endl;
        for(int i = 0; i < j->keyFrames.size(); i++)
            std::cout << j->keyFrames[i] << " ";
        std::cout << std::endl;
        for(int i = 0; i < j->pos.size(); i++)
            std::cout << glm::to_string(j->pos[i]) << std::endl;
        for(int i = 0; i < j->rot.size(); i++)
            std::cout << glm::to_string(j->rot[i]) << std::endl;
        for(int i = 0; i < j->children.size(); i++)
            std::cout << " -" << j->children[i].name;
        std::cout << std::endl;
        for(int i = 0; i < j->children.size(); i++)
            display(&(j->children[i]));
    }
}

void Model::draw(float time, float *MVP, float *V, float *M, glm::vec3 *lightPos)
{
    std::vector<glm::mat4> transforms(jointCount);
    root.update(glm::mat4(1.f), transforms, time);
    glUseProgram(programID);

    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, MVP);
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, V);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, M);
    glUniform3f(LightID, lightPos->x, lightPos->y, lightPos->z);
    glUniformMatrix4fv(transformsID, transforms.size(), GL_FALSE, reinterpret_cast<GLfloat *>(&transforms[0]));

    	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(textureLocation, 0);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, jointBuffer);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, weightBuffer);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glDrawArrays(GL_TRIANGLES, 0, v_size);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
}

template <class T>
void Model::arrange(std::string file, std::vector<T> &v)
{
    std::vector<T> v_temp;

    std::stringstream buffer = search(file, "library_geometries - geometry - mesh - polylist - p -");

    int v_index, vn_index, temp;
    while(buffer >> v_index)
    {
        buffer >> temp;
        buffer >> temp;
        buffer >> temp;

        v_temp.push_back(v[v_index]);
    }
    v = v_temp;
    return;
}
