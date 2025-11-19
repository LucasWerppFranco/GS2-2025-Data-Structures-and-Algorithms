#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Definições de constantes para otimização
#define MAX_NOME 50
#define MAX_CURSOS 1000
#define MAX_ALUNOS 1000
#define MAX_INSCRICOES 10000

// TAD: Struct para Curso
typedef struct {
    char nome[MAX_NOME];
    int duracao; // em horas
    int prioridade; // 1-10, maior prioridade primeiro
} Curso;

// TAD: Struct para Aluno
typedef struct {
    char nome[MAX_NOME];
    int id;
} Aluno;

// TAD: Struct para Inscricao (elemento da fila)
typedef struct {
    Aluno aluno;
    int curso_index; // índice do curso no array de cursos
    time_t timestamp; // para ordenação por tempo de chegada
} Inscricao;

// TAD: Fila (usando lista ligada para eficiência em inserções/removências)
typedef struct NoFila {
    Inscricao inscricao;
    struct NoFila* proximo;
} NoFila;

typedef struct {
    NoFila* frente;
    NoFila* tras;
    int tamanho;
} Fila;

// TAD: Pilha (usando lista ligada para eficiência)
typedef struct NoPilha {
    char acao[MAX_NOME]; // descrição da ação para undo
    struct NoPilha* proximo;
} NoPilha;

typedef struct {
    NoPilha* topo;
    int tamanho;
} Pilha;

// Arrays globais para cursos e alunos (para busca e ordenação)
Curso cursos[MAX_CURSOS];
int num_cursos = 0;
Aluno alunos[MAX_ALUNOS];
int num_alunos = 0;

// Funções para Fila
void inicializarFila(Fila* f) {
    f->frente = NULL;
    f->tras = NULL;
    f->tamanho = 0;
}

int filaVazia(Fila* f) {
    return f->tamanho == 0;
}

void enfileirar(Fila* f, Inscricao inscricao) {
    NoFila* novo = (NoFila*)malloc(sizeof(NoFila));
    if (!novo) {
        printf("Erro: Falha na alocação de memória.\n");
        return;
    }
    novo->inscricao = inscricao;
    novo->proximo = NULL;
    if (filaVazia(f)) {
        f->frente = f->tras = novo;
    } else {
        f->tras->proximo = novo;
        f->tras = novo;
    }
    f->tamanho++;
}

Inscricao desenfileirar(Fila* f) {
    if (filaVazia(f)) {
        printf("Erro: Fila vazia.\n");
        exit(1);
    }
    NoFila* temp = f->frente;
    Inscricao inscricao = temp->inscricao;
    f->frente = f->frente->proximo;
    if (f->frente == NULL) f->tras = NULL;
    free(temp);
    f->tamanho--;
    return inscricao;
}

// Funções para Pilha
void inicializarPilha(Pilha* p) {
    p->topo = NULL;
    p->tamanho = 0;
}

int pilhaVazia(Pilha* p) {
    return p->tamanho == 0;
}

void empilhar(Pilha* p, const char* acao) {
    NoPilha* novo = (NoPilha*)malloc(sizeof(NoPilha));
    if (!novo) {
        printf("Erro: Falha na alocação de memória.\n");
        return;
    }
    strcpy(novo->acao, acao);
    novo->proximo = p->topo;
    p->topo = novo;
    p->tamanho++;
}

char* desempilhar(Pilha* p) {
    if (pilhaVazia(p)) {
        printf("Erro: Pilha vazia.\n");
        return NULL;
    }
    NoPilha* temp = p->topo;
    char* acao = (char*)malloc(strlen(temp->acao) + 1);
    strcpy(acao, temp->acao);
    p->topo = p->topo->proximo;
    free(temp);
    p->tamanho--;
    return acao;
}

// Algoritmo de Ordenação: QuickSort para cursos por prioridade (otimizado)
void trocar(Curso* a, Curso* b) {
    Curso temp = *a;
    *a = *b;
    *b = temp;
}

int particionar(Curso arr[], int baixo, int alto) {
    int pivo = arr[alto].prioridade;
    int i = baixo - 1;
    for (int j = baixo; j < alto; j++) {
        if (arr[j].prioridade >= pivo) { // Maior prioridade primeiro
            i++;
            trocar(&arr[i], &arr[j]);
        }
    }
    trocar(&arr[i + 1], &arr[alto]);
    return i + 1;
}

void quickSort(Curso arr[], int baixo, int alto) {
    if (baixo < alto) {
        int pi = particionar(arr, baixo, alto);
        quickSort(arr, baixo, pi - 1);
        quickSort(arr, pi + 1, alto);
    }
}

// Algoritmo de Busca: Busca Binária para cursos (assume ordenado por prioridade)
int buscaBinariaCurso(Curso arr[], int n, int prioridade) {
    int baixo = 0, alto = n - 1;
    while (baixo <= alto) {
        int meio = baixo + (alto - baixo) / 2;
        if (arr[meio].prioridade == prioridade) return meio;
        if (arr[meio].prioridade > prioridade) baixo = meio + 1;
        else alto = meio - 1;
    }
    return -1;
}

// Busca Linear para alunos (não ordenado)
int buscaLinearAluno(Aluno arr[], int n, int id) {
    for (int i = 0; i < n; i++) {
        if (arr[i].id == id) return i;
    }
    return -1;
}

// Função para adicionar curso
void adicionarCurso() {
    if (num_cursos >= MAX_CURSOS) {
        printf("Erro: Limite de cursos atingido.\n");
        return;
    }
    printf("Nome do curso: ");
    scanf(" %[^\n]", cursos[num_cursos].nome);
    printf("Duração (horas): ");
    scanf("%d", &cursos[num_cursos].duracao);
    printf("Prioridade (1-10): ");
    scanf("%d", &cursos[num_cursos].prioridade);
    num_cursos++;
    printf("Curso adicionado.\n");
}

// Função para adicionar aluno
void adicionarAluno() {
    if (num_alunos >= MAX_ALUNOS) {
        printf("Erro: Limite de alunos atingido.\n");
        return;
    }
    printf("Nome do aluno: ");
    scanf(" %[^\n]", alunos[num_alunos].nome);
    printf("ID do aluno: ");
    scanf("%d", &alunos[num_alunos].id);
    num_alunos++;
    printf("Aluno adicionado.\n");
}

// Função para inscrever aluno em curso (adiciona à fila)
void inscreverAluno(Fila* fila, Pilha* historico) {
    int id_aluno, index_curso;
    printf("ID do aluno: ");
    scanf("%d", &id_aluno);
    int aluno_index = buscaLinearAluno(alunos, num_alunos, id_aluno);
    if (aluno_index == -1) {
        printf("Erro: Aluno não encontrado.\n");
        return;
    }
    printf("Nome do curso: ");
    char nome_curso[MAX_NOME];
    scanf(" %[^\n]", nome_curso);
    for (index_curso = 0; index_curso < num_cursos; index_curso++) {
        if (strcmp(cursos[index_curso].nome, nome_curso) == 0) break;
    }
    if (index_curso == num_cursos) {
        printf("Erro: Curso não encontrado.\n");
        return;
    }
    Inscricao nova;
    nova.aluno = alunos[aluno_index];
    nova.curso_index = index_curso;
    nova.timestamp = time(NULL);
    enfileirar(fila, nova);
    char acao[100];
    sprintf(acao, "Inscrição: %s em %s", alunos[aluno_index].nome, cursos[index_curso].nome);
    empilhar(historico, acao);
    printf("Inscrição realizada.\n");
}

// Função para processar inscrição (remover da fila)
void processarInscricao(Fila* fila, Pilha* historico) {
    if (filaVazia(fila)) {
        printf("Nenhuma inscrição para processar.\n");
        return;
    }
    Inscricao processada = desenfileirar(fila);
    printf("Processando inscrição: %s em %s\n", processada.aluno.nome, cursos[processada.curso_index].nome);
    char acao[100];
    sprintf(acao, "Processamento: %s em %s", processada.aluno.nome, cursos[processada.curso_index].nome);
    empilhar(historico, acao);
}

// Função para ordenar cursos
void ordenarCursos() {
    quickSort(cursos, 0, num_cursos - 1);
    printf("Cursos ordenados por prioridade.\n");
}

// Função para buscar curso
void buscarCurso() {
    int prioridade;
    printf("Prioridade do curso: ");
    scanf("%d", &prioridade);
    int index = buscaBinariaCurso(cursos, num_cursos, prioridade);
    if (index != -1) {
        printf("Curso encontrado: %s, Duração: %d horas\n", cursos[index].nome, cursos[index].duracao);
    } else {
        printf("Curso não encontrado.\n");
    }
}

// Função para undo (usando pilha)
void undo(Pilha* historico) {
    char* acao = desempilhar(historico);
    if (acao) {
        printf("Undo: %s\n", acao);
        free(acao);
    } else {
        printf("Nenhuma ação para desfazer.\n");
    }
}

// Menu principal
int main() {
    Fila fila_inscricoes;
    Pilha historico;
    inicializarFila(&fila_inscricoes);
    inicializarPilha(&historico);

    int opcao;
    do {
        printf("\nSistema de Gerenciamento de Cursos\n");
        printf("1. Adicionar Curso\n");
        printf("2. Adicionar Aluno\n");
        printf("3. Inscrever Aluno em Curso\n");
        printf("4. Processar Inscrição\n");
        printf("5. Ordenar Cursos por Prioridade\n");
        printf("6. Buscar Curso por Prioridade\n");
        printf("7. Undo Última Ação\n");
        printf("0. Sair\n");
        printf("Escolha: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1: adicionarCurso(); break;
            case 2: adicionarAluno(); break;
            case 3: inscreverAluno(&fila_inscricoes, &historico); break;
            case 4: processarInscricao(&fila_inscricoes, &historico); break;
            case 5: ordenarCursos(); break;
            case 6: buscarCurso(); break;
            case 7: undo(&historico); break;
            case 0: printf("Saindo...\n"); break;
            default: printf("Opção inválida.\n");
        }
    } while (opcao != 0);

    // Liberar memória (otimização: evitar vazamentos)
    while (!filaVazia(&fila_inscricoes)) desenfileirar(&fila_inscricoes);
    while (!pilhaVazia(&historico)) free(desempilhar(&historico));

    return 0;
}

