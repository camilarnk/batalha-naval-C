// Escuta conexões e controla o jogo (primeiro jogador), executado antes do client
// compile: gcc server.c -o server.exe -lws2_32

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <ctype.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define TAM 10
#define PORTA 5000 // porta de comunicaçao, pode ser alterada

char meu_tabuleiro[TAM][TAM];
char tabuleiro_inimigo[TAM][TAM];

void tela_inicial();
void inicializar_tabuleiros();
void mostrar_tabuleiros();
void posicionar_barcos(); 
void mostrar_tabuleiro_posicionando();
void realizar_ataque(SOCKET sock); // envia ataque via socket
void receber_ataque(SOCKET sock); // recebe ataque do cliente

int main() {
    setlocale(LC_ALL, "Portuguese");

    // ==== INICIALIZANDO SERVIDOR SOCKET ====
    WSADATA wsa;
    SOCKET servidor_socket, cliente_socket;
    struct sockaddr_in servidor, cliente;
    int tamanho_cliente = sizeof(cliente);

    printf("Inicializando servidor\n");
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Falha ao inicializar Winsock. Erro: %d\n", WSAGetLastError());
        return 1;
    }

    // criando socket
    servidor_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor_socket == INVALID_SOCKET) {
        printf("Erro ao criar socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    servidor.sin_family = AF_INET;
    servidor.sin_addr.s_addr = INADDR_ANY; // aceita conexões locais
    servidor.sin_port = htons(PORTA);

    // ligando o socket à porta
    if (bind(servidor_socket, (struct sockaddr *)&servidor, sizeof(servidor)) == SOCKET_ERROR) {
        printf("Erro ao associar porta: %d\n", WSAGetLastError());
        closesocket(servidor_socket);
        WSACleanup();
        return 1;
    }

    // esperando outra conexão
    listen(servidor_socket, 1);
    printf("Aguardando conexao do jogador 2...\n");

    cliente_socket = accept(servidor_socket, (struct sockaddr *)&cliente, &tamanho_cliente);
    if (cliente_socket == INVALID_SOCKET) {
        printf("Erro ao aceitar conexao: %d\n", WSAGetLastError());
        closesocket(servidor_socket);
        WSACleanup();
        return 1;
    }
    printf("Jogador 2 conectado!\n");
    // ==== FIM DA INICIALIZAÇÃO ====

    tela_inicial();
    inicializar_tabuleiros();
    posicionar_barcos();
    mostrar_tabuleiros();

    // loop principal do jogo
    while(1) {
        // jogador 1 (server) ataca primeiro
        realizar_ataque(cliente_socket); // envia ataque
        mostrar_tabuleiros();

        // jogador 1 recebe o ataque do jogador 2 (cliente)
        receber_ataque(cliente_socket);
        mostrar_tabuleiros();
    }

    closesocket(cliente_socket);
    closesocket(servidor_socket);
    WSACleanup();

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
    printf("- 'X' marca acertos e 'O' marca tiros na agua.\n");
    printf("- Vence quem afundar todos os navios do adversario primeiro\n");

    printf("\nPressione Enter para continuar\n");
    getchar(); // espera o jogador apertar enter para continuar
    //system("cls"); 
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
                printf("Ha outro navio nessa posicao. Tente novamente\n");
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
    printf("\nPressione Enter para continuar o jogo");
    while (getchar() != '\n'); // limpa o buffer
    getchar(); // espera o Enter
    system("cls");
}

void realizar_ataque(SOCKET sock) {
    int linha, col;
    char mensagem[32], resposta[16];

    printf("\nSua vez! Escolha onde atirar:\n");
    printf("Linha (0-%d): ", TAM-1);
    scanf("%d", &linha);
    printf("Coluna (0-%d): ", TAM-1);
    scanf("%d", &col);

    // envia ataque para o cliente "linha,coluna"
    sprintf(mensagem, "%d,%d", linha, col);
    send(sock, mensagem, strlen(mensagem), 0);

    // recebe resposta "HIT" se acertou ou "MISS" se errou
    recv(sock, resposta, sizeof(resposta), 0);

    if(strcmp(resposta, "HIT") == 0) {
        printf("\n💥 Acertou um navio inimigo!\n");
        tabuleiro_inimigo[linha][col] = 'X';
    } else {
        printf("\n🌊 Apenas agua! Nenhum navio atingido!\n");
        tabuleiro_inimigo[linha][col] = 'O';
    }
}

void receber_ataque(SOCKET sock) {
    int linha, col;
    char mensagem[32], resposta[16];

    printf("\nAguardando ataque do inimigo...\n");
    recv(sock, mensagem, sizeof(mensagem), 0);
    sscanf(mensagem, "%d,%d", &linha, &col); // atribuindo o ataque do inimigo para a linha e coluna

    // verificando se o ataque atingiu alguma posicao
    if(meu_tabuleiro[linha][col] != '~' && meu_tabuleiro[linha][col] != 'X') {
        printf("💣 O inimigo acertou em (%d,%d)!\n", linha, col);
        meu_tabuleiro[linha][col] = 'X';
        strcpy(resposta, "HIT");
    } else {
        printf("\n😌 O inimigo errou (%d,%d)!\n", linha, col);
        if(meu_tabuleiro[linha][col] == '~') {
            meu_tabuleiro[linha][col] = 'O';
        }
        strcpy(resposta, "MISS");
    }

    // envia o resultado de volta
    send(sock, resposta, strlen(resposta), 0);
}

