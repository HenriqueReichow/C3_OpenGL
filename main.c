#include <stdlib.h>
#include <math.h>
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
int   mouseX, mouseY;     /* última posição do mouse durante o arrasto */
int   botaoMouse;         /* botão que está arrastando */
float droneX = -7.7f, droneZ = 18.3f;   /* drone no mini mundo, começa em cima do estacionamento (setas ou botão direito) */
int   largura = 800, altura = 600;

/*
 * Mini mundo (M), o "pequeno planeta" dos drones:
 * 1. a cena é desenhada 6 vezes de um ponto no alto, olhando para os 6 lados
 *    de um cubo (90 graus cada), e cada imagem é lida com glReadPixels;
 * 2. cada pixel da tela vira uma direção pela projeção estereográfica inversa
 *    (centro da tela = direto pra baixo, longe do centro = céu);
 * 3. a direção é procurada na face do cubo certa e o resultado vai pra tela com glDrawPixels.
 */
#define FACE 512
int   miniMundo = 0;
float faceRot[6][2] = {               /* rotacionaX, rotacionaY de cada face */
    {0, 0}, {0, 90}, {0, 180}, {0, -90}, {90, 0}, {-90, 0}
};
float faceMat[6][9];                  /* mesma rotação em 3x3, para o passo 3 */
unsigned char faces[6][FACE * FACE * 3];
unsigned char *tela = NULL;
float matEspecular[4] = { 0.4f, 0.4f, 0.4f, 1 };   /* brilho do material, volta depois do mini mundo */

/* Luz e objetos; a câmera já tem que estar na MODELVIEW */
void cena(void) {
    float posLuz[4] = { 0.5f, 1.0f, 0.8f, 0.0f };   /* luz direcional (w = 0), de cima e da frente */

    glLightfv(GL_LIGHT0, GL_POSITION, posLuz);   /* depois da câmera: luz fixa no mundo */
    chao();
    desenhaPredio();
}

/* Matriz 3x3 de rotacionaX(a) * rotacionaY(b), por linhas, igual à que vai pro OpenGL */
void montaFace(int f) {
    float a = faceRot[f][0] * 3.14159265f / 180, b = faceRot[f][1] * 3.14159265f / 180;
    float ca = cos(a), sa = sin(a), cb = cos(b), sb = sin(b);
    float *m = faceMat[f];
    m[0] = cb;       m[1] = 0;   m[2] = sb;
    m[3] = sa * sb;  m[4] = ca;  m[5] = -sa * cb;
    m[6] = -ca * sb; m[7] = sa;  m[8] = ca * cb;
}

float alturaDrone(void)  { return 5 + inclinacao * 0.6f; }          /* W / S sobem e descem o drone */
float pixelsPorRaio(void) { return 0.25f * altura * 90 / distancia; } /* + / - dão zoom no planeta */

void desenhaMiniMundo(void) {
    float alturaCam = alturaDrone();
    float pixels = pixelsPorRaio();
    float giroRad = giro * 3.14159265f / 180;
    float semBrilho[4] = { 0, 0, 0, 1 };
    int f, px, py;

    /* 1. as 6 faces do cubo. Sem especular: o brilho depende de pra onde a câmera
       olha, e mudaria de uma face pra outra (dá pra ver a emenda) */
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, semBrilho);
    glViewport(0, 0, FACE, FACE);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    projecaoPerspectiva(90, 1, 0.5f, 1000);
    glMatrixMode(GL_MODELVIEW);
    for (f = 0; f < 6; f++) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        rotacionaX(faceRot[f][0]);
        rotacionaY(faceRot[f][1]);
        translada(-droneX, -alturaCam, -droneZ);
        cena();
        glReadPixels(0, 0, FACE, FACE, GL_RGB, GL_UNSIGNED_BYTE, faces[f]);
    }
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matEspecular);

    /* 2 e 3. projeção estereográfica: r = 2 tan(theta / 2), theta medido a partir do chão */
    for (py = 0; py < altura; py++)
        for (px = 0; px < largura; px++) {
            float u = (px - largura / 2.0f) / pixels, v = (py - altura / 2.0f) / pixels;
            float theta = 2 * atan(sqrt(u * u + v * v) / 2);
            float phi = atan2(v, u) + giroRad;
            float d[3], melhor = 0;
            int fm = 0, i, j;
            unsigned char *c;
            unsigned char *saida = tela + (py * largura + px) * 3;

            d[0] = sin(theta) * cos(phi);
            d[1] = -cos(theta);
            d[2] = -sin(theta) * sin(phi);          /* visto de cima, com x pra direita o z aponta pra baixo da tela */

            /* a face certa é a que olha mais na direção d (maior -z no espaço da câmera);
               a linha 3 da matriz dá o z, as outras dão x e y */
            for (f = 0; f < 6; f++) {
                float *m = faceMat[f];
                float ez = m[6] * d[0] + m[7] * d[1] + m[8] * d[2];
                if (-ez > melhor) { melhor = -ez; fm = f; }
            }
            {
                float *m = faceMat[fm];
                float ex = m[0] * d[0] + m[1] * d[1] + m[2] * d[2];
                float ey = m[3] * d[0] + m[4] * d[1] + m[5] * d[2];
                i = (int)((ex / melhor + 1) / 2 * (FACE - 1) + 0.5f);
                j = (int)((ey / melhor + 1) / 2 * (FACE - 1) + 0.5f);
            }
            c = faces[fm] + (j * FACE + i) * 3;
            saida[0] = c[0];  saida[1] = c[1];  saida[2] = c[2];
        }

    glViewport(0, 0, largura, altura);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);                    /* glRasterPos no canto de baixo */
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glRasterPos2f(-1, -1);
    glDrawPixels(largura, altura, GL_RGB, GL_UNSIGNED_BYTE, tela);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void desenha(void) {
    if (miniMundo) {
        desenhaMiniMundo();
        glutSwapBuffers();
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (ortogonal) {
        /* mesmo tamanho de cena que a perspectiva mostra (tan(22.5) = 0.414) */
        float h = distancia * 0.414f;
        projecaoOrtogonal(-h * aspecto, h * aspecto, -h, h, 1, 1000);
    } else {
        projecaoPerspectiva(45, aspecto, 1, 1000);
    }

    /* A câmera não se move: o mundo é que é movido ao contrário.
       De baixo para cima: centraliza, gira em Y, inclina em X e afasta. */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    translada(0, 0, -distancia);
    rotacionaX(inclinacao);
    rotacionaY(giro);
    translada(0, 0, 8);

    cena();

    glutSwapBuffers();
}

void redimensiona(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    aspecto = (float)w / h;
    largura = w;
    altura = h;
    tela = realloc(tela, w * h * 3);
}

/*
 * Anda com o drone 'frente' metros pra cima da tela e 'lado' pra direita.
 * No mini mundo a direita da tela é a direção (cos phi, -sin phi) com phi = giro,
 * e o alto é a mesma coisa com phi = giro + 90.
 */
void andaDrone(float lado, float frente) {
    float g = giro * 3.14159265f / 180;
    droneX += lado * cos(g) - frente * sin(g);
    droneZ -= lado * sin(g) + frente * cos(g);
}

void setas(int tecla, int x, int y) {
    if (!miniMundo) return;
    if (tecla == GLUT_KEY_UP)    andaDrone(0, 2);
    if (tecla == GLUT_KEY_DOWN)  andaDrone(0, -2);
    if (tecla == GLUT_KEY_RIGHT) andaDrone(2, 0);
    if (tecla == GLUT_KEY_LEFT)  andaDrone(-2, 0);
    glutPostRedisplay();
}

void teclado(unsigned char tecla, int x, int y) {
    if (tecla == 'a') giro -= 5;
    if (tecla == 'd') giro += 5;
    if (tecla == 'w' && inclinacao < 89) inclinacao += 5;
    if (tecla == 's' && inclinacao > 5)  inclinacao -= 5;
    if (tecla == '+' && distancia > 20)  distancia -= 5;
    if (tecla == '-' && distancia < 300) distancia += 5;
    if (tecla == 'p') ortogonal = !ortogonal;
    if (tecla == 'm') miniMundo = !miniMundo;
    if (tecla == 27)  exit(0);
    glutPostRedisplay();
}

/* Botão esquerdo arrasta a câmera; a rodinha (botões 3 e 4) dá zoom */
void mouse(int botao, int estado, int x, int y) {
    if ((botao == GLUT_LEFT_BUTTON || botao == GLUT_RIGHT_BUTTON) && estado == GLUT_DOWN) {
        botaoMouse = botao;
        mouseX = x;
        mouseY = y;
    }
    if (estado == GLUT_DOWN && botao == 3 && distancia > 20)  distancia -= 5;
    if (estado == GLUT_DOWN && botao == 4 && distancia < 300) distancia += 5;
    glutPostRedisplay();
}

void arrasta(int x, int y) {
    if (botaoMouse == GLUT_RIGHT_BUTTON) {
        /* botão direito no mini mundo: o chão acompanha o mouse.
           No centro da tela r = 2 tan(theta / 2) ~ distância / altura do drone */
        float metro = pixelsPorRaio() / alturaDrone();
        if (miniMundo) andaDrone(-(x - mouseX) / metro, (y - mouseY) / metro);
        mouseX = x;
        mouseY = y;
        glutPostRedisplay();
        return;
    }
    giro       += (x - mouseX) * 0.5f;
    inclinacao += (y - mouseY) * 0.5f;
    if (inclinacao > 89) inclinacao = 89;
    if (inclinacao < 5)  inclinacao = 5;
    mouseX = x;
    mouseY = y;
    glutPostRedisplay();
}

int main(int argc, char **argv) {
    float luzAmbiente[4]  = { 0.45f, 0.45f, 0.45f, 1 };
    float luzDifusa[4]    = { 0.8f, 0.8f, 0.8f, 1 };
    float luzEspecular[4] = { 1.0f, 1.0f, 1.0f, 1 };

    int f;

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

    for (f = 0; f < 6; f++) montaFace(f);
    tela = malloc(largura * altura * 3);

    glutDisplayFunc(desenha);
    glutReshapeFunc(redimensiona);
    glutKeyboardFunc(teclado);
    glutMouseFunc(mouse);
    glutMotionFunc(arrasta);
    glutSpecialFunc(setas);
    glutMainLoop();
    return 0;
}
