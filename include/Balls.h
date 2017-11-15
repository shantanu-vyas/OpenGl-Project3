#ifndef _GLToolkit_
#define _GLToolkit_

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

void idle_func();
void keyboard(unsigned char key, int mousex, int mousey);
void genModels();

#endif
