#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include "Model.hpp"
#include <iostream>


using namespace std;

int main(int argc, char **argv)
{
    int width = 800;
    int height = 600;
    sf::ContextSettings settings(24);
    settings.antialiasingLevel = 8;
    sf::Window window(sf::VideoMode(width, height), "animation", sf::Style::Default, settings);
    window.setVerticalSyncEnabled(true);
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "failed to initialize glew\n";
        return -1;
    }

    window.setActive(true);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    glClearColor(.1f, .1f, .1f, 1.f);
    glEnable(GL_MULTISAMPLE);
    Model crowboy("./res/model.dae");
    window.setActive(false);

    float r = 16;
    glm::mat4 Projection = glm::perspective(glm::radians(45.f), float(width)/float(height), .1f, 100.f);
    glm::mat4 View = glm::lookAt(glm::vec3(0, -r, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
    glm::mat4 Model = glm::translate(glm::vec3(0, 0, -4));
    glm::mat4 MVP = Projection * View * Model;
    glm::vec3 dir(0, 1, 0);
    glm::vec3 up(0, 0, 1);
    glm::vec3 right(-1, 0, 0);

    bool running = true;
    int mouse_start_x, mouse_start_y;
    float ax = 0;
    float ay = -M_PI/2;
    float prev_ax = 0;
    float prev_ay = -M_PI/2;
    bool mesh = false;
    bool back_culling = true;

    sf::Clock clock;
    while (running)
    {
        sf::Event event;

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                running = false;
            else if (event.type == sf::Event::Resized)
                glViewport(0, 0, event.size.width, event.size.height);
            else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::W && !mesh)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                mesh = true;
            }
            else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::W && mesh)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                mesh = false;
            }
            else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::S && !back_culling)
            {
                glEnable(GL_CULL_FACE);
                back_culling = true;
            }
            else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::S && back_culling)
            {        
                glDisable(GL_CULL_FACE);
                back_culling = false;
            }
            else if (event.type == sf::Event::MouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                mouse_start_x = sf::Mouse::getPosition().x;
                mouse_start_y = sf::Mouse::getPosition().y;
            }
            else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Button::Left)
            {
                prev_ax = ax;
                prev_ay = ay;
            }
            else if (event.type == sf::Event::MouseMoved && sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                ax = prev_ax - (sf::Mouse::getPosition().x - mouse_start_x)/100.;
                ay = prev_ay + (sf::Mouse::getPosition().y - mouse_start_y)/100.;

                right = glm::vec3(sin(ax-3.14f/2.f), 0, cos(ax-3.14f/2.f));
                glm::vec3 dir(cos(ay) * sin(ax), sin(ay), cos(ay)*cos(ax));
                up = glm::cross(right, dir);

                View = glm::lookAt(glm::vec3(r * cos(ay) * sin(ax), r*sin(ay), r*cos(ax)*cos(ay)), glm::vec3(0, 0, 0), up);

                MVP = Projection * View * Model;
            }
            else if(event.type == sf::Event::MouseWheelScrolled)
            {
                r -= event.mouseWheelScroll.delta;
                View = glm::lookAt(glm::vec3(r * cos(ay) * sin(ax), r*sin(ay), r*cos(ax)*cos(ay)), glm::vec3(0, 0, 0), up);
                MVP = Projection * View * Model;
            }
        }
        window.setActive(true);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 lightPos =  glm::vec3(r*sin(ax) * cos(ay), r*sin(ay), r*cos(ax)*cos(ay)) + 0.5f * r * up - 0.5f * r * right;
        crowboy.draw(float(clock.getElapsedTime().asMilliseconds()) / 1000., &MVP[0][0], &View[0][0], &Model[0][0], &lightPos);

        window.setActive(false);
        window.display();
    }

    return 0;
}