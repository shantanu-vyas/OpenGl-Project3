#include <math.h>
#include "SpherePhysics.h"
#include "VecLib.h"

void checkWalls(SphereEntity * ent, GLfloat boundary, GLfloat delta_sec){
  Vec4 speed;
  scalarMultVec4(&speed, &ent->velocity, delta_sec);
  if (fabsf(ent->position->x + speed.x) + ent->radius >= fabsf(boundary)){
    ent->velocity.x *= -1.f;
  }
  if (fabsf(ent->position->z + speed.z) + ent->radius >= fabsf(boundary)){
    ent->velocity.z *= -1.f;
  }

}

void sphereCollisionTick(SphereEntity * ents, int num_ents, GLfloat boundary, GLfloat delta_sec){
  for (int i = 0; i < num_ents; i++){
    checkWalls(&ents[i], boundary, delta_sec);
    for (int j = 0; j < num_ents; j++){
      if (i < j && collidesWith(&ents[i],&ents[j], delta_sec)){
        collision(&ents[i], &ents[j], delta_sec); 
      }
    }
  }
}

int collidesWith(SphereEntity * e1, SphereEntity * e2, GLfloat delta_sec){
  Vec4 next1, next2;
  Vec4 speed1, speed2;
  Vec4 diff;
  GLfloat dist;
  scalarMultVec4(&speed1, &e1->velocity, delta_sec);
  scalarMultVec4(&speed2, &e2->velocity, delta_sec);
  addVec4(&next1, e1->position, &speed1);
  addVec4(&next2, e2->position, &speed2);
  subVec4(&diff, &next1, &next2);
  vecSize(&dist, &diff);
  return(dist <= e1->radius + e2->radius);
}

void collision(SphereEntity * e1, SphereEntity * e2, GLfloat delta_sec){
  Vec4 n1, n2;
  GLfloat dot1, dot2;
  GLfloat dist;
  Vec4 temp1, temp2;
  Vec4 next1, next2;
  Vec4 speed1, speed2;
  scalarMultVec4(&speed1, &e1->velocity, delta_sec);
  scalarMultVec4(&speed2, &e2->velocity, delta_sec);
  addVec4(&next1, e1->position, &speed1);
  addVec4(&next2, e2->position, &speed2);
  subVec4(&n1, &next1, &next2);
  vecSize(&dist, &n1);
  subVec4(&n2, &next2, &next1);
  normalize(&n1, &n1);
  normalize(&n2, &n2);
  // object stack push  
  Vec4 push;
  dist = (dist - (e1->radius + e2->radius))/2.f;
  scalarMultVec4(&push, &n1, dist);
  push.w = 0.f;
  subVec4(e1->position, e1->position, &push);
  addVec4(e2->position, e2->position, &push);
  // reflect momentums
  dot(&dot1, &e1->velocity, &n1);
  dot(&dot2, &e2->velocity, &n2);
  scalarMultVec4(&temp1, &n1, dot1 * 2.f);
  scalarMultVec4(&temp2, &n2, dot2 * 2.f);
  temp1.w = 0.f;
  temp2.w = 0.f;
  subVec4(&e1->velocity, &e1->velocity, &temp1);
  //try dis subVec4(&e2->velocity, &e2->velocity, &temp2);
  //try dis addVec4(&e1->velocity, &e1->velocity, &temp2);
  addVec4(&e2->velocity, &e2->velocity, &temp1);
}







