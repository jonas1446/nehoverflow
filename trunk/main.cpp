#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cmath>

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>

#include "timer.h"
#include "camera.h"
#include "map.h"

#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480
#define SCREEN_BPP      16

#define TRUE    1
#define FALSE   0

#define MAXFPS  60

#define CHUNKX 64
#define CHUNKY 32
#define CHUNKZ 64

using namespace std;

SDL_Surface *surface;
Camera camera1, camera2;
Timer __time, delta;
Map sector1(0.0f, 0.0f, "tutorial.bmp");

int light = FALSE;
int blend = FALSE;
int ratio;
int frame = 0;
int w1x = 0;
int w1y = 0;

float speed = 1;

GLfloat LightAmbient[] = {0.5f, 0.5f, 0.5f, 1.0f};
GLfloat LightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat LightPosition[] = {0.0f, 0.0f, 2.0f, 1.0f};

GLuint filter;
GLuint texture[7];
GLuint box, chunk;

void quit(int returnCode) {
    SDL_Quit();
    // if (sector1.triangle) free(sector1.triangle);
    exit(returnCode);
}

GLvoid buildLists() {
    box = glGenLists(1);
    chunk = glGenLists(1);

    glNewList(box, GL_COMPILE);

    glBindTexture(GL_TEXTURE_2D, texture[6]);
    glBegin(GL_QUADS);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, 0.0f, 1.0f);

        glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 0.0f, 1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 1.0f, 1.0f);

        glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f, 0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 0.0f, 0.0f);

        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 0.0f, 1.0f);

        glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, 0.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 1.0f, 0.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glBegin(GL_QUADS);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, 0.0f);
    glEnd();
    glEndList();

    glNewList(chunk, GL_COMPILE);
    for (int i = 0; i < CHUNKX; i++) {
        for (int j = 0; j < CHUNKY; j++) {
            for (int k = 0; k < CHUNKZ; k++) {
                if (sector1.mat[i][j][k] == 1) {
                    glTranslatef(i+1, j+1, k+1);
                    glCallList(box);
                    glTranslatef(-(i+1), -(j+1), -(k+1));
                }
            } 
        }
    }
    glEndList();
}
        

int loadGLTextures() {
    int status = FALSE;

    SDL_Surface *textureImage[7];

    if ((textureImage[0] = SDL_LoadBMP("gfx/grasstop.bmp"))) {
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

    if ((textureImage[6] = SDL_LoadBMP("gfx/grassside.bmp"))) {
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
//	gluLookAt(	camera1.at.x			  , camera1.at.y			  , camera1.at.z, 
//    			camera1.at.x + camera1.n.x, camera1.at.y + camera1.n.y, camera1.at.z + camera1.n.z, 
//    			camera1.v.x				  , camera1.v.y				  , camera1.v.z);

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

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0f, ratio, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

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
    
    if (keys[SDLK_w]) {
        camera1.moveForward(speed, delta.get_ticks());
    }

    if (keys[SDLK_s]) { 
        camera1.moveForward(-speed, delta.get_ticks());
    }
    
    if (keys[SDLK_UP]) {
        camera1.pitch -= 3;
        camera1.rotate();
    }

    if (keys[SDLK_DOWN]) {
        camera1.pitch += 3;
        camera1.rotate();
    }

    if (keys[SDLK_RIGHT]) {
        camera1.yaw -= 3;
        camera1.rotate();
    }

    if (keys[SDLK_LEFT]) {
        camera1.yaw += 3;
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

    if (keys[SDLK_F1])
        SDL_WM_ToggleFullScreen(surface); 


}

int initGL() {
    if (!loadGLTextures()) return FALSE;
    
    sector1.fillMat();
    sector1.optimizeMat();

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

    return TRUE;
}

int renderScene(Camera camera) {
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    renderSky();

    gluLookAt(	camera.at.x			     , camera.at.y			   , camera.at.z, 
    			camera.at.x + camera.n.x , camera.at.y + camera.n.y, camera.at.z + camera.n.z, 
    			camera.v.x               , camera.v.y              , 0);

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTranslatef(0.0f, -10.0f, 0.0f);

    glCallList(chunk);

/*    for (int i = 0; i < 60; i++) {
        glTranslatef(2.0f, 0.0f, 0.0f);
        glCallList(box); 
        for (int j = 0; j < 60; j++) {
            glTranslatef(0.0f, 0.0f, 2.0f);
            glCallList(box); 
        }
        glTranslatef(0.0f, 0.0f, -120.0f);
    }*/
    
    SDL_GL_SwapBuffers();
}


int drawGLScene() {
    static GLint frames, TO;
    
    glViewport(0, 0, (GLint)640, (GLint)480);
    renderScene(camera1);
	
	//glViewport(480, 0, (GLint)160, (GLint)120);
  	//renderScene(camera2);

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

    return TRUE;
}

int main(int argc, char **argv) {
    int videoFlags;
    int done = FALSE;
    int isActive = TRUE;
    SDL_Event event;
    const SDL_VideoInfo *videoInfo;
    
	camera2 = Camera(vector3f(-1,0,0), vector3f(1,0,0));

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
        if (isActive) drawGLScene();
        delta.start();
        // if (__time.get_ticks() < 1000 / MAXFPS) SDL_Delay((1000/MAXFPS) - __time.get_ticks());
    }

    quit(0);
    
    return 0;
}
