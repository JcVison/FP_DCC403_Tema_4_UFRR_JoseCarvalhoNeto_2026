# Resultados Experimentais

## Trace de Teste

Configuração:

- Página: 4 KB
- Memória Física: 128 KB
- Quadros: 32

Resultados:

| Política | Page Faults | Taxa |
|-----------|-------------|--------|
| LRU | 8 | 80,00% |
| FIFO | 8 | 80,00% |
| Random | 8 | 80,00% |

Observação:

Como a memória física possui quantidade suficiente de quadros para armazenar as páginas acessadas, as três políticas apresentaram comportamento semelhante.

---

## Análise da Anomalia de Bélády

Trace utilizado:

tests/belady.log

Resultados obtidos utilizando FIFO:

| Quadros | Page Faults |
|----------|-------------|
| 3 | 9 |
| 4 | 10 |
| 5 | 5 |

Análise:

Ao aumentar a memória de 3 para 4 quadros, o número de Page Faults aumentou de 9 para 10.

Esse comportamento caracteriza a Anomalia de Bélády, fenômeno observado em algoritmos FIFO, onde o aumento da memória física não garante redução das falhas de página.

---

## Hiperpaginação

A hiperpaginação (thrashing) ocorre quando a memória física é insuficiente para manter o conjunto de páginas necessárias à execução do programa.

Nessas situações, o sistema passa a realizar um número excessivo de Page Faults, gastando mais tempo carregando páginas do que executando instruções.

Os experimentos demonstraram que a redução da memória física aumenta a frequência de Page Faults, aproximando o sistema de cenários de hiperpaginação.

---

## Conclusão

O algoritmo LRU apresentou comportamento mais consistente em cenários com reutilização de páginas.

O algoritmo FIFO apresentou a ocorrência da Anomalia de Bélády.

O algoritmo Random apresentou resultados variáveis dependendo da execução.

A ferramenta desenvolvida permite analisar o comportamento da memória virtual e compreender o impacto das políticas de substituição em dispositivos de borda.
