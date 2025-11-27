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