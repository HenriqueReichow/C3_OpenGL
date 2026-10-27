#ifndef MATRIZES_H
#define MATRIZES_H

/* Transformações geométricas: cada função monta uma matriz 4x4
   (column-major) e multiplica a matriz atual com glMultMatrixf(). */
void translada(float tx, float ty, float tz);
void escala(float sx, float sy, float sz);
void rotacionaX(float graus);
void rotacionaY(float graus);
void rotacionaZ(float graus);

/* Projeções: usar com glMatrixMode(GL_PROJECTION) */
void projecaoPerspectiva(float fovy, float aspecto, float perto, float longe);
void projecaoOrtogonal(float esq, float dir, float baixo, float cima, float perto, float longe);

#endif
