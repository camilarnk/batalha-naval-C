// Escuta conexões e controla o jogo (primeiro jogador), executado antes do client
// compile: gcc server.c -o server.exe -lws2_32

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <ctype.h>
#include <string.h>
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define TAM 10
#define TOTAL_BLOCOS 14 // quantidade de blocos do tabuleiro que possuem navios
#define PORTA 5000      // porta de comunicaçao, pode ser alterada

char meu_tabuleiro[TAM][TAM];
char tabuleiro_inimigo[TAM][TAM];

// 1 = jogador pode digitar (é a vez dele)
// 0 = jogador NÃO deve digitar (vez do inimigo)
int pode_digitar = 1;

void tela_inicial();
void inicializar_tabuleiros();
void mostrar_tabuleiros();
void posicionar_barcos();
void mostrar_tabuleiro_posicionando();
void realizar_ataque(SOCKET sock); // envia ataque via socket
void receber_ataque(SOCKET sock);  // recebe ataque do cliente
void bloquear_entrada_usuario();
void liberar_entrada_usuario();
void limpar_tela();
void aguardar_sua_vez();
int verificar_derrota();
int verificar_vitoria();
char verificar_navio_afundado(char simbolo); // verifica se um navio foi completamente afundado
char *obter_nome_navio(char simbolo);        // retorna o nome do navio baseado no simbolo

int ler_coordenada(const char *rotulo);      // <<< NOVA FUNÇÃO

void esperar_enter() {
    // espera o jogador apertar enter de forma segura, substitui getchar()
    char buf[32];
    fgets(buf, sizeof(buf), stdin); // lê até o enter (ou fim da linha)
}

int main() {
    SetConsoleOutputCP(CP_UTF8);

    // ==== INICIALIZAÇÃO DO WINSOCK ====
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Falha ao inicializar Winsock. Erro: %d\n", WSAGetLastError());
        return 1;
    }

    while (1) {
        int opcao;

        printf("\n=====================================\n");
        printf("=====     MENU BATALHA NAVAL     =====\n");
        printf("1 - Jogar\n");
        printf("2 - Sair\n");
        printf("Escolha: ");
        scanf("%d", &opcao);
        getchar(); // consome o ENTER

        if (opcao == 2) {
            printf("\nEncerrando o programa...\n");
            break;
        }

        if (opcao != 1) {
            printf("\nOpcao invalida.\n");
            continue;
        }

        // ==== CONFIGURAÇÃO DE CONEXÃO (somente servidor) ====
        WSADATA wsa2;
        SOCKET sock_principal, sock_conexao;
        struct sockaddr_in servidor, cliente;
        int tamanho_cliente = sizeof(cliente);

        sock_principal = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_principal == INVALID_SOCKET) {
            printf("Erro ao criar socket: %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        servidor.sin_family = AF_INET;
        servidor.sin_addr.s_addr = INADDR_ANY;
        servidor.sin_port = htons(PORTA);

        bind(sock_principal, (struct sockaddr *)&servidor, sizeof(servidor));
        listen(sock_principal, 1);
        printf("\nAguardando conexao do jogador 2...\n");

        sock_conexao = accept(sock_principal, (struct sockaddr *)&cliente, &tamanho_cliente);
        if (sock_conexao == INVALID_SOCKET) {
            printf("Erro ao aceitar conexao: %d\n", WSAGetLastError());
            closesocket(sock_principal);
            continue;
        }

        printf("Jogador 2 conectado!\n");

        // ==== LOOP DE UMA PARTIDA ====
        tela_inicial();
        inicializar_tabuleiros();
        posicionar_barcos();
        mostrar_tabuleiros();

        send(sock_conexao, "PRONTO", 6, 0);
        char buf_sync[16];
        int bytes = recv(sock_conexao, buf_sync, sizeof(buf_sync) - 1, 0);
        if (bytes > 0) {
            buf_sync[bytes] = '\0';
            if (strcmp(buf_sync, "PRONTO") == 0) {
                printf("\n✅ Ambos os jogadores estao prontos. Iniciando partida!\n");
            }
        }

        // começa como sendo a vez do servidor
        pode_digitar = 1;

        while (1) {
            printf("\n\n===========================================\n");
            printf("=====        SUA VEZ DE ATACAR         =====\n");
            printf("===========================================\n");
            realizar_ataque(sock_conexao);
            mostrar_tabuleiros();

            if (verificar_vitoria()) {
                printf("\n🎉 Voce venceu!\n");
                send(sock_conexao, "VITORIA", 7, 0);
                break;
            }

            printf("\n\n===========================================\n");
            printf("=====  AGUARDANDO ATAQUE DO INIMIGO   =====\n");
            printf("===========================================\n");
            receber_ataque(sock_conexao);
            mostrar_tabuleiros();

            if (verificar_derrota()) {
                printf("\n💥 Voce perdeu!\n");
                send(sock_conexao, "DERROTA", 7, 0);
                break;
            }

            char mensagem[32];
            fd_set fds;
            struct timeval timeout;
            FD_ZERO(&fds);
            FD_SET(sock_conexao, &fds);
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;

            int activity = select(0, &fds, NULL, NULL, &timeout);
            if (activity > 0) {
                int bytes2 = recv(sock_conexao, mensagem, sizeof(mensagem) - 1, 0);
                if (bytes2 > 0) {
                    mensagem[bytes2] = '\0';
                    if (strstr(mensagem, "DERROTA")) {
                        printf("\n🏆 O inimigo foi derrotado!\n");
                        break;
                    }
                }
            }
        }

        // ==== ENCERRAMENTO DA PARTIDA ====
        closesocket(sock_conexao);
        closesocket(sock_principal);
        printf("\nPartida encerrada. Voltando ao menu principal...\n");
    }

    WSACleanup();
    return 0;
}

void tela_inicial() {
    printf("\n\n====     BEM VINDO AO BATALHA NAVAL     ====\n");
    printf("============================================\n");
    printf("====         BARCOS DISPONIVEIS:        ====\n");

    printf("1. Porta-avioes (P)  (5 blocos)   #####\n");
    printf("2. Encouracado (E)   (4 blocos)   ####\n");
    printf("3. Submarino (S)     (3 blocos)   ###\n");
    printf("4. Destroyer (D)     (2 blocos)   ##\n");

    printf("\nRegras do jogo:\n");
    printf("- Cada jogador tem um tabuleiro proprio e outro com os acertos/erros do inimigo\n");
    printf("- Posicione seus navios escolhendo a posicao inicial (linha/coluna) e a orientacao (H ou V)\n");
    printf("- Escolha posicoes para atacar o adversario e tente afundar todos os navios\n");
    printf("- 'X' = acerto, 'O' = tiro na agua\n");
    printf("- Vence quem afundar todos os navios inimigos primeiro\n");

    printf("\nPressione Enter para continuar...\n");
    fflush(stdout);
    esperar_enter();
    limpar_tela();
}

void inicializar_tabuleiros() {
    // inicializando os tabuleiros com ~ representando a agua
    for (int i = 0; i < TAM; i++) {
        for (int j = 0; j < TAM; j++) {
            meu_tabuleiro[i][j] = '~';
            tabuleiro_inimigo[i][j] = '~';
        }
    }
}

void mostrar_tabuleiros() {

    printf("\nMeu tabuleiro:\n   ");
    for (int j = 0; j < TAM; j++) {
        printf("%d ", j); // imprimindo o numero das colunas
    }
    printf("\n");

    for (int i = 0; i < TAM; i++) {
        printf("%d  ", i); // imprimindo o numero das linhas

        for (int j = 0; j < TAM; j++) {
            printf("%c ", meu_tabuleiro[i][j]); // imprimindo cada posicao do tabuleiro
        }
        printf("\n");
    }

    printf("\nTabuleiro do Inimigo:\n   ");
    for (int j = 0; j < TAM; j++) {
        printf("%d ", j); // imprimindo o numero das colunas
    }
    printf("\n");

    for (int i = 0; i < TAM; i++) {
        printf("%d  ", i); // imprimindo o numero das linhas

        for (int j = 0; j < TAM; j++) {
            printf("%c ", tabuleiro_inimigo[i][j]); // imprimindo cada posicao do tabuleiro do inimigo
        }
        printf("\n");
    }
}

void mostrar_tabuleiro_posicionando() {
    // função para imprimir apenas o meu tabuleiro enquanto usuario ainda posiciona os barcos
    printf("\n   ");
    for (int j = 0; j < TAM; j++) {
        printf("%d ", j); // imprimindo o numero das colunas
    }
    printf("\n");

    for (int i = 0; i < TAM; i++) {
        printf("%d  ", i); // imprimindo o numero das linhas

        for (int j = 0; j < TAM; j++) {
            printf("%c ", meu_tabuleiro[i][j]); // imprimindo cada posicao do tabuleiro
        }
        printf("\n");
    }
}

// Lê coordenada como string, valida se é dígito único 0–9 e está dentro do tabuleiro
int ler_coordenada(const char *rotulo) {
    char temp[16];
    int valor;

    while (1) {
        printf("%s (0-%d): ", rotulo, TAM - 1);

        if (scanf("%15s", temp) != 1) {
            printf("Entrada invalida. Tente novamente.\n");
            continue;
        }

        // precisa ser exatamente 1 caractere e dígito
        if (strlen(temp) == 1 && isdigit((unsigned char)temp[0])) {
            valor = temp[0] - '0';

            if (valor >= 0 && valor < TAM) {
                return valor;
            }
        }

        printf("Digite apenas um numero de 0 a %d.\n", TAM - 1);
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
        {"Destroyer", 2, 'D'}};

    mostrar_tabuleiro_posicionando(); // mostrando o tabuleiro para ajudar o usuario a posicionar
    for (int barco = 0; barco < 4; barco++) { // loop para posicionar os 4 barcos
        int linha, col;
        char orientacao;
        int valido = 0;

        while (!valido) { // enquanto a posiçao escolhida for invalida
            printf("\nPosicione o navio %s (tamanho %d)\n", barcos[barco].nome, barcos[barco].tamanho);

            // >>> validação: só aceita 0–9
            linha = ler_coordenada("Linha");
            col   = ler_coordenada("Coluna");

            printf("Orientacao (H/V): ");
            scanf(" %c", &orientacao);
            orientacao = toupper((unsigned char)orientacao);

            // verificando limites do tabuleiro
            if ((orientacao == 'H' && col + barcos[barco].tamanho > TAM) ||
                (orientacao == 'V' && linha + barcos[barco].tamanho > TAM)) {
                printf("\n%s nao cabe na posicao escolhida. Tente novamente\n", barcos[barco].nome);
                continue;
            }

            // verificando se nao ha barcos se sobrepondo
            int sobreposicao = 0;
            for (int i = 0; i < barcos[barco].tamanho; i++) {
                int x = linha + (orientacao == 'V' ? i : 0); // se for vertical, linha + i. se nao for, linha + 0
                int y = col + (orientacao == 'H' ? i : 0);   // se for horizontal, col + i. se nao for, col + 0
                if (meu_tabuleiro[x][y] != '~')
                    sobreposicao = 1; // se nao tiver agua na posicao escolhida, ela esta invalida
            }
            if (sobreposicao) {
                printf("Ha outro navio nessa posicao. Tente novamente\n");
                continue;
            }

            // posicionando o barco
            for (int i = 0; i < barcos[barco].tamanho; i++) {
                int x = linha + (orientacao == 'V' ? i : 0);
                int y = col + (orientacao == 'H' ? i : 0);
                meu_tabuleiro[x][y] = barcos[barco].simbolo;
            }
            mostrar_tabuleiro_posicionando(); // mostrando como ficou o tabuleiro depois de posicionar cada barco

            valido = 1;
        }
    }
    printf("\n\n===========================================\n");
    printf("=====  TODOS OS NAVIOS POSICIONADOS   =====\n");
    printf("===========================================\n");
    printf("\nPressione Enter para continuar o jogo...");
    esperar_enter();
    limpar_tela(); // limpar o terminal
}

// Espera até ser "sua vez", se por algum motivo for chamada fora de hora
void aguardar_sua_vez() {
    while (!pode_digitar) {
        Sleep(10); // evita ocupar 100% da CPU
    }
}

void realizar_ataque(SOCKET sock) {
    int linha, col;
    char mensagem[32], resposta[16];
    char msg_afundado[32];

    // garante que só ataca quando for a vez do jogador
    aguardar_sua_vez();

    printf("\nSua vez! Escolha onde atirar:\n");

    // >>> validação: só aceita 0–9
    linha = ler_coordenada("Linha");
    col   = ler_coordenada("Coluna");

    // envia ataque para o cliente "linha,coluna"
    sprintf(mensagem, "%d,%d", linha, col);
    send(sock, mensagem, strlen(mensagem), 0);

    // recebe resposta "HIT" se acertou ou "MISS" se errou
    int bytes = recv(sock, resposta, sizeof(resposta) - 1, 0);
    if (bytes > 0) {
        resposta[bytes] = '\0';
    }

    if (strcmp(resposta, "HIT") == 0) {
        limpar_tela();
        printf("\n💥 Acertou um navio inimigo!\n");
        tabuleiro_inimigo[linha][col] = 'X';

        // verifica se há mensagem de navio afundado disponível
        fd_set fds;
        struct timeval timeout;
        FD_ZERO(&fds);
        FD_SET(sock, &fds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // 100ms para receber mensagem de afundado

        int activity = select(0, &fds, NULL, NULL, &timeout);

        if (activity > 0) {
            bytes = recv(sock, msg_afundado, sizeof(msg_afundado) - 1, 0);

            if (bytes > 0) {
                msg_afundado[bytes] = '\0';

                if (strstr(msg_afundado, "AFUNDADO:") != NULL) {
                    char simbolo = msg_afundado[9]; // "AFUNDADO:X" -> posição 9
                    char *nome_navio = obter_nome_navio(simbolo);
                    printf("\n🎯 VOCE AFUNDOU O %s INIMIGO!\n", nome_navio);
                }
            }
        }
    } else {
        limpar_tela();
        printf("\n🌊 Apenas agua! Nenhum navio atingido!\n");
        tabuleiro_inimigo[linha][col] = 'O';
    }
}

void receber_ataque(SOCKET sock) {
    int linha, col;
    char mensagem[32], resposta[16];
    char simbolo_navio = '\0'; // inicializa com valor padrão

    // Enquanto estou esperando o inimigo jogar, NÃO é a minha vez
    bloquear_entrada_usuario();

    printf("\nAguardando ataque do inimigo...\n");
    int bytes = recv(sock, mensagem, sizeof(mensagem) - 1, 0);

    if (bytes > 0) {
        mensagem[bytes] = '\0';
    }
    sscanf(mensagem, "%d,%d", &linha, &col); // atribuindo o ataque do inimigo para a linha e coluna

    // verificando se o ataque atingiu alguma posicao
    if (meu_tabuleiro[linha][col] != '~' &&
        meu_tabuleiro[linha][col] != 'X' &&
        meu_tabuleiro[linha][col] != 'O') {
        limpar_tela();
        printf("💣 O inimigo acertou em (%d,%d)!\n", linha, col);
        simbolo_navio = meu_tabuleiro[linha][col]; // salva o simbolo antes de marcar como X
        meu_tabuleiro[linha][col] = 'X';
        strcpy(resposta, "HIT");
    } else {
        limpar_tela();
        printf("\n😌 O inimigo errou (%d,%d)!\n", linha, col);

        if (meu_tabuleiro[linha][col] == '~') {
            meu_tabuleiro[linha][col] = 'O';
        }
        strcpy(resposta, "MISS");
    }

    // envia a resposta primeiro (HIT ou MISS)
    send(sock, resposta, strlen(resposta), 0);

    // se foi HIT, verifica se o navio foi completamente afundado
    if (strcmp(resposta, "HIT") == 0 &&
        simbolo_navio != '\0' &&
        verificar_navio_afundado(simbolo_navio)) {
        char msg_afundado[32];
        sprintf(msg_afundado, "AFUNDADO:%c", simbolo_navio);
        send(sock, msg_afundado, strlen(msg_afundado), 0);
        char *nome_navio = obter_nome_navio(simbolo_navio);
        printf("\n💀 SEU %s FOI AFUNDADO PELO INIMIGO!\n", nome_navio);
    }

    // terminou o turno do inimigo, agora volta a ser a minha vez
    liberar_entrada_usuario();
}

void bloquear_entrada_usuario() {
    pode_digitar = 0;
}

void liberar_entrada_usuario() {
    // Descarta qualquer coisa que o jogador possa ter digitado "fora da vez"
    // sem bloquear caso ele não tenha digitado nada
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn != INVALID_HANDLE_VALUE) {
        FlushConsoleInputBuffer(hIn);
    }

    pode_digitar = 1;
}

void limpar_tela() {
    printf("\033[2J\033[H");
}

int verificar_derrota() {
    // verifica se ainda há algum navio no tabuleiro do jogador
    for (int i = 0; i < TAM; i++) {
        for (int j = 0; j < TAM; j++) {
            if (meu_tabuleiro[i][j] == 'P' ||
                meu_tabuleiro[i][j] == 'E' ||
                meu_tabuleiro[i][j] == 'S' ||
                meu_tabuleiro[i][j] == 'D') {
                return 0; // ainda tem navios
            }
        }
    }
    return 1; // todos navios foram afundados
}

int verificar_vitoria() {
    // verifica se o jogador afundou todos os navios do inimigo
    int acertos = 0;
    for (int i = 0; i < TAM; i++) {
        for (int j = 0; j < TAM; j++) {
            if (tabuleiro_inimigo[i][j] == 'X')
                acertos++;
        }
    }
    return acertos >= TOTAL_BLOCOS; // total atual de blocos de navios inimigos
}

char verificar_navio_afundado(char simbolo) {
    // verifica se todas as casas de um navio específico foram acertadas
    for (int i = 0; i < TAM; i++) {
        for (int j = 0; j < TAM; j++) {
            if (meu_tabuleiro[i][j] == simbolo) {
                return 0; // ainda há casas não acertadas deste navio
            }
        }
    }
    return 1; // todas as casas deste navio foram acertadas (navio afundado)
}

char *obter_nome_navio(char simbolo) {
    // retorna o nome do navio baseado no simbolo
    switch (simbolo) {
    case 'P':
        return "PORTA-AVIOES";
    case 'E':
        return "ENCOURACADO";
    case 'S':
        return "SUBMARINO";
    case 'D':
        return "DESTROYER";
    default:
        return "NAVIO";
    }
}
