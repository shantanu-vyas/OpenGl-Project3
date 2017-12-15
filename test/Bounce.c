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
#define WINDOW 1000
#define FPS 60.f
#define TICK 1.f/FPS

#define BALL_RADIUS 0.5f
#define NUM_STATIC 2

#define ROLL .999f // roll slowdown percentage

#define WOOD_TEXTURE 0
#define GEN_TEXTURE1 1

#define TEXTURE_WIDTH 500

clock_t global_clock_prev; // last time rendered/physics ticked
GLfloat curr_fps;

/* VSHADER INPUTS */
GLuint pj_location;
GLuint mv_location;
GLuint tr_location;
GLuint is_shadow;
GLuint is_texture;

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

GLfloat theta = 0.01f;
GLfloat phi = M_PI/2.f;

GLfloat eye_radius;

Vec4 eye = {0.f, 3.f, 3.f, 1.f};
Vec4 at =  {0.f, 0.f, 0.f, 1.f};
Vec4 up =  {0.f, 1.f, 0.f, 0.f};

GLfloat atten_const = 1.f;
GLfloat atten_linear = 0.01f;
GLfloat atten_quad = 0.001f;

Vec4 * lightPos;

Vec4* vertices;
int num_vertices;

ShaderModel * model_list;
SphereEntity * physics_list;
GLfloat boundaries;
int num_models;
int num_balls = 0;
int texture = -1;

void init(void)
{
  // texture
  int refs[36] = {1,0,3,1,3,2, 2,3,7,2,7,6, 3,0,4,3,4,7, 6,5,1,6,1,2, 4,5,6,4,6,7, 5,4,0,5,0,1};        // Order of vertices
  Vec2 texCoord_ref[6] = {{1.0, 0.0}, {0.0, 0.0}, {0.0, 1.0}, {0.0, 1.0}, {1.0, 1.0}, {1.0, 0.0}};
  int i, v_index = 0;
  Vec2 tex_coords[36];
  for(i = 0; i < 36; i++) {
    tex_coords[v_index] = texCoord_ref[i % 6];
    v_index++;
  }

  GLubyte my_texels[TEXTURE_WIDTH][TEXTURE_WIDTH][3];
  if (texture == WOOD_TEXTURE){
    unsigned char uc;
    FILE *fp;
    fp = fopen("textures/wood_500_500.raw", "r");
    if(fp == NULL) {
        printf("Unable to open file\n");
        exit(0);
      }
    for(int i = 0; i < TEXTURE_WIDTH; i++) {
      for(int j = 0; j < TEXTURE_WIDTH; j++) {
        for(int k = 0; k < 3; k++) {
            fread(&uc, 1, 1, fp);
            my_texels[i][j][k] = uc;
          }
        }
      }
    fclose(fp);
  }else if (texture == GEN_TEXTURE1){
    // spotty
    GLfloat s_radius = (GLfloat) TEXTURE_WIDTH / 5.f;
    int s_num = 2 * TEXTURE_WIDTH;
    int s_pos[s_num][2];
    Vec4 s_color[s_num];
    for (int s = 0; s < s_num; s++){
      scalarMultVec4(&s_color[s], &diffuse[rand()%(NUM_COLORS)], 1.f);
      s_pos[s][0] = -(int)s_radius + rand()%(TEXTURE_WIDTH + 2 * (int)s_radius);
      s_pos[s][1] = -(int)s_radius + rand()%(TEXTURE_WIDTH + 2 * (int)s_radius);
    }
    Vec4 inter_color, intra_color;
    int mask = 0xFF;
    for(int i = 0; i < TEXTURE_WIDTH; i++) {
      for(int j = 0; j < TEXTURE_WIDTH; j++) {
        inter_color = (Vec4){0.f,0.f,0.f,0.f};
        for (int s = 0; s < s_num; s++){
          GLfloat diff =(s_radius -
                         powf(((i - s_pos[s][0]) * (i - s_pos[s][0])) +
                              ((j - s_pos[s][1]) * (j - s_pos[s][1])), 0.5f))/
                         s_radius;
            if(diff > 0){
              scalarMultVec4(&intra_color, &s_color[s], diff);
              addVec4(&inter_color, &inter_color, &intra_color);
            }
        }
        scalarMultVec4(&inter_color, &inter_color, 256);
        my_texels[i][j][0] =  (GLubyte) (mask & ((int) inter_color.x))/5;
        my_texels[i][j][1] =  (GLubyte) (mask & ((int) inter_color.y))/5;
        my_texels[i][j][2] =  (GLubyte) (mask & ((int) inter_color.z))/5;
        }
    }
  }
  GLuint mytex;
  glGenTextures(1, &mytex);
  glBindTexture(GL_TEXTURE_2D, mytex);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGB,
               TEXTURE_WIDTH,     // Width
               TEXTURE_WIDTH,     // Height
               0,
               GL_RGB,
               GL_UNSIGNED_BYTE,
               my_texels);
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

  // other
  int size = 2*sizeof(Vec4)*num_vertices; //need at least double because shadows
  
  GLuint program = initShader("shaders/vshader.glsl", "shaders/fshader.glsl");
  glUseProgram(program);
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint buffer;
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, 2 * size + sizeof(Vec2)*36, NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, vertices);
  glBufferSubData(GL_ARRAY_BUFFER, size, size, vertices);
  glBufferSubData(GL_ARRAY_BUFFER, size*2, sizeof(Vec2)*36, tex_coords);

  GLuint vPosition = glGetAttribLocation(program, "vPosition");
  glEnableVertexAttribArray(vPosition);
  glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vec4), BUFFER_OFFSET(0));

  GLuint vNormal = glGetAttribLocation(program, "vNormal");
  glEnableVertexAttribArray(vNormal);
  glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, sizeof(Vec4), (GLvoid *) size);

  GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
  glEnableVertexAttribArray(vTexCoord);
  glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (GLvoid *)(size*2));

  is_shadow = glGetUniformLocation(program, "isShadow");
  is_texture = glGetUniformLocation(program, "isTexture");

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

  glUniform1i(glGetUniformLocation(program, "texture"), 0);
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
      if (i == 0){
        glUniform1i(is_texture,0);
      }else{
        glUniform1i(is_texture,1);
      }
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
    glUniform1i(is_texture,1);
    glUniform1i(is_shadow,1);
    glUniformMatrix4fv(tr_location, 1, GL_FALSE, (GLfloat *) &model_list[i].transform);
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
  for (int i = 0; i < num_balls; i++){
    physics_list[i].velocity.x *= ROLL;
    physics_list[i].velocity.z *= ROLL;
    scalarMultVec4(&tick_speed, &physics_list[i].velocity, delta_sec);
    addVec4(&model_list[i+NUM_STATIC].transform.w, &model_list[i+NUM_STATIC].transform.w, &tick_speed);
  }
  sphereCollisionTick(physics_list, num_balls, boundaries, delta_sec);
}

void setToPoolBreak(){
  int iter = 0;
  int row_iter = 0;
  int row = 1;
  GLfloat row_offset = powf(powf(2.f*BALL_RADIUS,2.f) - powf(BALL_RADIUS,2.f),0.5f);
  Vec4 shoot_start = {-4.f * boundaries/5.f, BALL_RADIUS, 0.f, 1.f};
  Vec4 iter_pos = {boundaries/3.f, BALL_RADIUS, 0.f, 1.f};
  while (iter < num_balls){
    row_iter++;
    iter++;
    *physics_list[iter - 1].position = iter_pos;
    physics_list[iter - 1].velocity = (Vec4) {0.f,0.f,0.f,0.f};
    if (iter == num_balls){
      *physics_list[iter - 1].position = shoot_start;
      physics_list[iter - 1].velocity = (Vec4) {BALL_RADIUS*num_balls/2.f,0.f,0.f,0.f};
    }else{
      if(row_iter%row == 0){
        row++;
        row_iter = 0;
        iter_pos.z = -1.f * BALL_RADIUS * (row-1);
        iter_pos.x += row_offset;
      }else{
        iter_pos.z += 2.f * BALL_RADIUS;
      }
    }
  }

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
  if (key == 'p') setToPoolBreak();
  if (key == ' ') {
    for (int i = 0; i < num_balls; i++){
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
}

void genModels()
{
  ShaderModel ground;
  ShaderModel light;
  ShaderModel balls[num_balls];

  GLfloat shine;
  boundaries = (GLfloat)(num_balls + 1) / 2.f;
 
  shine = 50.f;
  
  makeCubeSM(&ground, &ambient[GREEN], &specular[GREEN], &diffuse[GREEN], &shine);
  scaleXModelSM(&ground, &ground.num_vertices, boundaries);
  scaleYModelSM(&ground, &ground.num_vertices, .005f);
  scaleZModelSM(&ground, &ground.num_vertices, boundaries);
  
  shine = 30.f;
  
  makeSphereSM(&light,&specular[WHITE], &specular[WHITE], &specular[WHITE], &shine);
  scaleXModelSM(&light, &light.num_vertices, .2f);
  scaleYModelSM(&light, &light.num_vertices, .2f);
  scaleZModelSM(&light, &light.num_vertices, .2f);
  
  int color1;
  int color2;
  int color3;
  Vec4 amb, spec, diff;
  for (int i = 0; i < num_balls; i++){
    // init model
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

  num_models = NUM_STATIC + num_balls;

  model_list = malloc(sizeof(ShaderModel) * num_models);
  physics_list = malloc(sizeof(SphereEntity) * num_balls); 

  model_list[0] = ground;
  model_list[1] = light;
  for (int i = 0; i < num_balls; i++) model_list[i+NUM_STATIC] = balls[i];
  // init tranformations
  for (int i = 0; i < num_models; i++) identity(&model_list[i].transform);
  // init ball positions
  for (int i = 0; i < num_balls; i++){
    GLfloat len = (GLfloat) i - ((GLfloat)num_balls / 2.f) + 0.5f;
    Vec4 offset = {(GLfloat) len, 0.5f, (GLfloat) len, 1.f};
    model_list[i+NUM_STATIC].transform.w = offset;
    
  }
  // init physics
  for (int i = 0; i < num_balls; i++){
    GLfloat speed = (GLfloat)(rand() % 1000) / 5000.f;
    GLfloat angle = fmodf((GLfloat) rand(), 2.f * M_PI);
    Vec4 vel = {speed * cosf(angle), 0.f, speed * sinf(angle), 0.f};
    
    physics_list[i].velocity = vel;
    physics_list[i].position = &model_list[i+NUM_STATIC].transform.w;
    physics_list[i].elasticity = 1.f;
    physics_list[i].radius = BALL_RADIUS;
  }
  lightPos = &model_list[1].transform.w;
  *lightPos = (Vec4) {(GLfloat) num_balls / 4.f, (GLfloat) num_balls / 4.f, 0.f, 1.f};
}

int main(int argc, char **argv)
{
  srand(time(NULL)); rand();
  // prompt user input
  char line[5];
  printf("Number of balls (best with triangle number + 1): ");
  do{
    fgets (line, sizeof(line), stdin);
    sscanf (line, "%d", &num_balls);
  } while (num_balls < 1);
  printf("Texture (0 = wood, 1 = crazy): ");
  do{
    fgets (line, sizeof(line), stdin);
    sscanf (line, "%d", &texture);
  } while (texture < 0 || texture > 1);
  eye_radius = (GLfloat) num_balls;

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








