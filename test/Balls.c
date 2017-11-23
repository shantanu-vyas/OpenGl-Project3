/*
  TODO
  Application:
  1) Add 5 balls radius .5 all touching each other
  2) Animate balls to all move at different speeds around origin
  3) Add small all along +y for the light source
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
#include "ShaderModel.h"
#include "GLToolkit.h"
#include "Balls.h"

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))
#define FPS 60.f

clock_t global_clock_prev; // last time rendered/physics ticked


/* VSHADER INPUTS */
GLuint pj_location;
GLuint mv_location;
GLuint tr_location;
GLuint is_shadow;


GLuint ambient_location;
GLuint diffuse_location;
GLuint specular_location;
GLuint shininess_location;

GLuint light_pos_location;
GLuint atten_const_location;
GLuint atten_linear_location;
GLuint atten_quad_location;
/* VSHADER INPUTS END */

const Vec4 red =    {1.f, 0.f, 0.f, 1.f};
const Vec4 darkred = {.5f, 0.f, 0.f, 1.f};
const Vec4 green =  {0.f, 1.f, 0.f, 1.f};
const Vec4 darkgreen =  {0.f, 0.5f, 0.f, 1.f};
const Vec4 blue =   {0.f, 0.f, 1.f, 1.f};
const Vec4 darkblue =   {0.f, 0.f, .5f, 1.f};
const Vec4 cgray =  {.3f, .3f, .3f, 1.f};
const Vec4 yellow = {1.f, 1.f, 0.f, 1.f};
const Vec4 darkyellow = {.5f, .5f, 0.f, 1.f};
const Vec4 purple = {1.f, 0.f, 1.f, 1.f};
const Vec4 darkpurple = {.5f, 0.f, .5f, 1.f};
const Vec4 cyan =   {0.f, 1.f, 1.f, 1.f};
const Vec4 darkcyan =   {0.f, .5f, .5f, 1.f};
const Vec4 black =   {0.f, .0f, .0f, 1.f};
const Vec4 white =   {1.f, 1.f, 1.f, 1.f};


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

GLfloat theta = M_PI/32.f;
GLfloat phi = 0.f;

GLfloat eye_radius = 20.f;

Vec4 eye = {0.f, 30.f, 0.f, 1.f};
Vec4 at =  {0.f, 0.f, 0.f, 1.f};
Vec4 up =  {0.f, -1.f, 0.f, 0.f};

GLfloat atten_const = -5.f;
GLfloat atten_linear = 1.f;
GLfloat atten_quad = 1.f;
Vec4 lightPos = {0.f, 3.f, 0.f, 1.f};

Vec4* vertices;
//Vec4* colors;
int num_vertices;

ShaderModel * model_list;
int num_models;

float ball_rot;


void init(void)
{
  int size = 2*sizeof(Vec4)*num_vertices; //need at least double because shadows

  GLuint program = initShader("shaders/vshader.glsl", "shaders/fshader.glsl");
  glUseProgram(program);

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint buffer;
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, 2 * size, NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, vertices);
  glBufferSubData(GL_ARRAY_BUFFER, size, size, vertices);
  //  glBufferSubData(GL_ARRAY_BUFFER, size, size, colors);

  /* VSHADER VARIABLES

     in vec4 vPosition;
     in vec4 vNormal;
     in int isShadow;
     out vec4 color;

     uniform mat4 model_view, projection, transformation;
     uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct, LightPosition;
     uniform float shininess, attenuation_constant, attenuation_linear, attenuation_quadratic;
  */
  GLuint vPosition = glGetAttribLocation(program, "vPosition");
  glEnableVertexAttribArray(vPosition);
  glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vec4), BUFFER_OFFSET(0));

  GLuint vNormal = glGetAttribLocation(program, "vNormal");
  glEnableVertexAttribArray(vNormal);
  glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_TRUE, sizeof(Vec4), (GLvoid *) size);

  GLuint isShadow = glGetAttribLocation(program, "isShadow");

  pj_location = glGetUniformLocation(program, "projection");
  mv_location = glGetUniformLocation(program, "model_view");
  tr_location = glGetUniformLocation(program, "transformation");

  ambient_location = glGetUniformLocation(program, "AmbientProduct");
  diffuse_location = glGetUniformLocation(program, "DiffuseProduct");
  specular_location = glGetUniformLocation(program, "SpecularProduct");
  shininess_location = glGetUniformLocation(program, "shininess");

  light_pos_location = glGetUniformLocation(program, "LightPosition");

  atten_const_location = glGetUniformLocation(program, "attenuation_constant");
  atten_linear_location = glGetUniformLocation(program, "attenuation_linear");
  atten_quad_location = glGetUniformLocation(program, "attenuation_quadratic");

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.f, 0.f, 0.f, 1.f);
  glDepthRange(1,0);
}

void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glRasterPos3f(.6,-.9,0);
  char* s = "FPS is XX.XX";
  void * font = GLUT_BITMAP_9_BY_15;
  for (int i = 0; i < 11; i++)
    {
      char c = s[i];
      glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
    } 


  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glUniformMatrix4fv(pj_location, 1, GL_FALSE, (GLfloat *) &pj_matrix);
  glUniformMatrix4fv(mv_location, 1, GL_FALSE, (GLfloat *) &mv_matrix);

  glUniform1fv(atten_const_location, 1, (GLfloat *) &atten_const);
  glUniform1fv(atten_linear_location, 1, (GLfloat *) &atten_linear);
  glUniform1fv(atten_quad_location, 1, (GLfloat *) &atten_quad);

  //julian look at this
  glUniform4fv(light_pos_location, 1, (GLfloat *) &lightPos);

  int vc = 0;
  for (int i = 0; i < num_models; i++)
    {
      glUniformMatrix4fv(tr_location, 1, GL_FALSE, (GLfloat *) &model_list[i].transform);
      glUniform4fv(ambient_location, 1, (GLfloat *) &model_list[i].ambient);
      glUniform4fv(diffuse_location, 1, (GLfloat *) &model_list[i].diffuse);
      glUniform4fv(specular_location, 1, (GLfloat *) &model_list[i].specular);
      glUniform1fv(shininess_location, 1, (GLfloat *) &model_list[i].shine);
      glUniform1i(is_shadow,0);
      glDrawArrays(GL_TRIANGLES, vc, model_list[i].num_vertices);
      vc+=model_list[i].num_vertices;
    }

  glUniform4fv(light_pos_location, 1, (GLfloat *) &lightPos);
  vc = 36; //dont draw shadow for the ground cube
  for (int i = 1; i < num_models-1; i++)
    {
      glUniformMatrix4fv(tr_location, 1, GL_FALSE, (GLfloat *) &model_list[i].transform);
      glUniform1i(is_shadow,1);
      glDrawArrays(GL_TRIANGLES, vc, model_list[i].num_vertices);
      vc+=model_list[i].num_vertices;
    }

  eye.x = eye_radius * sinf(theta) * cosf(phi);
  eye.z = eye_radius * sinf(theta) * sinf(phi);
  eye.y = eye_radius * cosf(theta);
  genLookAt(&mv_matrix,&eye,&at,&up);

  glutSwapBuffers();
}

void modelPhysics(GLfloat delta_sec)
{
  // hackish: index of model is speed. Note floor is index 0 therefore rotation = 0. Light sphere excluded
  for (int i = 1; i < num_models-1; i++){
    /*calculate translation using polar coords */
    model_list[i].transform.w.x = 2*(i-1)*cosf(ball_rot*i);
    model_list[i].transform.w.z = 2*(i-1)*sinf(ball_rot*i);
    model_list[i].transform.w.y = 1;
    ball_rot+=.005;
  }
  model_list[num_models-1].transform.w.x = lightPos.x;
  model_list[num_models-1].transform.w.y = lightPos.y;
  model_list[num_models-1].transform.w.z = lightPos.z;
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

  if (delta_sec >= 1.f / FPS) {
    render = 1;
    modelPhysics(delta_sec);
    genModelShadows();
  }
  if (render){
    glutPostRedisplay();
    global_clock_prev = now; // only changes if physics/camera ticked
  }


}


void keyboard(unsigned char key, int mousex, int mousey)
{
  if (key == 'q')  exit(0);
  if (key == '1') theta+=M_PI/32.f;
  if (key == '2') theta-=M_PI/32.f;
  if (key == '3') phi+=M_PI/32.f;
  if (key == '4') phi-=M_PI/32.f;
  if (key == '-') eye_radius++;
  if (key == '=') eye_radius--;
  if (key == 'z') atten_const += .1f;
  if (key == 'Z') atten_const -= .1f;
  if (key == 'x') atten_linear += .1f;
  if (key == 'X') atten_linear -= .1f;
  if (key == 'c') atten_quad += .1f;
  if (key == 'C') atten_quad -= .1f;
  if (key == 'a') lightPos.x++;
  if (key == 's') lightPos.x--;
  if (key == 'd') lightPos.y++;
  if (key == 'f') lightPos.y--;
  if (key == 'g') lightPos.z++;
  if (key == 'h') lightPos.z--;

  /* printf("sin theta %f\n",sin(theta)); */
  /* printf("cos phi %f\n",cos(phi)); */
  /* printf("cos theta %f\n",cos(theta)); */
  /* printf("theta %f\n",theta); */
  /* printf("phi %f\n",phi); */

  /* printVector(&eye); */
}

void genModels()
{
  ShaderModel sphere1;
  ShaderModel sphere2;
  ShaderModel sphere3;
  ShaderModel sphere4;
  ShaderModel sphere5;
  ShaderModel light_sphere;
  ShaderModel ground_cube;

  GLfloat shine = .1f;

  makeCube(&ground_cube,&darkgreen, &green, &green, &shine);
  scaleYModel(&ground_cube,&ground_cube.num_vertices,.001f);
  scaleXModel(&ground_cube,&ground_cube.num_vertices,15.f);
  scaleZModel(&ground_cube,&ground_cube.num_vertices,15.f);

  /*change these to spheres after getting julians sphere code */
  makeSphere(&sphere1, &black, &red, &darkred, &shine);
  Vec4 trans1 = {0.f , 1.f, 0.f, 0.f};

//  shine = .2f;

  makeSphere(&sphere2, &black, &blue, &darkblue, &shine);
  Vec4 trans2 = {2.f, 1.f, 0.f, 0.f};

//  shine = .4f;
  
  makeSphere(&sphere3, &black, &yellow, &darkyellow, &shine);
  Vec4 trans3 = {4.f, 1.f, 0.f, 0.f};

//  shine = .8f;
  
  makeSphere(&sphere4, &black, &purple, &darkpurple, &shine);
  Vec4 trans4 = {6.f, 1.f, 0.f, 0.f};

//  shine = 1.6f;
  
  makeSphere(&sphere5, &black, &cyan, &darkcyan, &shine);
  Vec4 trans5 = {8.f, 1.f, 0.f, 0.f};

  GLfloat light_shine = 1000.f;
  makeSphere(&light_sphere, &white, &white, &white, &light_shine);
  scaleXModel(&light_sphere,&light_sphere.num_vertices,.5f);
  scaleYModel(&light_sphere,&light_sphere.num_vertices,.5f);
  scaleZModel(&light_sphere,&light_sphere.num_vertices,.5f);
  Vec4 trans6 = {1.f, 5.f, 1.f, 0.f};

  /* for (int i = 0; i < 36; i++) */
  /*   { */
  /*     GLfloat x = 0 + rand() % (1+1); */
  /*     GLfloat y = 0 + rand() % (1+1); */
  /*     GLfloat z = 0 + rand() % (1+1); */
  /*     Vec4 temp = {x,y,z,1}; */
  /*     ground_cube.colors[i] = temp; */
  /*   } */
  /*
    setColor(&sphere1,&red);
    setColor(&sphere2,&green);
    setColor(&sphere3,&blue);
    setColor(&sphere4,&yellow);
    setColor(&sphere5,&purple);
  */
  num_models = 7;

  model_list = malloc(sizeof(ShaderModel)*num_models);

  // define offsets
  //  model_offset_list[0] =

  model_list[0] = ground_cube;
  model_list[1] = sphere1;
  model_list[2] = sphere2;
  model_list[3] = sphere3;
  model_list[4] = sphere4;
  model_list[5] = sphere5;
  model_list[6] = light_sphere;
  // init tranformations
  for (int i = 0; i < num_models; i++)
    {
      identity(&model_list[i].transform);
    }
}

int main(int argc, char **argv)
{
  genModels();
  flattenModelList(&model_list,&vertices,&num_vertices,&num_models);

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
