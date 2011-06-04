#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480
#define SCREEN_BPP      16

#define TRUE    1
#define FALSE   0

typedef struct {
    float x, y, z;
    float u, v;
} vertexx;

typedef struct {
    vertexx vertex[3];
} trianglee;

typedef struct {
    int nTriangles;
    trianglee *triangle;
} sector;

SDL_Surface *surface;

int light = FALSE;
int blend = FALSE;

sector sector1;

GLfloat yrot;
GLfloat xpos, zpos;
GLfloat walkbias, walkbiasangle;
GLfloat lookupdown;

GLfloat LightAmbient[] = {0.5f, 0.5f, 0.5f, 1.0f};
GLfloat LightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat LightPosition[] = {0.0f, 0.0f, 2.0f, 1.0f};

const float piover180 = 0.0174532925f;

GLuint filter;
GLuint texture[3];

void quit(int returnCode) {
    SDL_Quit();
    if (sector1.triangle) free(sector1.triangle);
    exit(returnCode);
}

int loadGLTextures() {
    int status = FALSE;

    SDL_Surface *textureImage[1];

    if ((textureImage[0] = SDL_LoadBMP("1.bmp"))) {
        status = TRUE;
        
        glGenTextures(3, &texture[0]);
        glBindTexture(GL_TEXTURE_2D, texture[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, 3, textureImage[0]->w, textureImage[0]->h,
                    0, GL_BGR, GL_UNSIGNED_BYTE, textureImage[0]->pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, texture[1]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, 3, textureImage[0]->w, textureImage[0]->h, 
                    0, GL_BGR, GL_UNSIGNED_BYTE, textureImage[0]->pixels);
        glBindTexture(GL_TEXTURE_2D, texture[2]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, textureImage[0]->w, textureImage[0]->h,
                        GL_BGR, GL_UNSIGNED_BYTE, textureImage[0]->pixels);    
    }

    if (textureImage[0]) SDL_FreeSurface(textureImage[0]);

    return status;
}

void readstr(FILE *f, char *string) {
    do {
        fgets(string, 255, f);
    } while ((string[0] == '/') ||(string[0] == '\n'));
}

void setupWorld(char *worldFile) {
    FILE *fileIn;
    int nTriangles, triLoop, verLoop;
    char oneLine[255];
    float x, y, z, u, v;

    fileIn = fopen(worldFile, "rt");
    readstr(fileIn, oneLine);
    sscanf(oneLine, "NUMPOLLIES %d\n", &nTriangles);

    sector1.triangle = (trianglee*) malloc(nTriangles * sizeof(trianglee));
    if (sector1.triangle == NULL) {
        fprintf(stderr, "Could not allocate memory for triangles.\n");
        quit(1);
    }
    sector1.nTriangles = nTriangles;

    for (triLoop = 0; triLoop < nTriangles; triLoop++) {
        for (verLoop = 0; verLoop < 3; verLoop++) {
            readstr(fileIn, oneLine);
            sscanf(oneLine, "%f %f %f %f %f\n", &x, &y, &z, &u, &v);
            sector1.triangle[triLoop].vertex[verLoop].x = x;
            sector1.triangle[triLoop].vertex[verLoop].y = y;
            sector1.triangle[triLoop].vertex[verLoop].z = z;
            sector1.triangle[triLoop].vertex[verLoop].u = u;
            sector1.triangle[triLoop].vertex[verLoop].v = v;
        }
    }

    fclose(fileIn);
}

int resizeWindow(int w, int h) {
    if (h == 0) h = 1;

    GLfloat ratio;
    ratio = (GLfloat)w/(GLfloat)h;

    glViewport(0, 0, (GLint)w, (GLint)h); 

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, ratio, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    return TRUE;
}

void handleKeyPress(SDL_keysym *keysym) {
    switch(keysym->sym) {
        case SDLK_ESCAPE: quit(0); break;
        case SDLK_f: filter = (++filter) % 3; break;

        case SDLK_b:
            blend = !blend;
            if (blend) { 
                glEnable(GL_BLEND); 
                glDisable(GL_DEPTH_TEST);
            } else {
                glDisable(GL_BLEND); 
                glEnable(GL_DEPTH_TEST); 
            } break;

        case SDLK_l: 
            light = !light;
            if (!light) glDisable(GL_LIGHTING);
            else glEnable(GL_LIGHTING); break;

        case SDLK_UP:
            xpos -= (float)sin(yrot*piover180)*0.005f;
            zpos -= (float)cos(yrot*piover180)*0.005f;
            if (walkbiasangle >= 359.0f) walkbiasangle = 0.0f;
            else walkbiasangle += 0.001f;
            walkbias = (float)sin(walkbiasangle*piover180)/20.0f; break;

        case SDLK_DOWN:
            xpos += (float)sin(yrot*piover180)*0.005f;
            zpos += (float)cos(yrot*piover180)*0.005f;
            if (walkbiasangle <= 1.0f) walkbiasangle = 359.0f;
            else walkbiasangle -= 0.001f;
            walkbias = (float)sin(walkbiasangle*piover180)/20.0f; break;

        case SDLK_RIGHT: yrot -= 0.015f; break;
        case SDLK_LEFT: yrot += 0.015f; break;
        case SDLK_F1: SDL_WM_ToggleFullScreen(surface); break;
        default: break;
    }
}

int initGL() {
    if (!loadGLTextures()) return FALSE;

    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
    glEnable(GL_LIGHT1);
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    lookupdown = 0.0f;
    walkbias = 0.0f;
    walkbiasangle = 0.0f;

    return TRUE;
}

int drawGLScene() {
    static GLint TO = 0;
    static GLint frames = 0;
    int loop_m;
    
    GLfloat x_m, y_m, z_m, u_m, v_m;
    GLfloat xtrans = -xpos;
    GLfloat ztrans = -zpos;
    GLfloat ytrans = -walkbias - 0.25f;
    GLfloat sceneroty = 360.0f - yrot;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    glRotatef(lookupdown, 1.0f, 0.0f, 0.0f);
    glRotatef(sceneroty, 0.0f, 1.0f, 0.0f);
    glTranslatef(xtrans, ytrans, ztrans);
    glBindTexture(GL_TEXTURE_2D, texture[filter]);

    for (loop_m = 0; loop_m < sector1.nTriangles; loop_m++) {
        glBegin(GL_TRIANGLES);
            glNormal3f(0.0f, 0.0f, 1.0f);
            x_m = sector1.triangle[loop_m].vertex[0].x;
            y_m = sector1.triangle[loop_m].vertex[0].y;
            z_m = sector1.triangle[loop_m].vertex[0].z;
            u_m = sector1.triangle[loop_m].vertex[0].u;
            v_m = sector1.triangle[loop_m].vertex[0].v;
            glTexCoord2f(u_m, v_m);
            glVertex3f(x_m, y_m, z_m);

            x_m = sector1.triangle[loop_m].vertex[1].x;
            y_m = sector1.triangle[loop_m].vertex[1].y;
            z_m = sector1.triangle[loop_m].vertex[1].z;
            u_m = sector1.triangle[loop_m].vertex[1].u;
            v_m = sector1.triangle[loop_m].vertex[1].v;
            glTexCoord2f(u_m, v_m);
            glVertex3f(x_m, y_m, z_m);
            
            x_m = sector1.triangle[loop_m].vertex[2].x;
            y_m = sector1.triangle[loop_m].vertex[2].y;
            z_m = sector1.triangle[loop_m].vertex[2].z;
            u_m = sector1.triangle[loop_m].vertex[2].u;
            v_m = sector1.triangle[loop_m].vertex[2].v;
            glTexCoord2f(u_m, v_m);
            glVertex3f(x_m, y_m, z_m);
        glEnd();
    }
    
    SDL_GL_SwapBuffers();

    frames++;
    {
        GLint t = SDL_GetTicks();
        if (t - TO >= 5000) {
            GLfloat seconds = (t-TO)/1000.0;
            GLfloat fps = frames/seconds;
            printf("%d frames in %g seconds = %g FPS\n", frames, seconds, fps);
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

    initGL();
    setupWorld("world.txt");
    resizeWindow(SCREEN_WIDTH, SCREEN_HEIGHT);

    while (!done) {
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

            case SDL_KEYDOWN: handleKeyPress(&event.key.keysym); break;
            case SDL_QUIT: done = TRUE; break;
            default: break;
        }

        if (isActive) drawGLScene();
    }

    quit(0);
    
    return 0;
}
