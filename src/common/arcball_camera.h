#ifndef _ARCBALL_CAMERA_2018_12_16_H_
#define _ARCBALL_CAMERA_2018_12_16_H_

#include "glm_usage.h"
using glm::mat4;
using glm::vec3;
using glm::ivec4;

//
//
//

struct Camera
{
    ivec4 viewport = ivec4(0, 0, 640, 480);

    vec3 target = vec3(0, 0, 0);
    vec3 eye = glm::vec3(0.0f, 0.0f, 3.0f);
    float near_plane = 1.0f;
    float far_plane = 5.0f;
    float fovy_degrees = 45.0f;
    int _mx = 320;
    int _my = 240;

    mat4 proj;
    mat4 view;

    virtual void translate(const glm::vec3 &);
    virtual void mouse_click(int mx, int my);
    virtual void mouse_move(int mx, int my);
    virtual void update();
};

//
//
//

struct FpsCamera : Camera
{
    vec3 dir = vec3(0,0,-1);

    void mouse_click(int mx, int my) override;
    void mouse_move(int mx, int my) override;
    void update() override;
};

//
//
//

struct ArcballCamera : Camera
{
    mat4 _quat = mat4(1);
    mat4 _last = mat4(1);
    mat4 _next = mat4(1);

    // the distance from the origin to the eye
    float _zoom = 1.0f;
    float _zoom2 = 1.0f;
    // the radius of the arcball
    float _sphere = 1.0f;
    float _sphere2 = 1.0f;
    // the distance from the origin of the plane that intersects
    // the edge of the visible sphere (tangent to a ray from the eye)
    float _edge = 1.0f;
    // whether we are using a sphere or plane
    bool _planar = false;
    float _planedist = 0.5f;

    vec3 _start = vec3(0, 0, 1);
    vec3 _curr = vec3(0, 0, 1);
    vec3 _eyedir = vec3(0, 0, 1);
    vec3 _up = vec3(0, 1, 0);
    vec3 _out = vec3(1, 0, 0);

    void setup(float radius);

    void reset();

    // on click (grab a point on sphere)
    void mouse_click(int mx, int my) override;

    // on move the grabbed point of the sphere.
    void mouse_move(int mx, int my) override;


    void update() override;


private:
    
    vec3 edge_coords(vec3 m);
    vec3 sphere_coords(int mx, int my);
    vec3 planar_coords(int mx, int my);
};


#endif //_ARCBALL_CAMERA_2018_12_16_H_
