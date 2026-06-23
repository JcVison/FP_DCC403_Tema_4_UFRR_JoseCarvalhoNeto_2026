# Edge Memory Profiler

## Simulador de Memória Virtual para Dispositivos de Borda

### Disciplina

DCC403 – Sistemas Operacionais

---

## Descrição do Projeto

O Edge Memory Profiler é um simulador de memória virtual desenvolvido em linguagem C para analisar o comportamento de sistemas de paginação sob demanda em dispositivos de borda (Edge Computing).

O sistema processa traços de memória contendo endereços virtuais em formato hexadecimal e simula o gerenciamento da memória física utilizando diferentes algoritmos de substituição de páginas.

O objetivo é avaliar o impacto das políticas de paginação no desempenho do sistema através da contabilização de métricas como Page Faults e Escritas em Disco.

---

## Objetivos

### Objetivo Geral

Desenvolver um simulador de memória virtual capaz de reproduzir o comportamento de sistemas operacionais modernos sob restrições de memória física.

### Objetivos Específicos

* Simular paginação sob demanda.
* Processar traços de memória reais.
* Calcular Virtual Page Number (VPN).
* Implementar políticas FIFO.
* Implementar políticas LRU.
* Implementar política Random.
* Controlar páginas sujas (Dirty Pages).
* Medir Page Faults.
* Medir Escritas em Disco.
* Avaliar a Anomalia de Bélády.

---

## Estrutura do Projeto

```text
FP_DCC403_Tema_4_UFRR/
│
├── Makefile
├── README.md
│
├── src/
│   └── edge_mem_profiler.c
│
├── tests/
│   ├── trace_teste.log
│   └── belady.log
│
├── docs/
│   └── relatorio.md
│
└── scripts/
    └── run_experiments.sh
```

---

## Tecnologias Utilizadas

* Linguagem C (ANSI C)
* GCC
* Makefile
* GitHub
* Valgrind

---

## Políticas de Substituição

### FIFO

First-In First-Out.

Remove a página mais antiga da memória física.

### LRU

Least Recently Used.

Remove a página menos recentemente utilizada.

### Random

Escolhe aleatoriamente uma página para remoção.

---

## Formato do Arquivo de Entrada

Exemplo:

```text
001fa3c0 R
001fa3c4 R
7f32b1a0 W
```

Onde:

* R = Leitura
* W = Escrita

---

## Compilação

```bash
make
```

---

## Execução

Formato:

```bash
./edge-mem-profiler <polisub> <arquivo.log> <sizePg> <sizeFis>
```

Exemplo:

```bash
./edge-mem-profiler lru tests/trace_teste.log 4 1024
```

---

## Parâmetros

### Política de Substituição

```text
lru
fifo
random
```

### Tamanho da Página

Valores permitidos:

```text
2 KB
4 KB
8 KB
16 KB
32 KB
64 KB
```

### Memória Física

Intervalo permitido:

```text
128 KB até 16384 KB
```

---

## Métricas Coletadas

* Total de acessos processados
* Total de Page Faults
* Total de Escritas em Disco
* Taxa de Falhas de Página
* Comparação entre políticas

---

## Testes

Executar LRU:

```bash
./edge-mem-profiler lru tests/trace_teste.log 4 1024
```

Executar FIFO:

```bash
./edge-mem-profiler fifo tests/trace_teste.log 4 1024
```

Executar Random:

```bash
./edge-mem-profiler random tests/trace_teste.log 4 1024
```

---

## Verificação com Valgrind

```bash
valgrind --leak-check=full ./edge-mem-profiler lru tests/trace_teste.log 4 1024
```

---

## Aplicação Prática

O simulador pode ser utilizado para estudar o comportamento da memória virtual em:

* Sistemas embarcados
* Internet das Coisas (IoT)
* Computação de Borda
* Sistemas de Tempo Real
* Aplicações de Inteligência Artificial
* Infraestrutura de Saúde Digital

---

## Referências

* TANENBAUM, Andrew S. Modern Operating Systems.
* STALLINGS, William. Operating Systems: Internals and Design Principles.
* HENNESSY, John; PATTERSON, David. Computer Architecture.
* KERRISK, Michael. The Linux Programming Interface.

---
