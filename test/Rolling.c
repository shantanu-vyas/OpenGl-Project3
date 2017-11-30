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
#include "Rolling.h"

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))
#define FPS 60.f
#define ROLL 1.f //speed of ground and roll of ball
#define PATH_SIZE 200 //speed of ground and roll of ball

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

/**
 * AMBIENT: color of object
 * DIFFUSE: direct light color
 * SPECULAR: shine
 */

enum color {
  RED,
  GREEN,
  BLUE,
  YELLOW,
  PURPLE,
  CYAN,
  BLACK,
  WHITE,
  NUM_COLORS
};

Vec4 ambient[] = {
  {.2f, 0.f, 0.f, 1.f},
  {0.f, .2f, 0.f, 1.f},
  {0.f, 0.f, .2f, 1.f},
  {.2f, .2f, 0.f, 1.f},
  {.2f, 0.f, .2f, 1.f},
  {0.f, .2f, .2f, 1.f},
  {0.f, .0f, .0f, 1.f},
  {1.f, 1.f, 1.f, 1.f}
};
Vec4 diffuse[] = {
  {1.f, 0.f, 0.f, 1.f},
  {0.f, 1.f, 0.f, 1.f},
  {0.f, 0.f, 1.f, 1.f},
  {1.f, 1.f, 0.f, 1.f},
  {1.f, 0.f, 1.f, 1.f},
  {0.f, 1.f, 1.f, 1.f},
  {0.f, .0f, .0f, 1.f},
  {1.f, 1.f, 1.f, 1.f}
};

Vec4 specular[] = {
  {1.f, .8f, .8f, 1.f},
  {.8f, 1.f, .8f, 1.f},
  {.8f, .8f, 1.f, 1.f},
  {1.f, 1.f, .8f, 1.f},
  {1.f, .8f, 1.f, 1.f},
  {.8f, 1.f, 1.f, 1.f},
  {0.f, .0f, .0f, 1.f},
  {1.f, 1.f, 1.f, 1.f}
};


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
GLfloat ground_set = 0.f;
GLfloat ball_rot = 0.f;

GLfloat eye_radius = 5.f;

Vec4 eye = {0.f, 5.f, 0.f, 1.f};
Vec4 at =  {0.f, 0.f, 0.f, 1.f};
Vec4 up =  {0.f, -1.f, 0.f, 0.f};

GLfloat atten_const = 1.f;
GLfloat atten_linear = .01;
GLfloat atten_quad = .01;
Vec4 * lightPos;

Vec4* vertices;
int num_vertices;

ShaderModel * model_list;
int num_models;



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

  glUniform4fv(light_pos_location, 1, (GLfloat *) lightPos);

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

  glUniform4fv(light_pos_location, 1, (GLfloat *) lightPos);
  //for (int i = 1; i < num_models-1; i++)
  //  {
      glUniformMatrix4fv(tr_location, 1, GL_FALSE, (GLfloat *) &model_list[0].transform);
      glUniform1i(is_shadow,1);
      glDrawArrays(GL_TRIANGLES, vc, model_list[0].num_vertices);
      vc+=model_list[0].num_vertices;
  //  }

  eye.x = eye_radius * sinf(theta) * cosf(phi);
  eye.z = eye_radius * sinf(theta) * sinf(phi);
  eye.y = eye_radius * cosf(theta);
  genLookAt(&mv_matrix,&eye,&at,&up);

  glutSwapBuffers();
}

void modelPhysics(GLfloat delta_sec)
{
  GLfloat dist = delta_sec * (GLfloat) ROLL;
  ground_set = fmod(ground_set + dist, (GLfloat) PATH_SIZE);
  GLfloat offset;
  ball_rot = 2.f * asinf((dist / 2.f)/1.f);
  rotateZ(&model_list[0].transform, ball_rot);
  for (int i = 2; i < num_models; i++){
    offset = fmod(ground_set + (GLfloat) i, (GLfloat) PATH_SIZE) - ((GLfloat)PATH_SIZE) / 2.f;
    model_list[i].transform.w.z = offset;
  }
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
  if (key == 'a') lightPos->x++;
  if (key == 'A') lightPos->x--;
  if (key == 's') lightPos->y++;
  if (key == 'S') lightPos->y--;
  if (key == 'd') lightPos->z++;
  if (key == 'D') lightPos->z--;

  /* printf("sin theta %f\n",sin(theta)); */
  /* printf("cos phi %f\n",cos(phi)); */
  /* printf("cos theta %f\n",cos(theta)); */
  /* printf("theta %f\n",theta); */
  /* printf("phi %f\n",phi); */

  /* printVector(&eye); */
}

void genModels()
{
  ShaderModel ball;
  ShaderModel light;
  ShaderModel ground_cubes[PATH_SIZE];

  GLfloat shine = 15.f;
  
  int color;
  for (int i = 0; i < PATH_SIZE; i++){
    color = rand() % (NUM_COLORS - 2);//skip black and white
    makeCubeSM(&ground_cubes[i],&ambient[color], &specular[color], &diffuse[color], &shine);
    scaleYModelSM(&ground_cubes[i],&ground_cubes[i].num_vertices,.005f);
    scaleZModelSM(&ground_cubes[i],&ground_cubes[i].num_vertices,.5f);
  }
  color = rand() % NUM_COLORS;
  makeSphereSM(&ball,&ambient[color], &specular[color], &diffuse[color], &shine);

  makeSphereSM(&light,&ambient[WHITE], &specular[WHITE], &diffuse[WHITE], &shine);
  scaleXModelSM(&light,&light.num_vertices,.5f);
  scaleYModelSM(&light,&light.num_vertices,.5f);
  scaleZModelSM(&light,&light.num_vertices,.5f);

  num_models = 2 + PATH_SIZE;

  model_list = malloc(sizeof(ShaderModel)*num_models);

  model_list[0] = ball;
  model_list[1] = light;
  for (int i = 0; i < PATH_SIZE; i++) model_list[i+2] = ground_cubes[i];
  
  // init tranformations
  for (int i = 0; i < num_models; i++)
    {
      identity(&model_list[i].transform);
    }
  lightPos = &model_list[1].transform.w;
  *lightPos = (Vec4) {0.f, 3.f, 0.f, 1.f};
  model_list[0].transform.w.y = 0.5f;
}

int main(int argc, char **argv)
{
  genModels();
  flattenModelListSM(&model_list,&vertices,&num_vertices,&num_models);

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
