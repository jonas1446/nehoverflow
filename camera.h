#ifndef __CAMERA_H
#define __CAMERA_H

#include "vector3f.h"

class Camera {
	vector3f at;	//camera position
	vector3f u;		//x axis
	vector3f v;		//y axis
	vector3f n;		//z axis
	double	 yaw;	//rotation in v axis
	double	 pitch;	//rotation in u axis
	double	 roll;	//rotation in n axis

	Camera();
	Camera(vector3f at, vector3f u, vector3f v, vector3f n, double yaw, double pitch, double roll);
	~Camera();
	
	void rotate(double yaw, double pitch, double roll);
	void translate(vector3f translation);
	void moveForward(double power);
};


#endif
