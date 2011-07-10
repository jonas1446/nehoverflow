#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cmath>
#include <time.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
//#include <noise/noise.h>

#include "timer.h"
#include "camera.h"
#include "map.h"
//#include "noiseutils.h"

#include "lib/glm.h"
#include "lib/imageloader.h"
#include "lib/Texture.h"

#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480
#define SCREEN_BPP      16

#define TRUE    1
#define FALSE   0

#define MAXFPS  60

#define NPYR    200
using namespace std;

GLMmodel* pmodel1 = NULL;

SDL_Surface *surface;
Camera camera1, camera2, plane_camera1, plane_camera2, plane_camera3, plane_camera4;
Timer __time, delta;

float planeix = 0;
float planeiz = 3;

int light = FALSE;
int blend = FALSE;
int ratio;
int frame = 0;
int w1x = 0;
int w1y = 0;
int tmodifier = 1;
int boost = 1;
float npoffset = 5.0;
int n_camera = 0;

float speed = 0.0f;
float acel = 0.0001f;

GLfloat LightAmbient[] = {0.5f, 0.5f, 0.5f, 1.0f};
GLfloat LightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat LightPosition[] = {0.0f, 0.0f, 2.0f, 1.0f};

int exp_frame, exp_x, exp_y, exp_z;

GLuint filter;
GLuint texture[7];
GLuint bottom, top, front, back, lefta, righta;
GLuint chunk[4];

GLfloat fogColor[4] = {0.8f, 0.8f, 0.3f, 0.2f};
    
Map sector1(0.0, 5.0, 0.0, 5.0, "sector1.bmp");

typedef struct PYR {
    float x, y, z; // pyramids x,y,z -> y being the center, not top
    float height, width;
} pyramid;

pyramid pyramids[NPYR];
int lpl = 0;
//GLuint plane_texture;
Texture treeTexture;

// if number x is close to number y
bool nearr(int nx, int ny) {
    if (15 < abs(nx-ny)) return false;
    else return true;
}

bool verifyrand(int rx, int rz) {
    for (int i = 0; i < lpl; i++) {
        if (nearr(pyramids[i].x, rx)) { 
            if (nearr(pyramids[i].z, rz)) return false;
        }

        if (nearr(pyramids[i].z, rz)) {
            if (nearr(pyramids[i].x, rx)) return false;
        }
    }
    return true;
}

void createPyramid() {
    srand(time(NULL));

    lpl++;

    int randx = rand() % 2900 + 20;
    int randy = rand() % 40 + 10;
    int randz = rand() % 2900 + 20;
    while (!verifyrand(randx, randz)) { randx = rand() % 2900 + 20; randz = rand() % 2900 + 20; }

    pyramids[lpl-1].x = randx; 
    pyramids[lpl-1].y = randy;
    pyramids[lpl-1].z = randz;
    pyramids[lpl-1].height = 2*randy;
    pyramids[lpl-1].width = 2*2*randy*0.4;
}

void createPyramids() {
    int i;
    for (i = 0; i < NPYR; i++) { createPyramid(); printf("%d, ", pyramids[i].x); } 
}

bool LoadTreeTextures() {

    // first of all we call the tga file loader. It doesn't do anything special: it fills the Texture struct with information about
    // the image (height, width, bits per pixel). You can easily replace it with a function to load bmps or jpegs.
    // The important thing is do load the image corectly in the structure you give it
    if (LoadTGA(&treeTexture, "airplane_01_body1.tga"))
    {

        // This tells opengl to create 1 texture and put it's ID in the given integer variable
        // OpenGL keeps a track of loaded textures by numbering them: the first one you load is 1, second is 2, ...and so on.
        glGenTextures(1, &treeTexture.texID);
        // Binding the texture to GL_TEXTURE_2D is like telling OpenGL that the texture with this ID is now the current 2D texture in use
        // If you draw anything the used texture will be the last binded texture
        glBindTexture(GL_TEXTURE_2D, treeTexture.texID);
        // This call will actualy load the image data into OpenGL and your video card's memory. The texture is allways loaded into the current texture
        // you have selected with the last glBindTexture call
        // It asks for the width, height, type of image (determins the format of the data you are giveing to it) and the pointer to the actual data
        glTexImage2D(GL_TEXTURE_2D, 0, treeTexture.bpp / 8, treeTexture.width, treeTexture.height, 0, treeTexture.type, GL_UNSIGNED_BYTE, treeTexture.imageData);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glEnable(GL_TEXTURE_2D);
        if (treeTexture.imageData)
        {

            // You can now free the image data that was allocated by LoadTGA
            // You don't want to keep a few Mb of worthless data on heap. It's worthless because OpenGL stores the image someware else after
            // you call glTexImage2D (usualy in you video card)
            free(treeTexture.imageData); 

        }

        return true;

    } // Return The Status
    else return false;

}

void drawmodel_box(float x, float y, float z) {

    // Load the model only if it hasn't been loaded before
    // If it's been loaded then pmodel1 should be a pointer to the model geometry data...otherwise it's null
    if (!pmodel1) {
        // this is the call that actualy reads the OBJ and creates the model object
        pmodel1 = glmReadOBJ("airplane.obj");
        if (!pmodel1) {
        	exit(0);
        }
        // This will rescale the object to fit into the unity matrix
        // Depending on your project you might want to keep the original size and positions you had in 3DS Max or GMAX so you may have to comment this.
        glmUnitize(pmodel1);
        // These 2 functions calculate triangle and vertex normals from the geometry data.
        // To be honest I had some problem with very complex models that didn't look to good because of how vertex normals were calculated
        // So if you can export these directly from you modeling tool do it and comment these line
        // 3DS Max can calculate these for you and GLM is perfectly capable of loading them
        glmFacetNormals(pmodel1);
        glmVertexNormals(pmodel1, 90.0);

    }
    // This is the call that will actually draw the model
    // Don't forget to tell it if you want textures or not :))
 	glScalef(2,2,2);
 	// before you draw anything you should bind the right texture
	// If you have more then one texture you will have to make sure you bind the right one
	glBindTexture(GL_TEXTURE_2D, treeTexture.texID);
    glmDraw(pmodel1, GLM_SMOOTH| GLM_TEXTURE);
} 

void quit(int returnCode) {
    SDL_Quit();
    exit(returnCode);
}

/*
void createNoiseMap(char img[], int width, int height, float xi, float xf, float zi, float zf) {
    module::Perlin myModule;
    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderPlane heightMapBuilder;
    heightMapBuilder.SetSourceModule(myModule);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    heightMapBuilder.SetDestSize(width, height);
    heightMapBuilder.SetBounds(xi, xf, zi, zf);
    heightMapBuilder.Build();

    utils::RendererImage renderer;
    utils::Image image;
    renderer.SetSourceNoiseMap(heightMap);
    renderer.SetDestImage(image);
   
     
    renderer.ClearGradient();
    renderer.AddGradientPoint(-1.0000, utils::Color(  0,   0, 128, 255));
    renderer.AddGradientPoint(-0.2500, utils::Color(  0,   0, 255, 255));
    renderer.AddGradientPoint( 0.0000, utils::Color(  0, 128, 255, 255));
    renderer.AddGradientPoint( 0.0625, utils::Color(240, 240,  64, 255));
    renderer.AddGradientPoint( 0.1250, utils::Color( 32, 160,   0, 255));
    renderer.AddGradientPoint( 0.3750, utils::Color(224, 224,   0, 255));
    renderer.AddGradientPoint( 0.7500, utils::Color(128, 255, 128, 255));
    renderer.AddGradientPoint( 1.0000, utils::Color(255, 255, 255, 255));
    
    
    renderer.Render();

    utils::WriterBMP writer;
    writer.SetSourceImage(image);
    writer.SetDestFilename(img);
    writer.WriteDestFile();
}
*/

GLvoid buildLists() {
    chunk[0] = glGenLists(1);
    
    /*
    bottom = glGenLists(1);
    top = glGenLists(1);
    front = glGenLists(1);
    back = glGenLists(1);
    lefta = glGenLists(1);
    righta = glGenLists(1);

    glNewList(bottom, GL_COMPILE); // bottom
    glBindTexture(GL_TEXTURE_2D, texture[6]);
    glBegin(GL_QUADS); 
        glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, 0.0f, 1.0f);
    glEnd();
    glEndList();

    glNewList(back, GL_COMPILE); // back
    glBindTexture(GL_TEXTURE_2D, texture[6]);
    glBegin(GL_QUADS); 
        glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 0.0f, 1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 1.0f, 1.0f);
    glEnd();
    glEndList();

    glNewList(front, GL_COMPILE); // front
    glBindTexture(GL_TEXTURE_2D, texture[6]);
    glBegin(GL_QUADS);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f, 0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 0.0f, 0.0f);
    glEnd();
    glEndList();

    glNewList(righta, GL_COMPILE); // right
    glBindTexture(GL_TEXTURE_2D, texture[6]);
    glBegin(GL_QUADS);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 0.0f, 1.0f);
    glEnd();
    glEndList();

    glNewList(lefta, GL_COMPILE); // left:
    glBindTexture(GL_TEXTURE_2D, texture[6]);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, 0.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 1.0f, 0.0f);
    glEnd();
    glEndList();

    glNewList(top, GL_COMPILE); // top
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glBegin(GL_QUADS);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, 0.0f);
    glEnd();
    glEndList();
    */

//  createNoiseMap("sector1.bmp", 512, 512, 0.0, 5.0, 0.0, 5.0);
    Map sector1(0.0, 5.0, 0.0, 5.0, "sector1.bmp");
   	
    glNewList(chunk[0], GL_COMPILE);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    int cur_y1, cur_y2, cur_y3, cur_y4;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            cur_y1 = sector1.getYFromImage(i*16, j*16);
            cur_y2 = sector1.getYFromImage(16+i*16, j*16);
            cur_y3 = sector1.getYFromImage(i*16, 16+j*16);
            cur_y4 = sector1.getYFromImage(16+i*16, 16+j*16);
            glBegin(GL_TRIANGLES);
                glTexCoord2f(1.0f, 1.0f); glVertex3f(i*16, cur_y1, j*16);
                glTexCoord2f(1.0f, 0.0f); glVertex3f(16+i*16, cur_y2, j*16);
                glTexCoord2f(0.0f, 0.0f); glVertex3f(i*16, cur_y3, 16+j*16);
            glEnd();

            glBegin(GL_TRIANGLES);
                glTexCoord2f(1.0f, 1.0f); glVertex3f(16+i*16, cur_y4, 16+j*16);
                glTexCoord2f(1.0f, 0.0f); glVertex3f(16+i*16, cur_y2, j*16);
                glTexCoord2f(0.0f, 0.0f); glVertex3f(i*16, cur_y3, 16+j*16);
            glEnd();

        }
    }
    glEndList();

    /* int cur_y, cur_y_back, cur_y_front, cur_y_right, cur_y_left;

    glNewList(chunk[0], GL_COMPILE);
    for (int i = 1; i < 128; i++) {
        glTranslatef(i, 0.0f, 0.0f);
        for (int j = 1; j < 128; j++) {
            cur_y = sector1.getYFromImage(i, j);
            printf("%d, ", cur_y);
            glTranslatef(0.0f, cur_y, j);

            cur_y_back = sector1.getYFromImage(i, j+1);
            if (cur_y > cur_y_back) {
                for (int k = cur_y; k > cur_y_back+1; k--) {
                    glTranslatef(0.0f, -1.0f, 0.0f);
                    glCallList(back);
                }
                glTranslatef(0.0f, cur_y-cur_y_back-1, 0.0f);
                glCallList(back);
            }

            cur_y_front = sector1.getYFromImage(i, j-1);
            if (cur_y > cur_y_front) {
                for (int k = cur_y; k > cur_y_front+1; k--) {
                    glTranslatef(0.0f, -1.0f, 0.0f);
                    glCallList(front);
                }
                glTranslatef(0.0f, cur_y-cur_y_front-1, 0.0f);
                glCallList(front);
            }

            cur_y_right = sector1.getYFromImage(i+1, j);
            if (cur_y > cur_y_right) {
                for (int k = cur_y; k > cur_y_right+1; k--) {
                    glTranslatef(0.0f, -1.0f, 0.0f);
                    glCallList(righta);
                }
                glTranslatef(0.0f, cur_y-cur_y_right-1, 0.0f);
                glCallList(righta);
            }

            cur_y_left = sector1.getYFromImage(i-1, j);
            if (cur_y > cur_y_left) {
                for (int k = cur_y; k > cur_y_left+1; k--) {
                    glTranslatef(0.0f, -1.0f, 0.0f);
                    glCallList(lefta);
                }
                glTranslatef(0.0f, cur_y-cur_y_left-1, 0.0f);
                glCallList(lefta);
            }

            glCallList(top);
            glTranslatef(0.0f, -cur_y, -j); 
        }
        glTranslatef(-i, 0.0f, 0.0f);
    }
    glEndList();
    */
}
        

int loadGLTextures() {
    int status = FALSE;

    SDL_Surface *textureImage[7];

    if ((textureImage[0] = SDL_LoadBMP("gfx/sand3.bmp"))) {
        status = TRUE;
        
        glGenTextures(1, &texture[0]);
        glBindTexture(GL_TEXTURE_2D, texture[0]);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, textureImage[0]->w, textureImage[0]->h,
                        GL_BGR, GL_UNSIGNED_BYTE, textureImage[0]->pixels);    
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    }

    if ((textureImage[1] = SDL_LoadBMP("gfx/top.bmp"))) {
        status = TRUE;

        glGenTextures(1, &texture[1]);
        glBindTexture(GL_TEXTURE_2D, texture[1]);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, textureImage[1]->w, textureImage[1]->h,
                        GL_BGR, GL_UNSIGNED_BYTE, textureImage[1]->pixels);    
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        
    }

    if ((textureImage[2] = SDL_LoadBMP("gfx/right.bmp"))) {
        status = TRUE;

        glGenTextures(1, &texture[2]);
        glBindTexture(GL_TEXTURE_2D, texture[2]);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, textureImage[2]->w, textureImage[2]->h,
                        GL_BGR, GL_UNSIGNED_BYTE, textureImage[2]->pixels);    
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    } 

    if ((textureImage[3] = SDL_LoadBMP("gfx/left.bmp"))) {
        status = TRUE;

        glGenTextures(1, &texture[3]);
        glBindTexture(GL_TEXTURE_2D, texture[3]);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, textureImage[3]->w, textureImage[3]->h,
                        GL_BGR, GL_UNSIGNED_BYTE, textureImage[3]->pixels);    
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    } 


    if ((textureImage[4] = SDL_LoadBMP("gfx/front.bmp"))) {
        status = TRUE;

        glGenTextures(1, &texture[4]);
        glBindTexture(GL_TEXTURE_2D, texture[4]);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, textureImage[4]->w, textureImage[4]->h,
                        GL_BGR, GL_UNSIGNED_BYTE, textureImage[4]->pixels);    
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    } 

    if ((textureImage[5] = SDL_LoadBMP("gfx/back.bmp"))) {
        status = TRUE;

        glGenTextures(1, &texture[5]);
        glBindTexture(GL_TEXTURE_2D, texture[5]);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, textureImage[5]->w, textureImage[5]->h,
                        GL_BGR, GL_UNSIGNED_BYTE, textureImage[5]->pixels);    
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    } 

    if ((textureImage[6] = SDL_LoadBMP("gfx/pyramid.bmp"))) {
        status = TRUE;

        glGenTextures(1, &texture[6]);
        glBindTexture(GL_TEXTURE_2D, texture[6]);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, textureImage[6]->w, textureImage[6]->h,
                        GL_BGR, GL_UNSIGNED_BYTE, textureImage[6]->pixels);    
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } 

    if (textureImage[0]) SDL_FreeSurface(textureImage[0]);
    if (textureImage[1]) SDL_FreeSurface(textureImage[1]);
    if (textureImage[2]) SDL_FreeSurface(textureImage[2]);
    if (textureImage[3]) SDL_FreeSurface(textureImage[3]);
    if (textureImage[4]) SDL_FreeSurface(textureImage[4]);
    if (textureImage[5]) SDL_FreeSurface(textureImage[5]);
    if (textureImage[6]) SDL_FreeSurface(textureImage[6]);

    return status;
}

void renderSky() {
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    // front
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex3f(  0.5f, -0.5f, -0.5f );
        glTexCoord2f(1, 1); glVertex3f( -0.5f, -0.5f, -0.5f );
        glTexCoord2f(1, 0); glVertex3f( -0.5f,  0.5f, -0.5f );
        glTexCoord2f(0, 0); glVertex3f(  0.5f,  0.5f, -0.5f );
    glEnd();
 
    // left
    glBindTexture(GL_TEXTURE_2D, texture[3]);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex3f(  0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 1); glVertex3f(  0.5f, -0.5f, -0.5f );
        glTexCoord2f(1, 0); glVertex3f(  0.5f,  0.5f, -0.5f );
        glTexCoord2f(0, 0); glVertex3f(  0.5f,  0.5f,  0.5f );
    glEnd();
 
    // back
    glBindTexture(GL_TEXTURE_2D, texture[5]); 
    glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex3f( -0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 1); glVertex3f(  0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 0); glVertex3f(  0.5f,  0.5f,  0.5f );
        glTexCoord2f(0, 0); glVertex3f( -0.5f,  0.5f,  0.5f );
    glEnd();
 
    // right
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex3f( -0.5f, -0.5f, -0.5f );
        glTexCoord2f(1, 1); glVertex3f( -0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 0); glVertex3f( -0.5f,  0.5f,  0.5f );
        glTexCoord2f(0, 0); glVertex3f( -0.5f,  0.5f, -0.5f );
    glEnd();
 
    // top
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glBegin(GL_QUADS);
        glTexCoord2f(1, 1); glVertex3f( -0.5f,  0.5f, -0.5f );
        glTexCoord2f(1, 0); glVertex3f( -0.5f,  0.5f,  0.5f );
        glTexCoord2f(0, 0); glVertex3f(  0.5f,  0.5f,  0.5f );
        glTexCoord2f(0, 1); glVertex3f(  0.5f,  0.5f, -0.5f );
    glEnd();
 
    // bottom
    glBindTexture(GL_TEXTURE_2D, texture[1]); 
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f( -0.5f, -0.5f, -0.5f );
        glTexCoord2f(0, 1); glVertex3f( -0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 1); glVertex3f(  0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 0); glVertex3f(  0.5f, -0.5f, -0.5f );
    glEnd();

    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
}

int resizeWindow(int w, int h) {
    if (h == 0) h = 1;

    ratio = (GLfloat)w/(GLfloat)h;

    glViewport(w1x, w1y, (GLint)w, (GLint)h); 

    return TRUE;
}

void handleKeyPress() {
    Uint8* keys = SDL_GetKeyState(NULL); 

    if (keys[SDLK_ESCAPE]) 
        quit(0);

    if (keys[SDLK_f]) 
        filter = (++filter) % 3;

    
    if (keys[SDLK_b]) {
        blend = !blend;
        if (blend) { 
            glEnable(GL_BLEND); 
            glDisable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_BLEND); 
            glEnable(GL_DEPTH_TEST); 
        } 
    }

    if (keys[SDLK_l]) {
        light = !light;
        if (!light) glDisable(GL_LIGHTING);
        else glEnable(GL_LIGHTING);
    }

    if (keys[SDLK_LSHIFT]) {
        tmodifier = 3;
    }

    if (keys[SDLK_LCTRL]) {
        boost = 10;
    }
    
    if (keys[SDLK_w]) {
        if (speed <= 0.3f) speed += boost*acel;
    }

    if (keys[SDLK_s]) { 
        if (speed > 0) speed -= boost*acel;
    }

    if (keys[SDLK_i])
        camera2.moveForward(speed, delta.get_ticks());

    if (keys[SDLK_k])
        camera2.moveForward(-speed, delta.get_ticks());

    if (keys[SDLK_j]) {
        camera2.yaw += 3;
        camera2.rotate();
    }
    
    if (keys[SDLK_l]) {
        camera2.yaw -= 3;
        camera2.rotate();
    }

    if (keys[SDLK_g]) {
        camera2.pitch -= 3;
        camera2.rotate();
    }

    if (keys[SDLK_b]) {
        camera2.pitch += 3;
        camera2.rotate();
    }

    if (keys[SDLK_UP]) {
        camera1.pitch -= 0.3f*tmodifier;
        camera1.rotate();
    }

    if (keys[SDLK_DOWN]) {
        camera1.pitch += 0.3f*tmodifier;
        camera1.rotate();
    }

    if (keys[SDLK_RIGHT]) {
        camera1.yaw -= 0.3f*tmodifier;
        camera1.rotate();
    }

    if (keys[SDLK_LEFT]) {
        camera1.yaw += 0.3f*tmodifier;
        camera1.rotate();
    }

	if (keys[SDLK_e]) {
        camera1.roll += 1;
        camera1.rotate();
    }

    if (keys[SDLK_r]) {
        camera1.roll -= 1;
        camera1.rotate();
    }

    if (keys[SDLK_n]) {
        camera1.at.x = 5.0f;
        camera1.at.y = 0.5f;
        camera1.at.z = 5.0f;
        camera1.n.x = 0.0f;
        camera1.n.y = 0.0f;
        camera1.n.z = 1.0f;
        camera1.to = camera1.at + camera1.n;
        speed = 0.0f;
        acel = 0.0001f;
    }
    
    if (keys[SDLK_0]) n_camera = 0;
    if (keys[SDLK_1]) n_camera = 1;
    if (keys[SDLK_2]) n_camera = 2;
    if (keys[SDLK_3]) n_camera = 3;
    if (keys[SDLK_4]) n_camera = 4;


    if (keys[SDLK_F1])
        SDL_WM_ToggleFullScreen(surface); 

    tmodifier = 1;
    boost = 1;
}


int initGL() {
    if (!loadGLTextures()) return FALSE;
     
    buildLists();
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.35f);
    glFogf(GL_FOG_START, 0.5f);
    glFogf(GL_FOG_END, 10.0f);
    //glEnable(GL_FOG);
    createPyramids();
    
    if(!LoadTreeTextures()) {
    	cout<<"texture success\n";
    }

    return TRUE;
}

void drawPyramids() {
    int i;
    glBindTexture(GL_TEXTURE_2D, texture[6]);
    for (i = 0; i < NPYR; i++) {
        int x = pyramids[i].x;
        int y = pyramids[i].y;
        int z = pyramids[i].z;
        float width = pyramids[i].width;
        float height = pyramids[i].height;
        
        glBegin(GL_TRIANGLES);
            // front face
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z); 
            glTexCoord2f(1.0f, 0.0f); glVertex3f(x - width/2, 0, z - width/2); 
            glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width/2, 0, z - width/2); 

            // right face
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z); 
            glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width/2, 0, z - width/2); 
            glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width/2, 0, z + width/2); 

            // back face
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z); 
            glTexCoord2f(0.0f, 1.0f); glVertex3f(x - width/2, 0, z + width/2); 
            glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width/2, 0, z + width/2); 

            // left face
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z); 
            glTexCoord2f(1.0f, 0.0f); glVertex3f(x - width/2, 0, z - width/2); 
            glTexCoord2f(0.0f, 1.0f); glVertex3f(x - width/2, 0, z + width/2); 
        glEnd();
    }
}

void explosion(float x, float y, float z, double time) {

}

int renderScene(Camera camera, bool v) {
//  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    renderSky();

    if (v == true) {
        gluLookAt(	camera.at.x			     , camera.at.y			   , camera.at.z, 
                    camera.at.x + camera.n.x , camera.at.y + camera.n.y, camera.at.z + camera.n.z, 
                    camera.v.x               , camera.v.y              , 0);
    } else {
        gluLookAt(camera.at.x, camera.at.y, camera.at.z,
                  camera.to.x, camera.to.y, camera.to.z,
                  camera.v.x, camera.v.y, 0);
    }
        
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glBegin(GL_QUADS);
        //bottom left
        glTexCoord2d(0, 0); glVertex3f(0.0f, 0.0f, 1000.0f);
        glTexCoord2d(1, 0); glVertex3f(1000.0f, 0.0f, 1000.0f);
        glTexCoord2d(1, 1); glVertex3f(1000.0f, 0.0f, 0.0f);
        glTexCoord2d(0, 1); glVertex3f(0.0f, 0.0f, 0.0f);

        //bottom mid
        glTexCoord2d(0, 0); glVertex3f(1000.0f, 0.0f, 1000.0f);
        glTexCoord2d(1, 0); glVertex3f(2000.0f, 0.0f, 1000.0f);
        glTexCoord2d(1, 1); glVertex3f(2000.0f, 0.0f, 0.0f);
        glTexCoord2d(0, 1); glVertex3f(1000.0f, 0.0f, 0.0f);

        //bottom right
        glTexCoord2d(0, 0); glVertex3f(2000.0f, 0.0f, 1000.0f);
        glTexCoord2d(1, 0); glVertex3f(3000.0f, 0.0f, 1000.0f);
        glTexCoord2d(1, 1); glVertex3f(3000.0f, 0.0f, 0.0f);
        glTexCoord2d(0, 1); glVertex3f(2000.0f, 0.0f, 0.0f);

        //mid left
        glTexCoord2d(0, 0); glVertex3f(0.0f, 0.0f, 2000.0f);
        glTexCoord2d(1, 0); glVertex3f(1000.0f, 0.0f, 2000.0f);
        glTexCoord2d(1, 1); glVertex3f(1000.0f, 0.0f, 1000.0f);
        glTexCoord2d(0, 1); glVertex3f(0.0f, 0.0f, 1000.0f);

        //mid mid
        glTexCoord2d(0, 0); glVertex3f(1000.0f, 0.0f, 2000.0f);
        glTexCoord2d(1, 0); glVertex3f(2000.0f, 0.0f, 2000.0f);
        glTexCoord2d(1, 1); glVertex3f(2000.0f, 0.0f, 1000.0f);
        glTexCoord2d(0, 1); glVertex3f(1000.0f, 0.0f, 1000.0f);

        //mid right
        glTexCoord2d(0, 0); glVertex3f(2000.0f, 0.0f, 2000.0f);
        glTexCoord2d(1, 0); glVertex3f(3000.0f, 0.0f, 2000.0f);
        glTexCoord2d(1, 1); glVertex3f(3000.0f, 0.0f, 1000.0f);
        glTexCoord2d(0, 1); glVertex3f(2000.0f, 0.0f, 1000.0f);

        //top left
        glTexCoord2d(0, 0); glVertex3f(0.0f, 0.0f, 3000.0f);
        glTexCoord2d(1, 0); glVertex3f(1000.0f, 0.0f, 3000.0f);
        glTexCoord2d(1, 1); glVertex3f(1000.0f, 0.0f, 2000.0f);
        glTexCoord2d(0, 1); glVertex3f(0.0f, 0.0f, 2000.0f);

        //top mid
        glTexCoord2d(0, 0); glVertex3f(1000.0f, 0.0f, 3000.0f);
        glTexCoord2d(1, 0); glVertex3f(2000.0f, 0.0f, 3000.0f);
        glTexCoord2d(1, 1); glVertex3f(2000.0f, 0.0f, 2000.0f);
        glTexCoord2d(0, 1); glVertex3f(1000.0f, 0.0f, 2000.0f);

        //top right
        glTexCoord2d(0, 0); glVertex3f(2000.0f, 0.0f, 3000.0f);
        glTexCoord2d(1, 0); glVertex3f(3000.0f, 0.0f, 3000.0f);
        glTexCoord2d(1, 1); glVertex3f(3000.0f, 0.0f, 2000.0f);
        glTexCoord2d(0, 1); glVertex3f(2000.0f, 0.0f, 2000.0f);
    glEnd();
    
    drawPyramids();

    glTranslatef(camera1.at.x, camera1.at.y, camera1.at.z);
    glRotatef(camera1.yaw, 0, 1, 0);
    glRotatef(camera1.pitch, 1, 0, 0);
    glRotatef(camera1.roll, 0, 0, 1);
    drawmodel_box(camera1.at.x, camera1.at.y, camera1.at.z);
    glRotatef(-camera1.roll, 0, 0, 1);
    glRotatef(-camera1.pitch, 1, 0, 0);
    glRotatef(-camera1.yaw, 0, 1, 0);
    glTranslatef(-camera1.at.x, -camera1.at.y, -camera1.at.z);
}


int drawGLScene() {
    static GLint frames, TO;
    glClear(GL_COLOR_BUFFER_BIT);
    
    glViewport(0, 0, (GLint)640, (GLint)480);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)SCREEN_WIDTH/(GLfloat)SCREEN_HEIGHT, 0.1f, 2000.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    renderScene(camera1, true);

    glViewport(400, 320, (GLint)240, (GLint)200);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)SCREEN_WIDTH/(GLfloat)SCREEN_HEIGHT, 0.1f, 2000.0f);
    //glOrtho(-50, 50, 0, 100, 0, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (n_camera == 0) renderScene(camera2, false);
    if (n_camera == 1) renderScene(plane_camera1, false);
    if (n_camera == 2) renderScene(plane_camera2, false);
    if (n_camera == 3) renderScene(plane_camera3, false);
    if (n_camera == 4) renderScene(plane_camera4, false);

    frames++;
    {
        GLint t = SDL_GetTicks();
        if (t - TO >= 1000) {
            GLfloat seconds = (t-TO)/1000.0;
            GLfloat fps = frames/seconds;
            printf("%d frames in %g seconds = %g FPS, Yaw: %2.4f, Pitch: %2.4f, Roll: %2.4f, __time: %2.4f\n", frames, seconds, fps, camera1.yaw, camera1.pitch, camera1.roll, __time.get_ticks()/1000.0f);
            TO = t;
            frames = 0;
        }
    }

    glFlush();

    SDL_GL_SwapBuffers();
    return TRUE;
}

bool insidePyramid(int cx, int cz, float widthl, int x, int z) {
    if (((cx >= x-widthl) && (cz >= z-widthl)) &&
        ((cx <= x+widthl) && (cz >= z-widthl)) &&
        ((cx <= x+widthl) && (cz <= z+widthl)) &&
        ((cx >= x-widthl) && (cz <= z+widthl))) return true;
    else return false;
} 

void detectCollision() {
    // if the plane is too slow simply dont let it go through the floor
    // else it will EXPLODE!!!11!1!11one
    if (camera1.at.y < 0.5f) {
        camera1.at.y = 0.5f;
        camera1.to.y = 0.5f;
    }

    // if the plane hits a pyramid it just stops, the player can press n to restart at a new position 
    int i;
    for (int i = 0; i < NPYR; i++) {
        int x = pyramids[i].x;
        int y = pyramids[i].y;
        int z = pyramids[i].z;
        if (camera1.at.y < y) {
            float widthl = abs(pyramids[i].width/2-0.8*camera1.at.y);
            if (insidePyramid(camera1.at.x, camera1.at.z, widthl, x, z)) {
                speed = 0.0f;
                acel = 0.0f;
            }
        }
    }
}

int main(int argc, char **argv) {
    int videoFlags;
    int done = FALSE;
    int isActive = TRUE;
    SDL_Event event;
    const SDL_VideoInfo *videoInfo;
    Map sector1(0.0, 5.0, 0.0, 5.0, "sector1.bmp");
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Video initialization failed: %s\n", SDL_GetError());
        quit(1);
    }

    videoInfo = SDL_GetVideoInfo();

    if (!videoInfo) {
        fprintf(stderr, "Video query failed: %s\n", SDL_GetError());
        quit(1);
    }

    videoFlags  = SDL_OPENGL;
    videoFlags |= SDL_GL_DOUBLEBUFFER;
    videoFlags |= SDL_HWPALETTE;
    videoFlags |= SDL_RESIZABLE;
	
    if (videoInfo->hw_available) videoFlags |= SDL_HWSURFACE;
    else videoFlags |= SDL_SWSURFACE;
    if (videoInfo->blit_hw) videoFlags |= SDL_HWACCEL;
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    surface = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, videoFlags);
    if (!surface) {
        fprintf(stderr, "Video mode set failed: %s\n", SDL_GetError());
        quit(1);
    }

    SDL_ShowCursor(0);
    SDL_WM_GrabInput(SDL_GRAB_ON);

    initGL();
 
    resizeWindow(SCREEN_WIDTH, SCREEN_HEIGHT);

    delta.start(); 

    while (!done) {
        __time.start();
        SDL_PollEvent(&event);
        switch(event.type) {
            case SDL_ACTIVEEVENT:
                if (event.active.gain == 0) isActive = FALSE;
                else isActive = TRUE; break;

            case SDL_VIDEORESIZE:
                surface = SDL_SetVideoMode(event.resize.w, event.resize.h, 16, videoFlags);
                if (!surface) {
                    fprintf(stderr, "Video mode set failed: %s\n", SDL_GetError());
                    quit(1);
                }
                resizeWindow(event.resize.w, event.resize.h); break;

            case SDL_KEYDOWN: handleKeyPress(); break;
            case SDL_KEYUP: handleKeyPress(); break;
            case SDL_QUIT: done = TRUE; break;
            default: break;
        }

        frame++;
        camera1.moveForward(speed*boost, delta.get_ticks());
        camera2.at.x = camera1.at.x;
        camera2.at.y = camera1.at.y + 100;
        camera2.at.z = camera1.at.z;
        camera2.to.x = camera1.at.x + camera1.n.x;
        camera2.to.y = camera1.at.y + camera1.n.y;
        camera2.to.z = camera1.at.z + camera1.n.z;

        plane_camera1.at.x = camera1.at.x - 10;
        plane_camera1.at.y = camera1.at.y + 10;
        plane_camera1.at.z = camera1.at.z - 10;
        plane_camera1.to.x = camera1.at.x + camera1.n.x;
        plane_camera1.to.y = camera1.at.y + camera1.n.y;
        plane_camera1.to.z = camera1.at.z + camera1.n.z;

        plane_camera2.at.x = camera1.at.x - 3;
        plane_camera2.at.y = camera1.at.y;
        plane_camera2.at.z = camera1.at.z - 3;
        plane_camera2.to.x = camera1.at.x + camera1.n.x;
        plane_camera2.to.y = camera1.at.y + camera1.n.y;
        plane_camera2.to.z = camera1.at.z + camera1.n.z;
        
        plane_camera3.at.x = camera1.at.x + 5;
        plane_camera3.at.y = camera1.at.y + 5;
        plane_camera3.at.z = camera1.at.z + 5;
        plane_camera3.to.x = camera1.at.x + camera1.n.x;
        plane_camera3.to.y = camera1.at.y + camera1.n.y;
        plane_camera3.to.z = camera1.at.z + camera1.n.z;

        plane_camera4.at.x = camera1.at.x;
        plane_camera4.at.y = camera1.at.y;
        plane_camera4.at.z = camera1.at.z;
        plane_camera4.to.x = camera1.at.x + camera1.n.x;
        plane_camera4.to.y = camera1.at.y + camera1.n.y;
        plane_camera4.to.z = camera1.at.z + camera1.n.z;

        detectCollision();
        if (isActive) drawGLScene();
        delta.start();
        //if (__time.get_ticks() < 1000 / MAXFPS) SDL_Delay((1000/MAXFPS) - __time.get_ticks());
        SDL_Delay((1000/MAXFPS));
    }

    quit(0);
    
    return 0;
}
