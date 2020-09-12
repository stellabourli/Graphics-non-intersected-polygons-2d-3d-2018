#include "triangulate.h"
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
using namespace std;

#define KEY_ESC 27
#define KEY_T_UPPER  84
#define KEY_T_LOWER  116
#define KEY_P_UPPER  80
#define KEY_P_LOWER  112
#define KEY_L_UPPER  76
#define KEY_L_LOWER  108

float line_color[] = {0.0, 0.0, 0.0};
float fill_color[] = {1.0, 1.0, 1.0};

typedef struct {
  int x;
  int y;
} point;

vector<point> points;

//Polygon Creation
int count_points = 0;   //Counts points of a new polygon.
bool last_point = false;    //End of polygon, draw last line.

typedef struct {
  int first;    //Positions of elements in points vector.
  int last;
  float cline[3];   //Colors of a polygon.
  float cfill[3];
  int extrude_length;   //The given z length.
} polygon;

vector<polygon> polygons;

polygon tempPolygon;    //For temporary uses.

//Drawing Points and Lines
bool polygon_flag = false;   //Enables user clicks to draw points and lines.
bool draw_point = true;    //Enables drawing the first point.

//Triangulation
bool triangulate_ON = false;   //Enables showing triangulation triangles.
vector<point> trianglePoints;   //Store triangulation points and triangles.
vector<polygon> triangles;

//Clipping
int clippingPointsCounter = 0;   //Fills with 2 points for clipping.
point clippingPointDownLeft;    //Store points for clipping.
point clippingPointUpRight;

vector<point> clippedPolygonPointsStart;   //They are used to store points and
vector<point> clippedPolygonPointsEnd;     //polygons for every step of
vector<polygon> clippedPolygonsStart;      //clipping.
vector<polygon> clippedPolygonsEnd;

int counterFindLast = 0;    //For first and last points of a polygon
int counterFindFirst = 0;   //that is result of clipping.

point startOfPolygon;    //Stores the first point of a polygon before clipping
                         //to check if the polygon is closed

//Extrude
bool extrude_mode = false;  //Enables draw_3D and 3D projection.

float angle = 0.0f;    //Movement Variables (stable)
float lx=0.0f,lz=-1.0f;
float x=300.0f, z=600.0f;
float up=250.0f;

float deltaAngle = 0.0f;  //Movement Variables (changing)
float deltaMove = 0;
float deltaUp = 0;

//Function Protorypes
GLvoid initGL();
GLvoid display();
void createPointsAndLines();
void draw();
void menu();
void selectFunction(int);
void selectAction(int);
void selectLineColor(int);
void selectFillColor(int);
void keyboard(unsigned char, int, int);
void releaseKey(unsigned char, int, int);
void specialKeyboard(int, int, int);
void releaseSpecialKey(int, int, int);
void mouse(int, int, int, int);
void idle();
bool checkIntersection(point, point, point, point);
void createTriangles();
Vector2dVector convertInput(int);
void convertOutput(Vector2dVector);
void clippingAlgorithm();
point intersection_point(char, int, point, point);
void check_first_and_last_points(int);
void check_last_point_intersection();
void switch_polygons_to_clippedPolygons();
void extrude_polygon();
void draw_3D();
void computePos(float);
void computeDir(float);
void computeUp(float);

int main(int argc, char *argv[]) {
  glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(600, 500);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Project");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  initGL();
  glutKeyboardFunc(keyboard);
  glutKeyboardUpFunc(releaseKey);
  glutSpecialFunc(specialKeyboard);
  glutSpecialUpFunc(releaseSpecialKey);
  glutMouseFunc(mouse);
  menu();
  glutMainLoop();
  return 1;
}

void idle() {   //Flushes the screen.
  glutPostRedisplay();
}

GLvoid initGL() {
  glClearDepth(1.0f);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glMatrixMode(GL_PROJECTION);
  gluOrtho2D(0, 600, 0, 500);
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if(extrude_mode==false) {
    draw();
  }
  else {
    draw_3D();
  }
  glutSwapBuffers();
}

void draw() {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if(polygons.size() > 0) {
    for(int i=0; i<polygons.size(); i++){   //Coloring inside part of a poligon.
      for(int j=triangles[i].first; j<triangles[i].last-2; j+=3) {
        glColor3f(polygons[i].cfill[0], polygons[i].cfill[1], polygons[i].cfill[2]);
        glBegin(GL_TRIANGLES);
          glVertex2f(trianglePoints[j].x, trianglePoints[j].y);
          glVertex2f(trianglePoints[j+1].x, trianglePoints[j+1].y);
          glVertex2f(trianglePoints[j+2].x, trianglePoints[j+2].y);
        glEnd();
      }

      for(int j=polygons[i].first+1; j<=polygons[i].last; j++) {    //Drawing a polygon except last line.
        glColor3f(polygons[i].cline[0], polygons[i].cline[1], polygons[i].cline[2]);
        glBegin(GL_LINES);
          glVertex2f(points[j-1].x, points[j-1].y);
          glVertex2f(points[j].x, points[j].y);
        glEnd();
      }

      glBegin(GL_LINES);    //Drawing last line.
        glVertex2f(points[polygons[i].first].x, points[polygons[i].first].y);
        glVertex2f(points[polygons[i].last].x, points[polygons[i].last].y);
      glEnd();
    }
  }

  if(polygon_flag == false && triangulate_ON == true) {   //Drawing Triangulation Triangles (green)
    for(int i=0; i<trianglePoints.size()-2; i+=3) {
      glColor3f(0.0f, 1.0f, 0.0f);
      glBegin(GL_LINES);
        glVertex2f(trianglePoints[i].x, trianglePoints[i].y);
        glVertex2f(trianglePoints[i+1].x, trianglePoints[i+1].y);
      glEnd();
      glBegin(GL_LINES);
        glVertex2f(trianglePoints[i].x, trianglePoints[i].y);
        glVertex2f(trianglePoints[i+2].x, trianglePoints[i+2].y);
      glEnd();
      glBegin(GL_LINES);
        glVertex2f(trianglePoints[i+1].x, trianglePoints[i+1].y);
        glVertex2f(trianglePoints[i+2].x, trianglePoints[i+2].y);
      glEnd();
    }
  }
  createPointsAndLines();
}

void createPointsAndLines() {   //Drawing user points and lines.
  if(count_points == 1) {
    if(draw_point) {    //Drawing first point, and defining colors.
      tempPolygon.first = points.size()-1;
      for(int i=0; i<3; i++) {
        tempPolygon.cline[i] = line_color[i];
        tempPolygon.cfill[i] = fill_color[i];
      }
    }

    glPointSize(3.0f);
    glColor3f(tempPolygon.cline[0], tempPolygon.cline[1], tempPolygon.cline[2]);
    glBegin(GL_POINTS);
      glVertex2f(points[points.size()-1].x, points[points.size()-1].y);
    glEnd();

    draw_point = false;
  }
  else if(count_points > 1) {   //Drawing lines except last line.
    glColor3f(tempPolygon.cline[0], tempPolygon.cline[1], tempPolygon.cline[2]);
    for(int i=tempPolygon.first+1; i<points.size(); i++) {
      glBegin(GL_LINES);
        glVertex2f(points[i-1].x, points[i-1].y);
        glVertex2f(points[i].x, points[i].y);
      glEnd();
    }
  }

  if(last_point) {    //Drawing last line, and adding polygon on the polygons list.
    tempPolygon.last = points.size()-1;
    glColor3f(tempPolygon.cline[0], tempPolygon.cline[1], tempPolygon.cline[2]);
    glBegin(GL_LINES);
      glVertex2f(points[points.size()-1].x, points[points.size()-1].y);
      glVertex2f(points[tempPolygon.first].x, points[tempPolygon.first].y);
    glEnd();
    polygons.push_back(tempPolygon);
    createTriangles();
    count_points = 0;
    last_point = false;
    polygon_flag = false;
    draw_point = true;
  }
}

void draw_3D() {

  if (deltaMove)    //Check for movement.
    computePos(deltaMove);
  if (deltaAngle)
    computeDir(deltaAngle);
  if (deltaUp)
    computeUp(deltaUp);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  gluLookAt(x, up, z,
				    x+lx, up, z+lz,
				    0.0f, 1.0f, 0.0f);

  int empty_field_length = 50;  //Field beside polygons.

  for(int i=0; i<polygons.size(); i++){   //Coloring front side.
    for(int j=triangles[i].first; j<triangles[i].last-2; j+=3) {
      glColor3f(polygons[i].cfill[0], polygons[i].cfill[1], polygons[i].cfill[2]);
      glBegin(GL_TRIANGLES);
        glVertex3f(trianglePoints[j].x, trianglePoints[j].y, i*(polygons[i].extrude_length - empty_field_length));
        glVertex3f(trianglePoints[j+1].x, trianglePoints[j+1].y, i*(polygons[i].extrude_length - empty_field_length));
        glVertex3f(trianglePoints[j+2].x, trianglePoints[j+2].y, i*(polygons[i].extrude_length - empty_field_length));
      glEnd();
    }
  }

  for(int i=0; i<polygons.size(); i++) {  //Drawing front side polygon.
    for(int j=polygons[i].first+1; j<=polygons[i].last; j++) {
      glColor3f(polygons[i].cline[0], polygons[i].cline[1], polygons[i].cline[2]);
      glBegin(GL_LINES);
        glVertex3f(points[j-1].x, points[j-1].y, i*(polygons[i].extrude_length - empty_field_length));
        glVertex3f(points[j].x, points[j].y, i*(polygons[i].extrude_length - empty_field_length));
      glEnd();
    }
    glBegin(GL_LINES);
      glVertex3f(points[polygons[i].first].x, points[polygons[i].first].y, i*(polygons[i].extrude_length - empty_field_length));
      glVertex3f(points[polygons[i].last].x, points[polygons[i].last].y, i*(polygons[i].extrude_length - empty_field_length));
    glEnd();
  }


  for(int i=0; i<polygons.size(); i++){   //Coloring back side.
    for(int j=triangles[i].first; j<triangles[i].last-2; j+=3) {
      glColor3f(polygons[i].cfill[0], polygons[i].cfill[1], polygons[i].cfill[2]);
      glBegin(GL_TRIANGLES);
        glVertex3f(trianglePoints[j].x, trianglePoints[j].y, i*(polygons[i].extrude_length - empty_field_length) + polygons[i].extrude_length);
        glVertex3f(trianglePoints[j+1].x, trianglePoints[j+1].y, i*(polygons[i].extrude_length - empty_field_length) + polygons[i].extrude_length);
        glVertex3f(trianglePoints[j+2].x, trianglePoints[j+2].y, i*(polygons[i].extrude_length - empty_field_length) + polygons[i].extrude_length);
      glEnd();
    }
  }


  for(int i=0; i<polygons.size(); i++) {    //Drawing back side polygon.
    for(int j=polygons[i].first+1; j<=polygons[i].last; j++) {
      glColor3f(polygons[i].cline[0], polygons[i].cline[1], polygons[i].cline[2]);
      glBegin(GL_LINES);
        glVertex3f(points[j-1].x, points[j-1].y, i*(polygons[i].extrude_length - empty_field_length) + polygons[i].extrude_length);
        glVertex3f(points[j].x, points[j].y, i*(polygons[i].extrude_length - empty_field_length) + polygons[i].extrude_length);
      glEnd();
    }
    glBegin(GL_LINES);
      glVertex3f(points[polygons[i].first].x, points[polygons[i].first].y, i*(polygons[i].extrude_length - empty_field_length) + polygons[i].extrude_length);
      glVertex3f(points[polygons[i].last].x, points[polygons[i].last].y, i*(polygons[i].extrude_length - empty_field_length) + polygons[i].extrude_length);
    glEnd();
  }


  for(int i=0; i<polygons.size(); i++) {  //Drawing the other sides.
    for(int j=polygons[i].first+1; j<=polygons[i].last; j++) {
      glColor3f(polygons[i].cline[0], polygons[i].cline[1], polygons[i].cline[2]);
      glBegin(GL_QUADS);
        glVertex3f(points[j-1].x, points[j-1].y, i*(polygons[i].extrude_length - empty_field_length));
        glVertex3f(points[j].x, points[j].y, i*(polygons[i].extrude_length - empty_field_length));
        glVertex3f(points[j].x, points[j].y, i*(polygons[i].extrude_length - empty_field_length) + polygons[i].extrude_length);
        glVertex3f(points[j-1].x, points[j-1].y, i*(polygons[i].extrude_length - empty_field_length) + polygons[i].extrude_length);
      glEnd();
    }
    glBegin(GL_QUADS);
      glVertex3f(points[polygons[i].first].x, points[polygons[i].first].y, i*(polygons[i].extrude_length - empty_field_length));
      glVertex3f(points[polygons[i].last].x, points[polygons[i].last].y, i*(polygons[i].extrude_length - empty_field_length));
      glVertex3f(points[polygons[i].last].x, points[polygons[i].last].y, i*(polygons[i].extrude_length - empty_field_length) + polygons[i].extrude_length);
      glVertex3f(points[polygons[i].first].x, points[polygons[i].first].y, i*(polygons[i].extrude_length - empty_field_length) + polygons[i].extrude_length);
    glEnd();
  }

}

void menu() {
  int action_submenu = glutCreateMenu(selectAction);
	glutAddMenuEntry("POLYGON", 1);
  glutAddMenuEntry("CLIPPING", 2);
  glutAddMenuEntry("EXTRUDE", 3);
	glutAddMenuEntry("EXIT", 4);
  glutCreateMenu(selectFunction);
	int line_color_submenu = glutCreateMenu(selectLineColor);
	glutAddMenuEntry("White", 0);
	glutAddMenuEntry("Black", 1);
	glutAddMenuEntry("Brown", 2);
	glutAddMenuEntry("Red", 3);
	glutAddMenuEntry("Orange", 4);
	glutAddMenuEntry("Yellow", 5);
	glutAddMenuEntry("BlueViolet", 6);
	glutAddMenuEntry("Blue", 7);
	glutAddMenuEntry("Purple", 8);
	glutAddMenuEntry("Gray", 9);
	glutAddMenuEntry("Pink", 10);
	glutAddMenuEntry("Gold", 11);
	glutAddMenuEntry("Silver", 12);
	glutAddMenuEntry("Bronze", 13);
	glutAddMenuEntry("Coral", 14);
	glutAddMenuEntry("Scarlet", 15);
	glutCreateMenu(selectFunction);
	int fill_color_submenu = glutCreateMenu(selectFillColor);
	glutAddMenuEntry("White", 0);
	glutAddMenuEntry("Black", 1);
	glutAddMenuEntry("Brown", 2);
	glutAddMenuEntry("Red", 3);
	glutAddMenuEntry("Orange", 4);
	glutAddMenuEntry("Yellow", 5);
	glutAddMenuEntry("BlueViolet", 6);
	glutAddMenuEntry("Blue", 7);
	glutAddMenuEntry("Purple", 8);
	glutAddMenuEntry("Gray", 9);
	glutAddMenuEntry("Pink", 10);
	glutAddMenuEntry("Gold", 11);
	glutAddMenuEntry("Silver", 12);
	glutAddMenuEntry("Bronze", 13);
	glutAddMenuEntry("Coral", 14);
	glutAddMenuEntry("Scarlet", 15);
	glutCreateMenu(selectFunction);
	glutAddSubMenu("ACTION", action_submenu);
	glutAddSubMenu("LINE_COLOR", line_color_submenu);
	glutAddSubMenu("FILL_COLOR", fill_color_submenu);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);
}

void selectFunction(int value) {
  glutPostRedisplay();
}

void selectAction(int action) {
  switch (action) {
  case 1:
    polygon_flag = true;
    break;
  case 2:
    clippingPointsCounter = 2;
    break;
  case 3:
    extrude_polygon();
    extrude_mode=true;
    break;
  case 4:
    exit(-1);
    break;
  }
}

void selectLineColor(int option) {
	switch (option) {
	case 0:
		line_color[0] = 1.0; line_color[1] = 1.0; line_color[2] = 1.0;
		break;
	case 1:
		line_color[0] = 0.0; line_color[1] = 0.0; line_color[2] = 0.0;
		break;
	case 2:
		line_color[0] = 0.647059; line_color[1] = 0.164706; line_color[2] = 0.164706;
		break;
	case 3:
		line_color[0] = 1.0; line_color[1] = 0.0; line_color[2] = 0.0;
		break;
	case 4:
		line_color[0] = 1.0; line_color[1] = 0.5; line_color[2] = 0.0;
		break;
	case 5:
		line_color[0] = 1.0; line_color[1] = 1.0; line_color[2] = 0.0;
		break;
	case 6:
		line_color[0] = 0.62352; line_color[1] = 0.372549; line_color[2] = 0.623529;
		break;
	case 7:
		line_color[0] = 0.0; line_color[1] = 0.0; line_color[2] = 1.0;
		break;
	case 8:
		line_color[0] = 0.73; line_color[1] = 0.16; line_color[2] = 0.96;
		break;
	case 9:
		line_color[0] = 0.752941; line_color[1] = 0.752941; line_color[2] = 0.752941;
		break;
	case 10:
		line_color[0] = 1.0; line_color[1] = 0.0; line_color[2] = 1.0;
		break;
	case 11:
		line_color[0] = 0.8; line_color[1] = 0.498039; line_color[2] = 0.196078;
		break;
	case 12:
		line_color[0] = 0.90; line_color[1] = 0.91; line_color[2] = 0.98;
		break;
	case 13:
		line_color[0] = 0.55; line_color[1] = 0.47; line_color[2] = 0.14;
		break;
	case 14:
		line_color[0] = 1.0; line_color[1] = 0.498039; line_color[2] = 0.0;
		break;
	case 15:
		line_color[0] = 0.55; line_color[1] = 0.09; line_color[2] = 0.09;
		break;
	}
}

void selectFillColor(int option) {
	switch (option) {
	case 0:
		fill_color[0] = 1.0; fill_color[1] = 1.0; fill_color[2] = 1.0;
		break;
	case 1:
		fill_color[0] = 0.0; fill_color[1] = 0.0; fill_color[2] = 0.0;
		break;
	case 2:
		fill_color[0] = 0.647059; fill_color[1] = 0.164706; fill_color[2] = 0.164706;
		break;
	case 3:
		fill_color[0] = 1.0; fill_color[1] = 0.0; fill_color[2] = 0.0;
		break;
	case 4:
		fill_color[0] = 1.0; fill_color[1] = 0.5; fill_color[2] = 0.0;
		break;
	case 5:
		fill_color[0] = 1.0; fill_color[1] = 1.0; fill_color[2] = 0.0;
		break;
	case 6:
		fill_color[0] = 0.62352; fill_color[1] = 0.372549; fill_color[2] = 0.623529;
		break;
	case 7:
		fill_color[0] = 0.0; fill_color[1] = 0.0; fill_color[2] = 1.0;
		break;
	case 8:
		fill_color[0] = 0.73; fill_color[1] = 0.16; fill_color[2] = 0.96;
		break;
	case 9:
		fill_color[0] = 0.752941; fill_color[1] = 0.752941; fill_color[2] = 0.752941;
		break;
	case 10:
		fill_color[0] = 1.0; fill_color[1] = 0.0; fill_color[2] = 1.0;
		break;
	case 11:
		fill_color[0] = 0.8; fill_color[1] = 0.498039; fill_color[2] = 0.196078;
		break;
	case 12:
		fill_color[0] = 0.90; fill_color[1] = 0.91; fill_color[2] = 0.98;
		break;
	case 13:
		fill_color[0] = 0.55; fill_color[1] = 0.47; fill_color[2] = 0.14;
		break;
	case 14:
		fill_color[0] = 1.0; fill_color[1] = 0.498039; fill_color[2] = 0.0;
		break;
	case 15:
		fill_color[0] = 0.55; fill_color[1] = 0.09; fill_color[2] = 0.09;
		break;
	}
}

void keyboard(unsigned char key, int x, int y) {
  switch(key) {
  case KEY_ESC:
    exit(1);
    break;
  case KEY_T_UPPER:
  case KEY_T_LOWER:
    if(polygon_flag == false)
      triangulate_ON = !triangulate_ON;
    break;
  case KEY_P_UPPER:
  case KEY_P_LOWER:
    deltaUp = 0.05;
    break;
  case KEY_L_UPPER:
  case KEY_L_LOWER:
    deltaUp = -0.05;
    break;
  }
}

void releaseKey(unsigned char key, int x, int y) {
  switch (key) {
    case KEY_P_UPPER:
    case KEY_P_LOWER:
    case KEY_L_UPPER:
    case KEY_L_LOWER:
      deltaUp = 0;
      break;
  }
}

void specialKeyboard(int key, int x, int y) {
  switch (key) {
		case GLUT_KEY_LEFT:
      deltaAngle = -0.0003f;
      break;
		case GLUT_KEY_RIGHT:
      deltaAngle = 0.0003f;
      break;
		case GLUT_KEY_UP:
      deltaMove = 0.5f;
      break;
		case GLUT_KEY_DOWN:
      deltaMove = -0.5f;
      break;
	}
}

void releaseSpecialKey(int key, int x, int y) {
  switch (key) {
	case GLUT_KEY_LEFT:
	case GLUT_KEY_RIGHT:
    deltaAngle = 0.0f;
    break;
	case GLUT_KEY_UP:
	case GLUT_KEY_DOWN:
    deltaMove = 0;
    break;
	}
}

void mouse(int button, int state, int x, int y) {
  if(polygon_flag) {
    if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN) {    //Get points of a polygon.
      point p = {x, 500-y};
      points.push_back(p);
      count_points++;

      bool flagIntersects = false;    //Checking for intersection.

      if(count_points>2) {
        point checkPointA = points[points.size()-1];
        point checkPointB = points[points.size()-2];
        for(int i=tempPolygon.first+1; i<(points.size()-1)-1; i++) {
          if(checkIntersection(checkPointA, checkPointB, points[i-1], points[i])) {
            cout << "ERROR: INTERSECTION!!" << endl;
            flagIntersects = true;
          }
        }
      }

      if(flagIntersects==true) {    //Remove points so far.
        for(int i=0; i<count_points; i++) {
          points.pop_back();
        }
        count_points = 0;
        polygon_flag = false;
        draw_point = true;
      }
    }
    else if(button==GLUT_RIGHT_BUTTON && state==GLUT_DOWN) {    //Last line of polygon.

      bool flagIntersects = false;    //Checking for intersection.

      if(count_points>2) {
        point checkPointA = points[tempPolygon.first];
        point checkPointB = points[points.size()-1];
        for(int i=tempPolygon.first+2; i<points.size()-1; i++) {
          if(checkIntersection(checkPointA, checkPointB, points[i-1], points[i])) {
            cout << "ERROR: INTERSECTION!!" << endl;
            flagIntersects = true;
          }
        }
      }

      if(flagIntersects==true) {    //Remove points so far.
        for(int i=0; i<count_points; i++)
          points.pop_back();
        count_points = 0;
        polygon_flag = false;
        draw_point = true;

      }
      else {    //Polygon is valid.
        last_point = true;
      }
    }
  }
  else if(clippingPointsCounter>0) {    //Get points for clipping-quad.
    if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN) {
      if(clippingPointsCounter==2) {
        clippingPointDownLeft.x = x;
        clippingPointDownLeft.y = 500-y;
        clippingPointsCounter-=1;
      }
      else if(clippingPointsCounter==1) {
        clippingPointUpRight.x = x;
        clippingPointUpRight.y = 500-y;
        clippingPointsCounter-=1;

        clippingAlgorithm();
      }
    }
  }
}

bool checkIntersection(point line1a, point line1b, point line2a, point line2b) {
  int x1 = line1a.x;
  int y1 = line1a.y;
  int x2 = line1b.x;
  int y2 = line1b.y;
  int x3 = line2a.x;
  int y3 = line2a.y;
  int x4 = line2b.x;
  int y4 = line2b.y;
  double s = (-(y2 - y1)*(x1 - x3) + (x2 - x1)*(y1 - y3)) / (double)(-(x4 - x3)*(y2 - y1) + (x2 - x1)*(y4 - y3));
  double t = ( (x4 - x3)*(y1 - y3) - (y4 - y3)*(x1 - x3)) / (double)(-(x4 - x3)*(y2 - y1) + (x2 - x1)*(y4 - y3));
  if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    return true;
  return false;
}

void createTriangles() {
  Vector2dVector newInput;
  Vector2dVector output;
  newInput = convertInput(polygons.size()-1);
  Triangulate::Process(newInput,output);
  convertOutput(output);
}

Vector2dVector convertInput(int polygonNumber) {
  Vector2dVector input;
  for(int i=polygons[polygonNumber].first; i<=polygons[polygonNumber].last; i++) {
    input.push_back(Vector2d(points[i].x, points[i].y));
  }
  return input;
}

void convertOutput(Vector2dVector input) {
  polygon trianglePolygon;
  trianglePolygon.first = trianglePoints.size();
  trianglePolygon.last = trianglePolygon.first+input.size();
  triangles.push_back(trianglePolygon);
  for (int i=0; i<input.size(); i++) {
    point tempPoint = {(int)input[i].GetX(), (int)input[i].GetY()};
    trianglePoints.push_back(tempPoint);
  }
}

void clippingAlgorithm() {
  clippedPolygonPointsStart = points;
  clippedPolygonsStart = polygons;
  point tempPoint;
  int next;

  clippedPolygonPointsEnd.clear();
  clippedPolygonsEnd.clear();

  for(int i=0; i<clippedPolygonsStart.size(); i++) {    //Left side of clipping-quad.
    startOfPolygon = clippedPolygonPointsStart[clippedPolygonsStart[i].first];
    for(int j=clippedPolygonsStart[i].first; j<=clippedPolygonsStart[i].last; j++){
      if(j == clippedPolygonsStart[i].last) {
        next = clippedPolygonsStart[i].first;
      } else {
        next = j+1;
      }

      if(clippedPolygonPointsStart[j].x < clippingPointDownLeft.x && clippedPolygonPointsStart[next].x > clippingPointDownLeft.x) {
        tempPoint = intersection_point('x', clippingPointDownLeft.x, clippedPolygonPointsStart[j], clippedPolygonPointsStart[next]);
        clippedPolygonPointsEnd.push_back(tempPoint);
        check_first_and_last_points(i);
        check_last_point_intersection();
        clippedPolygonPointsEnd.push_back(clippedPolygonPointsStart[next]);
        check_first_and_last_points(i);
      }
      else if(clippedPolygonPointsStart[j].x > clippingPointDownLeft.x && clippedPolygonPointsStart[next].x > clippingPointDownLeft.x) {
        clippedPolygonPointsEnd.push_back(clippedPolygonPointsStart[next]);
        check_first_and_last_points(i);
      }
      else if(clippedPolygonPointsStart[j].x > clippingPointDownLeft.x && clippedPolygonPointsStart[next].x < clippingPointDownLeft.x) {
        tempPoint = intersection_point('x', clippingPointDownLeft.x, clippedPolygonPointsStart[j], clippedPolygonPointsStart[next]);
        clippedPolygonPointsEnd.push_back(tempPoint);
        check_first_and_last_points(i);
        check_last_point_intersection();
      }
    }
  }

  clippedPolygonPointsStart = clippedPolygonPointsEnd;
  clippedPolygonPointsEnd.clear();
  clippedPolygonsStart = clippedPolygonsEnd;
  clippedPolygonsEnd.clear();

  for(int i=0; i<clippedPolygonsStart.size(); i++) {    //Down side of clipping-quad.
    startOfPolygon = clippedPolygonPointsStart[clippedPolygonsStart[i].first];
    for(int j=clippedPolygonsStart[i].first; j<=clippedPolygonsStart[i].last; j++){
      if(j == clippedPolygonsStart[i].last) {
        next = clippedPolygonsStart[i].first;
      } else {
        next = j+1;
      }

      if(clippedPolygonPointsStart[j].y < clippingPointDownLeft.y && clippedPolygonPointsStart[next].y > clippingPointDownLeft.y) {
        tempPoint = intersection_point('y', clippingPointDownLeft.y, clippedPolygonPointsStart[j], clippedPolygonPointsStart[next]);
        clippedPolygonPointsEnd.push_back(tempPoint);
        check_first_and_last_points(i);
        check_last_point_intersection();
        clippedPolygonPointsEnd.push_back(clippedPolygonPointsStart[next]);
        check_first_and_last_points(i);
      }
      else if(clippedPolygonPointsStart[j].y > clippingPointDownLeft.y && clippedPolygonPointsStart[next].y > clippingPointDownLeft.y) {
        clippedPolygonPointsEnd.push_back(clippedPolygonPointsStart[next]);
        check_first_and_last_points(i);
      }
      else if(clippedPolygonPointsStart[j].y > clippingPointDownLeft.y && clippedPolygonPointsStart[next].y < clippingPointDownLeft.y) {
        tempPoint = intersection_point('y', clippingPointDownLeft.y, clippedPolygonPointsStart[j], clippedPolygonPointsStart[next]);
        clippedPolygonPointsEnd.push_back(tempPoint);
        check_first_and_last_points(i);
        check_last_point_intersection();
      }
    }
  }

  clippedPolygonPointsStart = clippedPolygonPointsEnd;
  clippedPolygonPointsEnd.clear();
  clippedPolygonsStart = clippedPolygonsEnd;
  clippedPolygonsEnd.clear();

  for(int i=0; i<clippedPolygonsStart.size(); i++) {    //Right side of clipping-quad.
    startOfPolygon = clippedPolygonPointsStart[clippedPolygonsStart[i].first];
    for(int j=clippedPolygonsStart[i].first; j<=clippedPolygonsStart[i].last; j++){
      if(j == clippedPolygonsStart[i].last) {
        next = clippedPolygonsStart[i].first;
      } else {
        next = j+1;
      }
      if(clippedPolygonPointsStart[j].x > clippingPointUpRight.x && clippedPolygonPointsStart[next].x < clippingPointUpRight.x) {
        tempPoint = intersection_point('x', clippingPointUpRight.x, clippedPolygonPointsStart[j], clippedPolygonPointsStart[next]);
        clippedPolygonPointsEnd.push_back(tempPoint);
        check_first_and_last_points(i);
        check_last_point_intersection();
        clippedPolygonPointsEnd.push_back(clippedPolygonPointsStart[next]);
        check_first_and_last_points(i);
      }
      else if(clippedPolygonPointsStart[j].x < clippingPointUpRight.x && clippedPolygonPointsStart[next].x < clippingPointUpRight.x) {
        clippedPolygonPointsEnd.push_back(clippedPolygonPointsStart[next]);
        check_first_and_last_points(i);
      }
      else if(clippedPolygonPointsStart[j].x < clippingPointUpRight.x && clippedPolygonPointsStart[next].x > clippingPointUpRight.x) {
        tempPoint = intersection_point('x', clippingPointUpRight.x, clippedPolygonPointsStart[j], clippedPolygonPointsStart[next]);
        clippedPolygonPointsEnd.push_back(tempPoint);
        check_first_and_last_points(i);
        check_last_point_intersection();
      }
    }
  }

  clippedPolygonPointsStart = clippedPolygonPointsEnd;
  clippedPolygonPointsEnd.clear();
  clippedPolygonsStart = clippedPolygonsEnd;
  clippedPolygonsEnd.clear();

  for(int i=0; i<clippedPolygonsStart.size(); i++) {    //Up side of clipping-quad.
    startOfPolygon = clippedPolygonPointsStart[clippedPolygonsStart[i].first];
    for(int j=clippedPolygonsStart[i].first; j<=clippedPolygonsStart[i].last; j++){
      if(j == clippedPolygonsStart[i].last) {
        next = clippedPolygonsStart[i].first;
      } else {
        next = j+1;
      }

      if(clippedPolygonPointsStart[j].y > clippingPointUpRight.y && clippedPolygonPointsStart[next].y < clippingPointUpRight.y) {
        tempPoint = intersection_point('y', clippingPointUpRight.y, clippedPolygonPointsStart[j], clippedPolygonPointsStart[next]);
        clippedPolygonPointsEnd.push_back(tempPoint);
        check_first_and_last_points(i);
        check_last_point_intersection();
        clippedPolygonPointsEnd.push_back(clippedPolygonPointsStart[next]);
        check_first_and_last_points(i);
      }
      else if(clippedPolygonPointsStart[j].y < clippingPointUpRight.y && clippedPolygonPointsStart[next].y < clippingPointUpRight.y) {
        clippedPolygonPointsEnd.push_back(clippedPolygonPointsStart[next]);
        check_first_and_last_points(i);
      }
      else if(clippedPolygonPointsStart[j].y < clippingPointUpRight.y && clippedPolygonPointsStart[next].y > clippingPointUpRight.y) {
        tempPoint = intersection_point('y', clippingPointUpRight.y, clippedPolygonPointsStart[j], clippedPolygonPointsStart[next]);
        clippedPolygonPointsEnd.push_back(tempPoint);
        check_first_and_last_points(i);
        check_last_point_intersection();
      }
    }
  }

  switch_polygons_to_clippedPolygons();
}

point intersection_point(char mode, int valueClipping, point pointA, point pointB) {

  point returnPoint;

  if(mode == 'x') {
    returnPoint.x = valueClipping;
    returnPoint.y = pointA.y + ((valueClipping - pointA.x)/(double)(pointB.x - pointA.x))*(pointB.y - pointA.y);
  }
  else if(mode == 'y') {
    returnPoint.x = pointA.x + ((valueClipping - pointA.y)/(double)(pointB.y - pointA.y))*(pointB.x - pointA.x);
    returnPoint.y = valueClipping;
  }
  return returnPoint;
}

void check_first_and_last_points(int position) {
  counterFindFirst++;   //Stores points so far.

  if(counterFindFirst==1){    //Creates a polygon.
    tempPolygon.first = clippedPolygonPointsEnd.size()-1;
    for (int i=0; i<3; i++) {
      tempPolygon.cline[i] = clippedPolygonsStart[position].cline[i];
      tempPolygon.cfill[i] = clippedPolygonsStart[position].cfill[i];
    }
    clippedPolygonsEnd.push_back(tempPolygon);
    counterFindLast=0;
  }   //If the first point of this polygon = current point, current is the last point.
  if(clippedPolygonPointsEnd[clippedPolygonPointsEnd.size()-1].x == startOfPolygon.x && clippedPolygonPointsEnd[clippedPolygonPointsEnd.size()-1].y == startOfPolygon.y) {
    clippedPolygonsEnd[clippedPolygonsEnd.size()-1].last = clippedPolygonPointsEnd.size()-1;
    counterFindFirst=0;
  }
}

void check_last_point_intersection() {    //Close a polygon, when on intersection 2 times.
  counterFindLast++;
  if(counterFindLast==2 && counterFindFirst!=2) {
    clippedPolygonsEnd[clippedPolygonsEnd.size()-1].last = clippedPolygonPointsEnd.size()-1;
    counterFindFirst=0;
  }
}

void switch_polygons_to_clippedPolygons() {

  trianglePoints.clear(); //fill polygons and points vectors with the new data
  triangles.clear();      //and create triangles for them.
  polygons.clear();
  points.clear();

  points = clippedPolygonPointsEnd;

  for(int i=0; i<clippedPolygonsEnd.size(); i++) {
    polygons.push_back(clippedPolygonsEnd[i]);
    createTriangles();
  }
}

void extrude_polygon() {
  int extrude_length;
  for(int i=0; i<polygons.size(); i++) {
    cout << "Please insert extrude length for polygon " << i << ": ";
    cin >> extrude_length;
    extrude_length = -extrude_length;
    polygons[i].extrude_length = extrude_length;
  }

  glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, 600/(double)500, 0.1f, 1000.0f);

  glEnable(GL_DEPTH_TEST);
}

void computePos(float deltaMove) {
	x += deltaMove * lx * 0.1f;
	z += deltaMove * lz * 0.1f;
}

void computeDir(float deltaAngle) {
	angle += deltaAngle;
	lx = sin(angle);
	lz = -cos(angle);
}

void computeUp(float deltaUp) {
  up = up + deltaUp;
}
