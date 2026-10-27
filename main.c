#include <stdlib.h>
#include <GL/glut.h>
#include "matrizes.h"
#include "formas.h"
#include "predio.h"

/* Câmera girando em volta do prédio, olhando de cima */
float giro       = 0;     /* A / D */
float inclinacao = 35;    /* W / S */
float distancia  = 90;    /* + / - */
int   ortogonal  = 0;     /* P troca entre perspectiva e ortogonal */
float aspecto    = 800.0f / 600;

void desenha(void) {
    float posLuz[4] = { 0.5f, 1.0f, 0.8f, 0.0f };   /* luz direcional (w = 0), de cima e da frente */

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (ortogonal) {
        /* mesmo tamanho de cena que a perspectiva mostra (tan(22.5) = 0.414) */
        float h = distancia * 0.414f;
        projecaoOrtogonal(-h * aspecto, h * aspecto, -h, h, 1, 500);
    } else {
        projecaoPerspectiva(45, aspecto, 1, 500);
    }

    /* A câmera não se move: o mundo é que é movido ao contrário.
       De baixo para cima: centraliza, gira em Y, inclina em X e afasta. */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    translada(0, 0, -distancia);
    rotacionaX(inclinacao);
    rotacionaY(giro);
    translada(0, 0, 8);

    glLightfv(GL_LIGHT0, GL_POSITION, posLuz);   /* depois da câmera: luz fixa no mundo */

    chao();
    desenhaPredio();

    glutSwapBuffers();
}

void redimensiona(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    aspecto = (float)w / h;
}

void teclado(unsigned char tecla, int x, int y) {
    if (tecla == 'a') giro -= 5;
    if (tecla == 'd') giro += 5;
    if (tecla == 'w' && inclinacao < 89) inclinacao += 5;
    if (tecla == 's' && inclinacao > 5)  inclinacao -= 5;
    if (tecla == '+' && distancia > 20)  distancia -= 5;
    if (tecla == '-' && distancia < 300) distancia += 5;
    if (tecla == 'p') ortogonal = !ortogonal;
    if (tecla == 27)  exit(0);
    glutPostRedisplay();
}

int main(int argc, char **argv) {
    float luzAmbiente[4]  = { 0.45f, 0.45f, 0.45f, 1 };
    float luzDifusa[4]    = { 0.8f, 0.8f, 0.8f, 1 };
    float luzEspecular[4] = { 1.0f, 1.0f, 1.0f, 1 };
    float matEspecular[4] = { 0.4f, 0.4f, 0.4f, 1 };

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Projeto CG - C3");

    glEnable(GL_DEPTH_TEST);              /* visibilidade: Z-buffer */
    glEnable(GL_NORMALIZE);               /* a escala estraga o tamanho das normais */
    glClearColor(0.6f, 0.8f, 1.0f, 1);    /* céu */

    /* Phong: ambiente + difusa + especular */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  luzAmbiente);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  luzDifusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular);

    /* o glColor vira a cor ambiente/difusa do material; o brilho é igual pra tudo */
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matEspecular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 30);

    /* Gouraud: cor calculada nos vértices e interpolada na face */
    glShadeModel(GL_SMOOTH);

    glutDisplayFunc(desenha);
    glutReshapeFunc(redimensiona);
    glutKeyboardFunc(teclado);
    glutMainLoop();
    return 0;
}
