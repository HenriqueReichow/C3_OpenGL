#include <math.h>
#include <GL/glut.h>
#include "matrizes.h"
#include "formas.h"
#include "predio.h"

/*
 * Prédio do C3 (FURG). Medidas em metros, tiradas da foto aérea e do Google Earth.
 * X para a direita, Y para cima, Z para a frente. O átrio fica perto da origem.
 *
 * Partes:
 *   átrio         prisma de 6 lados, com a claraboia (pirâmide) em cima;
 *                 a estrutura da claraboia é desenhada com o algoritmo DDA
 *   miolo         prisma de 3 lados que preenche o espaço entre o átrio e as alas
 *   painéis       tronco de pirâmide de 6 lados, o telhado em volta da claraboia
 *   bloco central caixa atrás do átrio, até a barra do fundo
 *   ala esquerda  caixa girada 30 graus, fachadas de tijolo
 *   ala direita   caixa girada -30 graus, com uma faixa mais baixa do lado da entrada
 *   barra do fundo trecho reto + dois trechos dobrados, só com paredes
 *   anexos        nas pontas da barra do fundo, com varandas abertas
 *   entorno       estacionamento, caminho, rotatória e árvores
 *
 * Paredes são caixas finas (cubo esticado). Cada parte é desenhada com
 * paredes + mureta (a parede passa um pouco do telhado) + telhado por dentro.
 */

/* Barra do fundo */
#define FUNDO_Z         -36    /* parede de trás do trecho reto */
#define FUNDO_MEIO       20    /* metade do trecho reto (40 m) */
#define FUNDO_LADO       13    /* comprimento dos trechos dobrados */
#define FUNDO_LARG       12    /* 6 m da parte alta + 6 m da parte baixa */
#define FUNDO_ANG        30
#define FUNDO_ALT         8    /* parte alta */
#define FUNDO_FRONT_ALT   7    /* parte baixa, virada para o átrio */

/* Anexos nas pontas da barra do fundo */
#define ANEXO_COMP        5    /* quanto sai da ponta */
#define ANEXO_LARG        8
#define ANEXO_RECUO       4    /* distância da parede da frente da barra */
#define ANEXO_ALT         6
#define VARANDA         1.5

/* Bloco central */
#define CENTRAL_X      0
#define CENTRAL_Z    -18
#define CENTRAL_ANG   90
#define CENTRAL_COMP  13
#define CENTRAL_LARG  12
#define CENTRAL_ALT    8

/* Átrio e claraboia */
#define ATRIO_RAIO     3.33
#define ATRIO_ALT      8.5
#define ATRIO_X        0
#define ATRIO_Z       -5
#define CLARABOIA_ALT  0.5

/* Miolo triangular (mesmo centro do átrio, com a face reta virada para a frente) */
#define TRI_RAIO     16.7
#define TRI_ALT      8

/* Ala esquerda */
#define ESQ_X       -9.2
#define ESQ_Z        1.15
#define ESQ_ANG     30
#define ESQ_COMP    12
#define ESQ_LARG    12
#define ESQ_ALT      9

/* Ala direita */
#define DIR_X       10.5
#define DIR_Z        1.9
#define DIR_ANG     -30
#define DIR_COMP    15
#define DIR_LARG    12
#define DIR_ALT      9
#define DIR_FRONT_LARG  2      /* faixa mais baixa do lado da entrada */
#define DIR_FRONT_ALT   8

#define PLATIBANDA   1.0       /* quanto a parede passa da laje */
#define ESPESSURA    0.3       /* espessura das paredes */
#define TELHADO_ALT  0.8
#define SAIA_RAIO   (TRI_RAIO * 0.577)   /* painéis vão até a face do miolo */
#define ANDAR        3.3

/* Tipos de fachada */
#define SEM      0     /* sem janela */
#define SIMPLES  1     /* janelas de 2 vidros, uma a cada 3 m */
#define FITA     2     /* grupos de 4 vidros quase colados */
#define PILARES  3     /* grupos de 4 vidros com pilar branco entre eles */

#define VIDRO_L  0.9
#define JANELA_A 1.6

#define BRANCO   glColor3f(0.85f, 0.85f, 0.82f)
#define TELHA    glColor3f(0.82f, 0.86f, 0.9f)


/* ---------- peças pequenas ---------- */

/* Caixa: cubo esticado com centro em (x, y, z) */
static void caixa(float x, float y, float z, float sx, float sy, float sz) {
    glPushMatrix();
        translada(x, y, z);
        escala(sx, sy, sz);
        cubo();
    glPopMatrix();
}

/* Retângulo em pé no plano z, virado para +Z */
static void retangulo(float x1, float y1, float x2, float y2, float z) {
    glBegin(GL_QUADS);
        glNormal3f(0, 0, 1);
        glVertex3f(x1, y1, z);  glVertex3f(x2, y1, z);
        glVertex3f(x2, y2, z);  glVertex3f(x1, y2, z);
    glEnd();
}

/*
 * Janela com centro em x e base em y: moldura, vidro e esquadria
 * (um montante entre cada vidro e uma travessa). Cada camada fica 1 cm
 * mais pra fora que a anterior para não brigar no Z-buffer.
 */
static void janela(float x, float y, float z, int panos) {
    float largura = panos * VIDRO_L;
    float x1 = x - largura / 2, x2 = x + largura / 2;
    int i;

    glColor3f(0.93f, 0.93f, 0.9f);
    retangulo(x1 - 0.08f, y - 0.08f, x2 + 0.08f, y + JANELA_A + 0.08f, z + 0.01f);   /* moldura */
    glColor3f(0.55f, 0.6f, 0.52f);
    retangulo(x1, y, x2, y + JANELA_A, z + 0.02f);                                    /* vidro */

    glColor3f(0.93f, 0.93f, 0.9f);
    for (i = 1; i < panos; i++)                                                       /* montantes */
        retangulo(x1 + i * VIDRO_L - 0.03f, y, x1 + i * VIDRO_L + 0.03f, y + JANELA_A, z + 0.03f);
    retangulo(x1, y + JANELA_A * 0.55f - 0.03f, x2, y + JANELA_A * 0.55f + 0.03f, z + 0.03f);   /* travessa */
}

/* Porta de uma folha (lado esquerdo em x). Fica na frente do vidro da janela do térreo. */
static void porta(float x, float z) {
    glColor3f(0.93f, 0.93f, 0.9f);
    retangulo(x - 0.08f, 0, x + VIDRO_L + 0.04f, 2.68f, z + 0.05f);   /* moldura */
    glColor3f(0.12f, 0.22f, 0.2f);
    retangulo(x, 0, x + VIDRO_L - 0.04f, 2.6f, z + 0.06f);            /* vidro */
}

/*
 * Tijolo e janelas de um lado de um bloco (já posicionado e girado).
 * giro escolhe o lado (0 = +Z, 180 = -Z, 90 = +X, -90 = -X); depois de girar,
 * a face fica sempre em +Z, a 'dist' do centro.
 */
static void fachada(float giro, float dist, float largura, float altura, int tijolo, int tipo) {
    float y, x, passo = 3;
    int n, i, panos = 2;

    if (tipo == PILARES) { panos = 4; passo = 4 * VIDRO_L + 0.6f; }
    n = (int)((largura - 1) / passo);        /* quantas janelas cabem */

    glPushMatrix();
        rotacionaY(giro);

        if (tijolo) {
            glColor3f(0.72f, 0.4f, 0.27f);
            retangulo(-largura / 2, 0, largura / 2, altura, dist + 0.005f);
        }

        if (tipo == FITA) {
            /* grupos de 4 vidros da esquerda pra direita; o último fica com o que sobrar */
            for (y = 1.0f; y + JANELA_A < altura - PLATIBANDA; y += ANDAR) {
                float sobra = largura - 2;
                x = -sobra / 2;
                while (sobra >= VIDRO_L) {
                    int p = (int)(sobra / VIDRO_L);
                    if (p > 4) p = 4;
                    janela(x + p * VIDRO_L / 2, y, dist, p);
                    x += p * VIDRO_L + 0.2f;
                    sobra -= p * VIDRO_L + 0.2f;
                }
            }
        } else if (tipo != SEM) {
            for (y = 1.0f; y + JANELA_A < altura - PLATIBANDA; y += ANDAR)
                for (i = 0; i < n; i++) {
                    x = (i - (n - 1) / 2.0f) * passo;
                    janela(x, y, dist, panos);
                }

            if (tipo == PILARES) {
                glColor3f(0.95f, 0.95f, 0.93f);
                for (i = 0; i <= n; i++) {
                    x = (i - n / 2.0f) * passo;
                    retangulo(x - 0.15f, 0, x + 0.15f, altura, dist + 0.04f);
                }
            }
        }
    glPopMatrix();
}

/* Parede de (x1, z1) até (x2, z2) com n janelas de 1 vidro por andar.
   ladoJanelas: +1 janelas em +Z, -1 em -Z, 0 nenhuma. */
static void parede(float x1, float z1, float x2, float z2, float altura, int ladoJanelas, int n) {
    float dx = x2 - x1, dz = z2 - z1;
    float y;
    int i;
    float comprimento = sqrt(dx * dx + dz * dz);
    float angulo = atan2(-dz, dx) * 180 / 3.14159265f;

    glPushMatrix();
        translada((x1 + x2) / 2, 0, (z1 + z2) / 2);
        rotacionaY(angulo);
        BRANCO;
        caixa(0, altura / 2, 0, comprimento + ESPESSURA, altura, ESPESSURA);   /* + ESPESSURA fecha a quina */
        if (ladoJanelas != 0) {
            rotacionaY(ladoJanelas > 0 ? 0 : 180);
            for (y = 1.0f; y + JANELA_A < altura - PLATIBANDA; y += ANDAR)
                for (i = 0; i < n; i++)          /* espaçadas igualmente ao longo da parede */
                    janela((i + 0.5f - n / 2.0f) * comprimento / n, y, ESPESSURA / 2, 1);
        }
    glPopMatrix();
}


/* ---------- blocos ---------- */

/* Bloco retangular: 4 paredes, telhado de duas águas por dentro da mureta */
static void bloco(float x, float z, float angulo,
                  float comprimento, float largura, float altura) {
    float meioC = comprimento / 2 - ESPESSURA / 2;
    float meioL = largura / 2 - ESPESSURA / 2;

    glPushMatrix();
        translada(x, 0, z);
        rotacionaY(angulo);

        BRANCO;
        caixa(0,  altura / 2,  meioL, comprimento, altura, ESPESSURA);
        caixa(0,  altura / 2, -meioL, comprimento, altura, ESPESSURA);
        caixa( meioC, altura / 2, 0, ESPESSURA, altura, largura);
        caixa(-meioC, altura / 2, 0, ESPESSURA, altura, largura);

        glPushMatrix();
            translada(0, altura - PLATIBANDA, 0);
            escala(comprimento - 2 * ESPESSURA, TELHADO_ALT, largura - 2 * ESPESSURA);
            TELHA;
            telhado();
        glPopMatrix();
    glPopMatrix();
}

/* Bloco com uma faixa mais baixa de 'largBaixa' metros no lado +Z (ala direita) */
static void alaDegrau(float x, float z, float angulo, float comprimento, float largura,
                      float largBaixa, float altura, float altBaixa) {
    float c = comprimento / 2 - ESPESSURA / 2;
    float tras = -largura / 2;
    float frente = largura / 2;
    float degrau = frente - largBaixa;       /* onde muda a altura */
    int lado;

    glPushMatrix();
        translada(x, 0, z);
        rotacionaY(angulo);

        BRANCO;
        caixa(0, altura / 2,   tras + ESPESSURA / 2,   comprimento, altura,   ESPESSURA);
        caixa(0, altura / 2,   degrau,                 comprimento, altura,   ESPESSURA);
        caixa(0, altBaixa / 2, frente - ESPESSURA / 2, comprimento, altBaixa, ESPESSURA);
        for (lado = -1; lado <= 1; lado += 2) {
            caixa(lado * c, altura / 2,   (tras + degrau) / 2,   ESPESSURA, altura,   degrau - tras);
            caixa(lado * c, altBaixa / 2, (degrau + frente) / 2, ESPESSURA, altBaixa, largBaixa);
        }

        TELHA;
        glPushMatrix();                      /* parte alta */
            translada(0, altura - PLATIBANDA, (tras + degrau) / 2);
            escala(comprimento - 2 * ESPESSURA, TELHADO_ALT, degrau - tras - 2 * ESPESSURA);
            telhado();
        glPopMatrix();
        glPushMatrix();                      /* faixa baixa */
            translada(0, altBaixa - PLATIBANDA, (degrau + frente) / 2);
            escala(comprimento - 2 * ESPESSURA, TELHADO_ALT, largBaixa - ESPESSURA);
            telhado();
        glPopMatrix();
    glPopMatrix();
}


/* ---------- barra do fundo ---------- */

/*
 * Pontos de uma linha da barra que fica a 'o' metros da parede de trás
 * (o = 0 parede de trás, 6 desnível, 12 parede da frente). Lado esquerdo;
 * o direito é o mesmo com x trocado de sinal.
 *   (xj, zj) quina entre o trecho reto e o dobrado (fica na bissetriz, por isso o tan(ANG/2))
 *   (xp, zp) ponta do trecho dobrado
 */
static void contornoFundo(float o, float *xj, float *zj, float *xp, float *zp) {
    float a = FUNDO_ANG * 3.14159265f / 180;
    *xj = -(FUNDO_MEIO + o * tan(a / 2));
    *zj = FUNDO_Z + o;
    *xp = -(FUNDO_MEIO + FUNDO_LADO * cos(a) + o * sin(a));
    *zp = FUNDO_Z - FUNDO_LADO * sin(a) + o * cos(a);
}

/* Parede ao longo da barra inteira: trecho esquerdo, reto e direito */
static void paredeFundo(float o, float altura, int ladoJanelas) {
    float xj, zj, xp, zp;
    contornoFundo(o, &xj, &zj, &xp, &zp);
    parede(xp, zp, xj, zj, altura, ladoJanelas, 4);      /* trecho dobrado: 4 janelas */
    parede(xj, zj, -xj, zj, altura, 0, 0);               /* trecho reto */
    parede(xj, zj, -CENTRAL_LARG / 2, zj, altura, ladoJanelas, 5);   /* 5 janelas de cada lado do bloco central */
    parede(CENTRAL_LARG / 2, zj, -xj, zj, altura, ladoJanelas, 5);
    parede(-xj, zj, -xp, zp, altura, ladoJanelas, 4);
}

/* Faixa de telhado entre as linhas oA (altura yA) e oB (altura yB), nos 3 trechos */
static void faixaFundo(float oA, float yA, float oB, float yB) {
    float xjA, zjA, xpA, zpA, xjB, zjB, xpB, zpB;
    contornoFundo(oA, &xjA, &zjA, &xpA, &zpA);
    contornoFundo(oB, &xjB, &zjB, &xpB, &zpB);

    glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glVertex3f(xpA, yA, zpA);   glVertex3f(xjA, yA, zjA);
        glVertex3f(xjB, yB, zjB);   glVertex3f(xpB, yB, zpB);

        glVertex3f(xjA, yA, zjA);   glVertex3f(-xjA, yA, zjA);
        glVertex3f(-xjB, yB, zjB);  glVertex3f(xjB, yB, zjB);

        glVertex3f(-xjA, yA, zjA);  glVertex3f(-xpA, yA, zpA);
        glVertex3f(-xpB, yB, zpB);  glVertex3f(-xjB, yB, zjB);
    glEnd();
}

/* Duas águas entre as linhas o1 e o2, cumeeira no meio */
static void telhadoFundo(float o1, float o2, float laje) {
    float meio = (o1 + o2) / 2;
    TELHA;
    faixaFundo(o1, laje, meio, laje + TELHADO_ALT);
    faixaFundo(meio, laje + TELHADO_ALT, o2, laje);
}

/*
 * Anexo com varandas na frente. X = comprimento, Z = largura, +Z = frente.
 * Laterais de tijolo, parede escura no fundo das varandas e
 * faixas brancas (piso, laje do meio e mureta) na frente.
 */
static void anexo(float x, float z, float angulo) {
    float c = ANEXO_COMP / 2, l = ANEXO_LARG / 2, h = ANEXO_ALT;
    float recuo = l - VARANDA;
    float zFaixa = l - VARANDA / 2;
    float faixa = ANEXO_COMP - 2.2f * ESPESSURA;    /* um pouco menor pra não encostar no tijolo */

    glPushMatrix();
        translada(x, 0, z);
        rotacionaY(angulo);

        glColor3f(0.7f, 0.38f, 0.25f);
        caixa( c - ESPESSURA / 2, h / 2, 0, ESPESSURA, h, ANEXO_LARG);
        caixa(-c + ESPESSURA / 2, h / 2, 0, ESPESSURA, h, ANEXO_LARG);

        BRANCO;
        caixa(0, h / 2, -l + ESPESSURA / 2, ANEXO_COMP, h, ESPESSURA);

        glColor3f(0.3f, 0.27f, 0.25f);
        caixa(0, h / 2, recuo, ANEXO_COMP - 2 * ESPESSURA, h, ESPESSURA);

        BRANCO;
        caixa(0, 0.25f,              zFaixa, faixa, 0.5f,       VARANDA);
        caixa(0, h / 2,              zFaixa, faixa, 0.5f,       VARANDA);
        caixa(0, h - PLATIBANDA / 2, zFaixa, faixa, PLATIBANDA, VARANDA);

        TELHA;                               /* laje plana */
        caixa(0, h - PLATIBANDA, (-l + recuo) / 2, ANEXO_COMP - 2 * ESPESSURA, 0.1f, recuo + l - ESPESSURA);
    glPopMatrix();
}

static void desenhaFundo(void) {
    float meio = FUNDO_LARG / 2;
    float xj, zj, xp0, zp0, xpm, zpm, xpf, zpf, xa, za;
    float vao, xt;
    float a = FUNDO_ANG * 3.14159265f / 180;

    paredeFundo(0,    FUNDO_ALT, -1);       /* trás */
    paredeFundo(meio, FUNDO_ALT,  0);       /* desnível */

    /* tijolo dos dois lados do bloco central: pega as 3 janelas mais perto dele
       (vao = espaço de cada uma das 5 janelas do trecho reto) */
    contornoFundo(FUNDO_LARG, &xj, &zj, &xp0, &zp0);
    vao = (-xj - CENTRAL_LARG / 2) / 5;
    xt = CENTRAL_LARG / 2 + 1.5f * vao;
    glPushMatrix();
        translada(-xt, 0, FUNDO_Z + FUNDO_LARG);
        fachada(0, ESPESSURA / 2, 3 * vao, FUNDO_FRONT_ALT, 1, SEM);
    glPopMatrix();
    glPushMatrix();
        translada(xt, 0, FUNDO_Z + FUNDO_LARG);
        fachada(0, ESPESSURA / 2, 3 * vao, FUNDO_FRONT_ALT, 1, SEM);
    glPopMatrix();
    paredeFundo(FUNDO_LARG, FUNDO_FRONT_ALT, 1);   /* frente */

    /* paredes das pontas */
    contornoFundo(0,          &xj, &zj, &xp0, &zp0);
    contornoFundo(meio,       &xj, &zj, &xpm, &zpm);
    contornoFundo(FUNDO_LARG, &xj, &zj, &xpf, &zpf);
    parede(xp0,  zp0, xpm,  zpm, FUNDO_ALT,       0, 0);
    parede(xpm,  zpm, xpf,  zpf, FUNDO_FRONT_ALT, 0, 0);
    parede(-xp0, zp0, -xpm, zpm, FUNDO_ALT,       0, 0);
    parede(-xpm, zpm, -xpf, zpf, FUNDO_FRONT_ALT, 0, 0);

    telhadoFundo(0,    meio,       FUNDO_ALT - PLATIBANDA);
    telhadoFundo(meio, FUNDO_LARG, FUNDO_FRONT_ALT - PLATIBANDA);

    /* anexos: centro no meio dos 8 m encostados na ponta, depois meio comprimento pra fora */
    contornoFundo(FUNDO_LARG - ANEXO_RECUO - ANEXO_LARG / 2, &xj, &zj, &xa, &za);
    xa -= (ANEXO_COMP / 2 + ESPESSURA / 2) * cos(a);
    za -= (ANEXO_COMP / 2 + ESPESSURA / 2) * sin(a);
    anexo( xa, za, -FUNDO_ANG);
    anexo(-xa, za,  FUNDO_ANG);
}


/* ---------- entorno ---------- */

/* Árvore: tronco (prisma de 8 lados) + copa (pirâmide) */
static void arvore(float x, float z, float altura) {
    glPushMatrix();
        translada(x, 0, z);
        glPushMatrix();
            escala(0.3f, altura * 0.4f, 0.3f);
            glColor3f(0.4f, 0.28f, 0.18f);
            prisma(8);
        glPopMatrix();
        glPushMatrix();
            translada(0, altura * 0.3f, 0);
            escala(altura * 0.35f, altura * 0.7f, altura * 0.35f);
            glColor3f(0.2f, 0.4f, 0.18f);
            piramide(8);
        glPopMatrix();
    glPopMatrix();
}

static void entorno(void) {
    float arvores[][3] = {                      /* x, z, altura */
        { 30,  2, 7}, { 36, 10, 8}, { 27, 18, 6}, { 42, -6, 9}, { 46,  6, 7},
        { 38, 24, 8}, { 22, 28, 7}, {-42, 10, 6}, {-44, -8, 8}, { 48, -24, 9}
    };
    int i;

    glColor3f(0.6f, 0.6f, 0.58f);
    caixa(-18, 0.01f, 22.5f, 26, 0.02f, 8);     /* estacionamento */
    caixa(-1, 0.01f, 13, 2.5f, 0.02f, 18);      /* caminho até a entrada */

    glPushMatrix();                             /* rotatória: prisma de 24 lados bem achatado */
        translada(-15, 0.02f, -15);
        escala(4, 0.02f, 4);
        prisma(24);
    glPopMatrix();

    for (i = 0; i < 10; i++)
        arvore(arvores[i][0], arvores[i][1], arvores[i][2]);

}


/* ---------- prédio inteiro ---------- */

void desenhaPredio(void) {
    int i, j;

    /* átrio: prisma de 6 lados */
    BRANCO;
    glPushMatrix();
        translada(ATRIO_X, 0, ATRIO_Z);
        escala(ATRIO_RAIO, ATRIO_ALT, ATRIO_RAIO);
        prisma(6);
    glPopMatrix();

    /* claraboia: pirâmide de vidro */
    glPushMatrix();
        translada(ATRIO_X, ATRIO_ALT, ATRIO_Z);
        glPushMatrix();
            escala(ATRIO_RAIO, CLARABOIA_ALT, ATRIO_RAIO);
            glColor3f(0.55f, 0.75f, 0.85f);
            piramide(6);
        glPopMatrix();

        /* estrutura metálica da claraboia com DDA: base do hexágono e as 6 arestas até a ponta.
           Sobe 2 cm para os pontos não sumirem dentro do vidro. */
        glColor3f(0.3f, 0.3f, 0.3f);
        glPointSize(3);
        for (i = 0; i < 6; i++) {
            float a1 = i * 3.14159265f / 3, a2 = (i + 1) * 3.14159265f / 3;
            float x1 = ATRIO_RAIO * cos(a1), z1 = ATRIO_RAIO * sin(a1);
            float x2 = ATRIO_RAIO * cos(a2), z2 = ATRIO_RAIO * sin(a2);
            linhaDDA(x1, 0.02f, z1, x2, 0.02f, z2);
            linhaDDA(x1, 0.02f, z1, 0, CLARABOIA_ALT + 0.02f, 0);
        }
    glPopMatrix();

    /* miolo: prisma de 3 lados até a laje; só a face da frente aparece (entre as alas) */
    BRANCO;
    glPushMatrix();
        translada(ATRIO_X, 0, ATRIO_Z);
        rotacionaY(90);
        glPushMatrix();
            escala(TRI_RAIO, TRI_ALT - PLATIBANDA, TRI_RAIO);
            prisma(3);
        glPopMatrix();

        /* mureta e janelas no pedaço de 2 m entre as alas (a face fica a TRI_RAIO/2 do centro) */
        rotacionaY(-90);
        caixa(0, TRI_ALT - PLATIBANDA / 2, TRI_RAIO / 2 - ESPESSURA / 2, 2, PLATIBANDA, ESPESSURA);
        janela(0, 1.0f,         TRI_RAIO / 2, 2);
        janela(0, 1.0f + ANDAR, TRI_RAIO / 2, 2);

        /* tijolo nas duas faces de trás do miolo (aparecem entre o bloco central e as alas) */
        fachada( 120, TRI_RAIO / 2, TRI_RAIO * 1.732f, TRI_ALT - PLATIBANDA, 1, SEM);
        fachada(-120, TRI_RAIO / 2, TRI_RAIO * 1.732f, TRI_ALT - PLATIBANDA, 1, SEM);
    glPopMatrix();

    /* painéis de telhado em volta da claraboia: tronco de 6 lados */
    glPushMatrix();
        translada(ATRIO_X, TRI_ALT - PLATIBANDA, ATRIO_Z);
        escala(SAIA_RAIO, ATRIO_ALT - (TRI_ALT - PLATIBANDA), SAIA_RAIO);
        TELHA;
        tronco(6, ATRIO_RAIO / SAIA_RAIO);
    glPopMatrix();

    bloco(CENTRAL_X, CENTRAL_Z, CENTRAL_ANG, CENTRAL_COMP, CENTRAL_LARG, CENTRAL_ALT);
    bloco(ESQ_X,     ESQ_Z,     ESQ_ANG,     ESQ_COMP,     ESQ_LARG,     ESQ_ALT);
    alaDegrau(DIR_X, DIR_Z, DIR_ANG, DIR_COMP, DIR_LARG, DIR_FRONT_LARG, DIR_ALT, DIR_FRONT_ALT);

    /* fachadas: mesmo translada/rotaciona de cada bloco */
    glPushMatrix();                                         /* bloco central */
        translada(CENTRAL_X, 0, CENTRAL_Z);
        rotacionaY(CENTRAL_ANG);
        fachada(0,   CENTRAL_LARG / 2, CENTRAL_COMP, CENTRAL_ALT, 0, SIMPLES);
        fachada(180, CENTRAL_LARG / 2, CENTRAL_COMP, CENTRAL_ALT, 0, SIMPLES);
    glPopMatrix();

    glPushMatrix();                                         /* ala esquerda */
        translada(ESQ_X, 0, ESQ_Z);
        rotacionaY(ESQ_ANG);
        fachada(-90, ESQ_COMP / 2, ESQ_LARG, ESQ_ALT, 1, FITA);      /* ponta */
        fachada(180, ESQ_LARG / 2, ESQ_COMP, ESQ_ALT, 1, SIMPLES);   /* trás */
        fachada(0,   ESQ_LARG / 2, ESQ_COMP, ESQ_ALT, 1, FITA);      /* lado da entrada */
    glPopMatrix();

    glPushMatrix();                                         /* ala direita */
        translada(DIR_X, 0, DIR_Z);
        rotacionaY(DIR_ANG);
        fachada(90, DIR_COMP / 2, DIR_LARG, DIR_FRONT_ALT, 1, SEM);   /* ponta, tijolo */
        glPushMatrix();                                     /* ar-condicionados na ponta */
            rotacionaY(90);
            glColor3f(0.95f, 0.95f, 0.95f);
            for (j = 0; j < 2; j++)
                for (i = -2; i <= 1; i++)
                    caixa(i * 2.5f + 1.25f, 2.8f + j * ANDAR, DIR_COMP / 2 + 0.2f, 0.8f, 0.6f, 0.4f);
        glPopMatrix();
        fachada(0, DIR_LARG / 2, DIR_COMP, DIR_FRONT_ALT, 0, PILARES);   /* lado da entrada */
        porta(-(4 * VIDRO_L + 0.6f) - 2 * VIDRO_L, DIR_LARG / 2);       /* no 1º vidro do vão perto do átrio */
        fachada(180, DIR_LARG / 2, DIR_COMP, DIR_ALT, 1, SIMPLES);       /* trás, tijolo */
    glPopMatrix();

    desenhaFundo();
    entorno();
}
