# GS2-2025-Data-Structures-and-Algorithms

```
                                  |>>>                              
                                  |                                 
                    |>>>      _  _|_  _         |>>>                
                    |        |;| |;| |;|        |                   
                _  _|_  _    \\.    .  /    _  _|_  _               
               |;|_|;|_|;|    \\:. ,  /    |;|_|;|_|;|              
               \\..      /    ||;   . |    \\.    .  /              
                \\.  ,  /     ||:  .  |     \\:  .  /               
                 ||:   |_   _ ||_ . _ | _   _||:   |                
                 ||:  .|||_|;|_|;|_|;|_|;|_|;||:.  |                
                 ||:   ||.    .     .      . ||:  .|                
                 ||: . || .     . .   .  ,   ||:   |       \,/      
                 ||:   ||:  ,  _______   .   ||: , |            /`\ 
                 ||:   || .   /+++++++\    . ||:   |                
                 ||:   ||.    |+++++++| .    ||: . |                
              __ ||: . ||: ,  |+++++++|.  . _||_   |                
     ____--`~    '--~~__|.    |+++++__|----~    ~`---,              
-~--~                   ~---__|,--~'                  ~~----_____-~'
```

Nesta atividade foi solicitado o uso de IAs como apoio ao raciocínio algorítmico na criação de um programa a nossa escolha com o tema "O Futuro do Trabalho". Eu utilizei o [blackbox.ai](https://www.blackbox.ai/) para para elaborar essa etapa utilizando engenharia de prompt para garantir o funcioinamento e a otimização do programa.

> ⚠️ **Attention:** - No fim do README.md existe um projeto extra elaborado para complementar a GS, por favor acesse para conferir. 😉

## Prompt Utilizado:

```
Criar um sistema em C que gerencia uma 'fila' de cursos para requalificação profissional.
Utilize algoritimos de Busca, Ordenação, TAD, Fila e Pilha para que o prograna fique completo.
Foque na otimização do código.
```

## Analise Técnica do Código Gerado

O código gerado está presente no arquivo:

- [prompt.c](https://github.com/LucasWerppFranco/GS2-2025-Data-Structures-and-Algorithms/blob/main/prompt.c)

Tanto as explicaçoes quanto o código estão divididos com base nas técnicas que foram aplicadas em sua elaboração, mostrando como elas foram utilizadas no projeto e enfatizando o aprendizado que tive no decorrer do ano:

- **TADs**: Usados structs para `Curso`, `Aluno` e `Inscricao`. Fila e Pilha implementadas como listas ligadas para eficiência em operações O(1) para inserção/remoção.
  
- **Fila**: Gerencia inscrições em ordem FIFO, com timestamp para possível extensão a prioridades.
  
- **Pilha**: Mantém histórico de ações para "undo", útil para reversões.
  
- **Busca**: Busca binária para cursos (após ordenação), O(log n); busca linear para alunos, O(n) – adequada pois alunos não são ordenados.
  
- **Ordenação**: QuickSort para cursos por prioridade, O(n log n) em média, eficiente.
  
- **Otimização Geral**: 
  - Alocação dinâmica apenas quando necessário.
  - Limites de arrays para evitar overflow.
  - Uso de ponteiros para evitar cópias de structs grandes.
  - Liberação de memória ao final para prevenir vazamentos.
  - Algoritmos escolhidos são eficientes para os tamanhos esperados (até 1000 cursos/alunos, 10000 inscrições).
    
- **Funcionalidades**: O programa permite adicionar cursos/alunos, inscrever (enfileirar), processar (desenfileirar), ordenar, buscar e undo. É completo e focado em eficiência.

# Rodando o Programa

O código gerado pelo agente de IA pode ser compilado normalmente

```
gcc -O prompt.c
```

A versão finl utiliz-se de um arquitetura ja utilizada por mim em outros projetos que necessita de um sistema operacional Linux, mas para economizar o seu tempo eu criei um Dockerfile com tudo necessario para rodar o programa, bast utilizar essa sequencia de comandos:

```
docker build -t sistema-cursos .                     
docker run -it sistema-cursos
```

Caso queira compilar na sua mquina, disponibilizarei o comando para compilar a versão final aqui

```
apt-get update && apt-get install -y
```

# Video do Projeto

Video disponivel no [YouTube](https://youtu.be/QEJWnRzxiKA).

# EXTRA

Como um projeto extra para somar na GS, eu desenvolvi um programa de mentoria inteligente em C utilizando o Gemini para encontrar o melhor mentor que atenda os requisitos que melhor atenda seu mentorado.

[Clique aqui para checar!](https://github.com/LucasWerppFranco/GS2-2025-Data-Structures-and-Algorithms/tree/main/Mentoria-Inteligente)

---

## Membros do Trabalho

- Lucas Franco | RM: 556044
- Lucca Rosseto Rezende | RM: 564180
- Massayoshi Bando | RM: 561779

---
