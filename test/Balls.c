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
#include <time.h>
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
#define FPS 60.f

clock_t global_clock_prev; // last time rendered/physics tick

GLuint pj_location;
GLuint mv_location;
GLuint tr_location;

const Vec4 red =    {1.f, 0.f, 0.f, 1.f};
const Vec4 green =  {0.f, 1.f, 0.f, 1.f};
const Vec4 blue =   {0.f, 0.f, 1.f, 1.f};
const Vec4 cgray =  {.3f, .3f, .3f, 1.f};
const Vec4 yellow = {1.f, 1.f, 0.f, 1.f};
const Vec4 purple = {1.f, 0.f, 1.f, 1.f};
const Vec4 cyan =   {0.f, 1.f, 1.f, 1.f};


Mat4 pj_matrix =
  {{1.f, 0.f, 0.f, 0.f},
   {0.f, 1.f, 0.f, 0.f},
   {0.f, 0.f, 1.f, 0.f},
   {0.f, 0.f, 0.f, 1.f}};
Mat4 mv_matrix =
  {{1.f, 0.f, 0.f, 0.f},
   {0.f, 1.f, 0.f, 0.f},
   {0.f, 0.f, 1.f, 0.f},
   {0.f, 0.f, 0.f, 1.f}};

GLfloat theta = 0.f;
GLfloat phi = 0.f;

GLfloat eye_radius = 20.f;

Vec4 eye = {0.f, 30.f, 0.f, 1.f};
Vec4 at =  {0.f, 0.f, 0.f, 1.f};
Vec4 up =  {0.f, -1.f, 0.f, 0.f};

Vec4 lightPos = {0.f, 2.f, 0.f, 1.f};

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
  glClearColor(0.f, 0.f, 0.f, 1.f);
  glDepthRange(1,0);
}

void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUniformMatrix4fv(pj_location, 1, GL_FALSE, (GLfloat *) &pj_matrix);
  glUniformMatrix4fv(mv_location, 1, GL_FALSE, (GLfloat *) &mv_matrix);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


  int vc = 0;
  for (int i = 0; i < num_models; i++)
    {
      glUniformMatrix4fv(tr_location, 1, GL_FALSE, (GLfloat *) &model_list[i].transform);
      glDrawArrays(GL_TRIANGLES, vc, model_list[i].num_vertices);
      vc+=model_list[i].num_vertices;
    }

  glutSwapBuffers();
}

void modelPhysics(GLfloat delta_sec)
{ 
  // hackish: index of model is speed. Note floor is index 0 therefore rotation = 0
  for (int i = 0; i < num_models; i++){
    rotateY(&model_list[i].transform, ((GLfloat) i) * delta_sec);
  }

}

void genModelShadows()
{
}

void idle_func()
{
  // delta time from last rendering
  clock_t now = clock();
  GLfloat delta_sec = (((GLfloat)now - (GLfloat)global_clock_prev) / (GLfloat)CLOCKS_PER_SEC);
  int render = 0;

  while (delta_sec >= 1.f / FPS) {
    render = 1; 
    modelPhysics(delta_sec);
    genModelShadows();

    delta_sec--;
  }
  if (render){
    glutPostRedisplay();
    global_clock_prev = now; // only changes if physics/camera ticked
  } 


}


void keyboard(unsigned char key, int mousex, int mousey)
{
  if(key == 'q')
    exit(0);
  if (key == '1')
    {
      theta+=M_PI/32.f;
    }
  if (key == '2')
    {
      theta-=M_PI/32.f;
    }
  if (key == '3')
    {
      phi+=M_PI/32.f;
      //printf("%f\n",phi);
    }
  if(key == '4')
    {
      phi-=M_PI/32.f;
    }

  /* printf("sin theta %f\n",sin(theta)); */
  /* printf("cos phi %f\n",cos(phi)); */
  /* printf("cos theta %f\n",cos(theta)); */
  /* printf("theta %f\n",theta); */
  /* printf("phi %f\n",phi); */

  eye.x = eye_radius * sinf(theta) * cosf(phi);
  eye.z = eye_radius * sinf(theta) * sinf(phi);
  eye.y = eye_radius * cosf(theta);


  printVector(&eye);
  genLookAt(&mv_matrix,&eye,&at,&up);

  glutPostRedisplay();
}

void genModels()
{
  /* identity(&sphere1_tr); */
  /* identity(&sphere2_tr); */
  /* identity(&sphere3_tr); */
  /* identity(&sphere4_tr); */
  /* identity(&sphere5_tr); */
  /* identity(&light_sphere_tr); */

  Model sphere1;
  Model sphere2;
  Model sphere3;
  Model sphere4;
  Model sphere5;
  Model light_sphere;
  Model ground_cube;

  makeCube(&ground_cube);
  scaleYModel(&ground_cube,&ground_cube.num_vertices,.001f);
  scaleXModel(&ground_cube,&ground_cube.num_vertices,10.f);
  scaleZModel(&ground_cube,&ground_cube.num_vertices,10.f);
  /* scaleYModel(&ground_cube,&ground_cube.num_vertices,.4); */
  /* scaleXModel(&ground_cube,&ground_cube.num_vertices,.4); */
  /* scaleZModel(&ground_cube,&ground_cube.num_vertices,.4); */


  /*change these to spheres after getting julians sphere code */
  makeSphere(&sphere1);
  Vec4 trans1 = {0.f , 1.f, 0.f, 0.f};
  translateModelVec4(&sphere1, &sphere1.num_vertices, &trans1);

  makeSphere(&sphere2);
  Vec4 trans2 = {2.f, 1.f, 0.f, 0.f};
  translateModelVec4(&sphere2, &sphere2.num_vertices, &trans2);

  makeSphere(&sphere3);
  Vec4 trans3 = {4.f, 1.f, 0.f, 0.f};
  translateModelVec4(&sphere3, &sphere3.num_vertices, &trans3);

  makeSphere(&sphere4);
  Vec4 trans4 = {6.f, 1.f, 0.f, 0.f};
  translateModelVec4(&sphere4, &sphere4.num_vertices, &trans4);

  makeSphere(&sphere5);
  Vec4 trans5 = {8.f, 1.f, 0.f, 0.f};
  translateModelVec4(&sphere5, &sphere5.num_vertices, &trans5);

  setColor(&ground_cube,&cgray);
  /* for (int i = 0; i < 36; i++) */
  /*   { */
  /*     GLfloat x = 0 + rand() % (1+1); */
  /*     GLfloat y = 0 + rand() % (1+1); */
  /*     GLfloat z = 0 + rand() % (1+1); */
  /*     Vec4 temp = {x,y,z,1}; */
  /*     ground_cube.colors[i] = temp; */
  /*   } */

  setColor(&sphere1,&red);
  setColor(&sphere2,&green);
  setColor(&sphere3,&blue);
  setColor(&sphere4,&yellow);
  setColor(&sphere5,&purple);

  model_list = malloc(sizeof(Model)*6);
  model_list[0] = ground_cube;
  model_list[1] = sphere1;
  model_list[2] = sphere2;
  model_list[3] = sphere3;
  model_list[4] = sphere4;
  model_list[5] = sphere5;
  num_models = 6;
  // init tranformations
  for (int i = 0; i < num_models; i++)
    {
      identity(&model_list[i].transform);
    }
}

int main(int argc, char **argv)
{
  genModels();
  flattenModelList(&model_list,&vertices,&colors,&num_vertices,&num_models);

  eye.x = eye_radius * sin(theta) * cos(phi);
  eye.y = eye_radius * sin(theta) * sin(phi);
  eye.z = eye_radius * cos(theta);

  genPerspective(&pj_matrix, 30.f , 1.f, .01f, 10.f);
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
