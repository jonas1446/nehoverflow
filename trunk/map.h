#ifndef _MAP_H
#define _MAP_H

#define XX 64 
#define YY 32
#define ZZ 64

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <stdint.h>

// A map is a matrix of dimensions xyz.
// The contents of this matrix are (for now) either 0 or 1. 
// 1 = (1x1x1) cube, 0 = nothing.

// The map is dinamically generated using 3D Perlin noise. 
// An image (BMP) is generated where width and height represent the x and z plane
// and the intensity of the colors represent the y plane.
// 256x256 image is the default, where every pixel is a position in the matrix. 

class Map {
    public:
    SDL_Surface *map_img;
    GLfloat xi, yi;
    int mat[XX][YY][ZZ];

    Map();
    Map(GLfloat x_, GLfloat y_, char image[]);
    ~Map();

    void initMat();
    void fillMatXYZ(int x, int y, int z);
    int getYFromImage(int x, int z);
    Uint32 getPixel32(int x, int z);
    void fillMat(); 
    bool has26Neighbors(int x, int y, int z);
    void optimizeMat();
    void createNoiseMap();


};

#endif
