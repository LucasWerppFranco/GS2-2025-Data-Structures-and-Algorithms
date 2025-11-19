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

## Prompt Utilizado:

```
Criar um sistema em C que gerencia uma 'fila' de cursos para requalificação profissional.
Utilize algoritimos de Busca, Ordenação, TAD, Fila e Pilha para que o prograna fique completo.
Foque na otimização do código.
```

## Analise Técnica do Código Gerad

O código gerado está presente no arquivo:

- [main.c](https://github.com/LucasWerppFranco/GS2-2025-Data-Structures-and-Algorithms/blob/main/main.c)

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
