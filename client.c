// Conecta ao servidor e responde (segundo jogador)

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <ctype.h>

#define TAM 10

char meu_tabuleiro[TAM][TAM];
char tabuleiro_inimigo[TAM][TAM];

void tela_inicial();
void inicializar_tabuleiros();
void mostrar_tabuleiros();
void posicionar_barcos(); 
void mostrar_tabuleiro_posicionando();
void realizar_ataque(char tabuleiro_inimigo[TAM][TAM], char tabuleiro_real_inimigo[TAM][TAM]);

int main() {
    setlocale(LC_ALL, "Portuguese");
    printf("Cliente iniciado\n");

    tela_inicial();
    inicializar_tabuleiros();
    posicionar_barcos();
    mostrar_tabuleiros();
    
    return 0;
}

void tela_inicial() {
    printf("\nBEM VINDO AO BATALHA NAVAL\n--------------------------\nBarcos disponiveis: \n");
    printf("1. Porta-avioes (P)  (5 blocos)   #####\n");
    printf("2. Encouracado (E)   (4 blocos)   ####\n");
    printf("3. Submarino (S)     (3 blocos)   ###\n");
    printf("4. Destroyer (D)     (2 blocos)   ##\n");

    printf("\nRegras do jogo:\n");
    printf("- Cada jogador tem um tabuleiro proprio e outro com os acertos/erros do tabuleiro inimigo\n");
    printf("- Posicione seus navios escolhendo a posicao inicial (linha/coluna) e a orientacao (H ou V)\n");
    printf("- Escolha uma posicao por vez para descobrir se o adversario tem ou nao um pedaco de navio naquela casa\n");
    printf("- Acertos no tabulheiro do inimigo sao marcados com um 'X', e erros sao marcados com um 'O'\n");
    printf("- Vence quem afundar todos os navios do adversario primeiro\n");

    printf("\nPressione Enter para continuar\n");
    getchar(); // espera o jogador apertar enter para continuar
    system("cls"); 
}

void inicializar_tabuleiros(){
    // inicializando os tabuleiros com ~ representando a agua
    for(int i=0; i<TAM; i++) {
        for(int j=0; j<TAM; j++) {
            meu_tabuleiro[i][j] = '~';
            tabuleiro_inimigo[i][j] = '~';
        }
    }
}

void mostrar_tabuleiros() {

    printf("\nMeu tabuleiro:\n   ");
    for(int j=0; j<TAM; j++) {
        printf("%d ", j); // imprimindo o numero das colunas
    }
    printf("\n");

    for(int i=0; i<TAM; i++) {
        printf("%d  ", i); // imprimindo o numero das linhas

        for(int j=0; j<TAM; j++) {
            printf("%c ", meu_tabuleiro[i][j]); // imprimindo cada posicao do tabuleiro
        }
        printf("\n");
    }

    printf("\nTabuleiro do Inimigo:\n   ");
    for(int j=0; j<TAM; j++) {
        printf("%d ", j); // imprimindo o numero das colunas
    }
    printf("\n");

    for(int i=0; i<TAM; i++) {
        printf("%d  ", i); // imprimindo o numero das linhas

        for(int j=0; j<TAM; j++) {
            printf("%c ", meu_tabuleiro[i][j]); // imprimindo cada posicao do tabuleiro do inimigo
        }
        printf("\n");
    }

}

void mostrar_tabuleiro_posicionando() {
    // função para imprimir apenas o meu tabuleiro enquanto usuario ainda posiciona os barcos
    printf("\n   ");
    for(int j=0; j<TAM; j++) {
        printf("%d ", j); // imprimindo o numero das colunas
    }
    printf("\n");

    for(int i=0; i<TAM; i++) {
        printf("%d  ", i); // imprimindo o numero das linhas

        for(int j=0; j<TAM; j++) {
            printf("%c ", meu_tabuleiro[i][j]); // imprimindo cada posicao do tabuleiro
        }
        printf("\n");
    }
}

void posicionar_barcos() {
    struct { // definindo os tipos de barcos
        char *nome;
        int tamanho;
        char simbolo;
    } barcos[4] = {
        {"Porta-avioes", 5, 'P'},
        {"Encouracado", 4, 'E'},
        {"Submarino", 3, 'S'},
        {"Destroyer", 2, 'D'}
    };
    
    mostrar_tabuleiro_posicionando(); // mostrando o tabuleiro para ajudar o usuario a posicionar
    for(int barco=0; barco<4; barco++) { // loop para posicionar os 4 barcos
        int linha, col;
        char orientacao;
        int valido = 0;

        while(!valido) { // enquanto a posiçao escolhida for invalida
            printf("\nPosicione o navio %s (tamanho %d)\n", barcos[barco].nome, barcos[barco].tamanho);
            printf("Linha (0-%d): ", TAM-1);
            scanf("%d", &linha);
            printf("Coluna (0-%d): ", TAM-1);
            scanf("%d", &col);
            printf("Orientacao (H/V): ");
            scanf(" %c", &orientacao);
            orientacao = toupper(orientacao);

            // verificando limites do tabuleiro
            if((orientacao == 'H' && col + barcos[barco].tamanho > TAM) || 
                (orientacao == 'V' && linha + barcos[barco].tamanho > TAM)) {
                printf("\n%s nao cabe na posicao escolhida. Tente novamente\n", barcos[barco].nome);
                continue;
            }
            
            // verificando se nao ha barcos se soprepondo
            int sobreposicao = 0;
            for(int i=0; i<barcos[barco].tamanho; i++) {
                int x = linha + (orientacao == 'V' ? i : 0); // se for vertical, linha + i. se nao for, linha + 0
                int y = col + (orientacao == 'H' ? i : 0); // se for horizontal, linha + i. se nao for, linha + 0
                if(meu_tabuleiro[x][y] != '~') sobreposicao = 1; // se nao tiver agua na posicao escolhida, ela esta invalida
            }
            if(sobreposicao) {
                printf("Ja ha um barco nesta posicao. Tente novamente\n");
                continue;
            }

            // posicionando o barco
            for(int i=0; i<barcos[barco].tamanho; i++) {
                int x = linha + (orientacao == 'V' ? i : 0); // se for vertical, linha + i. se nao for, linha + 0
                int y = col + (orientacao == 'H' ? i : 0); // se for horizontal, linha + i. se nao for, linha + 0
                meu_tabuleiro[x][y] = barcos[barco].simbolo;
            }
            mostrar_tabuleiro_posicionando(); // mostrando como ficou o tabuleiro depois de posicionar cada barco

            valido = 1;
        }   
    }
    printf("\nTodos os navios foram posicionados!");
    printf("\nPressione Enter para iniciar o jogo");
    while (getchar() != '\n'); // limpa o buffer
    getchar(); // espera o Enter
    system("cls");
}

void realizar_ataque(char tabuleiro_inimigo[TAM][TAM], char tabuleiro_real_inimigo[TAM][TAM]) {
    int linha, col;

    printf("\nEscolha onde atirar:\n");
    printf("Linha (0-%d): ", TAM-1);
    scanf("%d", &linha);
    printf("Coluna (0-%d): ", TAM-1);
    scanf("%d", &col);

    if(tabuleiro_real_inimigo[linha][col] != '~') {
        printf("\n💥 Acertou um pedaço de navio do inimigo!\n");
        // marcando dano no tabuleiro de registro e no tabuleiro real do inimigo
        tabuleiro_inimigo[linha][col] = 'X';
        tabuleiro_real_inimigo[linha][col] = 'X';
    } else {
        printf("\n🌊 Apenas agua! Nenhum navio atingido!\n");
        // marcando erro no tabuleiro de registro
        tabuleiro_inimigo[linha][col] = 'O';
    }
}