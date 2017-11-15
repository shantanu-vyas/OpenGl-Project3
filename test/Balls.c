#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>


#include "initShader.h"
#include "VecLib.h"
#include "Model.h"
#include "GLToolkit.h"
#include "Balls.h"

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

GLuint pj_location;
GLuint mv_location;
GLuint tr_location;

Mat4 pj_matrix =
  {{1.0, 0.0, 0.0, 0.0},
   {0.0, 1.0, 0.0, 0.0},
   {0.0, 0.0, 1.0, 0.0},
   {0.0, 0.0, 0.0, 1.0}};
Mat4 mv_matrix =
  {{1.0, 0.0, 0.0, 0.0},
   {0.0, 1.0, 0.0, 0.0},
   {0.0, 0.0, 1.0, 0.0},
   {0.0, 0.0, 0.0, 1.0}};
Mat4 tr_matrix =
  {{1.0, 0.0, 0.0, 0.0},
   {0.0, 1.0, 0.0, 0.0},
   {0.0, 0.0, 1.0, 0.0},
   {0.0, 0.0, 0.0, 1.0}};

Vec4 eye = {0,10,0,1};
Vec4 at = {0,0,0,1};
Vec4 up = {0,1,0,0};
Vec4 lightPos = {0,2,0,1};

Mat4 sphere1_tr;
Mat4 sphere2_tr;
Mat4 sphere3_tr;
Mat4 sphere4_tr;
Mat4 sphere5_tr;
Mat4 light_sphere_tr;

Vec4* vertices;
Vec4* colors;
int num_vertices;

Model* model_list;
int num_models;


void init(void)
{
  int size = sizeof(Vec4)*num_vertices;

  GLuint program = initShader("shaders/vshader.glsl", "shaders/fshader.glsl");
  glUseProgram(program);

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint buffer;
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, size + size, NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, vertices);
  glBufferSubData(GL_ARRAY_BUFFER, size, size, colors);

  GLuint vPosition = glGetAttribLocation(program, "vPosition");
  glEnableVertexAttribArray(vPosition);
  glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vec4), BUFFER_OFFSET(0));

  GLuint vColor = glGetAttribLocation(program, "vColor");
  glEnableVertexAttribArray(vColor);
  glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) size);

  pj_location = glGetUniformLocation(program, "projection_matrix");
  mv_location = glGetUniformLocation(program, "modelview_matrix");
  tr_location = glGetUniformLocation(program, "transformation_matrix");

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glDepthRange(1,0);
}

void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUniformMatrix4fv(pj_location, 1, GL_FALSE, (GLfloat *) &pj_matrix);
  glUniformMatrix4fv(mv_location, 1, GL_FALSE, (GLfloat *) &mv_matrix);
  glUniformMatrix4fv(tr_location, 1, GL_FALSE, (GLfloat *) &tr_matrix);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glBindVertexArray(vertices);
  glDrawArrays(GL_TRIANGLES, 0, num_vertices);

  glutSwapBuffers();

}

void idle_func()
{
}
void keyboard(unsigned char key, int mousex, int mousey)
{
  if(key == 'q')
    exit(0);

  glutPostRedisplay();
}

void genModels()
{
  identity(&sphere1_tr);
  identity(&sphere2_tr);
  identity(&sphere3_tr);
  identity(&sphere4_tr);
  identity(&sphere5_tr);
  identity(&light_sphere_tr);

  Model sphere1;
  Model sphere2;
  Model sphere3;
  Model sphere4;
  Model sphere5;
  Model light_sphere;
  Model ground_cube;
 
  model_list = malloc(sizeof(Model)*7);
  num_models = 7;
  
  

}

int main(int argc, char **argv)
{
  genModels();
  
  genPerspective(&pj_matrix,30,1,.01,10);
  genLookAt(&mv_matrix,&eye,&at,&up);

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(1000, 1000);
  glutCreateWindow("Balls");
  glewInit();
  init();
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutIdleFunc(idle_func);
  glutMainLoop();

  return 0;
}
/*
  TODO
  Application:
  1) Add 5 balls radius .5 all touching each other
  2) Animate balls to all move at different speeds around origin
  3) Add small ball along +y for the light source
  4) Add plane for all balls to sit along
  5) Set specular ambient and diffuse for each ball
  6) Set keyboard bindings for moving eye about radius around the origin
  7) Set keybaord bindings for changing radius
  8) Set keyboard bindings for making light source move

  Shader:
  1) Create fake shadows by projecting from light source down to the plane y = 0;
  2) Handle lighting ...
  3) switch colors to shader from application
*/
