#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
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

//delete this struct -> use Camera class
typedef struct {
    GLdouble atx, aty, atz;
    GLdouble tox, toy, toz;
    GLfloat xd, yd, zd;
    GLfloat yaw, pitch;
} camera;

SDL_Surface *surface;
camera camera1, camera2;

int light = FALSE;
int blend = FALSE;
int ratio;

float speed = 0.01f;
GLfloat xrot, yrot;
GLfloat xpos, zpos;

GLfloat LightAmbient[] = {0.5f, 0.5f, 0.5f, 1.0f};
GLfloat LightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat LightPosition[] = {0.0f, 0.0f, 2.0f, 1.0f};

const float piover180 = 0.0174532925f;

GLuint filter;
GLuint texture[6];
GLuint box;

void quit(int returnCode) {
    SDL_Quit();
    // if (sector1.triangle) free(sector1.triangle);
    exit(returnCode);
}

GLvoid buildLists() {
    box = glGenLists(1);

    glNewList(box, GL_COMPILE);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);

        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);

        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);

        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);

        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);

        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
    glEnd();
    glEndList();
}
        

int loadGLTextures() {
    int status = FALSE;

    SDL_Surface *textureImage[6];

    if ((textureImage[0] = SDL_LoadBMP("1.bmp"))) {
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

    if (textureImage[0]) SDL_FreeSurface(textureImage[0]);
    if (textureImage[1]) SDL_FreeSurface(textureImage[1]);
    if (textureImage[2]) SDL_FreeSurface(textureImage[2]);
    if (textureImage[3]) SDL_FreeSurface(textureImage[3]);
    if (textureImage[4]) SDL_FreeSurface(textureImage[4]);
    if (textureImage[5]) SDL_FreeSurface(textureImage[5]);

    return status;
}

void readstr(FILE *f, char *string) {
    do {
        fgets(string, 255, f);
    } while ((string[0] == '/') ||(string[0] == '\n'));
}

/* void setupWorld(char *worldFile) {
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
} */

void renderSky() {
    glPushMatrix();
    glLoadIdentity();
    gluLookAt(0, 0, 0, camera1.atx + camera1.xd, camera1.aty + camera1.yd, camera1.atz + camera1.zd, 0, 1, 0);
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

    glViewport(0, 0, (GLint)w, (GLint)h); 

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0f, ratio, 0.1f, 500.0f);

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

        case SDLK_w: 
            camera1.atx += camera1.xd * speed;
            camera1.aty += camera1.yd * speed;
            camera1.atz += camera1.zd * speed;
            break;

        case SDLK_s:  
            camera1.atx -= camera1.xd * speed;
            camera1.aty -= camera1.yd * speed;
            camera1.atz -= camera1.zd * speed;
            break;

        case SDLK_UP:
            camera1.pitch += 0.001f;
            camera1.yd = atan(tan(atan(camera1.pitch)));
            break;

        case SDLK_DOWN:
            camera1.pitch -= 0.001f;
            camera1.yd = atan(tan(atan(camera1.pitch)));
            break;

        case SDLK_RIGHT: 
            camera1.yaw += 0.001f;
            camera1.xd = sin(camera1.yaw);
            camera1.zd = -cos(camera1.yaw); 
            break;

        case SDLK_LEFT: 
            camera1.yaw -= 0.001f; 
            camera1.xd = sin(camera1.yaw);
            camera1.zd = -cos(camera1.yaw);
            break;

        case SDLK_F1: SDL_WM_ToggleFullScreen(surface); break;
        default: break;
    }
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

    camera1.atx = camera1.aty = camera1.atz = 0.0f;
    camera1.tox = camera1.toy = camera1.toz = 0.0f;
    camera1.xd = camera1.yd = camera1.zd = 0.0f;
    camera1.yaw = camera1.pitch = 0.0f;

    return TRUE;
}

int drawGLScene() {
    static GLint TO = 0;
    static GLint frames = 0;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    renderSky();

    gluLookAt(camera1.atx, camera1.aty, camera1.atz, camera1.atx + camera1.xd,
            camera1.aty + camera1.yd, camera1.atz + camera1.zd, 0, 1, 0);

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTranslatef(0.0f, -10.0f, 0.0f);

    for (int i = 0; i < 60; i++) {
        glTranslatef(2.0f, 0.0f, 0.0f);
        glCallList(box); 
        for (int j = 0; j < 60; j++) {
            glTranslatef(0.0f, 0.0f, 2.0f);
            glCallList(box); 
        }
        glTranslatef(0.0f, 0.0f, -120.0f);
    }
    
    /* for (loop_m = 0; loop_m < sector1.nTriangles; loop_m++) {
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
    } */
    
    SDL_GL_SwapBuffers();

    frames++;
    {
        GLint t = SDL_GetTicks();
        if (t - TO >= 5000) {
            GLfloat seconds = (t-TO)/1000.0;
            GLfloat fps = frames/seconds;
            printf("%d frames in %g seconds = %g FPS, Yaw: %2.4f, Pitch: %2.4f, yd: %2.4f, speed: %2.4f\n", frames, seconds, fps, camera1.yaw, camera1.pitch, camera1.yd, speed);
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

    SDL_ShowCursor(0);
    SDL_WM_GrabInput(SDL_GRAB_ON);

    initGL();
    // setupWorld("world.txt");
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
