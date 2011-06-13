#include "map.h"

Map::Map() {
    map_img = NULL;
    xi = 0.0f;
    yi = 0.0f;
}

Map::Map(GLfloat x_, GLfloat y_, char image[]) {
    map_img = SDL_LoadBMP(image);
    xi = x_;
    yi = y_;
}

Map::~Map() {
}

void Map::initMat() {
    for (int i = 0; i < XX; i++) 
        for (int j = 0; j < YY; j++)
            for (int k = 0; k < ZZ; k++)
                mat[i][j][k] = 0;
}


Uint32 Map::getPixel32(int x, int z) {
    Uint32 *pixels = (Uint32*)map_img->pixels;
    return pixels[(z*map_img->w)+x];
}

void Map::fillMatXYZ(int x, int y, int z) {
    for (int j = 0; j < y; j++)
        mat[x][j][z] = 1;
}

int Map::getYFromImage(int x, int z) {
    Uint8 r, g, b;
    SDL_GetRGB(getPixel32(x, z), map_img->format, &r, &g, &b);
    return (int)(((r+g+b)*10.6)/255);
}

void Map::fillMat() {
    initMat();
    for (int i = 0; i < XX; i++) {
        for (int j = 0; j < ZZ; j++) {
            int y = getYFromImage(i, j);
            fillMatXYZ(i, y, j);
        }
    }
}

bool Map::has26Neighbors(int x, int y, int z) {
    return (mat[x-1][y-1][z-1] && mat[x-1][y-1][z] && mat[x-1][y-1][z+1] &&
            mat[x-1][y][z-1] && mat[x-1][y][z] && mat[x-1][y][z+1] &&
            mat[x-1][y+1][z-1] && mat[x-1][y+1][z] && mat[x-1][y+1][z+1] &&
            mat[x][y-1][z-1] && mat[x][y-1][z] && mat[x][y-1][z+1] &&
            mat[x][y][z-1] && mat[x][y][z+1] &&  
            mat[x][y+1][z-1] && mat[x][y+1][z] && mat[x][y+1][z+1] &&
            mat[x+1][y-1][z-1] && mat[x+1][y-1][z] && mat[x+1][y-1][z+1] &&
            mat[x+1][y][z-1] && mat[x+1][y][z] && mat[x+1][y][z+1] &&
            mat[x+1][y+1][z-1] && mat[x+1][y+1][z] && mat[x+1][y+1][z+1]);
}

// this functions simply removes cubes that won't be
// seen by the player, that is, cubes that have all 26 neighbors...
void Map::optimizeMat() {
    for (int i = 0; i < XX; i++) {
        for (int j = 0; j < YY; j++) {
            for (int k = 0; k < ZZ; k++) {
                if (mat[i][j][k] == 1) {
                    // can't remove cubes on the borders of the chunk
                    if ((i != 0) && (i != XX-1) && (j != 0) && (j != YY-1) && (k != 0) && (k != ZZ-1))
                        if (has26Neighbors(i,j,k))
                            mat[i][j][k] = 2;
                }
            }
        }
    }
}

