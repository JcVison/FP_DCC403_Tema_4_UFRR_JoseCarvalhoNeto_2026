# Desenvolvimento de um Motor Analítico e Simulador Matemático de Comportamento de Memória Virtual para Dispositivos de Borda

## José Carvalho Neto

Universidade Federal de Roraima (UFRR)

---

# Resumo

O crescimento da Computação de Borda (Edge Computing) e das aplicações de Inteligência Artificial tem aumentado significativamente a demanda por sistemas computacionais capazes de operar com eficiência sob severas restrições de hardware. Em muitos cenários, dispositivos embarcados possuem memória física limitada, tornando o gerenciamento de memória um fator crítico para o desempenho da aplicação. Neste contexto, este trabalho apresenta o desenvolvimento do Edge Memory Profiler, um simulador de memória virtual implementado em linguagem C capaz de reproduzir o comportamento de sistemas de paginação sob demanda. A ferramenta processa traços de memória contendo endereços virtuais e simula a execução de três políticas clássicas de substituição de páginas: FIFO, LRU e Random. O sistema registra métricas como quantidade de falhas de página (Page Faults), páginas sujas removidas e taxa de falhas de memória, permitindo uma análise detalhada do impacto das políticas de substituição em ambientes com memória física limitada.

**Palavras-chave:** Memória Virtual, Sistemas Operacionais, Paginação, Edge Computing, FIFO, LRU, Random.

---

# 1. Introdução

A evolução dos sistemas computacionais modernos permitiu a disseminação de aplicações que processam grandes volumes de dados em tempo real. Tecnologias como Internet das Coisas (IoT), Inteligência Artificial, Visão Computacional e Sistemas Embarcados passaram a exigir soluções capazes de realizar processamento próximo à origem dos dados. Essa abordagem, conhecida como Computação de Borda, reduz latência, diminui o tráfego de rede e aumenta a disponibilidade dos serviços.

Entretanto, os dispositivos utilizados nesses ambientes normalmente apresentam limitações significativas de hardware. Entre essas limitações, a memória RAM disponível constitui um dos principais gargalos de desempenho. Quando a memória física não é suficiente para armazenar todas as páginas necessárias à execução de um processo, o sistema operacional utiliza mecanismos de memória virtual para ampliar o espaço de endereçamento disponível.

A memória virtual funciona através da divisão do espaço de endereçamento em páginas. Quando uma página solicitada pelo processo não está presente na memória principal, ocorre uma falha de página, conhecida como Page Fault. Nessa situação, o sistema operacional precisa buscar a página na memória secundária, provocando aumento de latência e degradação de desempenho.

Em cenários extremos, a ocorrência excessiva de falhas de página pode desencadear o fenômeno denominado Thrashing, caracterizado pela intensa movimentação de páginas entre memória principal e armazenamento secundário. Esse comportamento reduz drasticamente o desempenho do sistema e aumenta o desgaste de dispositivos de armazenamento.

Diante desse contexto, torna-se importante compreender o comportamento dos algoritmos de substituição de páginas e sua influência sobre o desempenho dos sistemas computacionais. Este trabalho propõe o desenvolvimento de um simulador de memória virtual capaz de reproduzir diferentes políticas de substituição e fornecer métricas quantitativas para análise de desempenho.

---

# 2. Objetivos

## 2.1 Objetivo Geral

Desenvolver um simulador de memória virtual capaz de reproduzir o comportamento de sistemas de paginação sob demanda em dispositivos de borda, permitindo avaliar o impacto de diferentes políticas de substituição de páginas.

## 2.2 Objetivos Específicos

* Implementar o simulador utilizando linguagem ANSI C.
* Processar traços de memória contendo endereços em hexadecimal.
* Calcular números de páginas virtuais através de manipulação de bits.
* Implementar as políticas FIFO, LRU e Random.
* Gerenciar páginas sujas utilizando Dirty Bit.
* Registrar Page Faults e Escritas em Disco.
* Avaliar o impacto do tamanho das páginas.
* Avaliar o impacto da memória física disponível.
* Investigar experimentalmente a Anomalia de Bélády.

---

# 3. Fundamentação Teórica

## 3.1 Memória Virtual

A memória virtual é uma técnica utilizada pelos sistemas operacionais para fornecer aos processos um espaço de endereçamento maior do que a memória física disponível. Essa abstração permite que programas sejam executados independentemente da quantidade de memória RAM instalada.

O espaço de endereçamento é dividido em páginas de tamanho fixo. Cada página pode ser carregada para a memória física somente quando necessária, estratégia conhecida como Paginação Sob Demanda.

## 3.2 Page Fault

Um Page Fault ocorre quando um processo tenta acessar uma página que não está carregada na memória física. Nessa situação, o sistema operacional interrompe temporariamente a execução do processo e realiza a leitura da página a partir da memória secundária.

Embora necessário para o funcionamento da memória virtual, um grande número de Page Faults pode comprometer significativamente o desempenho do sistema.

## 3.3 Dirty Bit

O Dirty Bit é um indicador associado a cada página carregada na memória física. Quando uma página sofre uma operação de escrita, ela passa a ser considerada modificada.

Ao ser removida da memória, uma página marcada como suja deve ser escrita novamente no armazenamento secundário antes de sua substituição, gerando custo adicional de entrada e saída.

## 3.4 Algoritmos de Substituição

Quando não existem quadros livres na memória física, torna-se necessário escolher uma página para remoção.

### FIFO

A política FIFO (First-In First-Out) remove a página que está há mais tempo na memória. Sua implementação é simples, porém pode apresentar desempenho inferior em alguns cenários.

### LRU

A política LRU (Least Recently Used) remove a página utilizada há mais tempo. Essa estratégia explora o princípio da localidade temporal e costuma apresentar melhores resultados.

### Random

A política Random seleciona aleatoriamente a página que será removida. Apesar da simplicidade, seus resultados podem variar significativamente entre execuções.

---

# 4. Desenvolvimento da Solução

O simulador foi implementado integralmente em linguagem ANSI C seguindo uma abordagem procedural. O sistema recebe quatro parâmetros obrigatórios pela linha de comando:

```bash
./edge-mem-profiler <polisub> <arquivo.log> <sizePg> <sizeFis>
```

Onde:

* polisub: política de substituição (FIFO, LRU ou Random);
* arquivo.log: traço de memória;
* sizePg: tamanho da página em KB;
* sizeFis: tamanho da memória física em KB.

A memória física foi modelada através de um vetor de estruturas denominado Frame. Cada elemento armazena informações relacionadas à página carregada, incluindo número da página virtual, bit de validade, Dirty Bit e informações de histórico de acesso.

O cálculo do Virtual Page Number foi realizado através de operações de deslocamento de bits. Inicialmente é calculada a quantidade de bits correspondente ao deslocamento da página. Em seguida, o endereço virtual é deslocado para a direita, removendo o offset e obtendo o identificador da página virtual.

Durante a execução, cada acesso é classificado como Hit ou Page Fault. Em caso de falha de página, o sistema seleciona um quadro livre ou utiliza a política de substituição configurada para remover uma página existente.

---

# 5. Metodologia Experimental

Para validar o simulador foram utilizados arquivos de traço contendo acessos de leitura e escrita.

Os experimentos foram executados utilizando as três políticas de substituição:

```bash
./edge-mem-profiler fifo trace.log 4 1024
./edge-mem-profiler lru trace.log 4 1024
./edge-mem-profiler random trace.log 4 1024
```

Também foram realizados testes variando:

* tamanho da página: 4 KB, 8 KB e 16 KB;
* memória física: 128 KB, 256 KB, 512 KB, 1024 KB e 2048 KB.

As métricas observadas foram:

* Total de acessos;
* Quantidade de Page Faults;
* Quantidade de Escritas em Disco;
* Taxa de Falhas de Página.

---

# 6. Resultados e Discussão

Os resultados demonstraram que o algoritmo LRU apresentou menor quantidade de Page Faults na maioria dos cenários avaliados. Esse comportamento está relacionado à exploração do princípio da localidade temporal, mantendo na memória páginas recentemente utilizadas.

O algoritmo FIFO apresentou resultados intermediários, porém demonstrou sensibilidade à ordem dos acessos. Em determinadas situações, páginas importantes foram removidas prematuramente, aumentando o número de falhas.

A política Random apresentou comportamento mais instável, variando entre execuções devido à natureza probabilística da seleção de vítimas.

A análise também demonstrou que o aumento da memória física reduz significativamente a ocorrência de falhas de página, embora esse comportamento não seja garantido para todas as políticas de substituição.

---

# 7. Anomalia de Bélády

A Anomalia de Bélády é um fenômeno observado principalmente em algoritmos FIFO. Ela ocorre quando o aumento da quantidade de quadros disponíveis provoca um aumento inesperado no número de Page Faults.

Para investigar esse comportamento, foram realizados experimentos utilizando o mesmo traço de memória sob diferentes capacidades de memória física. Os resultados demonstraram que o algoritmo FIFO pode apresentar crescimento no número de falhas mesmo após a ampliação da memória disponível.

Esse comportamento evidencia limitações estruturais da política FIFO e reforça a importância da utilização de algoritmos que considerem informações de uso recente.

---

## 8. Validação com Valgrind

O simulador foi validado utilizando a ferramenta Valgrind para detecção de vazamentos de memória e acessos inválidos.

O teste foi executado através do comando:

valgrind --leak-check=full ./edge-mem-profiler lru tests/trace_teste.log 4 128

Os resultados indicaram:

- 0 bytes em uso ao final da execução;
- ausência de vazamentos de memória;
- nenhuma ocorrência de acesso inválido.

Dessa forma, a implementação atende aos requisitos de gerenciamento seguro de memória exigidos para aplicações em linguagem C.

---
# 9. Conclusão

O desenvolvimento do Edge Memory Profiler permitiu aplicar conceitos fundamentais de Sistemas Operacionais relacionados ao gerenciamento de memória virtual. A implementação das políticas FIFO, LRU e Random possibilitou comparar diferentes estratégias de substituição de páginas sob condições de memória limitada.

Os experimentos demonstraram que a escolha da política de substituição possui impacto direto sobre o desempenho do sistema, influenciando a quantidade de falhas de página e operações de entrada e saída.

Além de contribuir para a compreensão dos mecanismos internos dos sistemas operacionais, a ferramenta desenvolvida possui aplicação prática em dispositivos embarcados, sistemas de tempo real e ambientes de Computação de Borda.

---

# Referências

KERRISK, Michael. The Linux Programming Interface. No Starch Press, 2010.

STALLINGS, William. Operating Systems: Internals and Design Principles. Pearson, 2018.

TANENBAUM, Andrew S.; BOS, Herbert. Modern Operating Systems. Pearson, 2015.

HENNESSY, John L.; PATTERSON, David A. Arquitetura de Computadores: Uma Abordagem Quantitativa. Elsevier, 2014.

UFMG. Trabalho Prático 3 - Gerência de Memória. Disponível em: https://homepages.dcc.ufmg.br/~dorgival/cursos/so/tp3.html
