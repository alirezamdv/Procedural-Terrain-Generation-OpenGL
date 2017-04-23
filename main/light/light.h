#pragma once
#include <glm/gtc/type_ptr.hpp>

class Light{
private:
    glm::vec3 La;
    glm::vec3 Ld;
    glm::vec3 Ls;
    glm::vec3 lightPos;

public:

    Light(glm::vec3 lightPos =  glm::vec3(0.0f, 0.0f, 0.0f),
          glm::vec3 La =        glm::vec3(0.55, 0.55, 0.55),
          glm::vec3 Ld =        glm::vec3(1.1, 1.1, 1.1),
          glm::vec3 Ls =        glm::vec3(0.1f, 0.1f, 0.1f))
    {
        this->lightPos = lightPos;
        this->La = La;
        this->Ld = Ld;
        this->Ls = Ls;
    }

    glm::vec3 getPos(){
        return this->lightPos;
    }

    void setPos(glm::vec3 pos){
        this->lightPos = pos;
    }

    // pass light properties to the shader
    void Setup(GLuint program_id) {
        glUseProgram(program_id);

        GLuint lighPos_id = glGetUniformLocation(program_id, "lightPos");

        GLuint La_id = glGetUniformLocation(program_id, "La");
        GLuint Ld_id = glGetUniformLocation(program_id, "Ld");
        GLuint Ls_id = glGetUniformLocation(program_id, "Ls");

        glUniform3fv(lighPos_id, ONE, glm::value_ptr(lightPos));
        glUniform3fv(La_id, ONE, glm::value_ptr(La));
        glUniform3fv(Ld_id, ONE, glm::value_ptr(Ld));
        glUniform3fv(Ls_id, ONE, glm::value_ptr(Ls));
    }

    void updatePosUniform(GLuint program_id){
        GLuint lighPos_id = glGetUniformLocation(program_id, "lightPos");
        glUniform3fv(lighPos_id, ONE, glm::value_ptr(lightPos));
    }
};
