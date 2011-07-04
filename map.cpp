#include "map.h"

Map::Map() {
    map_img = NULL;
    xi = zi = 0.0;
    xf = zf = 5.0;
}

Map::Map(float xi_, float xf_, float zi_, float zf_, char image[]) {
    map_img = SDL_LoadBMP(image);
    xi = xi_; zi = zi_;
    xf = xf_; zf = zf_;
}

Map::~Map() {
}

Uint32 Map::getPixel32(int x, int z) {
    Uint32 *pixels = (Uint32*)map_img->pixels;
    return pixels[(z*map_img->w)+x];
}

int Map::getYFromImage(int x, int z) {
    Uint8 r, g, b;
    SDL_GetRGB(getPixel32(x, z), map_img->format, &r, &g, &b);
    return (int)(((r+g+b)*2)/255);
    
}

