#include "arcball_camera.h"

//
//
//

void Camera::update()
{
    proj = glm::perspective(fovy_degrees, viewport[2] / (float)viewport[3], near_plane, far_plane); // TODO: update only if dirty

    view = glm::lookAt(eye, target, glm::vec3(0, 1, 0));
}

void Camera::translate(const glm::vec3 &t)
{
    // TODO: camera speed
    float speed = 0.1f;
    eye += speed * t;
    target += speed * t;
}

void Camera::mouse_click(int dx, int dy)
{

}

void Camera::mouse_move(int mx, int my)
{
    
}

//
//
//

void FpsCamera::mouse_click(int mx, int my)
{
    _mx = mx;
    _my = my;
}

void FpsCamera::mouse_move(int mx, int my)
{
    int dx = mx - _mx;
    int dy = my - _my;

    if (dx == 0 || dy == 0)
        return;

    glm::mat4 rot(1);
    rot = glm::rotate(rot, dx / 100.0f, glm::vec3(0, 1, 0));
    rot = glm::rotate(rot, -dy / 100.0f, glm::vec3(1, 0, 0));

    dir = (glm::vec4(dir,1) * rot).xyz;

    _mx = mx;
    _my = my;
}

void FpsCamera::update()
{
    target = eye + dir;

    Camera::update();
}

//
//
//

void ArcballCamera::update()
{
    Camera::update();

    // Apply the arcball rotation on top of the normal camera view matrix.
    //view = view * _quat;
    view = view  * _quat;
}

void ArcballCamera::setup(float radius)
{
    _zoom2 = glm::dot(eye, eye);
    _zoom = sqrt(_zoom2); // store eye distance
    _sphere = radius; // sphere radius
    _sphere2 = _sphere * _sphere;
    _eyedir = eye * (1.0f / _zoom); // distance to eye
    _edge = _sphere2 / _zoom; // plane of visible edge

    if (_sphere <= 0.0) // trackball mode
    {
        _planar = true;
        _up = glm::vec3(0,1,0);
        _out = glm::cross(_eyedir, _up); //(_eyedir ^ _up);
        _planedist = (0.0f - _sphere) * _zoom;
    }
    else
    {
        _planar = false;
    }
}

void ArcballCamera::reset()
{

}

void ArcballCamera::mouse_click(int mx, int my)
{
    // saves a copy of the current rotation for comparison
    _last = _quat;
    if (_planar)
    {
        _start = planar_coords(mx, my);
    }
    else
    {
        _start = sphere_coords(mx, my);
    }
}

void ArcballCamera::mouse_move(int mx, int my)
{
    if (_planar)
    {
        _curr = planar_coords(mx, my);
        if (_curr == _start) return;

        // d is motion since the last position
        vec3 d = _curr - _start;

        float angle = d.length() * 0.5f;
        float cosa = cos(angle);
        float sina = sin(angle);
        // p is perpendicular to d
        vec3 p = glm::normalize((_out*d.x) - (_up*d.y)) * sina;

        _next = glm::toMat4(glm::quat(cosa, p.x, p.y, p.z));
        _quat = _last * _next;
        
        // planar style only ever relates to the last point
        _last = _quat;
        _start = _curr;
    }
    else 
    {
        _curr = sphere_coords(mx, my);
        if (_curr == _start)
        { // avoid potential rare divide by tiny
            _quat = _last;
            return;
        }

        // use a dot product to get the angle between them
        // use a cross product to get the vector to rotate around
        float cos2a = glm::dot(_start, _curr);
        float sina = sqrt((1.0f - cos2a)*0.5f);
        float cosa = sqrt((1.0f + cos2a)*0.5f);
        vec3 cross = glm::normalize(glm::cross(_start, _curr)) * sina;
        
        _next = glm::toMat4(glm::quat(cosa, cross.x, cross.y, cross.z));
        // update the rotation matrix
        _quat = _last * _next;
    }
}

// find the intersection with the plane through the visible edge
vec3 ArcballCamera::edge_coords(vec3 m)
{
    // find the intersection of the edge plane and the ray
    float t = (_edge - _zoom) / glm::dot(_eyedir, m);
    vec3 a = eye + (m*t);
    // find the direction of the eye-axis from that point
    // along the edge plane
    vec3 c = (_eyedir * _edge) - a;

    // find the intersection of the sphere with the ray going from
    // the plane outside the sphere toward the eye-axis.
    float ac = glm::dot(a, c);
    float c2 = glm::dot(c, c);
    float q = (0.0f - ac - sqrt(ac*ac - c2 * (glm::dot(a,a) - _sphere2))) / c2;

    return glm::normalize(a + (c*q));
}

// find the intersection with the sphere
vec3 ArcballCamera::sphere_coords(int mx, int my)
{
    vec3 point = glm::unProject(vec3(mx, my, 0.0f), mat4(1.0f), proj, viewport);
    vec3 m = point - eye;

    // mouse position represents ray: eye + t*m
    // intersecting with a sphere centered at the origin
    float a = glm::dot(m, m);
    float b = glm::dot(eye, m);
    float root = (b*b) - a * (_zoom2 - _sphere2);
    if (root <= 0.0)
    {
        return edge_coords(m);
    }
    float t = (0.0f - b - sqrt(root)) / a;
    return glm::normalize(eye + (m*t));
}

// get intersection with plane for "trackball" style rotation
vec3 ArcballCamera::planar_coords(int mx, int my)
{
    vec3 point = glm::unProject(vec3(mx, my, 0.0f), mat4(1.0f), proj, viewport);
 
    vec3 m = point - eye;
    // intersect the point with the trackball plane
    float t = (_planedist - _zoom) / glm::dot(_eyedir, m);
    vec3 d = eye + m * t;

    return vec3(glm::dot(d,_up), glm::dot(d,_out), 0.0f);
}

