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
#include "SpherePhysics.h"
#include "GLToolkit.h"
#include "Bounce.h"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))
#define WINDOW 600
#define FPS 60.f
#define TICK 1.f/FPS

#define BALL_RADIUS 0.5f
#define NUM_BALLS 10
#define NUM_STATIC 2

#define ROLL .999f // roll slowdown percentage

clock_t global_clock_prev; // last time rendered/physics ticked
GLfloat curr_fps;

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
  {.1f, 0.f, 0.f, 1.f},
  {0.f, .1f, 0.f, 1.f},
  {0.f, 0.f, .1f, 1.f},
  {.1f, .1f, 0.f, 1.f},
  {.1f, 0.f, .1f, 1.f},
  {0.f, .1f, .1f, 1.f},
  {.01f, .01f, .01f, 1.f},
  {.2f, .2f, .2f, .8f}
};
Vec4 diffuse[] = {
  {1.f, 0.f, 0.f, 1.f},
  {0.f, 1.f, 0.f, 1.f},
  {0.f, 0.f, 1.f, 1.f},
  {1.f, 1.f, 0.f, 1.f},
  {1.f, 0.f, 1.f, 1.f},
  {0.f, 1.f, 1.f, 1.f},
  {.05f, .05f, .05f, 1.f},
  {.6f, .6f, .6f, 1.f}
};

Vec4 specular[] = {
  {1.f, .8f, .8f, 1.f},
  {.8f, 1.f, .8f, 1.f},
  {.8f, .8f, 1.f, 1.f},
  {1.f, 1.f, .8f, 1.f},
  {1.f, .8f, 1.f, 1.f},
  {.8f, 1.f, 1.f, 1.f},
  {.1f, .1f, .1f, 1.f},
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

GLfloat theta = 2.f * M_PI/5.f;
GLfloat phi = M_PI/2.f;

GLfloat eye_radius = (GLfloat) NUM_BALLS;

Vec4 eye = {0.f, 3.f, 3.f, 1.f};
Vec4 at =  {0.f, 0.f, 0.f, 1.f};
Vec4 up =  {0.f, 1.f, 0.f, 0.f};

GLfloat atten_const = 1.f;
GLfloat atten_linear = 0.02f;
GLfloat atten_quad = 0.02f;

Vec4 * lightPos;

Vec4* vertices;
int num_vertices;

ShaderModel * model_list;
SphereEntity * physics_list;
GLfloat boundaries;
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
  glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, sizeof(Vec4), (GLvoid *) size);

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
  // FPS text
  glColor3f(1.f, 1.f, 1.f);
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D( 0, WINDOW, 0, WINDOW);
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();
  glRasterPos2i(25, WINDOW - 25);
  char s[12];
  sprintf(s, "FPS: %5.2f", curr_fps);
  for (int i = 0; i < 12; i++){
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

  int vc;
  // shapes
  vc = 0;
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
  // shadows
  vc = 0;
  vc += model_list[0].num_vertices;
  vc += model_list[1].num_vertices;
  for (int i = NUM_STATIC; i < num_models; i++){
    glUniformMatrix4fv(tr_location, 1, GL_FALSE, (GLfloat *) &model_list[i].transform);
    glUniform1i(is_shadow,1);
    glDrawArrays(GL_TRIANGLES, vc, model_list[i].num_vertices);
    vc += model_list[i].num_vertices;
  }
  // eyes/perspective
  eye.x = eye_radius * sinf(theta) * cosf(phi);
  eye.z = eye_radius * sinf(theta) * sinf(phi);
  eye.y = eye_radius * cosf(theta);
  genLookAt(&mv_matrix,&eye,&at,&up);

  glutSwapBuffers();
}

void modelPhysics(GLfloat delta_sec)
{
  Vec4 tick_speed;
  for (int i = 0; i < NUM_BALLS; i++){
    physics_list[i].velocity.x *= ROLL;
    physics_list[i].velocity.z *= ROLL;
    scalarMultVec4(&tick_speed, &physics_list[i].velocity, delta_sec);
    addVec4(&model_list[i+NUM_STATIC].transform.w, &model_list[i+NUM_STATIC].transform.w, &tick_speed);
  }
  sphereCollisionTick(physics_list, NUM_BALLS, boundaries, delta_sec);
}

void idle_func()
{
  // delta time from last rendering
  clock_t now = clock();
  GLfloat delta_sec = (((GLfloat)now - (GLfloat)global_clock_prev) / (GLfloat)CLOCKS_PER_SEC);
  int render = 0;

  while (delta_sec >= TICK){
    if (render == 0) {
      render = 1;
      curr_fps = (curr_fps + 1.f/delta_sec) / 2.f;
    }
    delta_sec = delta_sec - TICK;
    modelPhysics(TICK);
  }
  if (render){
    glutPostRedisplay();
    global_clock_prev = now; // only changes if physics/camera ticked
  }
}

void keyboard(unsigned char key, int mousex, int mousey)
{
  if (key == 'q')  exit(0);
  if (key == '1' && theta + M_PI/32.f < M_PI / 2.f) theta+=M_PI/32.f;
  if (key == '2' && theta - M_PI/32.f > 0.f)         theta-=M_PI/32.f;
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
  if (key == ' ') {
    for (int i = 0; i < NUM_BALLS; i++){
      if (physics_list[i].velocity.x == 0.f){
        GLfloat init = (GLfloat)( 1 - (rand() % 3)) * (1.f - ROLL); 
        physics_list[i].velocity.x = init;
      } else{
        physics_list[i].velocity.x *= 2.f*(2.f - ROLL);
      }
      if (physics_list[i].velocity.z == 0.f){
        GLfloat init = (GLfloat)( 1 - (rand() % 3)) * (1.f - ROLL); 
        physics_list[i].velocity.z = init;
      } else{
        physics_list[i].velocity.z *= 2.f*(2.f - ROLL);
      }
    }
  }
  /* printf("sin theta %f\n",sin(theta)); */
  /* printf("cos phi %f\n",cos(phi)); */
  /* printf("cos theta %f\n",cos(theta)); */
  /* printf("theta %f\n",theta); */
  /* printf("phi %f\n",phi); */

  /* printVector(&eye); */
}

void genModels()
{
  ShaderModel ground;
  ShaderModel light;
  ShaderModel balls[NUM_BALLS];

  GLfloat shine;
  boundaries = (GLfloat)(NUM_BALLS + 1) / 2.f;
 
  shine = 1.f;
  makeCubeSM(&ground, &ambient[GREEN], &specular[GREEN], &diffuse[GREEN], &shine);
  scaleXModelSM(&ground, &ground.num_vertices, boundaries);
  scaleYModelSM(&ground, &ground.num_vertices, .005f);
  scaleZModelSM(&ground, &ground.num_vertices, boundaries);
  
  shine = 100.f;
  makeSphereSM(&light,&specular[WHITE], &specular[WHITE], &specular[WHITE], &shine);
  scaleXModelSM(&light, &light.num_vertices, .2f);
  scaleYModelSM(&light, &light.num_vertices, .2f);
  scaleZModelSM(&light, &light.num_vertices, .2f);
  
  int color1;
  int color2;
  int color3;
  Vec4 amb, spec, diff;
  for (int i = 0; i < NUM_BALLS; i++){
    // init model
    shine = (GLfloat) (rand() % 495 + 5);
    color1 = rand() % (NUM_COLORS);
    color2 = rand() % (NUM_COLORS);
    color3 = rand() % (NUM_COLORS);
    addVec4(&amb, &ambient[color1], &ambient[color2]);
    addVec4(&amb, &amb, &ambient[color3]);
    scalarMultVec4(&amb, &amb, 1.f/3.f);
    addVec4(&spec, &specular[color1], &specular[color2]);
    addVec4(&spec, &spec, &specular[color3]);
    scalarMultVec4(&spec, &spec, 1.f/3.f);
    addVec4(&diff, &diffuse[color1], &diffuse[color2]);
    addVec4(&diff, &diff, &diffuse[color3]);
    scalarMultVec4(&diff, &diff, 1.f/3.f);
    makeSphereSM(&balls[i], &amb, &spec, &diff, &shine);
    scaleXModelSM(&balls[i], &balls[i].num_vertices, BALL_RADIUS);
    scaleYModelSM(&balls[i], &balls[i].num_vertices, BALL_RADIUS);
    scaleZModelSM(&balls[i], &balls[i].num_vertices, BALL_RADIUS);
  }

  num_models = NUM_STATIC + NUM_BALLS;

  model_list = malloc(sizeof(ShaderModel) * num_models);
  physics_list = malloc(sizeof(SphereEntity) * NUM_BALLS); 

  model_list[0] = ground;
  model_list[1] = light;
  for (int i = 0; i < NUM_BALLS; i++) model_list[i+NUM_STATIC] = balls[i];
  // init tranformations
  for (int i = 0; i < num_models; i++) identity(&model_list[i].transform);
  // init ball positions
  for (int i = 0; i < NUM_BALLS; i++){
    GLfloat len = (GLfloat) i - ((GLfloat)NUM_BALLS / 2.f) + 0.5f;
    Vec4 offset = {(GLfloat) len, 0.5f, (GLfloat) len, 1.f};
    model_list[i+NUM_STATIC].transform.w = offset;
    
  }
  // init physics
  for (int i = 0; i < NUM_BALLS; i++){
    GLfloat speed = (GLfloat)(rand() % 1000) / 10000.f;
    GLfloat angle = fmodf((GLfloat) rand(), 2.f * M_PI);
    Vec4 vel = {speed * cosf(angle), 0.f, speed * sinf(angle), 0.f};
    
    physics_list[i].velocity = vel;
    physics_list[i].position = &model_list[i+NUM_STATIC].transform.w;
    physics_list[i].mass = 750.f - model_list[i+NUM_STATIC].shine;
    physics_list[i].elasticity = 1.f;
    physics_list[i].radius = BALL_RADIUS;
  }
  lightPos = &model_list[1].transform.w;
  *lightPos = (Vec4) {0.f, 3.f, 0.f, 1.f};
}

int main(int argc, char **argv)
{
  srand(time(NULL)); rand();
  genModels();
  flattenModelListSM(&model_list,&vertices,&num_vertices,&num_models);

  eye.x = eye_radius * sin(theta) * cos(phi);
  eye.y = eye_radius * sin(theta) * sin(phi);
  eye.z = eye_radius * cos(theta);

  genPerspective(&pj_matrix, 30.f , 1.f, .01f, 10.f);
  genLookAt(&mv_matrix,&eye,&at,&up);

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(WINDOW, WINDOW);
  glutCreateWindow("Bouncy");
  glewInit();
  init();
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutIdleFunc(idle_func);
  glutMainLoop();

  return 0;
}

