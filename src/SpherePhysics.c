#include <math.h>
#include "SpherePhysics.h"
#include "VecLib.h"

void checkWalls(SphereEntity * ent, GLfloat boundary){
  if (fabsf(ent->position->x + ent->velocity.x) + ent->radius >= fabsf(boundary)){
    ent->velocity.x *= -1.f;
  }
  if (fabsf(ent->position->z + ent->velocity.z) + ent->radius >= fabsf(boundary)){
    ent->velocity.z *= -1.f;
  }

}

void sphereCollisionTick(SphereEntity * ents, int num_ents, GLfloat boundary){
  SphereEntity * collisions[num_ents];
  int count;
  for (int i = 0; i < num_ents; i++){
    count = 0;
    for (int j = 0; j < num_ents; j++){
      if (i < j && collidesWith(&ents[i],&ents[j])){
        collisions[count++] = &ents[j];
      }
    }
    for (int j = 0; j < count; j++){
      collision(&ents[i],collisions[j]); 
    }
    checkWalls(&ents[i], boundary);
  }
}

int collidesWith(SphereEntity * e1, SphereEntity * e2){
  Vec4 next1, next2;
  addVec4(&next1, e1->position, &e1->velocity);
  addVec4(&next2, e2->position, &e2->velocity);
  GLfloat dist = powf(powf(fabsf(next1.x - next2.x), 2.f) + powf(fabsf(next1.z - next2.z), 2.f), 0.5f);
  return(dist <= e1->radius + e2->radius);
}

void collision(SphereEntity * e1, SphereEntity * e2){
  Vec4 n1, n2;
  GLfloat dot1, dot2;
  GLfloat momentum1, momentum2;
  Vec4 temp1, temp2;
  Vec4 next1, next2;
  addVec4(&next1, e1->position, &e1->velocity);
  addVec4(&next2, e2->position, &e2->velocity);
  subVec4(&n1, &next1, &next2);
  normalize(&n1, &n1);
  scalarMultVec4(&n2, &n1, -1.f);
  dot(&dot1,&e1->velocity, &n1);
  dot(&dot2,&e2->velocity, &n2);
  vecSize(&momentum1, &e1->velocity);
  vecSize(&momentum2, &e2->velocity);
  momentum1 *= e1->mass;
  momentum2 *= e2->mass;
  scalarMultVec4(&temp1, &n1, (dot1 * 2.f) * (momentum2/momentum1));
  scalarMultVec4(&temp2, &n2, (dot2 * 2.f) * (momentum1/momentum2));
  //scalarMultVec4(&temp1, &n1, (dot1 * 2.f));
  //scalarMultVec4(&temp2, &n2, (dot2 * 2.f));
  temp1.w = 0.f;
  temp2.w = 0.f;
  subVec4(&e1->velocity, &e1->velocity, &temp1);
  subVec4(&e2->velocity, &e2->velocity, &temp2);
}







