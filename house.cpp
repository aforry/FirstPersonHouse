// 
// CS370 -- Fall 2015
// Final Project
//
// @author: Austin W Forry
//

#ifdef OSX
	#include <GLUT/glut.h>
#else
	#include <GL/glew.h>
	#include <GL/glut.h>
#endif
#include <SOIL/SOIL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lighting.h"
#include "materials.h"
#include "vectorops.h"

// Shader file utility functions
#include "shaderutils.h"

// Shader files
GLchar* lightVertexFile = "lightvert.vs";
GLchar* lightFragmentFile = "lightfrag.fs";
GLchar* defaultVertexFile = "defaultvert.vs";
GLchar* defaultFragmentFile = "defaultfrag.fs";
GLchar* texVertexFile = "texturevert.vs";
GLchar* texFragmentFile = "texturefrag.fs";

// Shader objects
GLuint textureShaderProg;
GLuint lightShaderProg;
GLuint defaultShaderProg;
GLuint num_lights_param;
GLint num_lights = 1;
GLint texSampler;

// CONSTANTS
#define RAD2DEG (180.0f/3.14159f)
#define DEG2RAD (3.14159f/180.0f)

#define X 0
#define Y 1
#define Z 2

#define POINTLIGHT 0
#define LIGHT_OFF 0
#define LIGHT_ON 1

#define WALLS 1
#define TABLE 2
#define WINDOWS 3
#define DOOR 4
#define ART 5
#define FAN 6
#define BLINDS 7
#define CHAIRS 8

// Camera Variables
GLfloat moveZ = 0.0;
GLfloat angle = 0.0;
GLfloat posX = 0.0;
GLfloat posZ = 0.0;
GLfloat lookY = 0.0;
GLfloat vectorX = sin(angle);
GLfloat vectorZ = -cos(angle);

// Animation Flags
GLfloat blindsFlag = 0;
GLfloat blindsStretch = 1.0;
GLfloat fanFlag = 0;
GLfloat fanSpin = 0.0;
GLfloat lightFlag = LIGHT_ON;

// TEXTURE CONSTANTS
#define END_TEXTURE_LIST 3
#define ENVIRONMENT 0
#define BRICK 1
#define ARTTEX 2
// Texture indices
GLuint tex_ids[END_TEXTURE_LIST];
// Texture files
char texture_files[END_TEXTURE_LIST][20] = { "blank.bmp", "brick.jpg", "art.jpg" };

// INSTANCES
GLfloat sqr[4][3] = {{1.0f,1.0f,0.0f},{-1.0f,1.0f,0.0f},{ -1.0f,-1.0f,0.0f},{ 1.0f,-1.0f,0.0f}};
GLfloat tri[3][2] = { { 0.0f, 0.0f }, { -1.0f, -2.0f }, { 1.0f, -2.0f } };
GLint subdivs = 5;

// LIGHTS
GLfloat pointlight_pos[4] = { 0.0f, 9.0f, 0.0f, 1.0f };

// Cylinder Quadratic
GLUquadric* quad;

// Function prototypes
void display();
void render_Scene();
void keyfunc(unsigned char key, int x, int y);
void reshape(int w, int h);
void idle_func();
void walls_list();
void table_list();
void windows_list();
void door_list();
void art_list();
void fan_list();
void blinds_list();
void chairs_list();
void draw_square();
void draw_fanblade();
void draw_fanMount();
void draw_drink();
void create_mirror();
void render_mirror();
bool load_textures();
void subdivision(GLfloat v1[], GLfloat v2[], GLfloat v3[], GLfloat v4[], GLfloat n1[], GLfloat n2[], GLfloat n3[], GLfloat n4[], int n);
void renderSubdiv(GLfloat v1[], GLfloat v2[], GLfloat v3[], GLfloat v4[], GLfloat n1[], GLfloat n2[], GLfloat n3[], GLfloat n4[]);


int main(int argc, char *argv[])
{
	// Initialize GLUT
	glutInit(&argc,argv);

	// Initialize the window with double buffering and RGB colors
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(512,512);
	glutCreateWindow("House");

#ifndef OSX
	// Initialize GLEW - MUST BE DONE AFTER CREATING GLUT WINDOW
	glewInit();
#endif

	// Define callbacks
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyfunc);
	glutIdleFunc(idle_func);

	// Set background color to white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	// Initialization for quadratic
	quad = gluNewQuadric();
	gluQuadricNormals(quad, GLU_SMOOTH);

	// Set shading model
	glShadeModel(GL_SMOOTH);

	// enable depth test
	glEnable(GL_DEPTH_TEST);
	
	// Enable lighting
	glEnable(GL_LIGHTING);

	// Load shader programs
	defaultShaderProg = load_shaders(defaultVertexFile, defaultFragmentFile);
	lightShaderProg = load_shaders(lightVertexFile, lightFragmentFile);
	textureShaderProg = load_shaders(texVertexFile, texFragmentFile);

	// Associate and assign sampler parameter
	texSampler = glGetUniformLocation(textureShaderProg, "texMap");
	glUniform1i(texSampler, 0);
	
	// load textures
	if (!load_textures())
	{
		//exit(0);
	}
	
	
	// Ambient lighting
	GLfloat ambiance[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	set_AmbientLight(ambiance);

	// Activate shader program
	glUseProgram(lightShaderProg);
	num_lights_param = glGetUniformLocation(lightShaderProg, "num_lights");

	// LIST CALLS
	walls_list();
	table_list();
	windows_list();
	door_list();
	art_list();
	fan_list();
	blinds_list();
	chairs_list();

	// Begin event loop
	glutMainLoop();
	return 0;
}

// Display callback
void display()
{
	// Create mirror
	create_mirror();

	// Reset background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 50.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(posX, 0.0f, posZ, posX + vectorX, lookY, posZ + vectorZ, 0.0f, 1.0f, 0.0f);

	// Render scene
	render_Scene();

	// Draw mirror
	render_mirror();

	// Flush buffer
	glFlush();

	// Swap buffers
	glutSwapBuffers();
}

// Scene render function
void render_Scene()
{
	glUseProgram(lightShaderProg);
	glUniform1i(num_lights_param, num_lights);
	if (lightFlag == LIGHT_ON)
	{
		set_PointLight(POINTLIGHT, &white_light, pointlight_pos);
	}
	// DRAW OBJECTS
	glCallList(WALLS);
	glCallList(TABLE);
	glCallList(CHAIRS);
	glCallList(DOOR);
	glUseProgram(defaultShaderProg);
	draw_drink();
	glCallList(WINDOWS);
	glCallList(ART);
	draw_fanMount();
	glPushMatrix();
		glRotatef(fanSpin,0,1,0);
		glCallList(FAN);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(0.0, -blindsStretch / 2.0, 0.0);
		glTranslatef(-14.8f, 3.0f, 0.0f);
		glScalef(1.0, blindsStretch, 1.0);
		glCallList(BLINDS);
	glPopMatrix();
}

void idle_func()
{
	if (fanFlag == 1){
		fanSpin++;
		if (fanSpin >= 360.0){
			fanSpin = 0;
		}
	}
	if (blindsFlag == 1){
		blindsStretch += 0.05;
		if (blindsStretch >= 6.0){
			blindsStretch = 6.0;
		}
	}
	if (blindsFlag == 0){
		blindsStretch -= 0.05;
		if (blindsStretch <= 1.0){
			blindsStretch = 1.0;
		}
	}
	glutPostRedisplay();
}

void keyfunc(unsigned char key, int x, int y)
{
	float fraction = 0.2f;
	// camera angle (LEFT AND RIGHT)
	if (key == 'a' || key == 'A')
	{
		angle -= 0.05f;
		vectorX = sin(angle);
		vectorZ = -cos(angle);
	}
	else if (key == 'd' || key == 'D')
	{
		angle += 0.05f;
		vectorX = sin(angle);
		vectorZ = -cos(angle);
	}
	// camera angle (UP AND DOWN)
	if (key == 'z' || key == 'Z')
	{
		lookY += 0.1;
		if (lookY > 3.0){
			lookY = 3.0;
		}
	}
	else if (key == 'x' || key == 'X')
	{
		lookY -= 0.1;
		if (lookY < -3.0){
			lookY = -3.0;
		}
	}
	// camera movement (forward and back)
	if (key == 'w' || key == 'W')
	{
		posX += vectorX * fraction;
		posZ += vectorZ * fraction;
	}
	else if (key == 's' || key == 'S')
	{
		posX -= vectorX * fraction;
		posZ -= vectorZ * fraction;
	}

	// movement bounds
	if (posX > 13){
		posX = 13;
	}
	if (posX < -13){
		posX = -13;
	}
	if (posZ > 18){
		posZ = 18;
	}
	if (posZ < -18){
		posZ = -18;
	}

	// fan movement (toggles on and off)
	if (key == 'f' || key == 'F')
	{
		if (fanFlag == 0){
			fanFlag = 1;
		}
		else{
			fanFlag = 0;
		}
	}
	// blinds control (toggles up and down)
	if (key == 'o' || key == 'O')
	{
		if (blindsFlag == 0){
			blindsFlag = 1;
		}
		else{
			blindsFlag = 0;
		}
	}
	// light switch controls
	if (key == 'l' || key == 'L')
	{
		if (lightFlag == LIGHT_OFF)
		{
			lightFlag = LIGHT_ON;
			num_lights++;
		}
		else {
			lightFlag = LIGHT_OFF;
			num_lights--;
		}
	}
	// Quit program
	if (key == 27)
	{
		exit(0);
	}

	glutPostRedisplay();
}

// Reshape callback
void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h)
		glOrtho(-10.0, 10.0, -10.0 * (GLfloat)h / (GLfloat)w,
		10.0 * (GLfloat)h / (GLfloat)w, -10.0, 10.0);
	else
		glOrtho(-10.0 * (GLfloat)w / (GLfloat)h,
		10.0 * (GLfloat)w / (GLfloat)h, -10.0, 10.0, -10.0, 10.0);
}

// LIST FUNCTIONS
void walls_list(){
	glNewList(WALLS, GL_COMPILE);
	glPushAttrib(GL_CURRENT_BIT);
	// TODO: TEXTURE MAP BRICK WALLS
	//draw far walls
	set_material(GL_FRONT_AND_BACK, &red);
	glPushMatrix();
		glTranslatef(0.0, 0.0, 20.0);
		glScalef(15.0, 10.0, 1.0);
		//glColor3f(1, .5, .5);
		draw_square();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(0.0, 0.0, -20.0);
		glScalef(15.0, 10.0, 1.0);
		//glColor3f(1, .5, .5);
		draw_square();
	glPopMatrix();
	//draw long walls
	glPushMatrix();
		glTranslatef(-15.0, 0.0, 0.0);
		glRotatef(90.0, 0, 1, 0);
		glScalef(20.0, 10.0, 1.0);
		//glColor3f(1, 0, 0);
		draw_square();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(15.0, 0.0, 0.0);
		glRotatef(90.0, 0, 1, 0);
		glScalef(20.0, 10.0, 1.0);
		//glColor3f(1, 0, 0);
		draw_square();
	glPopMatrix();
	//draw floor
	glPushMatrix();
		set_material(GL_FRONT_AND_BACK, &pearl);
		glTranslatef(0.0, -10.0, 0.0);
		glRotatef(90.0, 1, 0, 0);
		glScalef(15.0, 20.0, 1.0);
		//glColor3f(0.5, 0.5, 0.5);
		draw_square();
	glPopMatrix();
	//draw ceiling
	glPushMatrix();
		set_material(GL_FRONT_AND_BACK, &brass);
		glTranslatef(0.0, 10.0, 0.0);
		glRotatef(90.0, 1, 0, 0);
		glScalef(15.0, 20.0, 1.0);
		//glColor3f(0.5f, 0.35f, 0.05f);
		draw_square();
	glPopMatrix();
	glPopAttrib();
	glEndList();
}

void table_list(){
	glNewList(TABLE, GL_COMPILE);
	glPushAttrib(GL_CURRENT_BIT);
	set_material(GL_FRONT_AND_BACK, &brown);
	//draw table top
	glPushMatrix();
		glTranslatef(0.0, -5.0, -10.0);
		glScalef(10.0, 1.0, 10.0);
		//glColor3f(0.5, 0.35, 0.2);
		glutSolidCube(1.0);
	glPopMatrix();
	//draw dem legs
	glPushMatrix();
		glTranslatef(4.0, -7.5, -14.0);
		glScalef(1.0, 5.0, 1.0);
		//glColor3f(0.5, 0.35, 0.2);
		glutSolidCube(1.0);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(-4.0, -7.5, -14.0);
		glScalef(1.0, 5.0, 1.0);
		//glColor3f(0.5, 0.35, 0.2);
		glutSolidCube(1.0);
	glPopMatrix();
	glPopAttrib();
		glPushMatrix();
		glTranslatef(4.0, -7.5, -6.0);
		glScalef(1.0, 5.0, 1.0);
		//glColor3f(0.5, 0.35, 0.2);
		glutSolidCube(1.0);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(-4.0, -7.5, -6.0);
		glScalef(1.0, 5.0, 1.0);
		//glColor3f(0.5, 0.35, 0.2);
		glutSolidCube(1.0);
	glPopMatrix();
	glPopAttrib();
	glEndList();
}

void chairs_list(){
	glNewList(CHAIRS, GL_COMPILE);
	glPushAttrib(GL_CURRENT_BIT);
	set_material(GL_FRONT_AND_BACK, &brown);
	//draw chair bottom
	glPushMatrix();
		glTranslatef(8, -5.0, -8.0);
		glScalef(0.3, 0.5, 0.3);
		glCallList(TABLE);
	glPopMatrix();
	glPushMatrix();
		//draw chair back
		glTranslatef(9.2, -5.8, -11.0);
		glScalef(.5, 3.0, 3.0);
		glColor3f(0.5, 0.35, 0.2);
		glutSolidCube(1.0);
		glColor3f(0.0, 0.0, 0.0);
		glutWireCube(1.0);
	glPopMatrix();
	glPopAttrib();
	glEndList();
}

void windows_list(){
	glNewList(WINDOWS, GL_COMPILE);
	glPushAttrib(GL_CURRENT_BIT);
	// draw windows
	glPushMatrix();
		glTranslatef(-14.9, 0.0, 10.0);
		glScalef(0.0, 6.0, 5.0);
		glColor3f(0.0, 0.5, 0.7);
		glutSolidCube(1.0);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(-14.9, 0.0, -10.0);
		glScalef(0.0, 6.0, 5.0);
		glColor3f(0.0, 0.5, 0.7);
		glutSolidCube(1.0);
	glPopMatrix();
	glPopAttrib();
	glEndList();
}

void door_list(){
	glNewList(DOOR, GL_COMPILE);
	glPushAttrib(GL_CURRENT_BIT);
	// draw door
	glPushMatrix();
		set_material(GL_FRONT_AND_BACK, &brown);
		glTranslatef(14.9, -3.0, 10.0);
		glScalef(0.0, 14.0, 8.0);
		//glColor3f(0.5f, 0.3f, 0.01f);
		glutSolidCube(1.0);
	glPopMatrix();
	// draw door knob
	set_material(GL_FRONT_AND_BACK, &gold);
	glPushMatrix();
		glTranslatef(14.0, -3.0, 13.0);
		//glColor3f(1.0, 1.0, 0.0);
		glutSolidSphere(.5, 50, 50);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(14.5, -3.0, 13.0);
		glScalef(1.0, 0.25, 0.25);
		//glColor3f(1.0, 1.0, 0.0);
		glutSolidCube(1.0);
	glPopMatrix();
	glPopAttrib();
	glEndList();
}

void art_list(){
	glNewList(ART, GL_COMPILE);
	glPushAttrib(GL_CURRENT_BIT);
	// draw frame
	glPushMatrix();
		glTranslatef(0.0, 0.0, -19.0);
		glScalef(5.0, 5.0, 1.0);
		glColor3f(0.5f, 0.3f, 0.01f);
		glutSolidCube(1.0);
		glColor3f(0.0, 0.0, 0.0);
		glutWireCube(1.0);
	glPopMatrix();
	// draw canvas
	glPushMatrix();
		glUseProgram(textureShaderProg);
		glUniform1i(texSampler, 0);
		// Draw mirror surface
		glBindTexture(GL_TEXTURE_2D, tex_ids[ARTTEX]);
		glBegin(GL_POLYGON);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(-2.0f, -2.0f, -18.4f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(2.0f, -2.0f, -18.4f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(2.0f, 2.0f, -18.4f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(-2.0f, 2.0f, -18.4f);
		glEnd();


		glUseProgram(defaultShaderProg);
	glPopMatrix();
	glPopAttrib();
	glEndList();
}

void fan_list(){
	glNewList(FAN, GL_COMPILE);
	glPushAttrib(GL_CURRENT_BIT);
	// draw fanblades
	int loop_ang = 0.0;
	for (int i = 0; i < 4; i++){
		glPushMatrix();
			glRotatef(loop_ang, 0, 1, 0);
			glTranslatef(0.0, 9.0, 0.0);
			glRotatef(90.0, 1, 0, 0);
			glScalef(1.0, 3.0, 1.0);
			glColor3f(0.0, 1.0, 1.0);
			draw_fanblade();
		glPopMatrix();
		loop_ang += 90;
	}
	glPopAttrib();
	glEndList();
}

void blinds_list(){
	glNewList(BLINDS, GL_COMPILE);
	glPushAttrib(GL_CURRENT_BIT);
	// draw window blinds
	glPushMatrix();
		glTranslatef(0.0, 0.0, 10.0);
		glScalef(1.0, 1.0, 5.0);
		glColor3f(1.0, 1.0, 1.0);
		glutSolidCube(1.0);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(0.0, 0.0, -10.0);
		glScalef(1.0, 1.0, 5.0);
		glColor3f(1.0, 1.0, 1.0);
		glutSolidCube(1.0);
	glPopMatrix();
	glPopAttrib();
	glEndList();
}

// OTHER FUNCTIONS
void draw_square(){
	//without lighting, no recursive subdivision
	/*
	glBegin(GL_POLYGON);
		glVertex2f(sqr[0][0], sqr[0][1]);
		glVertex2f(sqr[1][0], sqr[1][1]);
		glVertex2f(sqr[2][0], sqr[2][1]);
		glVertex2f(sqr[3][0], sqr[3][1]);
	glEnd();
	*/
	//with lighting
	// nth dimension sequence of sqrs: 3, 1, 2, 0, 3, 2, 0 1
	subdivision(sqr[0], sqr[1], sqr[2], sqr[3], sqr[0], sqr[1], sqr[2], sqr[3], subdivs);
}

void draw_fanblade(){
	glBegin(GL_POLYGON);
		glVertex2f(tri[0][0], tri[0][1]);
		glVertex2f(tri[1][0], tri[1][1]);
		glVertex2f(tri[2][0], tri[2][1]);
	glEnd();
}

void draw_fanMount(){
	glPushMatrix();
	glTranslatef(0.0, 9.0, 0.0);
	glColor3f(0.0, 0.0, 1.0);
	glutSolidSphere(1.0, 50, 50);
	glPopMatrix();
}

void draw_drink(){
	// enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPushMatrix();
		glDepthMask(GL_FALSE);
		glTranslatef(2.0, -3.0, -6.0);
		glRotatef(90, 1, 0, 0);
		glColor4f(0.75, 0.0, 0.0, 0.5);			//low alpha, renders translucency
		gluCylinder(quad, .5, .5, 2.0, 50, 50);
		glDepthMask(GL_TRUE);
	glPopMatrix();
	glDisable(GL_BLEND);
}

void create_mirror()
{
	// Reset background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set projection matrix for flat "mirror" camera
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-10.0, 10.0, -5.0, 5.0, 0.0, 60.0);

	// Set modelview matrix positioning "mirror" camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 0.0, 19.8, -posX, 0.0, posZ, 0.0, 1.0, 0.0);

	// Render scene from mirror
	render_Scene();

	glFinish();

	// Copy scene to texture
	glBindTexture(GL_TEXTURE_2D, tex_ids[ENVIRONMENT]);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 512, 512, 0);
}

// Mirror render function
void render_mirror()
{
	glPushMatrix();
	glUseProgram(textureShaderProg);
	glUniform1i(texSampler, 0);
	// Draw mirror surface
	glBindTexture(GL_TEXTURE_2D, tex_ids[ENVIRONMENT]);
	glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-10.0f, -5.0f, 19.9f);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-10.0f, 5.0f, 19.9f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(10.0f, 5.0f, 19.9f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(10.0f, -5.0f, 19.9f);
	glEnd();

	
	glUseProgram(defaultShaderProg);
	glColor3f(0.0f, 0.0f, 0.0f);
	glPopMatrix();
	// Draw mirror frame
	glPushMatrix();
		glTranslatef(0.0, 0.0, 20);
		glScalef(21.0, 11.0, 0.1);
		glColor3f(0.0f, 0.0f, 0.0f);
		glutSolidCube(1.0);
	glPopMatrix();
}

// Routine to load textures using SOIL
bool load_textures()
{
	// Load environment map texture (NO MIPMAPPING)
	tex_ids[ENVIRONMENT] = SOIL_load_OGL_texture(texture_files[0], SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);

	// TODO: Set environment map properties if successfully loaded
	if (tex_ids[ENVIRONMENT] != 0)
	{
		// Set scaling filters (no mipmap)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// Set wrapping modes (clamped)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	}
	// Otherwise texture failed to load
	else
	{
		return false;
	}

	// Load object textures normally
	for (int i = 1; i < END_TEXTURE_LIST; i++)
	{
		// Load images
		tex_ids[i] = SOIL_load_OGL_texture(texture_files[i], SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);


		// Set texture properties if successfully loaded
		if (tex_ids[i] != 0)
		{
			// Set scaling filters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

			// Set wrapping modes
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		// Otherwise texture failed to load
		else
		{
			return false;
		}
	}
	return true;
}

// Routine to perform recursive subdivision
void subdivision(GLfloat v1[], GLfloat v2[], GLfloat v3[], GLfloat v4[], GLfloat n1[], GLfloat n2[], GLfloat n3[], GLfloat n4[], int n)
{
	GLfloat v1p[3], v2p[3], v3p[3], v4p[3], v5p[3];
	GLfloat n1p[3], n2p[3], n3p[3], n4p[3], n5p[3];
	if (n > 0)
	{
		for (int i = 0; i<3; i++)
		{
			v1p[i] = (v4[i] + v1[i]) / 2.0f;
			v2p[i] = (v1[i] + v2[i]) / 2.0f;
			v3p[i] = (v2[i] + v3[i]) / 2.0f;
			v4p[i] = (v3[i] + v4[i]) / 2.0f;
			v5p[i] = (v1[i] + v2[i] + v3[i] + v4[i]) / 4.0f;
			n1p[i] = (n4[i] + n1[i]) / 2.0f;
			n2p[i] = (n1[i] + n2[i]) / 2.0f;
			n3p[i] = (n2[i] + n3[i]) / 2.0f;
			n4p[i] = (n3[i] + n4[i]) / 2.0f;
			n5p[i] = (n1[i] + n2[i] + n3[i] + n4[i]) / 4.0f;
		}
		// subdivide stage
		subdivision(v1, v2p, v5p, v1p, n1, n2p, n5p, n1p, n - 1);
		subdivision(v2p, v2, v3p, v5p, n2p, n2, n3p, n5p, n - 1);
		subdivision(v1p, v5p, v4p, v4, n1p, n5p, n4p, n4, n - 1);
		subdivision(v5p, v3p, v3, v4p, n5p, n3p, n3, n4p, n - 1);
	}
	else
	{
		//render this stage face
		renderSubdiv(v1, v2, v3, v4, n1, n2, n3, n4);
	}
}

// render stage
void renderSubdiv(GLfloat v1[], GLfloat v2[], GLfloat v3[], GLfloat v4[], GLfloat n1[], GLfloat n2[], GLfloat n3[], GLfloat n4[])
{
	glBegin(GL_POLYGON);
		glNormal3fv(n1);
		glVertex3fv(v1);
		glNormal3fv(n2);
		glVertex3fv(v2);
		glNormal3fv(n3);
		glVertex3fv(v3);
		glNormal3fv(n4);
		glVertex3fv(v4);
	glEnd();
}