# Relatório: Mini-Projeto 1 - Quebra-Senhas Paralelo

**Aluno(s):** Bruna Amorim Maia (10431883), Fabyani Tiva Yan (10431835), Rafael Araujo Cabral Moreira (10441919), Rute Willemann (10436781) 
---

## 1. Estratégia de Paralelização


**Como você dividiu o espaço de busca entre os workers?**

O coordinator calcula o número total de combinações de senhas possíveis e divide essa quantidade igualmente entre o número de workers. Ele então define um intervalo de senhas de início e fim para cada worker e os passa como argumentos quando cria os processos filhos com fork() e exec(). Cada worker é responsável por executar a busca por força bruta apenas em seu intervalo designado, incrementando a senha de forma incremental com base no charset até encontrar o hash alvo ou atingir o fim de seu espaço de busca.

**Código relevante:** Cole aqui a parte do coordinator.c onde você calcula a divisão:
```c
long long total_passwords = total_space;

     //divisao entre os workers
     long long passwords_per_worker = total_passwords / num_workers;  // para o quociente
     long long remaining = total_passwords % num_workers;  // para o resto
   
    // Arrays para armazenar PIDs dos workers
    pid_t workers[MAX_WORKERS];
    
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
   index_to_password(start_idx, charset, charset_len, password_len, start_pwd);
   index_to_password(end_idx,   charset, charset_len, password_len, end_pwd);

   // ... parte de fork/exec ...

    // proximo range comeca apos o fim do atual
    start_idx = end_idx + 1;
}
```

---

## 2. Implementação das System Calls

**Descreva como você usou fork(), execl() e wait() no coordinator:**

No coordinator, utilizamos o fork() para criar multiplos processos filhos, cada um responsável por uma parte do espaço de busca. Em cada filho, chamamos o execl() para executar o programa worker, repassando todos os argumentos necessários(hash alvo, a senha inicial e final do intervalo, o conjunto de caracteres, o tamanho da senha e o identificador do worker). No processo pai, usamos o wait() em um loop para aguardar a finalização de todos os filhos, garantindo que cada término fosse devidamente coletado pelo sistema.

**Código do fork/exec:**
```c
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
   index_to_password(start_idx, charset, charset_len, password_len, start_pwd);
   index_to_password(end_idx,   charset, charset_len, password_len, end_pwd);


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
```

---

## 3. Comunicação Entre Processos

**Como você garantiu que apenas um worker escrevesse o resultado?**

A escrita atômica foi implementada usando a chamada de sistema open() com as flags O_CREAT e O_EXCL. A flag O_CREAT cria o arquivo, e O_EXCL garante que a chamada falhe se o arquivo já existir. Dessa forma, o primeiro worker que encontra a senha consegue criar o arquivo e registrar o resultado, enquanto os outros falham ao tentar criar o mesmo arquivo e são instruídos a parar a busca.

**Como o coordinator consegue ler o resultado?**

O coordinator consegue ler o resultado depois que todos os workers terminaram. Ele tenta abrir o arquivo password_found.txt no modo leitura. Se ele existir, o coordinator lê para um buffer e faz o parse da string, que está no formato "id_worker:password". A string então é dividida usando strchr(), para encontrar os dois pontos (:). Depois de extrair o ID do worker e a senha, o coordinator valida o resultado calculando o hash da senha encontrada, e comparando com o hash alvo.

---

## 4. Análise de Performance
Complete a tabela com tempos reais de execução:
O speedup é o tempo do teste com 1 worker dividido pelo tempo com 4 workers.

| Teste | 1 Worker | 2 Workers | 4 Workers | Speedup (4w) |
|-------|----------|-----------|-----------|--------------|
| Hash: 202cb962ac59075b964b07152d234b70<br>Charset: "0123456789"<br>Tamanho: 3<br>Senha: "123" | 0.005s | 0.007s | 0.009s | 0.55 |
| Hash: 5d41402abc4b2a76b9719d911017c592<br>Charset: "abcdefghijklmnopqrstuvwxyz"<br>Tamanho: 5<br>Senha: "hello" | 4.391s | 7.708s | 1.699s | 2.58 |

**O speedup foi linear? Por quê?**
Para o teste com a senha "123", o speedup não foi linear, com o tempo de execução aumentando de 0.005s para 0.009s. Isso ocorre porque a busca é muito rápida e o tempo de execução é dominado pelo overhead de criar e gerenciar os processos filhos, que é um custo fixo do sistema.

Já para o teste de estresse com a senha "hello", o speedup foi de 2.58x, sendo uma melhora significativa, mas não perfeitamente linear. O resultado com 2 workers (7.708s) foi, na verdade, maior do que com 1 worker (4.391s). Essa não linearidade e até a degradação de performance são explicadas pelo mesmo overhead de fork() e exec() e pelo tempo de sincronização para que o coordinator aguarde todos os processos. Aceleração real só é vista quando o tempo de computação de cada worker se torna muito maior do que o tempo de gerenciamento dos processos, como no caso do teste com 4 workers, onde o speedup foi de 2.58x.

---

## 5. Desafios e Aprendizados
**Qual foi o maior desafio técnico que você enfrentou?**

O maior desafio foi garantir que o coordinator e os workers ficassem sincronizados. O problema dos processos zumbis foi resolvido usando a chamada de sistema wait() em um loop, para assegurar que o status de cada filho foi coletado, liberando seus recursos, e evitar que múltiplos workers escrevam o resultado simultaneamente no arquivo. Isso é garantido com as flags O_CREAT e O_EXCL na função open(), garantindo que somente o primeiro worker a encontrar a senha consiga criar o arquivo, enquanto os outros param a busca.

---

## Comandos de Teste Utilizados

```bash
# Teste básico
./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 2

# Teste de performance
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 1
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 4

# Teste com senha maior
time ./coordinator "5d41402abc4b2a76b9719d911017c592" 5 "abcdefghijklmnopqrstuvwxyz" 4
```
---

**Checklist de Entrega:**
- [ ] Código compila sem erros
- [ ] Todos os TODOs foram implementados
- [ ] Testes passam no `./tests/simple_test.sh`
- [ ] Relatório preenchido
