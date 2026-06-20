CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = edge-mem-profiler
SRC = src/edge_mem_profiler.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean


## Resultados Experimentais

Os experimentos foram executados utilizando as políticas FIFO, LRU e Random, variando o tamanho da página entre 4 KB, 8 KB e 16 KB, além da memória física entre 128 KB e 2048 KB.

Os resultados foram registrados no arquivo `docs/resultados.csv`. A partir desses dados, foi possível comparar a taxa de Page Faults entre as políticas de substituição.

Nos testes iniciais, observou-se que algumas políticas apresentaram resultados semelhantes quando a memória física disponível era suficiente para manter grande parte das páginas acessadas. Entretanto, ao utilizar traços maiores e cenários com maior pressão de memória, as diferenças entre FIFO, LRU e Random tornam-se mais evidentes.

## Hiperpaginação

A hiperpaginação, também conhecida como thrashing, ocorre quando o sistema passa a gastar mais tempo realizando trocas de páginas do que executando o processo propriamente dito. No contexto do simulador, esse fenômeno pode ser observado quando a taxa de Page Faults permanece muito elevada.

Quando a memória física é pequena em relação ao conjunto de páginas acessadas, o simulador registra grande quantidade de falhas de página. Isso indica que o sistema precisaria realizar muitas leituras em disco, aumentando a latência e reduzindo o desempenho.

## Anomalia de Bélády

A Anomalia de Bélády foi analisada utilizando a política FIFO com o arquivo `tests/belady.log`. O objetivo foi verificar se o aumento da quantidade de quadros poderia provocar aumento no número de Page Faults.

Esse fenômeno demonstra uma limitação da política FIFO, pois ela remove páginas apenas pela ordem de chegada, sem considerar se elas serão utilizadas novamente em breve.

## Validação

A implementação foi validada por meio de testes com diferentes políticas, tamanhos de página e tamanhos de memória física. Também foi prevista a execução com Valgrind para verificar possíveis vazamentos de memória.
