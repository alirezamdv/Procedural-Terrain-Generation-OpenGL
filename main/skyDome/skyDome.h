#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>
#include "skyPlane.h"
#include "../gridmesh.h"
#include "../light/light.h"
#include "../light/lightable.h"
#include "../material/material.h"
#include "../camera/fractionalview.h"
#include "../utils.h"

using namespace glm;

class SkyDome: public ILightable{


private:
    GLuint vertex_array_id_;        // vertex array object
    GLuint program_id_;             // GLSL shader program ID
    GLuint vertex_buffer_object_position_;  // memory buffer for positions
    GLuint vertex_buffer_object_index_;     // memory buffer for indices
    GLuint MVPId;
    GLuint sunPosId, bottomSkyColorId, topSkyColorId, domeGradBottomId, domeGradTopId, sunColorId;
    Light* light;
    SkyPlane cloudPlane;
    SkyPlane farCloudPlane;

    mat4 cloudPlaneModelMatrix, farCloudPlaneModelMatrix;

    const float radius = 10.0f;
    const int rings = 24;
    const int sectors = 24;

    const vec3 sunOrbitXAxis = vec3(1.0, 0.0, 0.0);
    const vec3 sunOrbitYAxis = vec3(0.0, 0.958, 0.287);
    const vec3 sunOrbitCenter = vec3(0.0, 0.0, 0.0);

    //Sky color values
    const vec3 sunsetColor = vec3(1.0f, 0.568f, 0.078f);
    const vec3 nightSkyColor = vec3(0.05f, 0.07f, 0.150f);
    const vec3 SUNSETCOL_topSky = vec3(0.298f, 0.494f, 0.741f);
    const vec3 SUNSETCOL_bottomSky = vec3(0.894f, 0.333f, 0.333f);
    const vec3 mistColor = vec3(0.9f, 0.9f, 0.98f);
    const vec3 blueSkyColor = 1.5f * vec3(0.098f, 0.269f, 0.665f);
    const vec3 lightblueSkyColor = vec3(0.059f, 0.678f, 1.0f);
    const vec3 OUTERSPACECOL_bottomSky = vec3(0.098f, 0.269f, 0.665f);
    const vec3 OUTERSPACECOL_topSky = vec3(0.098f, 0.269f, 0.665f);

    // Sun color values
    const vec3 SUN_COLOR = vec3(5.0f, 5.0f, 5.0f);
    const vec3 SUN_COLOR_SUNSET = vec3(4.0f, 4.5f, 2.0f);

    // Light color values
    const vec3 LIGHTCOL_NIGHT = nightSkyColor;

    const vec3 ZERO_COL = vec3(0.0f, 0.0f, 0.0f);
    const vec3 LIGHTCOL_SUNSET_DIFFUSE = vec3(0.894f, 0.333f, 0.333f);
    const vec3 LIGHTCOL_SUNSET_SPECULAR = vec3(4.0f, 2.5f, 2.5f);
    const vec3 LIGHTCOL_DAY = vec3(0.0f, 0.0f, 0.0f);

    // General dome gradient begin/end positions values
    const float NIGHTGRADIENT_START = 0.0f;
    const float NIGHTGRADIENT_END = 3.0f;
    const float SUNSETGRADIENT_START = 0.0f;
    const float SUNSETGRADIENT_END = 3.0f;
    const float DAYGRADIENT_START = -0.4f;

    const float DAYGRADIENT_END = 1.0f;


    // Color change positions values
    const float sunsetEnd = 6.2f;
    const float sunsetBegin = -1.0f;
    const float nightBegin = -2.0f;
    const float outerSpaceBegin = 6.0f;
    const float outerSpaceEnd = 12.0f;

    // Values passed to GPU
    float domeGradBottom = DAYGRADIENT_START;
    float domeGradTop = DAYGRADIENT_END;
    vec3 bottomSkyColor = mistColor;
    vec3 topSkyColor = blueSkyColor;
    vec3 sunColor = SUN_COLOR;

    float PI = 3.14159265359f;
    float PIovr2 = PI * 0.5f;
    float PI2 = 2.0f * PI ;

    int numIndices;
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> texcoords;
    std::vector<GLuint> indices;

public:

    void Init() {

        // compile the shaders
        program_id_ = icg_helper::LoadShaders("skyDome_vshader.glsl",
                                              "skyDome_fshader.glsl");
        if(!program_id_) {
            exit(EXIT_FAILURE);
        }

        glUseProgram(program_id_);

        cloudPlane.Init(0, vec2(4.0, 3.0));
        farCloudPlane.Init(0, vec2(1.0, 1.0));
        cloudPlaneModelMatrix =
                translate(IDENTITY_MATRIX, vec3(0.0f, -4.0f, 0.0f))
                *
                rotate(IDENTITY_MATRIX, 0.0f, vec3(0.0f, 1.0f, 0.0f))
                *
                scale(IDENTITY_MATRIX, vec3(75.0f, 75.0f, 75.0f));

        farCloudPlaneModelMatrix =
                translate(IDENTITY_MATRIX, vec3(0.0f, 2.0f, 0.0f))
                *
                rotate(IDENTITY_MATRIX, 0.2f, vec3(0.0f, 1.0f, 0.0f))
                *
                scale(IDENTITY_MATRIX, vec3(75.0f, 75.0f, 75.0f));

        glGenVertexArrays(1, &vertex_array_id_);
        glBindVertexArray(vertex_array_id_);

        //Generate sphere,
        // adapted from http://stackoverflow.com/questions/5988686/creating-a-3d-sphere-in-opengl-using-visual-c
        {
            float const R = 1./(float)(rings-1);
            float const S = 1./(float)(sectors-1);
            int r, s;

            vertices.resize(rings * sectors * 3);
            normals.resize(rings * sectors * 3);
            texcoords.resize(rings * sectors * 2);
            std::vector<GLfloat>::iterator v = vertices.begin();
            std::vector<GLfloat>::iterator n = normals.begin();
            std::vector<GLfloat>::iterator t = texcoords.begin();
            for(r = 0; r < rings; r++) for(s = 0; s < sectors; s++) {
                float const y = sin( -PIovr2 + PI * r * R );
                float const x = cos(PI2 * s * S) * sin( PI * r * R );
                float const z = sin(PI2 * s * S) * sin( PI * r * R );

                *t++ = s*S;
                *t++ = r*R;

                *v++ = x * radius;
                *v++ = y * radius;
                *v++ = z * radius;

                *n++ = x;
                *n++ = y;
                *n++ = z;
            }

            indices.resize(rings * sectors * 6);
            std::vector<GLuint>::iterator i = indices.begin();
            for(r = 0; r < rings-1; r++) {
                for(s = 0; s < sectors-1; s++) {
                    *i++ = r     * sectors + s;
                    *i++ = r     * sectors + (s+1);
                    *i++ = (r+1) * sectors + (s+1);
                    *i++ = r     * sectors + s;
                    *i++ = (r+1) * sectors + (s+1);
                    *i++ = (r+1) * sectors + s;
                }
            }
        }

        numIndices = indices.size();

        // position buffer
        glGenBuffers(1, &vertex_buffer_object_position_);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_position_);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat),
                     &vertices[0], GL_STATIC_DRAW);

        // vertex indices
        glGenBuffers(1, &vertex_buffer_object_index_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffer_object_index_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                     &indices[0], GL_STATIC_DRAW);

        // position shader attribute
        GLuint loc_position = glGetAttribLocation(program_id_, "domePos");
        glEnableVertexAttribArray(loc_position);
        glVertexAttribPointer(loc_position, 3, GL_FLOAT, DONT_NORMALIZE,
                              ZERO_STRIDE, ZERO_BUFFER_OFFSET);

        // other ids
        sunPosId = glGetUniformLocation(program_id_, "sunPos");
        topSkyColorId = glGetUniformLocation(program_id_, "topSkyColor");
        bottomSkyColorId= glGetUniformLocation(program_id_, "bottomSkyColor");
        domeGradBottomId= glGetUniformLocation(program_id_, "domeGradBottom");
        domeGradTopId = glGetUniformLocation(program_id_, "domeGradTop");
        sunColorId = glGetUniformLocation(program_id_, "sunColor");
        MVPId = glGetUniformLocation(program_id_, "MVP");

        // to avoid the current object being polluted
        glBindVertexArray(0);
        glUseProgram(0);
    }

    void useLight(Light* l){
        this->light = l;
        light->registerProgram(program_id_);
        cloudPlane.useLight(l);
        farCloudPlane.useLight(l);
    }

    float getRadius(){
        return radius;
    }

    void Cleanup() {
        glBindVertexArray(0);
        glUseProgram(0);
        glDeleteBuffers(1, &vertex_buffer_object_position_);
        glDeleteBuffers(1, &vertex_buffer_object_index_);
        glDeleteVertexArrays(1, &vertex_array_id_);
        glDeleteProgram(program_id_);
    }

    void Draw(const mat4 &MODEL,
              const mat4 &VIEW,
              const mat4 &PROJECTION,
              const vec3 &viewPos) {
        glUseProgram(program_id_);

        mat4 skyboxMVP = PROJECTION * mat4(mat3(VIEW));
        mat4 sceneMVP = PROJECTION * VIEW * MODEL;
        mat4 cloudPlaneMVP = skyboxMVP * cloudPlaneModelMatrix;
        mat4 farCloudPlaneMVP = skyboxMVP * farCloudPlaneModelMatrix;

        float time = glfwGetTime();

        float theta = 0.01 * time - 0.0;

        vec3 sunPos = sunOrbitCenter + radius * cos(theta) * sunOrbitXAxis + radius * sin(theta) * sunOrbitYAxis;
        computeSkyColors(sunPos, viewPos);

        light->setPos(sunPos);

        glUniformMatrix4fv(MVPId, 1, GL_FALSE, value_ptr(skyboxMVP));
        glUniform3fv(sunPosId, 1, value_ptr(sunPos));
        glUniform3fv(topSkyColorId, 1, value_ptr(topSkyColor));
        glUniform3fv(bottomSkyColorId, 1, value_ptr(bottomSkyColor));
        glUniform3fv(sunColorId, 1, value_ptr(sunColor));
        glUniform1f(domeGradTopId, domeGradTop);
        glUniform1f(domeGradBottomId, domeGradBottom);

        // dome
        glBindVertexArray(vertex_array_id_);
        glDepthMask(GL_FALSE);
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE);

        farCloudPlane.Draw(farCloudPlaneMVP, sunPos);
        cloudPlane.Draw(cloudPlaneMVP, sunPos);

        glBindVertexArray(0);
        glUseProgram(0);
    }

    /** Easier to compute sky's color in CPU since it should have an affect on the light **/
    void computeSkyColors(const vec3& sunPos, const vec3& viewPos){

        /** Compute the sky's color gradient **/
        // Night
        if(sunPos.y < sunsetBegin){
            float sunSetCoeff = clamp((sunPos.y - nightBegin) / (sunsetBegin - nightBegin), 0.0f, 1.0f);
            bottomSkyColor = mix(nightSkyColor, SUNSETCOL_bottomSky, sunSetCoeff);
            topSkyColor = mix(nightSkyColor, SUNSETCOL_topSky, sunSetCoeff);
            light->setDiffuseIntensity(mix(ZERO_COL, LIGHTCOL_SUNSET_DIFFUSE, sunSetCoeff));
            light->setSpecularIntensity(mix(ZERO_COL, LIGHTCOL_SUNSET_SPECULAR, sunSetCoeff));

            sunColor = mix(SUN_COLOR, SUN_COLOR_SUNSET, sunSetCoeff);
            domeGradBottom = mix(NIGHTGRADIENT_START, SUNSETGRADIENT_START, sunSetCoeff);
            domeGradTop = mix(NIGHTGRADIENT_END, SUNSETGRADIENT_END, sunSetCoeff);
        }
        // Sunset
        else if(sunPos.y > sunsetBegin && sunPos.y < sunsetEnd){

            float sunSetCoeff = (sunPos.y - sunsetBegin) / (sunsetEnd - sunsetBegin);
            sunSetCoeff = Utils::smoothExpTransition(sunSetCoeff);
            bottomSkyColor = mix(SUNSETCOL_bottomSky, mistColor, sunSetCoeff);
            topSkyColor = mix(SUNSETCOL_topSky, blueSkyColor, sunSetCoeff);
            light->setDiffuseIntensity(mix(LIGHTCOL_SUNSET_DIFFUSE, light->getDefaultDiffuseIntensity(), sunSetCoeff));
            light->setSpecularIntensity(mix(LIGHTCOL_SUNSET_SPECULAR, light->getDefaultSpecularIntensity(), sunSetCoeff));

            sunColor = mix(SUN_COLOR_SUNSET, SUN_COLOR, sunSetCoeff);
            domeGradBottom = mix(SUNSETGRADIENT_START, DAYGRADIENT_START, sunSetCoeff);
            domeGradTop = mix(SUNSETGRADIENT_END, DAYGRADIENT_END, sunSetCoeff);
        }
        // Day
        else {
            bottomSkyColor = mistColor;
            topSkyColor = blueSkyColor;
            light->resetDiffuseIntensity();
            light->resetSpecularIntensity();
        }

        //Nuance the sky's color gradient based on altitude
        if(viewPos.y > outerSpaceBegin){
            float scatteringFade = (viewPos.y - outerSpaceBegin)/(outerSpaceEnd - outerSpaceBegin);
            scatteringFade = Utils::expDeceleratingTransition(scatteringFade);

            bottomSkyColor = mix(bottomSkyColor, OUTERSPACECOL_bottomSky, scatteringFade);
            topSkyColor = mix(topSkyColor, OUTERSPACECOL_topSky, scatteringFade);
        }

    }
};
