#include <math.h>
#include <GL/glut.h>
#include "formas.h"

/*
 * Formas básicas, todas com tamanho 1. O tamanho e a posição de verdade
 * são dados depois com escala/translada/rotaciona.
 * Os vértices de cada face vão em sentido anti-horário vistos de fora,
 * e cada face tem a sua normal para a iluminação.
 */

/* Cubo de -0.5 a 0.5 nos três eixos */
void cubo(void) {
    float h = 0.5f;

    glBegin(GL_QUADS);
        glNormal3f(0, 0, 1);    /* frente */
        glVertex3f(-h, -h,  h);  glVertex3f( h, -h,  h);
        glVertex3f( h,  h,  h);  glVertex3f(-h,  h,  h);

        glNormal3f(0, 0, -1);   /* trás */
        glVertex3f( h, -h, -h);  glVertex3f(-h, -h, -h);
        glVertex3f(-h,  h, -h);  glVertex3f( h,  h, -h);

        glNormal3f(1, 0, 0);    /* direita */
        glVertex3f( h, -h,  h);  glVertex3f( h, -h, -h);
        glVertex3f( h,  h, -h);  glVertex3f( h,  h,  h);

        glNormal3f(-1, 0, 0);   /* esquerda */
        glVertex3f(-h, -h, -h);  glVertex3f(-h, -h,  h);
        glVertex3f(-h,  h,  h);  glVertex3f(-h,  h, -h);

        glNormal3f(0, 1, 0);    /* cima */
        glVertex3f(-h,  h,  h);  glVertex3f( h,  h,  h);
        glVertex3f( h,  h, -h);  glVertex3f(-h,  h, -h);

        glNormal3f(0, -1, 0);   /* baixo */
        glVertex3f(-h, -h, -h);  glVertex3f( h, -h, -h);
        glVertex3f( h, -h,  h);  glVertex3f(-h, -h,  h);
    glEnd();
}

/* Prisma: os vértices ficam num círculo de raio 1 (x = cos, z = sin) */
void prisma(int lados) {
    int i;
    float passo = 2.0f * 3.14159265f / lados;

    glBegin(GL_QUADS);                  /* paredes */
    for (i = 0; i < lados; i++) {
        float a1 = i * passo, a2 = (i + 1) * passo;
        float meio = (a1 + a2) / 2;

        glNormal3f(cos(meio), 0, sin(meio));
        glVertex3f(cos(a1), 0, sin(a1));
        glVertex3f(cos(a1), 1, sin(a1));
        glVertex3f(cos(a2), 1, sin(a2));
        glVertex3f(cos(a2), 0, sin(a2));
    }
    glEnd();

    glBegin(GL_POLYGON);                /* tampa de cima (de trás pra frente pra ficar anti-horário) */
        glNormal3f(0, 1, 0);
        for (i = lados - 1; i >= 0; i--)
            glVertex3f(cos(i * passo), 1, sin(i * passo));
    glEnd();

    glBegin(GL_POLYGON);                /* tampa de baixo */
        glNormal3f(0, -1, 0);
        for (i = 0; i < lados; i++)
            glVertex3f(cos(i * passo), 0, sin(i * passo));
    glEnd();
}

/* Pirâmide com a mesma base do prisma, então encaixa em cima dele */
void piramide(int lados) {
    int i;
    float passo = 2.0f * 3.14159265f / lados;
    float apotema = cos(passo / 2);

    glBegin(GL_TRIANGLES);
    for (i = 0; i < lados; i++) {
        float a1 = i * passo, a2 = (i + 1) * passo;
        float meio = (a1 + a2) / 2;

        glNormal3f(cos(meio), apotema, sin(meio));   /* inclinada: para fora e para cima */
        glVertex3f(cos(a1), 0, sin(a1));
        glVertex3f(0, 1, 0);
        glVertex3f(cos(a2), 0, sin(a2));
    }
    glEnd();
}

/* Tronco: base de raio 1 em y = 0 e topo de raio raioTopo em y = 1 */
void tronco(int lados, float raioTopo) {
    int i;
    float passo = 2.0f * 3.14159265f / lados;
    float apotema = cos(passo / 2);

    glBegin(GL_QUADS);
    for (i = 0; i < lados; i++) {
        float a1 = i * passo, a2 = (i + 1) * passo;
        float meio = (a1 + a2) / 2;

        glNormal3f(cos(meio), (1 - raioTopo) * apotema, sin(meio));
        glVertex3f(cos(a1), 0, sin(a1));
        glVertex3f(raioTopo * cos(a1), 1, raioTopo * sin(a1));
        glVertex3f(raioTopo * cos(a2), 1, raioTopo * sin(a2));
        glVertex3f(cos(a2), 0, sin(a2));
    }
    glEnd();
}

/* Telhado: base 1 x 1 em y = 0, cumeeira ao longo do eixo X em y = 1 */
void telhado(void) {
    float h = 0.5f;

    glBegin(GL_QUADS);                  /* as duas águas */
        glNormal3f(0, 0.5f, 1);
        glVertex3f(-h, 0,  h);  glVertex3f( h, 0,  h);
        glVertex3f( h, 1,  0);  glVertex3f(-h, 1,  0);

        glNormal3f(0, 0.5f, -1);
        glVertex3f( h, 0, -h);  glVertex3f(-h, 0, -h);
        glVertex3f(-h, 1,  0);  glVertex3f( h, 1,  0);
    glEnd();

    glBegin(GL_TRIANGLES);              /* os triângulos das pontas */
        glNormal3f(1, 0, 0);
        glVertex3f( h, 0,  h);  glVertex3f( h, 0, -h);  glVertex3f( h, 1, 0);

        glNormal3f(-1, 0, 0);
        glVertex3f(-h, 0, -h);  glVertex3f(-h, 0,  h);  glVertex3f(-h, 1, 0);
    glEnd();
}

/* Gramado de 200 x 200 m */
void chao(void) {
    float t = 600;      /* grande pra chegar até o horizonte no mini mundo */
    glColor3f(0.3f, 0.6f, 0.25f);
    glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glVertex3f(-t, 0,  t);  glVertex3f( t, 0,  t);
        glVertex3f( t, 0, -t);  glVertex3f(-t, 0, -t);
    glEnd();
}

/*
 * DDA em 3D. Os "pixels" são uma grade de 2 cm.
 * O número de passos é o maior entre |dx|, |dy| e |dz|; a cada passo soma o
 * incremento em cada eixo e arredonda para a célula mais próxima.
 */
void linhaDDA(float x0, float y0, float z0, float x1, float y1, float z1) {
    float grade = 50;                   /* células por metro */
    float dx = (x1 - x0) * grade;
    float dy = (y1 - y0) * grade;
    float dz = (z1 - z0) * grade;
    float maior = fabs(dx);
    int passos, i;
    float incX, incY, incZ;
    float x = x0 * grade, y = y0 * grade, z = z0 * grade;

    if (fabs(dy) > maior) maior = fabs(dy);
    if (fabs(dz) > maior) maior = fabs(dz);
    passos = (int)maior;
    incX = dx / passos;  incY = dy / passos;  incZ = dz / passos;

    glBegin(GL_POINTS);
        glNormal3f(0, 1, 0);
        for (i = 0; i <= passos; i++) {
            glVertex3f(floor(x + 0.5f) / grade, floor(y + 0.5f) / grade, floor(z + 0.5f) / grade);
            x += incX;
            y += incY;
            z += incZ;
        }
    glEnd();
}
