#include <windows.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <string>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "glut.h"

using namespace std;

#define RED   1
#define GREEN 2
#define BLUE  3
#define WHITE 4

#define FILL 1
#define WIRE_FRAME 2

#define PERPECTIVE_PROJECTION 1
#define PARALLEL_PROJECTION   2
#define NEAR_INITIAL    5.0
#define FAR_INITIAL    100.0
#define WINDOW_SIZE 600
#define OBJECT_INITIAL "PenroseTriangle2.obj"

void readObjFile(string fileName);
void readFaceString(string face, int &tempNormal, int &tempTexture, vector<int> &tempVertices);
void reshape(int w, int h);

string objName = OBJECT_INITIAL;

int projection = PERPECTIVE_PROJECTION;
int objChoice = 1;
int width, height;  // display window width & height
float nearClip = NEAR_INITIAL;
float farClip = FAR_INITIAL;
float angleX = -34.5;
float angleY = -47.1;
float angleZ = 0;

float matSpecR = 1.0, matSpecG = 1.0, matSpecB = 1.0;
float matAmbientR = 0.2, matAmbientG = 0.2, matAmbientB = 0.2;
float matDiffuseR = 0.8, matDiffuseG = 0.8, matDiffuseB = 0.8;
float matShini = 50.0;      // [0.0, 128.0]  higher -> smaller & brighter

float globalAmbient = 0.1;
float lightX = 30.0, lightY = 80.0, lightZ = 70.0; 
float lightDiffuse = 0.9;

vector<vector<float>> vertices;
vector<vector<float>> normals;
vector<string> faces;

float xTrans, yTrans, zTrans;
float scale = 0.127;


void myInit(void) 
{
   glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
   glColorMaterial( GL_FRONT, GL_DIFFUSE );
   glEnable( GL_COLOR_MATERIAL );

   glClearColor (0.0, 0.0, 0.0, 0.0);
   glEnable(GL_LIGHTING);
   glEnable(GL_DEPTH_TEST);

   cout << "Switch between Objects with 1, 2, 3, 4" << endl;
   cout << "Rotate the object around the X, Y, Z Axes with A+D, W+S, and Q+E respectively" << endl;
   cout << "Move the light source along the X, Y, Z Axes with H+K, U+J, and Y+I respectively" << endl; 
}

void drawIllusion( void )
{
   //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
   //glColorMaterial( GL_FRONT, GL_DIFFUSE );
   //glEnable( GL_COLOR_MATERIAL );

   glColor3f( 1.0, 1.0, 1.0 );  

   int tempNormal = 0;
   int tempTexture = 0;
   vector<int> tempVertices;

   for (auto face = faces.begin(); face != faces.end(); face++) {
	   readFaceString(*face, tempNormal, tempTexture, tempVertices);
	   //cout << "Normal: " << tempNormal << endl;
	   for (auto d = tempVertices.begin(); d != tempVertices.end(); d++) {
		   //cout << *d << " ";
	   }
	   //cout << endl;

	   float tNormal[3];
	   float tVerts[3];
	   int currentV = 0;
	   //cout << "NORMAL" << endl;
	   for (auto n = normals.at(tempNormal-1).begin(); n != normals.at(tempNormal - 1).end(); n++) {
		   //cout << *n << " ";
		   tNormal[currentV] = *n;
		   currentV++;
	   }
	   //cout << endl;
	   currentV = 0;

		glBegin(GL_POLYGON);
			glNormal3f(tNormal[0], tNormal[1], tNormal[2]);

			//cout << "POLYGON" << endl;
			for (auto v = tempVertices.begin(); v != tempVertices.end(); v++) { // Goes through face vertices
				//cout << "VERTEX" << endl;
				for (auto z = vertices.at(*v - 1).begin(); z != vertices.at(*v - 1).end(); z++) { // locates vertiex and iterates through coordinates
					//cout << *z << " ";
					tVerts[currentV] = *z;
					currentV++;
				}
				glVertex3f((tVerts[0]-xTrans)*scale, (tVerts[1]-yTrans)*scale, (tVerts[2]-zTrans)*scale);
				currentV = 0;
				//cout << endl;
			}

		glEnd();

	   tempVertices.clear();
   }
}

void calculateAdjustments() {
	float minX = 1000000;
	float maxX = 0;
	float minY = 1000000;
	float maxY = 0;
	float minZ = 1000000;
	float maxZ = 0;

	for (auto row = vertices.begin(); row != vertices.end(); row++) {
		int current = 0;
		for (auto col = row->begin(); col != row->end(); col++) {
			if (current == 0) {
				if (*col > maxX) { maxX = *col; }
				if (*col < minX) { minX = *col; }
			}
			else if (current == 1) {
				if (*col > maxY) { maxY = *col; }
				if (*col < minY) { minY = *col; }
			}
			else if (current == 2) {
				if (*col > maxZ) { maxZ = *col; }
				if (*col < minZ) { minZ = *col; }
				current = -1;
			}
			current++;
		}
	}
	//cout << maxX << " " << maxY << " " << maxZ << " " << scaleMax << endl;

	xTrans = (minX + maxX) / 2;
	yTrans = (minY + maxY) / 2;
	zTrans = (minZ + maxZ) / 2;
}


void readFaceString(string face, int &tempNormal, int &tempTexture, vector<int> &tempVertices) {

	string tempStr = "";
	int currentP = 0;
	for (int c = 2; c < face.length(); c++) {
		if (face[c] == '/') {
			if (currentP == 0) {
				currentP++;
				tempVertices.push_back(atoi(tempStr.c_str()));
				tempStr = "";
			}
			else if (currentP == 1) {
				currentP++;
				tempTexture = atoi(tempStr.c_str());
				tempStr = "";
			}
		}
		else if (face[c] == ' ' || face[c] == '\0') {
			currentP = 0;
			tempNormal = atoi(tempStr.c_str());
			tempStr = "";
		}
		else {
			tempStr = tempStr + face[c];
		}
	}
}

void readObjFile(string fileName){
	ifstream file (fileName);

	vertices.clear();
	normals.clear();
	faces.clear();

    string str;
	if (file.is_open()) {
		while (getline(file, str)) //while we are still in the file
		{
			//cout << str << endl;
			if (str[0] == 'v') {
				if (str[1] == ' ') { // Vectors
					vector<float> vTemp;
					string word = "";
					bool head = true;
					bool prev = false;
					for (auto x : str) {
						if (x == ' ') {
							if (head == true) {
								head = false;
								word = "";
								prev = true;
							}
							else if (prev == true) {
								prev = false;
							}
							else if (head == false) {
								vTemp.push_back(atof(word.c_str()));
								//cout << word << endl;
								word = "";
							}
						}
						else {
							word = word + x;
							prev = false;
						}
					}
					vTemp.push_back(atof(word.c_str()));
					//cout << word << endl;
					word = "";
					vertices.push_back(vTemp);
					//for (int i = 0; i < vTemp.size(); i++) {
					//	cout << vTemp[i] << " ";
					//}
					vTemp.clear();
				}
				else if (str[1] == 'n') { // Vector Normals
					vector<float> vnTemp;
					string word = "";
					bool head = true;
					bool prev = false;
					for (auto x : str) {
						if (x == ' ') {
							if (head == true) {
								head = false;
								word = "";
								prev = true;
							}
							else if (prev == true) {
								prev = false;
							}
							else if (head == false) {
								vnTemp.push_back(atof(word.c_str()));
								//cout << word << endl;
								word = "";
								prev = false;
							}
						}
						else {
							word = word + x;
							prev = false;
						}
					}
					vnTemp.push_back(atof(word.c_str()));
					//cout << word << endl;
					word = "";
					normals.push_back(vnTemp);
					vnTemp.clear();
				}
			}
			else if (str[0] == 'f') {
				faces.push_back(str);
			}
		}
	}
	file.close();
}


void myKeyboard( unsigned char key, int x, int y ) 
{
	switch (key)
	{
		case '1':  	objChoice = 1;
					objName = "PenroseTriangle2.obj";
					readObjFile(objName);
					calculateAdjustments();
					angleX = -34.5;
					angleY = -47.1;
					angleZ = 0;
					lightX = 30.0; 
					lightY = 80.0; 
					lightZ = 70.0;
					scale = 0.127;
					break;
		case '2':  	objChoice = 2;
					objName = "PenroseStairs2.obj";
					readObjFile(objName);
					calculateAdjustments();
					angleX = -185;
					angleY = -138;
					angleZ = 75;
					scale = 0.05;
					break;
		case '3':  	objChoice = 3;
					objName = "EscherCube.obj";
					readObjFile(objName);
					calculateAdjustments();
					angleX = 25.55;
					angleY = -69.2;
					angleZ = 0;
					lightX = 30.0;
					lightY = 100.0;
					lightZ = 70.0;
					scale = 0.0095;
					break;
		case '4':  	objChoice = 4;
					objName = "ImpossibleTriangle.obj";
					readObjFile(objName);
					calculateAdjustments();
					angleX = 5;
					angleY = -4;
					angleZ = 0;
					lightX = 30.0;
					lightY = 70.0;
					lightZ = 30.0;
					scale = 0.09;
					break;
		case '.':  	objChoice = 1;
					objName = "PenroseTriangle2.obj";
					readObjFile(objName);
					calculateAdjustments();
					angleX = -34.5;
					angleY = -47.1;
					angleZ = -3;
					lightX = 30.0;
					lightY = 80.0;
					lightZ = 70.0;
					scale = 0.127;
					break;
		case 'a':  	angleX--;
			break;
		case 'd':  	angleX++;
			break;
		case 'w':  	angleY++;
			break;
		case 's':  	angleY--;
			break;
		case 'e':  	angleZ++;
			break;
		case 'q':  	angleZ--;
			break;
		case 'h':  	lightX--;
			break;
		case 'k':  	lightX++;
			break;
		case 'u':  	lightY++;
			break;
		case 'j':  	lightY--;
			break;
		case 'y':  	lightZ--;
			break;
		case 'i':  	lightZ++;
			break;
	}

	reshape(width, height);
	glutPostRedisplay();

}

void drawPenroseTriangle( void )
{
   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity ();             
   glPushMatrix();
   gluLookAt (0.0, 0.0, 30.0, 0.5, -0.5, 0.0, 0.0, 1.0, 0.0);
   glRotatef(angleX, 0, 1, 0);
   glRotatef(angleY, 1, 0, 0);
   glRotatef(angleZ, 0, 0, 1);
   drawIllusion();
   glPopMatrix();
   glFlush ();
}

void drawPenroseStairs( void )
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	gluLookAt(0.0, 0.0, 30.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glRotatef(angleX, 0, 1, 0);
	glRotatef(angleY, 1, 0, 0);
	glRotatef(angleZ, 0, 0, 1);
	drawIllusion();
	glPopMatrix();
	glFlush();
}

void drawEscherCube( void )
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	gluLookAt(0.0, 0.0, 30.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glRotatef(angleX, 0, 1, 0);
	glRotatef(angleY, 1, 0, 0);
	glRotatef(angleZ, 0, 0, 1);
	drawIllusion();
	glPopMatrix();
	glFlush();
}

void drawImpossibleTriangle( void )
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	gluLookAt(0.0, 0.0, 30.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glRotatef(angleX, 0, 1, 0);
	glRotatef(angleY, 1, 0, 0);
	glRotatef(angleZ, 0, 0, 1);
	drawIllusion();
	glPopMatrix();
	glFlush();
}

void setMaterials(void) 
{
   GLfloat mat_ambient[]   = { matAmbientR, matAmbientG, matAmbientB, 1.0 };
   GLfloat mat_diffuse[]   = { matDiffuseR, matDiffuseR, matDiffuseR, 1.0 };
   GLfloat mat_specular[]  = { matSpecR, matSpecG, matSpecB, 1.0 };
   GLfloat mat_shininess[] = { matShini };

   // set material properties
   glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

void setLighting(void) 
{
   const GLfloat DIRECTIONAL = 0.0;
   const GLfloat POSITIONAL = 1.0;

   // set global light properties
   GLfloat lmodel_ambient[] = { globalAmbient, globalAmbient, globalAmbient, 1.0 }; 
   glLightModelfv( GL_LIGHT_MODEL_AMBIENT, lmodel_ambient );

   GLfloat light_position[] = { lightX, lightY, lightZ, POSITIONAL };  
   GLfloat light_ambient[] = { 0.7, 0.7, 0.7, 1.0 };
   GLfloat light_diffuse[] = { lightDiffuse, lightDiffuse, lightDiffuse, 1.0 };
   GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };

   // set properties this light 
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
   glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

   glEnable(GL_LIGHT0);
}

void myDisplay(void)
{
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // z-buffering
   setMaterials();
   setLighting();
   glColor3f (1.0, 1.0, 1.0);
   glShadeModel (GL_FLAT);
   glEnable( GL_NORMALIZE );

   switch (objChoice)
   {
     case 1:  drawPenroseTriangle();
		      break;
     case 2:  drawPenroseStairs();
		      break;
     case 3:  drawEscherCube();
		      break;
     case 4:  drawImpossibleTriangle();
		      break;
  }

  glutPostRedisplay();
}

void reshape (int w, int h)
{
   width = w;
   height = h;

   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();


   switch (objChoice)
   {
     case 1:     if (projection == PERPECTIVE_PROJECTION)
					glFrustum(-1.0, 1.0, -1.0, 1.0, nearClip, farClip);
                 else
					glOrtho(-4.0, 4.0, -4.0, 4.0, nearClip, farClip);
		         break;

	 case 2:        if (projection == PERPECTIVE_PROJECTION) 
						 glFrustum(-1.0, 1.0, -1.0, 1.0, nearClip, farClip);
					else 
						 glOrtho(-5.0, 5.0, -5.0, 5.0, nearClip, farClip);
					break;

	 case 3:     if (projection == PERPECTIVE_PROJECTION)
						 glFrustum(-1.0, 1.0, -1.0, 1.0, nearClip + 24, farClip);
				 else 
						 glOrtho(-1.0, 1.0, -1.0, 1.0, nearClip, farClip);
				 break;

     case 4:     if (projection == PERPECTIVE_PROJECTION)
					 glFrustum(-1.0, 1.0, -1.0, 1.0, nearClip, farClip);
				else
					 glOrtho(-5.0, 5.0, -5.0, 5.0, nearClip, farClip);
				break;

  }
}

void processRightMenuEvents(int option) 
{
	projection = option;
	reshape(width, height);
	glutPostRedisplay();
}

void processColorSubmenuEvents(int option) {
	switch (option) {
	case RED: 
		matSpecR = 0.8, matSpecG = 0.0, matSpecB = 0.0;
		matAmbientR = 0.8, matAmbientG = 0.0, matAmbientB = 0.0;
		matDiffuseR = 0.8, matDiffuseG = 0.0, matDiffuseB = 0.0;
		break;
	case GREEN:
		matSpecR = 0.0, matSpecG = 0.8, matSpecB = 0.0;
		matAmbientR = 0.0, matAmbientG = 0.8, matAmbientB = 0.0;
		matDiffuseR = 0.0, matDiffuseG = 0.8, matDiffuseB = 0.0;
		break;
	case BLUE:
		matSpecR = 0.0, matSpecG = 0.0, matSpecB = 0.8;
		matAmbientR = 0.0, matAmbientG = 0.0, matAmbientB = 0.8;
		matDiffuseR = 0.0, matDiffuseG = 0.0, matDiffuseB = 0.8;
		break;
	case WHITE:
		matSpecR = 1.0, matSpecG = 1.0, matSpecB = 1.0;
		matAmbientR = 0.2, matAmbientG = 0.2, matAmbientB = 0.2;
		matDiffuseR = 0.8, matDiffuseG = 0.8, matDiffuseB = 0.8;
		break;
	}
}

void processDrawTypeSubmenuEvents(int option) {
	switch (option) {
	case FILL:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case WIRE_FRAME:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	}
}

//<<<<<<<<<<<<<<main>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int main(int argc, char** argv) {

	readObjFile(objName);
	calculateAdjustments();

    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH ); // z-buffering
    glutInitWindowSize(WINDOW_SIZE, WINDOW_SIZE);
    glutInitWindowPosition(250,100);
    glutCreateWindow("Impossible Objects");
	glEnable( GL_DEPTH_TEST );
    myInit();
    glutDisplayFunc(myDisplay);
    glutReshapeFunc(reshape);

	int ColorSubmenu;
	int DrawTypeSubmenu;

	ColorSubmenu = glutCreateMenu(processColorSubmenuEvents);
	glutAddMenuEntry("Red", RED);
	glutAddMenuEntry("Blue", BLUE);
	glutAddMenuEntry("Green", GREEN);
	glutAddMenuEntry("White", WHITE);

	DrawTypeSubmenu = glutCreateMenu(processDrawTypeSubmenuEvents);
	glutAddMenuEntry("Fill", FILL);
	glutAddMenuEntry("Wire Frame", WIRE_FRAME);

	glutCreateMenu(processRightMenuEvents);
	glutAddMenuEntry("perspective", PERPECTIVE_PROJECTION);
	glutAddMenuEntry("parallel", PARALLEL_PROJECTION);
	glutAddSubMenu("rgb colors", ColorSubmenu);
	glutAddSubMenu("Draw Type", DrawTypeSubmenu);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutKeyboardFunc( myKeyboard );

	glutMainLoop( );

	return 0;
}
