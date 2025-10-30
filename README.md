# 🛳️ Batalha Naval em C (Cliente/Servidor)

Este projeto implementa uma versão simples do jogo **Batalha Naval** utilizando **sockets** para comunicação entre **dois processos** — um **servidor** e um **cliente**.  
Cada jogador possui seu próprio tabuleiro e troca mensagens com o outro processo para controlar as jogadas.

---

## ⚙️ Estrutura do Projeto

```
📂 batalha-naval/
 ├── server.c       # Servidor - inicia o jogo e aguarda conexão do cliente
 ├── client.c       # Cliente - conecta ao servidor e participa da partida
 ├── .vscode/
 │    ├── settings.json
 │    └── tasks.json
 ├── README.md
 └── .gitignore
```

## 🧩 Compilação Manual via Terminal

Abra dois terminais (ou abas do PowerShell/CMD) na pasta do projeto.

### 🖥️ 1️⃣ Compilar o Servidor
```
gcc server.c -o server.exe -lws2_32
```
Em seguida, execute:
```
./server.exe
```
O servidor exibirá:
```
Inicializando servidor
Aguardando conexao do jogador 2...
```

### 💻 2️⃣ Compilar o Cliente
Em outro terminal (ou outra janela):
```
gcc client.c -o client.exe -lws2_32
```
E execute:
```
./client.exe
```
O cliente se conectará automaticamente ao servidor e o jogo começará.

## 🧠 Dica — Usando o VS Code
Se estiver usando Visual Studio Code, o projeto já contém um arquivo de tarefas (.vscode/tasks.json) configurado para compilar facilmente os dois programas.

### ▶️ Para compilar dentro do VS Code:

1. Abra o projeto no VS Code

2. Pressione Ctrl + Shift + B

3. Escolha:  
    • Compilar Server → gera server.exe  
    • Compilar Client → gera client.exe

Depois, execute os binários um em cada terminal:
```
./server.exe
```
```
./client.exe
```