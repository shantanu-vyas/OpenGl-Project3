#ifndef _SpherePhysics_
#define _SpherePhysics_

#include <stdlib.h>
#include <GL/glew.h>
#include "VecLib.h"

typedef struct SphereEntity {
  Vec4 velocity;
  Vec4 * position; // w of object transform
  GLfloat radius;
  GLfloat mass;
  GLfloat elasticity; // 0 - 1.00 ; determines bounce factor
} SphereEntity;

void checkWalls(SphereEntity * ent, GLfloat boundary, GLfloat delta_sec);
void sphereCollisionTick(SphereEntity * ents, int num_ents, GLfloat boundary, GLfloat delta_sec);
int collidesWith(SphereEntity * e1, SphereEntity * e2, GLfloat delta_sec);
void collision(SphereEntity * e1, SphereEntity * e2, GLfloat delta_sec);

#endif
