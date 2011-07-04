#include "camera.h"

Camera::Camera(){
    u.x = 1; u.y = 0; u.z = 0;
    v.x = 0; v.y = 1; v.z = 0;
    n.x = 0; n.y = 0; n.z = 1;

    yaw = pitch = roll = 0;
    at = vector3f(0,0,0);
    to = at + n;
}

Camera::Camera(vector3f at, vector3f to) {
    n = to - at;
    n.normalize();
        
    vector3f n_aux(n.x, n.y-1, n.z);
    u = vector3f::crossProduct(n_aux,n);
    u.normalize();
    v = vector3f::crossProduct(u,n);
    v.normalize();
}

Camera::~Camera() {
}
	
void Camera::rotate() {
    n = to - at;
    n.normalize();
        
    // n_aux is in the plane defined by n and v
    vector3f n_aux(n.x, n.y-1, n.z);
    u = vector3f::crossProduct(n_aux,n);
    u.normalize();
    v = vector3f::crossProduct(u,n);
    v.normalize();
        
    //rotation in x
    matrix4x4f rx;
    rx.rotate(pitch, vector3f(1,0,0));
    rx.transformVector(&n);
    rx.transformVector(&v);
            
    //rotation in y
    matrix4x4f ry;
    ry.rotate(yaw, vector3f(0,1,0));
//    ry.rotate(yaw, v);
    ry.transformVector(&n);
    ry.transformVector(&u);

    //rotation in z
    matrix4x4f rz;
    rz.rotate(roll, vector3f(0,0,1));
    rz.transformVector(&v);
    rz.transformVector(&u);

}

void Camera::translate(vector3f translation) {
    at += translation;
}

void Camera::moveForward(double speed, double dt) {
    // euler method and motion equations (s = si + vi*dt + a(dt)^2/2) for 
    // physics simulation (velocity, acceleration, gravity,...)
    at.x += speed*n.x*dt;
    at.y += speed*n.y*dt - 0.000983*dt*dt/2;
    at.z += speed*n.z*dt;
    to.x += speed*n.x*dt;
    to.y += speed*n.y*dt - 0.000983*dt*dt/2;
    to.z += speed*n.z*dt;
}
