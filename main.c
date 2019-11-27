/*******************************************************************
           Multi-Part Model Construction and Manipulation
********************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/glut.h>
#include "Vector3D.h"
#include "QuadMesh.h"
#include "CubeMesh.h"

# define M_PI 3.14159265358979323846

const int meshSize = 32;    // Default Mesh Size
const int vWidth = 650;     // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels

static int currentButton;
static unsigned char currentKey;

static double xPos = 0.0;
static double zPos = 0.0;
static int isON = 0;
static int direction = 1;
static int currentButton;
static unsigned char currentKey;
static double rotate_degree = 0.0;
static double prop_rotate_degree_update = 0.0;
static double sub_motion_h = 0.0;
static double sub_motion_v = 0.0;

// Lighting/shading and material properties for drone - upcoming lecture - just copy for now

// Light properties
static GLfloat light_position0[] = { -6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_position1[] = { 6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

// Material properties
static GLfloat drone_mat_ambient[] = { 0.4F, 0.2F, 0.0F, 1.0F };
static GLfloat drone_mat_specular[] = { 0.1F, 0.1F, 0.0F, 1.0F };
static GLfloat drone_mat_diffuse[] = { 0.9F, 0.5F, 0.0F, 1.0F };
static GLfloat drone_mat_shininess[] = { 0.0F };

// A quad mesh representing the ground
static QuadMesh groundMesh;
static CubeMesh cube;
static Vector3D quadMeshOrigin;

static GLUquadricObj* tower;

static GLfloat textureMap1[64][64][3];
static GLfloat textureMap2[64][64][3];
static GLuint tex[2];

// Structure defining a bounding box, currently unused
//struct BoundingBox {
//    Vector3D min;
//    Vector3D max;
//} BBox;

// Prototypes for functions in this module
void makeTextureMap();
void makeTextures();
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void timer(int value);
void instructions(void);
void assignColor(GLfloat col[3], GLfloat r, GLfloat g, GLfloat b);

void drawSub(void);
void drawBody(void);
void drawPropeller(void);
void drawTower(void);
void drawPropellerCover(void);
void drawFins(void);

Vector3D ScreenToWorld(int x, int y);


int main(int argc, char **argv)
{
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(vWidth, vHeight);
    glutInitWindowPosition(200, 30);
    glutCreateWindow("Assignment 1");

    // Initialize GL
    initOpenGL(vWidth, vHeight);

    // Register callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotionHandler);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(functionKeys);
	glutTimerFunc(0, *timer, 0);

    // Start event loop, never returns
    glutMainLoop();

    return 0;
}


// Set up OpenGL. For viewport and projection setup see reshape(). */
void initOpenGL(int w, int h)
{
    // Set up and enable lighting
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
	glEnable(GL_TEXTURE_2D);

	makeTextureMap();
	makeTextures();

    // Other OpenGL setup
    glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
    glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
    glClearColor(0.0F, 0.0F, 0.5F, 0.0F);  // Color and depth for glClear
    glClearDepth(1.0f);
    glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

    // Set up ground quad mesh
    Vector3D origin = NewVector3D(-32.0f, 0.0f, 32.0f);
	quadMeshOrigin = origin;
    Vector3D dir1v = NewVector3D(1.0f, 0.0f, 0.0f);
    Vector3D dir2v = NewVector3D(0.0f, 0.0f, -1.0f);
    groundMesh = NewQuadMesh(meshSize);
    InitMeshQM(&groundMesh, meshSize, origin, 64.0, 64.0, dir1v, dir2v);

    Vector3D ambient = NewVector3D(0.0f, 0.0f, 0.0f);
    Vector3D diffuse = NewVector3D(0.4f, 0.8f, 0.4f);
    Vector3D specular = NewVector3D(0.04f, 0.04f, 0.04f);
    SetMaterialQM(&groundMesh, ambient, diffuse, specular, 0.2);

    // Set up the bounding box of the scene0
    // Currently unused. You could set up bounding boxes for your objects eventually.
    //Set(&BBox.min, -8.0f, 0.0, -8.0);
    //Set(&BBox.max, 8.0f, 6.0,  8.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

void timer(int value) {
	if (isON)
		if (direction == 1)
			prop_rotate_degree_update += 10;
		else
			prop_rotate_degree_update -= 10;

	glutTimerFunc(16, *timer, 0);
	glutPostRedisplay();
}

void assignColor(GLfloat col[3], GLfloat r, GLfloat g, GLfloat b) {
	col[0] = r;
	col[1] = g;
	col[2] = b;
}

void makeTextureMap()
{
	double range;
	for (int i = 0; i < 64; i++)
		for (int j = 0; j < 64; j++) {
			range = ((double)(rand() % 200 + -100) / (double)1000);
			assignColor(textureMap1[i][j], 0.500+range, 0.500+range, 0.500+range);
		}
	for (int i = 0; i < 64; i++)
		for (int j = 0; j < 64; j++) {
			range = ((double)(rand() % 200 + -100) / (double)1000);
			assignColor(textureMap2[i][j], 0.500 + range, 0.0, 0.0);
		}
}

void makeTextures()
{

	glGenTextures(2, tex);
	
	//Texture mapping for the Quadmesh
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_FLOAT, textureMap1);

	//Texture mapping for the Player submarine
	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_FLOAT, textureMap2);

}


void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(0.0, 12.0, 40.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, tex[0]);

    /*glMaterialfv(GL_FRONT, GL_AMBIENT, drone_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, drone_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, drone_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, drone_mat_shininess);*/

	DrawMeshQM(&groundMesh, meshSize);

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	
	if (sub_motion_v + 2.8 > quadMeshOrigin.y)
	{
		glPushMatrix();
		glTranslatef(xPos, sub_motion_v + 4.0, zPos);
		glRotatef(rotate_degree, 0.0, 1.0, 0.0);
		drawSub();
		glPopMatrix();
	}
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

    glutSwapBuffers();   // Double buffering, swap buffers
}

void drawSub(void) {
	drawBody();
}

void drawBody(void) {

	glPushMatrix();
	glScalef(6.0, 1.0, 1.0);
	glutSolidSphere(1.0, 60, 60);
	drawPropeller();
	drawTower();
	drawPropellerCover();
	drawFins();
	glPopMatrix();
}

void drawPropeller(void) {

	glPushMatrix();
	glTranslatef(1.0, 0.0, 0.0);
	glRotatef(90, 0.0, 1.0, 0.0);
	glRotatef(prop_rotate_degree_update, 0.0, 0.0, 1.0);
	glScalef(0.1, 1.0, 0.1);
	glutSolidCube(2.0);
	glPopMatrix();
}

void drawPropellerCover(void) {

	glPushMatrix();
	glTranslatef(1.0, 0.0, 0.0);
	glRotatef(90, 0.0, 0.0, 1.0);
	glRotatef(90, 1.0, 0.0, 0.0);
	glScalef(1.0, 1.0, 0.3);
	glutSolidTorus(0.3, 1.5, 10, 50);
	glPopMatrix();
}

void drawTower(void) {

	glPushMatrix();
	glTranslatef(0.0, 0.2, 0.0);
	glRotatef(-90, 1.0, 0.0, 0.0);
	glScalef(0.2, 0.4, 1.0);
	tower = gluNewQuadric();
	gluQuadricDrawStyle(tower, GLU_LINE);
	gluCylinder(tower, 2.0, 1.5, 2.0, 1000, 1000);
	glPopMatrix();

}

void drawFins(void)
{
	glPushMatrix();
	glTranslatef(-0.7, 0.0, 0.0);
	glRotatef(90, 1.0, 0.0, 0.0);
	glRotatef(-45, 0.0, 1.0, 0.0);
	glScalef(0.3, 4.5, 0.1);
	glutSolidCube(1.0);
	glPopMatrix();
}

// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
    // Set up viewport, projection, then change to modelview matrix mode - 
    // display function will then set up camera and do modeling transforms.
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLdouble)w / h, 0.2, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	if (key == 's')
		isON = !isON;
	else if (key == 'f') {
		direction = 1;
		sub_motion_h -= 0.5;
		xPos -= cosf((-rotate_degree * M_PI) / 180);
		zPos -= sinf((-rotate_degree * M_PI) / 180);
	}
	else if (key == 'b') {
		direction = -1;
		sub_motion_h += 0.5;
		xPos += cosf((-rotate_degree * M_PI) / 180);
		zPos += sinf((-rotate_degree * M_PI) / 180);
	}
	glutPostRedisplay();   // Trigger a window redisplay
}

// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{

	if (key == GLUT_KEY_LEFT)
		rotate_degree += 10;
	else if (key == GLUT_KEY_RIGHT)
		rotate_degree -= 10;
	else if (key == GLUT_KEY_UP)
		sub_motion_v += 0.5;
	else if (key == GLUT_KEY_DOWN)
		sub_motion_v -= 0.5;
	else if (key == GLUT_KEY_F1)
		instructions();
	glutPostRedisplay();   // Trigger a window redisplay
}

void instructions(void) {
	printf(" Controls: \n Up: Up arrow key \n Down: Down arrow key \n Rotate right: Right arrow key \n Rotate left: Left arrow key \n Forward: 'f' \n Backward: 'b' \n Toggle propeller: 's' \n");
}

// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
    currentButton = button;

    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        if (state == GLUT_DOWN)
        {
            ;

        }
        break;
    case GLUT_RIGHT_BUTTON:
        if (state == GLUT_DOWN)
        {
            ;
        }
        break;
    default:
        break;
    }

    glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)
{
    if (currentButton == GLUT_LEFT_BUTTON)
    {
        ;
    }

    glutPostRedisplay();   // Trigger a window redisplay
}


Vector3D ScreenToWorld(int x, int y)
{
    // you will need to finish this if you use the mouse
    return NewVector3D(0, 0, 0);
}



