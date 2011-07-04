#ifndef _MAP_H
#define _MAP_H

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <stdint.h>

class Map {
    public:
    SDL_Surface *map_img;
    float xi, zi, xf, zf; // bounding rectangle used to generate the map image
    // default is 5.0 width/height

    Map();
    Map(float xi, float xf, float zi, float zf, char image[]);
    ~Map();

    int getYFromImage(int x, int z);
    Uint32 getPixel32(int x, int z);
};

#endif
