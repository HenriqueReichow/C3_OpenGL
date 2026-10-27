#ifndef FORMAS_H
#define FORMAS_H

void cubo(void);                        /* cubo de lado 1 centrado na origem */
void prisma(int lados);                 /* prisma de raio 1, de y = 0 a y = 1 */
void piramide(int lados);               /* pirâmide de raio 1, base em y = 0 e ponta em y = 1 */
void tronco(int lados, float raioTopo); /* pirâmide sem a ponta (só as faces inclinadas) */
void telhado(void);                     /* telhado de duas águas 1 x 1, cumeeira em y = 1 */
void chao(void);                        /* gramado em y = 0 */
void linhaDDA(float x0, float y0, float z0, float x1, float y1, float z1);   /* linha pelo DDA */

#endif
