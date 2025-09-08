# Relatório: Mini-Projeto 1 - Quebra-Senhas Paralelo

**Aluno(s):** Bruna Amorim Maia (10431883), Fabyani Tiva Yan (10431835), Rafael Araujo Cabral Moreira (10441919), Rute Willemann (10436781) 
---

## 1. Estratégia de Paralelização


**Como você dividiu o espaço de busca entre os workers?**

[Explique seu algoritmo de divisão]

**Código relevante:** Cole aqui a parte do coordinator.c onde você calcula a divisão:
```c
// Cole seu código de divisão aqui
```

---

## 2. Implementação das System Calls

**Descreva como você usou fork(), execl() e wait() no coordinator:**

[Explique em um parágrafo como você criou os processos, passou argumentos e esperou pela conclusão]

**Código do fork/exec:**
```c
// Cole aqui seu loop de criação de workers
```

---

## 3. Comunicação Entre Processos

**Como você garantiu que apenas um worker escrevesse o resultado?**

[Explique como você implementou uma escrita atômica e como isso evita condições de corrida]
Leia sobre condições de corrida (aqui)[https://pt.stackoverflow.com/questions/159342/o-que-%C3%A9-uma-condi%C3%A7%C3%A3o-de-corrida]

**Como o coordinator consegue ler o resultado?**

O coordinator consegue ler o resultado depois que todos os workers terminaram. Ele tenta abrir o arquivo password_found.txt no modo leitura. Se ele existir, o coordinator lê para um buffer e faz o parse da string, que está no formato "id_worker:password". A string então é dividida usando strchr(), para encontrar os dois pontos (:). Depois de extrair o ID do worker e a senha, o coordinator valida o resultado calculando o hash da senha encontrada, e comparando com o hash alvo.

---

## 4. Análise de Performance
Complete a tabela com tempos reais de execução:
O speedup é o tempo do teste com 1 worker dividido pelo tempo com 4 workers.

| Teste | 1 Worker | 2 Workers | 4 Workers | Speedup (4w) |
|-------|----------|-----------|-----------|--------------|
| Hash: 202cb962ac59075b964b07152d234b70<br>Charset: "0123456789"<br>Tamanho: 3<br>Senha: "123" | ___s | ___s | ___s | ___ |
| Hash: 5d41402abc4b2a76b9719d911017c592<br>Charset: "abcdefghijklmnopqrstuvwxyz"<br>Tamanho: 5<br>Senha: "hello" | ___s | ___s | ___s | ___ |

**O speedup foi linear? Por quê?**
[Analise se dobrar workers realmente dobrou a velocidade e explique o overhead de criar processos]

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
