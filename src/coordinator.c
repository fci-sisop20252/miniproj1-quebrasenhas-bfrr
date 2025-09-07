#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include "hash_utils.h"

/**
 * PROCESSO COORDENADOR - Mini-Projeto 1: Quebra de Senhas Paralelo
 * 
 * Este programa coordena múltiplos workers para quebrar senhas MD5 em paralelo.
 * O MD5 JÁ ESTÁ IMPLEMENTADO - você deve focar na paralelização (fork/exec/wait).
 * 
 * Uso: ./coordinator <hash_md5> <tamanho> <charset> <num_workers>
 * 
 * Exemplo: ./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 4
 * 
 * SEU TRABALHO: Implementar os TODOs marcados abaixo
 */

#define MAX_WORKERS 16
#define RESULT_FILE "password_found.txt"

/**
 * Calcula o tamanho total do espaço de busca
 * 
 * @param charset_len Tamanho do conjunto de caracteres
 * @param password_len Comprimento da senha
 * @return Número total de combinações possíveis
 */
long long calculate_search_space(int charset_len, int password_len) {
    long long total = 1;
    for (int i = 0; i < password_len; i++) {
        total *= charset_len;
    }
    return total;
}

/**
 * Converte um índice numérico para uma senha
 * Usado para definir os limites de cada worker
 * 
 * @param index Índice numérico da senha
 * @param charset Conjunto de caracteres
 * @param charset_len Tamanho do conjunto
 * @param password_len Comprimento da senha
 * @param output Buffer para armazenar a senha gerada
 */
void index_to_password(long long index, const char *charset, int charset_len, 
                       int password_len, char *output) {
    for (int i = password_len - 1; i >= 0; i--) {
        output[i] = charset[index % charset_len];
        index /= charset_len;
    }
    output[password_len] = '\0';
}

/**
 * Função principal do coordenador
 */
int main(int argc, char *argv[]) {
    // TODO 1: Validar argumentos de entrada//***************FEITO*******************
    // Verificar se argc == 5 (programa + 4 argumentos)
    // Se não, imprimir mensagem de uso e sair com código 1
    
    // IMPLEMENTE AQUI: verificação de argc e mensagem de erro   
    if (argc != 5){
     printf("ERRO! USO INVALIDO!\n");
            return 1;
    }
    
    // Parsing dos argumentos (após validação)
    const char *target_hash = argv[1];
    int password_len = atoi(argv[2]);
    const char *charset = argv[3];
    int num_workers = atoi(argv[4]);
    int charset_len = strlen(charset);
    
    // TODO: Adicionar validações dos parâmetros//***************FEITO*******************
    // - password_len deve estar entre 1 e 10
    // - num_workers deve estar entre 1 e MAX_WORKERS
    // - charset não pode ser vazio

    if (password_len < 1 || password_len > 10) {
        printf("Erro! Tamanho da senha deve estar entre 1 e 10!\n");
        return 1;
    }
    if (num_workers < 1 || num_workers > MAX_WORKERS) {
        printf("Erro! Número de workers deve estar entre 1 e %d!\n", MAX_WORKERS);
        return 1;
    }
    if (charset_len == 0) {
        printf("Erro! Charset não pode ser vazio!\n");
        return 1;
    }
    
    printf("=== Mini-Projeto 1: Quebra de Senhas Paralelo ===\n");
    printf("Hash MD5 alvo: %s\n", target_hash);
    printf("Tamanho da senha: %d\n", password_len);
    printf("Charset: %s (tamanho: %d)\n", charset, charset_len);
    printf("Número de workers: %d\n", num_workers);
    
    // Calcular espaço de busca total
    long long total_space = calculate_search_space(charset_len, password_len);
    printf("Espaço de busca total: %lld combinações\n\n", total_space);
    
    // Remover arquivo de resultado anterior se existir
    unlink(RESULT_FILE);
    
    // Registrar tempo de início
    time_t start_time = time(NULL);
    
    // TODO 2: Dividir o espaço de busca entre os workers //***************FEITO*******************
    // Calcular quantas senhas cada worker deve verificar
    // DICA: Use divisão inteira e distribua o resto entre os primeiros workers
    
    // IMPLEMENTE AQUI:
    // long long passwords_per_worker = ?
    // long long remaining = ?
    
    // total de combinacoes no espaço de busca
     long long total_passwords = 1;
     for (int i = 0; i < password_len; i++) {
     total_passwords *= charset_len;
    }

     //divisao entre os workers
     long long passwords_per_worker = total_passwords / num_workers;  // para o quociente
     long long remaining = total_passwords % num_workers;  // para o resto
   
    // Arrays para armazenar PIDs dos workers
    pid_t workers[MAX_WORKERS];
    
    // TODO 3: Criar os processos workers usando fork()
    printf("Iniciando workers...\n");
    
    // IMPLEMENTE AQUI: Loop para criar workers
    for (int i = 0; i < num_workers; i++) {
        // TODO: Calcular intervalo de senhas para este worker //***************FEITO*******************
        // TODO: Converter indices para senhas de inicio e fim//***************FEITO*******************
        // TODO 4: Usar fork() para criar processo filho //***************FEITO*******************
        // TODO 5: No processo pai: armazenar PID //***************FEITO*******************
        // TODO 6: No processo filho: usar execl() para executar worker //***************FEITO*******************
        // TODO 7: Tratar erros de fork() e execl() //***************FEITO*******************
    }

    // TODO 3: Criar os processos workers usando fork()
    printf("Iniciando workers...\n");

    //ira controlar o inicio do range deste worker em indice linear
    long long start_idx = 0;

    for (int i = 0; i < num_workers; i++) {
    // TODO: Calcular intervalo de senhas para este worker
       long long count   = passwords_per_worker + (i < remaining ? 1 : 0);
       long long end_idx = (count > 0) ? (start_idx + count - 1) : (start_idx - 1);

    //se nao tem trabalho para este worker, siga adiante
    if (count <= 0) { workers[i] = -1; continue; }

    // TODO: Converter indices para senhas de inicio e fim
    char start_pwd[128], end_pwd[128];
    long long tmp;

    //converte o indice do idx(inicial) em uma senha real chamada pwd
    tmp = start_idx;
    for (int pos = password_len - 1; pos >= 0; pos--) {
        start_pwd[pos] = charset[tmp % charset_len];
        tmp /= charset_len;
    }
    start_pwd[password_len] = '\0';

    //converte o indice do idx(final) em uma senha real chamada pwd
    tmp = end_idx;
    for (int pos = password_len - 1; pos >= 0; pos--) {
        end_pwd[pos] = charset[tmp % charset_len];
        tmp /= charset_len;
    }
    end_pwd[password_len] = '\0';

    // TODO 4: Usar fork() para criar processo filho
    pid_t pid = fork();
    if (pid < 0) {
        // TODO 7: Tratar erro de fork()
        perror("fork");
        return 1;
    } else if (pid == 0) {
        // TODO 6: No processo filho: usar execl() para executar worker
        char n_str[16], wid_str[16];
        snprintf(n_str,  sizeof n_str,  "%d", password_len);
        snprintf(wid_str, sizeof wid_str, "%d", i);

        execl("./worker", "worker",target_hash, start_pwd, end_pwd, charset, n_str, wid_str, (char*)NULL);

        // TODO 7: Tratar erro de execl()
        perror("execl");
        _exit(1);
    } else {
        // TODO 5: No processo pai: armazenar PID
        workers[i] = pid;
        // printf("[Coordinator] W%d (PID %d) -> [%s .. %s]\n", i, pid, start_pwd, end_pwd); (para testar quem e o worker e qual parte do espaco de busca ele vai cobrir)
    }

    // proximo range comeca apos o fim do atual
    start_idx = end_idx + 1;
}
 
    printf("\nTodos os workers foram iniciados. Aguardando conclusão...\n");
    
    // TODO 8: Aguardar todos os workers terminarem usando wait()
    // IMPORTANTE: O pai deve aguardar TODOS os filhos para evitar zumbis
    
    // IMPLEMENTE AQUI:
    // - Loop para aguardar cada worker terminar
    // - Usar wait() para capturar status de saída
    // - Identificar qual worker terminou
    // - Verificar se terminou normalmente ou com erro
    // - Contar quantos workers terminaram
    
    // Registrar tempo de fim
    time_t end_time = time(NULL);
    double elapsed_time = difftime(end_time, start_time);
    
    printf("\n=== Resultado ===\n");
    
    // TODO 9: Verificar se algum worker encontrou a senha
    // Ler o arquivo password_found.txt se existir
    
    // IMPLEMENTE AQUI:
    // - Abrir arquivo RESULT_FILE para leitura
    // - Ler conteúdo do arquivo
    // - Fazer parse do formato "worker_id:password"
    // - Verificar o hash usando md5_string()
    // - Exibir resultado encontrado
    
    // Estatísticas finais (opcional)
    // TODO: Calcular e exibir estatísticas de performance
    
    return 0;
}
