// ===========================================================
// SISTEMA DE MENTORIA INTELIGENTE – ARQUIVO ÚNICO
// ===========================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>
#include <termios.h>
#include <unistd.h>
#include <limits.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

// -----------------------------------------------------------
// DEFINIÇÕES
// -----------------------------------------------------------
#define WIDTH 60
#define ARQ_MENTORES "mentores.txt"
#define ARQ_MENTORADOS "mentorados.txt"
#define ARQ_PROMPT "prompt.txt"
#define GEMINI_API_KEY "AIzaSyA_bm2VqGRcWrkHOD-QwyiO_gPN4SV0PEA"
#define GEMINI_URL "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-pro:generateContent?key=" GEMINI_API_KEY

// -----------------------------------------------------------
// ESTRUTURAS
// -----------------------------------------------------------
typedef struct {
    char nome[100];
    char area[100];
    int experiencia;
} Mentor;

typedef struct {
    char nome[100];
    char objetivo[200];
} Mentorado;

// Estrutura para resposta do cURL
struct MemoryStruct {
    char *memory;
    size_t size;
};

// -----------------------------------------------------------
// FUNÇÕES DE INTERFACE
// -----------------------------------------------------------
// Calcula largura visual UTF-8
int visual_width(const char *s) {
    int width = 0;
    wchar_t wc;
    mbstate_t state;
    memset(&state, 0, sizeof state);
    const char *p = s;
    while (*p) {
        size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &state);
        if (len == (size_t)-1 || len == (size_t)-2 || len == 0) break;
        int w = wcwidth(wc);
        if (w > 0) width += w;
        p += len;
    }
    return width;
}

void print_border_top() {
    printf("╔");
    for (int i = 0; i < WIDTH - 2; i++) printf("═");
    printf("╗\n");
}

void print_border_bottom() {
    printf("╚");
    for (int i = 0; i < WIDTH - 2; i++) printf("═");
    printf("╝\n");
}

void print_line(const char *text) {
    printf("║");
    int content_width = visual_width(text);
    int padding = WIDTH - 2 - content_width;
    if (padding < 0) padding = 0;
    printf("%s", text);
    for (int j = 0; j < padding; j++) printf(" ");
    printf("║\n");
}

// Exibe texto multilinha (quebra por '\n')
void mostrarTextoMultiline(const char *texto) {
    print_border_top();
    if (!texto || texto[0] == '\0') {
        print_line("");
    } else {
        char *dup = strdup(texto);
        char *line = strtok(dup, "\n");
        while (line) {
            // Se a linha for maior que o WIDTH-2, quebra em pedaços menores
            int max_chars = WIDTH - 2;
            int len = (int)strlen(line);
            if (len <= max_chars) {
                print_line(line);
            } else {
                // quebra por caracteres (não perfeito para utf8, mas funcional)
                int start = 0;
                while (start < len) {
                    char buf[1024] = {0};
                    int take = (len - start > max_chars) ? max_chars : (len - start);
                    strncpy(buf, line + start, take);
                    buf[take] = '\0';
                    print_line(buf);
                    start += take;
                }
            }
            line = strtok(NULL, "\n");
        }
        free(dup);
    }
    print_border_bottom();
}

// Entrada de teclado
void limparBufferEntrada() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int capturaTecla() {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    if (ch == 27) {
        // sequência de escape
        int next1 = getchar(); // possivelmente '['
        (void)next1;
        int next2 = getchar(); // A/B/C/D
        if (next2 == 'A') ch = 'w';
        else if (next2 == 'B') ch = 's';
        else if (next2 == 'C') ch = 'd';
        else if (next2 == 'D') ch = 'a';
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

void esperarPressionarQ() {
    char input[10];
    print_border_top();
    print_line("Pressione 'q' para voltar ao menu...");
    print_border_bottom();
    do {
        printf("> ");
        if (fgets(input, sizeof(input), stdin)) {
            if (input[0] == 'q' || input[0] == 'Q') break;
        }
    } while (1);
}

// Confirmação de remoção: retorna 1 se 'y', 0 se 'n'
int confirmarRemocao(const char *mensagem) {
    char input[10];
    print_border_top();
    print_line(mensagem);
    print_line("Pressione 'y' para sim ou 'n' para não.");
    print_border_bottom();
    do {
        printf("> ");
        if (fgets(input, sizeof(input), stdin)) {
            if (input[0] == 'y' || input[0] == 'Y') return 1;
            if (input[0] == 'n' || input[0] == 'N') return 0;
        }
    } while (1);
}

// -----------------------------------------------------------
// FUNÇÕES DE DADOS
// -----------------------------------------------------------
// Carrega mentores do arquivo para memória
int carregarMentores(Mentor **lista) {
    FILE *f = fopen(ARQ_MENTORES, "r");
    if (!f) return 0;
    int count = 0;
    Mentor temp;
    while (fscanf(f, "%99[^;];%99[^;];%d\n", temp.nome, temp.area, &temp.experiencia) == 3) {
        Mentor *res = realloc(*lista, sizeof(Mentor) * (count + 1));
        if (!res) { fclose(f); return count; }
        *lista = res;
        (*lista)[count++] = temp;
    }
    fclose(f);
    return count;
}

// Carrega mentorados do arquivo para memória
int carregarMentorados(Mentorado **lista) {
    FILE *f = fopen(ARQ_MENTORADOS, "r");
    if (!f) return 0;
    int count = 0;
    Mentorado temp;
    while (fscanf(f, "%99[^;];%199[^\n]\n", temp.nome, temp.objetivo) == 2) {
        Mentorado *res = realloc(*lista, sizeof(Mentorado) * (count + 1));
        if (!res) { fclose(f); return count; }
        *lista = res;
        (*lista)[count++] = temp;
    }
    fclose(f);
    return count;
}

// Salva todos os mentores da memória para o arquivo (reescreve o arquivo)
void salvarMentores(Mentor *lista, int total) {
    FILE *f = fopen(ARQ_MENTORES, "w");  // Modo "w" para reescrever
    if (!f) return;
    for (int i = 0; i < total; i++) {
        fprintf(f, "%s;%s;%d\n", lista[i].nome, lista[i].area, lista[i].experiencia);
    }
    fclose(f);
}

// Salva todos os mentorados da memória para o arquivo (reescreve o arquivo)
void salvarMentorados(Mentorado *lista, int total) {
    FILE *f = fopen(ARQ_MENTORADOS, "w");  // Modo "w" para reescrever
    if (!f) return;
    for (int i = 0; i < total; i++) {
        fprintf(f, "%s;%s\n", lista[i].nome, lista[i].objetivo);
    }
    fclose(f);
}

// Adiciona mentor apenas em memória (persistência será feita ao salvar)
int adicionarMentor(Mentor **lista, int total, Mentor novo) {
    Mentor *res = realloc(*lista, sizeof(Mentor) * (total + 1));
    if (!res) return total;
    *lista = res;
    (*lista)[total] = novo;
    return total + 1;
}

// Adiciona mentorado apenas em memória (persistência será feita ao salvar)
int adicionarMentorado(Mentorado **lista, int total, Mentorado novo) {
    Mentorado *res = realloc(*lista, sizeof(Mentorado) * (total + 1));
    if (!res) return total;
    *lista = res;
    (*lista)[total] = novo;
    return total + 1;
}

// Remove mentor da memória pelo índice (shift dos elementos restantes)
int removerMentor(Mentor **lista, int total, int indice) {
    if (indice < 0 || indice >= total) return total;  // Índice inválido
    // Shift dos elementos após o índice
    for (int i = indice; i < total - 1; i++) {
        (*lista)[i] = (*lista)[i + 1];
    }
    // Reduz o tamanho da array
    if (total - 1 == 0) {
        free(*lista);
        *lista = NULL;
        return 0;
    }
    Mentor *res = realloc(*lista, sizeof(Mentor) * (total - 1));
    if (res || total == 1) *lista = res;  // Se realloc falhar e total > 1, mantém o original
    return total - 1;
}

// Remove mentorado da memória pelo índice (shift dos elementos restantes)
int removerMentorado(Mentorado **lista, int total, int indice) {
    if (indice < 0 || indice >= total) return total;  // Índice inválido
    // Shift dos elementos após o índice
    for (int i = indice; i < total - 1; i++) {
        (*lista)[i] = (*lista)[i + 1];
    }
    // Reduz o tamanho da array
    if (total - 1 == 0) {
        free(*lista);
        *lista = NULL;
        return 0;
    }
    Mentorado *res = realloc(*lista, sizeof(Mentorado) * (total - 1));
    if (res || total == 1) *lista = res;  // Se realloc falhar e total > 1, mantém o original
    return total - 1;
}

// -----------------------------------------------------------
// FUNÇÕES DA API DO GEMINI
// -----------------------------------------------------------
// Callback para cURL escrever resposta
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) return 0;
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

// Chama a API do Gemini e retorna a resposta gerada (ou NULL).
// Em caso de erro, imprime mensagens detalhadas de debug/erro.
char* chamarGemini(const char *url, const char *prompt) {
    CURL *curl;
    CURLcode res;
    long http_code = 0;
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Erro: falha ao inicializar cURL\n");
        free(chunk.memory);
        return NULL;
    }

    // Monta o JSON do corpo da requisição (adapte se quiser role:user)
    cJSON *root = cJSON_CreateObject();
    cJSON *contents = cJSON_AddArrayToObject(root, "contents");
    cJSON *content = cJSON_CreateObject();
    cJSON_AddItemToArray(contents, content);
    cJSON_AddStringToObject(content, "role", "user");
    cJSON *parts = cJSON_AddArrayToObject(content, "parts");
    cJSON *part = cJSON_CreateObject();
    cJSON_AddItemToArray(parts, part);
    cJSON_AddStringToObject(part, "text", prompt);

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    // Opcional: timeout curto para não travar indefinidamente
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    res = curl_easy_perform(curl);

    // Pega código HTTP (se disponível)
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (res != CURLE_OK) {
        fprintf(stderr, "cURL error: %s\n", curl_easy_strerror(res));
        if (chunk.size > 0 && chunk.memory) {
            fprintf(stderr, "Resposta bruta (quando cURL falhou):\n%s\n", chunk.memory);
        }
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(json_str);
        free(chunk.memory);
        return NULL;
    }

    // Se servidor respondeu com status HTTP >=400, imprime detalhe e tenta parsear JSON de erro
    if (http_code >= 400) {
        fprintf(stderr, "HTTP error: %ld\n", http_code);
        if (chunk.size > 0 && chunk.memory) {
            fprintf(stderr, "Resposta bruta:\n%s\n", chunk.memory);

            // Tenta parsear JSON e extrair campo "error"
            cJSON *resp = cJSON_Parse(chunk.memory);
            if (resp) {
                cJSON *error = cJSON_GetObjectItem(resp, "error");
                if (error) {
                    cJSON *msg = cJSON_GetObjectItem(error, "message");
                    cJSON *code = cJSON_GetObjectItem(error, "code");
                    cJSON *status = cJSON_GetObjectItem(error, "status");
                    if (msg) fprintf(stderr, "API error.message: %s\n", cJSON_GetStringValue(msg));
                    if (code) {
                        if (cJSON_IsNumber(code)) fprintf(stderr, "API error.code: %d\n", code->valueint);
                        else fprintf(stderr, "API error.code: %s\n", cJSON_GetStringValue(code));
                    }
                    if (status) fprintf(stderr, "API error.status: %s\n", cJSON_GetStringValue(status));
                } else {
                    // Às vezes a API retorna { "error": { ... } } ou { "errorMessage": "..."}
                    cJSON *errMsg = cJSON_GetObjectItem(resp, "errorMessage");
                    if (errMsg) fprintf(stderr, "API errorMessage: %s\n", cJSON_GetStringValue(errMsg));
                }
                cJSON_Delete(resp);
            } else {
                fprintf(stderr, "Resposta não é JSON válido.\n");
            }
        } else {
            fprintf(stderr, "Nenhuma resposta do servidor.\n");
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(json_str);
        free(chunk.memory);
        return NULL;
    }

    // Se chegamos aqui, HTTP ok (2xx). Parseia o JSON e tenta extrair a resposta em candidates->content->parts->text
    cJSON *response = cJSON_Parse(chunk.memory);
    if (response) {
        cJSON *candidates = cJSON_GetObjectItem(response, "candidates");
        if (candidates && cJSON_GetArraySize(candidates) > 0) {
            cJSON *candidate = cJSON_GetArrayItem(candidates, 0);
            cJSON *content_resp = cJSON_GetObjectItem(candidate, "content");
            if (content_resp) {
                cJSON *parts_resp = cJSON_GetObjectItem(content_resp, "parts");
                if (parts_resp && cJSON_GetArraySize(parts_resp) > 0) {
                    cJSON *part_resp = cJSON_GetArrayItem(parts_resp, 0);
                    cJSON *text = cJSON_GetObjectItem(part_resp, "text");
                    if (text) {
                        char *result = strdup(cJSON_GetStringValue(text));
                        cJSON_Delete(response);
                        curl_slist_free_all(headers);
                        curl_easy_cleanup(curl);
                        free(json_str);
                        free(chunk.memory);
                        return result;
                    }
                }
            }
        }
        // Se não encontrou o campo esperado, imprime a resposta e retorna NULL
        fprintf(stderr, "Resposta JSON recebida, mas campo esperado não encontrado. Resposta bruta:\n%s\n", chunk.memory);
        cJSON_Delete(response);
    } else {
        fprintf(stderr, "Erro ao parsear JSON de resposta.\n");
        if (chunk.size > 0) fprintf(stderr, "Resposta bruta:\n%s\n", chunk.memory);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(json_str);
    free(chunk.memory);
    return NULL;
}

// Lê prompt base de ARQ_PROMPT, se existir; caso contrário retorna uma string default (caller deve free)
char* carregarPromptBase() {
    FILE *f = fopen(ARQ_PROMPT, "r");
    if (!f) {
        const char *def = "Você é um mentor virtual que ajuda a combinar mentorados com mentores com base em experiência, área e objetivo profissional. Dê recomendações claras e razões.";
        return strdup(def);
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz + 1);
    if (!buf) { fclose(f); return strdup(""); }
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);
    return buf;
}

// -----------------------------------------------------------
// PROGRAMA PRINCIPAL
// -----------------------------------------------------------
int main() {
    setlocale(LC_ALL, "");
    curl_global_init(CURL_GLOBAL_DEFAULT);  // Inicializa cURL
    Mentor *mentores = NULL;
    Mentorado *mentorados = NULL;
    int totalMentores = carregarMentores(&mentores);
    int totalMentorados = carregarMentorados(&mentorados);
    const char *opcoes[] = {
        "Listar Mentores",
        "Listar Mentorados",
        "Adicionar Mentor",
        "Adicionar Mentorado",
        "Remover Mentor",
        "Remover Mentorado",
        "Encontrar Mentor Ideal",
        "Sair"
    };
    int total_opcoes = 8;
    int selected = 0;
    int running = 1;
    while (running) {
        system("clear");
        print_border_top();
        print_line(" <=== SISTEMA DE MENTORIA ===>");
        print_line("");
        for (int i = 0; i < total_opcoes; i++) {
            char buffer[200];
            if (i == selected) snprintf(buffer, sizeof(buffer), " < %s >", opcoes[i]);
            else snprintf(buffer, sizeof(buffer), " %s ", opcoes[i]);
            print_line(buffer);
        }
        print_line("");
        print_line("Use as setas ou W/S para navegar. Enter para selecionar.");
        print_border_bottom();
        int tecla = capturaTecla();
        if (tecla == 'w') selected = (selected - 1 + total_opcoes) % total_opcoes;
        else if (tecla == 's') selected = (selected + 1) % total_opcoes;
        else if (tecla == '\n') {
            system("clear");
            switch (selected) {
                case 0: {
                    // Listar Mentores
                    print_border_top();
                    print_line("Lista de Mentores:");
                    for (int i = 0; i < totalMentores; i++) {
                        char linha[200];
                        snprintf(linha, sizeof(linha), "%s - %s (%d anos XP)", mentores[i].nome, mentores[i].area, mentores[i].experiencia);
                        print_line(linha);
                    }
                    print_border_bottom();
                    esperarPressionarQ();
                    break;
                }
                case 1: {
                    // Listar Mentorados
                    print_border_top();
                    print_line("Lista de Mentorados:");
                    for (int i = 0; i < totalMentorados; i++) {
                        char linha[200];
                        snprintf(linha, sizeof(linha), "%s - Objetivo: %s", mentorados[i].nome, mentorados[i].objetivo);
                        print_line(linha);
                    }
                    print_border_bottom();
                    esperarPressionarQ();
                    break;
                }
                case 2: {
                    // Adicionar Mentor
                    Mentor novo;
                    print_border_top();
                    print_line("Cadastro de Mentor");
                    print_border_bottom();
                    printf("Nome: ");
                    fgets(novo.nome, sizeof(novo.nome), stdin);
                    novo.nome[strcspn(novo.nome, "\n")] = 0;
                    printf("Área: ");
                    fgets(novo.area, sizeof(novo.area), stdin);
                    novo.area[strcspn(novo.area, "\n")] = 0;
                    printf("Anos de experiência: ");
                    if (scanf("%d", &novo.experiencia) != 1) novo.experiencia = 0;
                    getchar();  // limpar buffer
                    totalMentores = adicionarMentor(&mentores, totalMentores, novo);
                    salvarMentores(mentores, totalMentores);
                    print_border_top();
                    print_line("Mentor cadastrado com sucesso!");
                    print_border_bottom();
                    esperarPressionarQ();
                    break;
                }
                case 3: {
                    // Adicionar Mentorado
                    Mentorado novo;
                    print_border_top();
                    print_line("Cadastro de Mentorado");
                    print_border_bottom();
                    printf("Nome: ");
                    fgets(novo.nome, sizeof(novo.nome), stdin);
                    novo.nome[strcspn(novo.nome, "\n")] = 0;
                    printf("Objetivo profissional: ");
                    fgets(novo.objetivo, sizeof(novo.objetivo), stdin);
                    novo.objetivo[strcspn(novo.objetivo, "\n")] = 0;
                    totalMentorados = adicionarMentorado(&mentorados, totalMentorados, novo);
                    salvarMentorados(mentorados, totalMentorados);
                    print_border_top();
                    print_line("Mentorado cadastrado com sucesso!");
                    print_border_bottom();
                    esperarPressionarQ();
                    break;
                }
                case 4: {
                    // Remover Mentor
                    if (totalMentores == 0) {
                        print_border_top();
                        print_line("Nenhum mentor cadastrado para remover.");
                        print_border_bottom();
                        esperarPressionarQ();
                        break;
                    }
                    int selected_mentor = 0;
                    int selecting = 1;
                    while (selecting) {
                        system("clear");
                        print_border_top();
                        print_line("Selecione o Mentor a Remover:");
                        print_line("");
                        for (int i = 0; i < totalMentores; i++) {
                            char linha[200];
                            char buffer[250];
                            snprintf(linha, sizeof(linha), "%s - %s (%d anos XP)", mentores[i].nome, mentores[i].area, mentores[i].experiencia);
                            if (i == selected_mentor) snprintf(buffer, sizeof(buffer), " < %s >", linha);
                            else snprintf(buffer, sizeof(buffer), " %s ", linha);
                            print_line(buffer);
                        }
                        print_line("");
                        print_line("Use as setas ou W/S para navegar. Enter para selecionar.");
                        print_border_bottom();
                        int tecla2 = capturaTecla();
                        if (tecla2 == 'w') selected_mentor = (selected_mentor - 1 + totalMentores) % totalMentores;
                        else if (tecla2 == 's') selected_mentor = (selected_mentor + 1) % totalMentores;
                        else if (tecla2 == '\n') {
                            char msg[200];
                            snprintf(msg, sizeof(msg), "Deseja remover o mentor '%s'?", mentores[selected_mentor].nome);
                            if (confirmarRemocao(msg)) {
                                totalMentores = removerMentor(&mentores, totalMentores, selected_mentor);
                                salvarMentores(mentores, totalMentores);
                                print_border_top();
                                print_line("Mentor removido com sucesso.");
                                print_border_bottom();
                                esperarPressionarQ();
                                selecting = 0;
                            } else {
                                selecting = 0;
                            }
                        }
                    }
                    break;
                }
                case 5: {
                    // Remover Mentorado
                    if (totalMentorados == 0) {
                        print_border_top();
                        print_line("Nenhum mentorado cadastrado para remover.");
                        print_border_bottom();
                        esperarPressionarQ();
                        break;
                    }
                    int selected_mentorado = 0;
                    int selecting = 1;
                    while (selecting) {
                        system("clear");
                        print_border_top();
                        print_line("Selecione o Mentorado a Remover:");
                        print_line("");
                        for (int i = 0; i < totalMentorados; i++) {
                            char linha[220];
                            char buffer[260];
                            snprintf(linha, sizeof(linha), "%s - Objetivo: %s", mentorados[i].nome, mentorados[i].objetivo);
                            if (i == selected_mentorado) snprintf(buffer, sizeof(buffer), " < %s >", linha);
                            else snprintf(buffer, sizeof(buffer), " %s ", linha);
                            print_line(buffer);
                        }
                        print_line("");
                        print_line("Use as setas ou W/S para navegar. Enter para selecionar.");
                        print_border_bottom();
                        int tecla2 = capturaTecla();
                        if (tecla2 == 'w') selected_mentorado = (selected_mentorado - 1 + totalMentorados) % totalMentorados;
                        else if (tecla2 == 's') selected_mentorado = (selected_mentorado + 1) % totalMentorados;
                        else if (tecla2 == '\n') {
                            char msg[220];
                            snprintf(msg, sizeof(msg), "Deseja remover o mentorado '%s'?", mentorados[selected_mentorado].nome);
                            if (confirmarRemocao(msg)) {
                                totalMentorados = removerMentorado(&mentorados, totalMentorados, selected_mentorado);
                                salvarMentorados(mentorados, totalMentorados);
                                print_border_top();
                                print_line("Mentorado removido com sucesso.");
                                print_border_bottom();
                                esperarPressionarQ();
                                selecting = 0;
                            } else {
                                selecting = 0;
                            }
                        }
                    }
                    break;
                }
                case 6: {
                    // Encontrar Mentor Ideal (usa Gemini)
                    if (totalMentores == 0) {
                        print_border_top();
                        print_line("Nenhum mentor cadastrado. Impossível sugerir combinacao.");
                        print_border_bottom();
                        esperarPressionarQ();
                        break;
                    }
                    if (totalMentorados == 0) {
                        print_border_top();
                        print_line("Nenhum mentorado cadastrado. Selecione um mentorado primeiro.");
                        print_border_bottom();
                        esperarPressionarQ();
                        break;
                    }
                    // Seleciona mentorado
                    int selected_ment = 0;
                    int selecting = 1;
                    while (selecting) {
                        system("clear");
                        print_border_top();
                        print_line("Selecione o Mentorado para buscar o Mentor Ideal:");
                        print_line("");
                        for (int i = 0; i < totalMentorados; i++) {
                            char linha[220];
                            char buffer[260];
                            snprintf(linha, sizeof(linha), "%s - Objetivo: %s", mentorados[i].nome, mentorados[i].objetivo);
                            if (i == selected_ment) snprintf(buffer, sizeof(buffer), " < %s >", linha);
                            else snprintf(buffer, sizeof(buffer), " %s ", linha);
                            print_line(buffer);
                        }
                        print_line("");
                        print_line("Use as setas ou W/S para navegar. Enter para selecionar.");
                        print_border_bottom();
                        int tecla2 = capturaTecla();
                        if (tecla2 == 'w') selected_ment = (selected_ment - 1 + totalMentorados) % totalMentorados;
                        else if (tecla2 == 's') selected_ment = (selected_ment + 1) % totalMentorados;
                        else if (tecla2 == '\n') {
                            selecting = 0;
                        }
                    }

                    // Monta prompt para o Gemini
                    char *base = carregarPromptBase();
                    // Constrói lista de mentores no prompt
                    size_t prompt_size = strlen(base) + 1024 + (size_t)totalMentores * 200 + 1024;
                    char *prompt = malloc(prompt_size);
                    if (!prompt) {
                        free(base);
                        print_border_top();
                        print_line("Erro ao alocar memoria para prompt.");
                        print_border_bottom();
                        esperarPressionarQ();
                        break;
                    }
                    snprintf(prompt, prompt_size, "%s\n\nMentorado: %s\nObjetivo: %s\n\nMentores disponiveis:\n",
                             base, mentorados[selected_ment].nome, mentorados[selected_ment].objetivo);
                    for (int i = 0; i < totalMentores; i++) {
                        char linha[256];
                        snprintf(linha, sizeof(linha), "%d) %s — Area: %s, XP: %d anos\n", i+1, mentores[i].nome, mentores[i].area, mentores[i].experiencia);
                        strncat(prompt, linha, prompt_size - strlen(prompt) - 1);
                    }
                    strncat(prompt, "\nCom base nisso, indique 1-3 mentores ideais (por numero e nome), explique o porquê da escolha, quais passos iniciais o mentorado deve seguir e sugira uma primeira atividade/prioridade.\n", prompt_size - strlen(prompt) - 1);

                    // Chama Gemini
                    mostrarTextoMultiline("Consultando o modelo generativo (Gemini) — aguarde...");
                    char *resposta = chamarGemini(GEMINI_URL, prompt);
                    if (!resposta) {
                        mostrarTextoMultiline("Erro: falha ao chamar a API do Gemini ou resposta vazia.\n");
                    } else {
                        mostrarTextoMultiline(resposta);
                        free(resposta);
                    }

                    free(base);
                    free(prompt);
                    esperarPressionarQ();
                    break;
                }
                case 7: {
                    // Sair
                    running = 0;
                    break;
                }
                default:
                    break;
            }
        }
    }

    // Antes de sair, salva (caso tenha sido alterado) e libera memoria
    if (totalMentores > 0) salvarMentores(mentores, totalMentores);
    if (totalMentorados > 0) salvarMentorados(mentorados, totalMentorados);
    free(mentores);
    free(mentorados);

    curl_global_cleanup();
    return 0;
}

