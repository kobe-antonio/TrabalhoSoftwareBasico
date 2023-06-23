#include "quadtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else

#include <GL/gl.h>     /* OpenGL functions */

#endif

unsigned int first = 1;
char desenhaBorda = 1;

QuadNode *newNode(int x, int y, int width, int height) {
    QuadNode *n = malloc(sizeof(QuadNode));
    n->x = x;
    n->y = y;
    n->width = width;
    n->height = height;
    n->NW = n->NE = n->SW = n->SE = NULL;
    n->color[0] = n->color[1] = n->color[2] = 0;
    n->id = first++;
    return n;
}

QuadNode *geraQuadtree(Img *pic, float minError) {
    // Converte o vetor RGBPixel para uma MATRIZ que pode acessada por pixels[linha][coluna]
    RGBPixel (*pixels)[pic->width] = (RGBPixel(*)[pic->height]) pic->img;

    // Veja como acessar os primeiros 10 pixels da imagem, por exemplo:
    int i;
    for (i = 0; i < 10; i++)
        printf("%02X %02X %02X\n", pixels[0][i].r, pixels[1][i].g, pixels[2][i].b);

    int width = pic->width;
    int height = pic->height;

    //////////////////////////////////////////////////////////////////////////
    // Implemente aqui o algoritmo que gera a quadtree, retornando o nodo raiz
    //////////////////////////////////////////////////////////////////////////

    // Gerar a raiz da árvore
    QuadNode *raiz = newNode(0, 0, width, height);
    raiz = expandNode(raiz, pic, minError);

// COMENTE a linha abaixo quando seu algoritmo ja estiver funcionando
// Caso contrario, ele ira gerar uma arvore de teste com 3 nodos

//#define DEMO
#ifdef DEMO

    /************************************************************/
    /* Teste: criando uma raiz e dois nodos a mais              */
    /************************************************************/

    QuadNode* raiz = newNode(0,0,width,height);
    raiz->status = PARCIAL;
    raiz->color[0] = 0;
    raiz->color[1] = 0;
    raiz->color[2] = 255;

    int meiaLargura = width/2;
    int meiaAltura = height/2;

    QuadNode* nw = newNode(meiaLargura, 0, meiaLargura, meiaAltura);
    nw->status = PARCIAL;
    nw->color[0] = 0;
    nw->color[1] = 0;
    nw->color[2] = 255;

    // Aponta da raiz para o nodo nw
    raiz->NW = nw;

    QuadNode* nw2 = newNode(meiaLargura+meiaLargura/2, 0, meiaLargura/2, meiaAltura/2);
    nw2->status = CHEIO;
    nw2->color[0] = 255;
    nw2->color[1] = 0;
    nw2->color[2] = 0;

    // Aponta do nodo nw para o nodo nw2
    nw->NW = nw2;

#endif
    // Finalmente, retorna a raiz da árvore
    return raiz;
}

QuadNode *expandNode(QuadNode *root, Img *pic, float minError) {
    //printf("(%d, %d)\n", (int)root->x, (int)root->y);
    //printf("W: %d, H: %d\n", (int)root->width, (int)root->height);
    // Converte o vetor RGBPixel para uma MATRIZ
    RGBPixel (*pixels)[pic->width] = (RGBPixel(*)[pic->height]) pic->img;

    // Quantidade de pixels na região
    int N = root->width * root->height;

    if (N == 0)
        return root;

    // Variáveis para calcular a cor média
    unsigned int r = 0;
    unsigned int g = 0;
    unsigned int b = 0;

    // Calcular a cor média da região
    for (int i = (int) root->y; i < (int) (root->y + root->height); i++) {
        for (int j = (int) root->x; j < (int) (root->x + root->width); j++) {
            r += pixels[i][j].r;
            g += pixels[i][j].g;
            b += pixels[i][j].b;
        }
    }
    r /= N;
    g /= N;
    b /= N;
    root->color[0] = r;
    root->color[1] = g;
    root->color[2] = b;

    // Calcular o histograma da região em tons de cinza
    float intensity;
    unsigned int hist[256] = {0};
    for (int i = (int) root->y; i < (int) (root->y + root->height); i++) {
        for (int j = (int) root->x; j < (int) (root->x + root->width); j++) {
            r = pixels[i][j].r;
            g = pixels[i][j].g;
            b = pixels[i][j].b;
            intensity = 0.3f * r + 0.59f * g + 0.11f * b;
            hist[(int) intensity] += 1;
        }
    }

    float mean_intensity = 0;
    for (int i = 0; i < 256; i++)
        mean_intensity += i*hist[i];
    mean_intensity /= N;

    // Calcular o nível de erro da região
    float error = 0;
    for (int i = (int)root->y; i < (int)(root->y + root->height); i++) {
        for (int j = (int)root->x; j < (int)(root->x + root->width); j++) {
            r = pixels[i][j].r;
            g = pixels[i][j].g;
            b = pixels[i][j].b;
            intensity = 0.3f*r + 0.59f*g + 0.11f*b;
            error += (intensity - mean_intensity) * (intensity - mean_intensity);
        }
    }
    error = sqrt(error/N);

    // Verificar se o nível de erro é menor do que o erro mínimo especificado
    if (error < minError) {
        root->status = CHEIO;
    }

        // Subdividir a região em quatro e repetir o algoritmo
    else {
        int x = root->x;
        int y = root->y;
        int width = (int)root->width;
        int height = (int)root->height;
        QuadNode* NW = newNode(x, y, width/2, height/2);
        QuadNode* NE = newNode(x+width/2, y, width/2, height/2);
        QuadNode* SW = newNode(x, y+height/2, width/2, height/2);
        QuadNode* SE = newNode(x+width/2, y+height/2, width/2, height/2);
        root->NW = expandNode(NW, pic, minError);
        root->NE = expandNode(NE, pic, minError);
        root->SW = expandNode(SW, pic, minError);
        root->SE = expandNode(SE, pic, minError);
        root->status = PARCIAL;
    }
}

// Limpa a memória ocupada pela árvore
void clearTree(QuadNode *n) {
    if (n == NULL) return; // se n for nulo a arvore nao foi formada, logo nao precisamos deletar ela
    if (n->status == PARCIAL) {
        clearTree(n->NE);
        clearTree(n->NW);
        clearTree(n->SE);
        clearTree(n->SW);
    }
    //printf("Liberando... %d - %.2f %.2f %.2f %.2f\n", n->status, n->x, n->y, n->width, n->height);
    free(n); // por ultimo liberamos a raiz
}

// Ativa/desativa o desenho das bordas de cada região
void toggleBorder() {
    desenhaBorda = !desenhaBorda;
    printf("Desenhando borda: %s\n", desenhaBorda ? "SIM" : "NÃO");
}

// Desenha toda a quadtree
void drawTree(QuadNode *raiz) {
    if (raiz != NULL)
        drawNode(raiz);
}

// Grava a árvore no formato do Graphviz
void writeTree(QuadNode *raiz) {
    FILE *fp = fopen("quad.dot", "w");
    fprintf(fp, "digraph quadtree {\n");
    if (raiz != NULL)
        writeNode(fp, raiz);
    fprintf(fp, "}\n");
    fclose(fp);
    printf("\nFim!\n");
}

void writeNode(FILE *fp, QuadNode *n) {
    if (n == NULL) return;

    if (n->NE != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->NE->id);
    if (n->NW != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->NW->id);
    if (n->SE != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->SE->id);
    if (n->SW != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->SW->id);
    writeNode(fp, n->NE);
    writeNode(fp, n->NW);
    writeNode(fp, n->SE);
    writeNode(fp, n->SW);
}

// Desenha todos os nodos da quadtree, recursivamente
void drawNode(QuadNode *n) {
    if (n == NULL) return;

    glLineWidth(0.1);

    if (n->status == CHEIO) {
        glBegin(GL_QUADS);
        glColor3ubv(n->color);
        glVertex2f(n->x, n->y);
        glVertex2f(n->x + n->width - 1, n->y);
        glVertex2f(n->x + n->width - 1, n->y + n->height - 1);
        glVertex2f(n->x, n->y + n->height - 1);
        glEnd();
    } else if (n->status == PARCIAL) {
        if (desenhaBorda) {
            glBegin(GL_LINE_LOOP);
            glColor3ubv(n->color);
            glVertex2f(n->x, n->y);
            glVertex2f(n->x + n->width - 1, n->y);
            glVertex2f(n->x + n->width - 1, n->y + n->height - 1);
            glVertex2f(n->x, n->y + n->height - 1);
            glEnd();
        }
        drawNode(n->NE);
        drawNode(n->NW);
        drawNode(n->SE);
        drawNode(n->SW);
    }
    // Nodos vazios não precisam ser desenhados... nem armazenados!
}
