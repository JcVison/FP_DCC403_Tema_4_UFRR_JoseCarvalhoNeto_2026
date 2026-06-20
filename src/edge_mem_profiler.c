#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

typedef enum {
    POLICY_LRU,
    POLICY_FIFO,
    POLICY_RANDOM
} Policy;

typedef struct {
    unsigned long address;
    char op;
} Access;

typedef struct {
    unsigned long page;
    int valid;
    int dirty;
    unsigned long last_used;
    unsigned long arrival_time;
} Frame;

static void print_usage(const char *program_name) {
    printf("Uso correto:\n");
    printf("%s <politica> <arquivo.log> <sizePg> <sizeFis>\n", program_name);
    printf("politica: lru | fifo | random\n");
    printf("sizePg: tamanho da pagina em KB, entre 2 e 64, potencia de 2\n");
    printf("sizeFis: memoria fisica em KB, entre 128 e 16384\n");
}

static Policy parse_policy(const char *name) {
    if (strcmp(name, "lru") == 0) {
        return POLICY_LRU;
    }

    if (strcmp(name, "fifo") == 0) {
        return POLICY_FIFO;
    }

    if (strcmp(name, "random") == 0) {
        return POLICY_RANDOM;
    }

    return (Policy)-1;
}

static int is_power_of_two(long value) {
    return value > 0 && (value & (value - 1)) == 0;
}

static long parse_kilobytes(const char *value) {
    char *endptr;
    errno = 0;

    long result = strtol(value, &endptr, 10);

    if (errno != 0 || endptr == value || *endptr != '\0' || result <= 0) {
        return -1;
    }

    return result;
}

static int calculate_offset_bits(long page_size_kb) {
    long bytes = page_size_kb * 1024L;
    int bits = 0;

    while (bytes > 1) {
        bytes = bytes >> 1;
        bits++;
    }

    return bits;
}

static int load_trace(FILE *file, Access **out_accesses, size_t *out_count) {
    Access *accesses = NULL;
    size_t count = 0;
    size_t capacity = 0;
    char address_text[64];
    char op;

    while (fscanf(file, "%63s %c", address_text, &op) == 2) {
        if (op != 'R' && op != 'W') {
            continue;
        }

        unsigned long address = strtoul(address_text, NULL, 16);

        if (count == capacity) {
            size_t new_capacity = capacity == 0 ? 256 : capacity * 2;
            Access *tmp = realloc(accesses, new_capacity * sizeof(Access));

            if (tmp == NULL) {
                free(accesses);
                return 0;
            }

            accesses = tmp;
            capacity = new_capacity;
        }

        accesses[count].address = address;
        accesses[count].op = op;
        count++;
    }

    *out_accesses = accesses;
    *out_count = count;

    return 1;
}

static size_t find_page_in_frames(const Frame *frames, size_t frames_count, unsigned long page) {
    for (size_t i = 0; i < frames_count; i++) {
        if (frames[i].valid && frames[i].page == page) {
            return i;
        }
    }

    return (size_t)-1;
}

static size_t find_free_frame(const Frame *frames, size_t frames_count) {
    for (size_t i = 0; i < frames_count; i++) {
        if (!frames[i].valid) {
            return i;
        }
    }

    return (size_t)-1;
}

static size_t select_victim_fifo(const Frame *frames, size_t frames_count) {
    size_t victim = 0;
    unsigned long oldest = frames[0].arrival_time;

    for (size_t i = 1; i < frames_count; i++) {
        if (frames[i].arrival_time < oldest) {
            oldest = frames[i].arrival_time;
            victim = i;
        }
    }

    return victim;
}

static size_t select_victim_lru(const Frame *frames, size_t frames_count) {
    size_t victim = 0;
    unsigned long oldest = frames[0].last_used;

    for (size_t i = 1; i < frames_count; i++) {
        if (frames[i].last_used < oldest) {
            oldest = frames[i].last_used;
            victim = i;
        }
    }

    return victim;
}

static size_t select_victim_random(size_t frames_count) {
    return (size_t)(rand() % frames_count);
}

static size_t select_victim(const Frame *frames, size_t frames_count, Policy policy) {
    if (policy == POLICY_FIFO) {
        return select_victim_fifo(frames, frames_count);
    }

    if (policy == POLICY_LRU) {
        return select_victim_lru(frames, frames_count);
    }

    return select_victim_random(frames_count);
}

static const char *policy_to_string(Policy policy) {
    if (policy == POLICY_LRU) {
        return "lru";
    }

    if (policy == POLICY_FIFO) {
        return "fifo";
    }

    return "random";
}

static void simulate(
    const Access *accesses,
    size_t count,
    Policy policy,
    int offset_bits,
    size_t frames_count,
    unsigned long *out_faults,
    unsigned long *out_disk_writes,
    unsigned long *out_accesses
) {
    Frame *frames = calloc(frames_count, sizeof(Frame));

    unsigned long faults = 0;
    unsigned long disk_writes = 0;
    unsigned long timer = 0;

    if (frames == NULL) {
        fprintf(stderr, "Erro de alocacao de memoria.\n");
        return;
    }

    for (size_t i = 0; i < count; i++) {
        unsigned long page = accesses[i].address >> offset_bits;
        char op = accesses[i].op;

        timer++;

        size_t frame_index = find_page_in_frames(frames, frames_count, page);

        if (frame_index != (size_t)-1) {
            frames[frame_index].last_used = timer;

            if (op == 'W') {
                frames[frame_index].dirty = 1;
            }

            continue;
        }

        faults++;

        size_t free_frame = find_free_frame(frames, frames_count);

        if (free_frame != (size_t)-1) {
            frames[free_frame].page = page;
            frames[free_frame].valid = 1;
            frames[free_frame].dirty = op == 'W' ? 1 : 0;
            frames[free_frame].last_used = timer;
            frames[free_frame].arrival_time = timer;
            continue;
        }

        size_t victim = select_victim(frames, frames_count, policy);

        if (frames[victim].dirty) {
            disk_writes++;
        }

        frames[victim].page = page;
        frames[victim].valid = 1;
        frames[victim].dirty = op == 'W' ? 1 : 0;
        frames[victim].last_used = timer;
        frames[victim].arrival_time = timer;
    }

    free(frames);

    *out_faults = faults;
    *out_disk_writes = disk_writes;
    *out_accesses = (unsigned long)count;
}

static void print_report(
    Policy policy,
    const char *filename,
    long page_size_kb,
    long physical_size_kb,
    size_t frames_count,
    unsigned long total_accesses,
    unsigned long faults,
    unsigned long disk_writes
) {
    double fault_rate = 0.0;

    if (total_accesses > 0) {
        fault_rate = ((double)faults / (double)total_accesses) * 100.0;
    }

    printf("\nExecutando Edge-AI Memory Performance Profiler...\n");
    printf("==================================================\n");
    printf("Configuracao do Sistema de Borda:\n");
    printf("Tecnica de Reposicao (Polisub): %s\n", policy_to_string(policy));
    printf("Arquivo de Traco Processado: %s\n", filename);
    printf("Tamanho de Cada Pagina/Quadro: %ld KB\n", page_size_kb);
    printf("Capacidade de Memoria Fisica: %ld KB (%zu quadros)\n", physical_size_kb, frames_count);
    printf("--------------------------------------------------\n");
    printf("Resultados Metricos da Simulacao:\n");
    printf("Total de Acessos Computados: %lu\n", total_accesses);
    printf("Ocorrencias de Page Faults (Lidas): %lu\n", faults);
    printf("Paginas Sujas Evictadas (Escritas): %lu\n", disk_writes);
    printf("Taxa de Page Faults: %.2f%%\n", fault_rate);
    printf("==================================================\n");
    printf("Simulacao Concluida com Sucesso.\n\n");
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    Policy policy = parse_policy(argv[1]);

    if (policy == (Policy)-1) {
        fprintf(stderr, "Politica desconhecida: %s\n", argv[1]);
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    long page_size_kb = parse_kilobytes(argv[3]);
    long physical_size_kb = parse_kilobytes(argv[4]);

    if (page_size_kb <= 0 || physical_size_kb <= 0) {
        fprintf(stderr, "Tamanho de pagina e memoria fisica devem ser inteiros positivos.\n");
        return EXIT_FAILURE;
    }

    if (!is_power_of_two(page_size_kb) || page_size_kb < 2 || page_size_kb > 64) {
        fprintf(stderr, "Erro: tamanho da pagina deve ser potencia de 2 entre 2 KB e 64 KB.\n");
        return EXIT_FAILURE;
    }

    if (physical_size_kb < 128 || physical_size_kb > 16384) {
        fprintf(stderr, "Erro: memoria fisica deve estar entre 128 KB e 16384 KB.\n");
        return EXIT_FAILURE;
    }

    if (physical_size_kb < page_size_kb) {
        fprintf(stderr, "Erro: memoria fisica deve ser maior ou igual ao tamanho da pagina.\n");
        return EXIT_FAILURE;
    }

    if (physical_size_kb % page_size_kb != 0) {
        fprintf(stderr, "Erro: memoria fisica deve ser multiplo do tamanho da pagina.\n");
        return EXIT_FAILURE;
    }

    size_t frames_count = (size_t)(physical_size_kb / page_size_kb);
    int offset_bits = calculate_offset_bits(page_size_kb);

    FILE *file = fopen(argv[2], "r");

    if (file == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo '%s': %s\n", argv[2], strerror(errno));
        return EXIT_FAILURE;
    }

    Access *accesses = NULL;
    size_t count = 0;

    if (!load_trace(file, &accesses, &count)) {
        fprintf(stderr, "Erro ao carregar o arquivo de trace.\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    fclose(file);

    srand((unsigned int)time(NULL));

    unsigned long faults = 0;
    unsigned long disk_writes = 0;
    unsigned long total_accesses = 0;

    simulate(
        accesses,
        count,
        policy,
        offset_bits,
        frames_count,
        &faults,
        &disk_writes,
        &total_accesses
    );

    print_report(
        policy,
        argv[2],
        page_size_kb,
        physical_size_kb,
        frames_count,
        total_accesses,
        faults,
        disk_writes
    );

    free(accesses);

    return EXIT_SUCCESS;
}
