#ifndef __CAMERA_H
#define __CAMERA_H

#include "matrix4x4f.h"

class Camera {
    public:
	vector3f at;	//camera position
    vector3f to;
	vector3f u;		//x axis
	vector3f v;		//y axis
	vector3f n;		//z axis
	double yaw;	    //rotation in v axis
	double pitch;	//rotation in u axis
	double roll;	//rotation in n axis

	Camera();
	Camera(vector3f at, vector3f to);
	~Camera();
	
	void rotate();
	void translate(vector3f translation);
    void moveForward(double speed, double dt);
};

#endif
