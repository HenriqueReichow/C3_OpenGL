#include <math.h>
#include <GL/glut.h>
#include "matrizes.h"

/*
 * O OpenGL lê o vetor m[16] coluna por coluna (column-major).
 * Então cada linha do vetor aqui no código é uma COLUNA da matriz:
 * por isso a translação aparece na última linha do vetor.
 */

#define PI 3.14159265358979f

static float radianos(float graus) {
    return graus * PI / 180.0f;
}

/*
 *  | 1 0 0 tx |
 *  | 0 1 0 ty |
 *  | 0 0 1 tz |
 *  | 0 0 0 1  |
 */
void translada(float tx, float ty, float tz) {
    float m[16] = {
        1,  0,  0,  0,
        0,  1,  0,  0,
        0,  0,  1,  0,
        tx, ty, tz, 1
    };
    glMultMatrixf(m);
}

/*
 *  | sx 0  0  0 |
 *  | 0  sy 0  0 |
 *  | 0  0  sz 0 |
 *  | 0  0  0  1 |
 */
void escala(float sx, float sy, float sz) {
    float m[16] = {
        sx, 0,  0,  0,
        0,  sy, 0,  0,
        0,  0,  sz, 0,
        0,  0,  0,  1
    };
    glMultMatrixf(m);
}

/*
 *  | 1  0   0  0 |
 *  | 0  c  -s  0 |
 *  | 0  s   c  0 |
 *  | 0  0   0  1 |
 */
void rotacionaX(float graus) {
    float c = cos(radianos(graus)), s = sin(radianos(graus));
    float m[16] = {
        1, 0,  0, 0,
        0, c,  s, 0,
        0, -s, c, 0,
        0, 0,  0, 1
    };
    glMultMatrixf(m);
}

/*
 *  |  c  0  s  0 |
 *  |  0  1  0  0 |
 *  | -s  0  c  0 |
 *  |  0  0  0  1 |
 */
void rotacionaY(float graus) {
    float c = cos(radianos(graus)), s = sin(radianos(graus));
    float m[16] = {
        c, 0, -s, 0,
        0, 1, 0,  0,
        s, 0, c,  0,
        0, 0, 0,  1
    };
    glMultMatrixf(m);
}

/*
 *  | c -s  0  0 |
 *  | s  c  0  0 |
 *  | 0  0  1  0 |
 *  | 0  0  0  1 |
 */
void rotacionaZ(float graus) {
    float c = cos(radianos(graus)), s = sin(radianos(graus));
    float m[16] = {
        c,  s, 0, 0,
        -s, c, 0, 0,
        0,  0, 1, 0,
        0,  0, 0, 1
    };
    glMultMatrixf(m);
}

/*
 * Perspectiva, com f = 1 / tan(fovy / 2):
 *
 *  | f/aspecto  0   0                         0                          |
 *  | 0          f   0                         0                          |
 *  | 0          0   (longe+perto)/(perto-longe)  2*longe*perto/(perto-longe) |
 *  | 0          0  -1                         0                          |
 *
 * O -1 faz w = -z. Depois o OpenGL divide x e y por w,
 * e é isso que deixa o que está longe menor.
 */
void projecaoPerspectiva(float fovy, float aspecto, float perto, float longe) {
    float f = 1.0f / tan(radianos(fovy) / 2.0f);
    float m[16] = {
        f / aspecto, 0, 0,                                   0,
        0,           f, 0,                                   0,
        0,           0, (longe + perto) / (perto - longe),  -1,
        0,           0, 2 * longe * perto / (perto - longe), 0
    };
    glMultMatrixf(m);
}

/*
 * Ortogonal: só leva a caixa de visão para o cubo de -1 a 1, sem dividir
 * pela profundidade, então tudo fica do mesmo tamanho.
 *
 *  | 2/(dir-esq)  0               0                -(dir+esq)/(dir-esq)         |
 *  | 0            2/(cima-baixo)  0                -(cima+baixo)/(cima-baixo)   |
 *  | 0            0              -2/(longe-perto)  -(longe+perto)/(longe-perto) |
 *  | 0            0               0                 1                           |
 */
void projecaoOrtogonal(float esq, float dir, float baixo, float cima, float perto, float longe) {
    float m[16] = {
        2 / (dir - esq), 0, 0, 0,
        0, 2 / (cima - baixo), 0, 0,
        0, 0, -2 / (longe - perto), 0,
        -(dir + esq) / (dir - esq), -(cima + baixo) / (cima - baixo), -(longe + perto) / (longe - perto), 1
    };
    glMultMatrixf(m);
}
