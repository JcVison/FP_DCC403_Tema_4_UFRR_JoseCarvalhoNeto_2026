#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef enum {
    POLICY_LRU,
    POLICY_FIFO,
    POLICY_OPTIMAL
} Policy;

typedef struct {
    unsigned long address;
    char op;
} Access;

static void print_usage(const char *program_name) {
    printf("Uso correto:\n");
    printf("%s <politica> <arquivo.log> <sizePg> <sizeFis>\n", program_name);
    printf("politica: lru | fifo | optimal | belady\n");
    printf("sizePg: tamanho da pagina em KB\n");
    printf("sizeFis: memoria fisica em KB\n");
}

static Policy parse_policy(const char *name) {
    if (strcmp(name, "lru") == 0) {
        return POLICY_LRU;
    }
    if (strcmp(name, "fifo") == 0) {
        return POLICY_FIFO;
    }
    if (strcmp(name, "optimal") == 0 || strcmp(name, "belady") == 0) {
        return POLICY_OPTIMAL;
    }
    return (Policy)-1;
}

static long parse_kilobytes(const char *value) {
    char *endptr;
    errno = 0;
    long result = strtol(value, &endptr, 10);
    if (errno != 0 || endptr == value || *endptr != '\0' || result <= 0) {
        return -1;
    }
    return result * 1024L;
}

static int load_trace(FILE *file, Access **out_accesses, size_t *out_count) {
    Access *accesses = NULL;
    size_t count = 0;
    size_t capacity = 0;
    char address[64];
    char op;

    while (fscanf(file, "%63s %c", address, &op) == 2) {
        unsigned long value = strtoul(address, NULL, 0);
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
        accesses[count].address = value;
        accesses[count].op = op;
        count++;
    }

    *out_accesses = accesses;
    *out_count = count;
    return 1;
}

static size_t find_page_in_frames(const unsigned long *frames, size_t loaded, unsigned long page) {
    for (size_t i = 0; i < loaded; ++i) {
        if (frames[i] == page) {
            return i;
        }
    }
    return (size_t)-1;
}

static size_t select_victim_fifo(size_t *fifo_position, size_t frames_count) {
    size_t victim = *fifo_position;
    *fifo_position = (*fifo_position + 1) % frames_count;
    return victim;
}

static size_t select_victim_lru(const unsigned long *last_used, size_t frames_count) {
    size_t victim = 0;
    unsigned long oldest = last_used[0];
    for (size_t i = 1; i < frames_count; ++i) {
        if (last_used[i] < oldest) {
            oldest = last_used[i];
            victim = i;
        }
    }
    return victim;
}

static size_t select_victim_optimal(const Access *accesses, size_t count, size_t current_index,
                                    const unsigned long *frames, size_t frames_count,
                                    unsigned long page_size) {
    size_t victim = 0;
    unsigned long farthest = 0;

    for (size_t i = 0; i < frames_count; ++i) {
        unsigned long frame_page = frames[i];
        unsigned long next_occurrence = count;
        for (size_t j = current_index + 1; j < count; ++j) {
            if (accesses[j].address / page_size == frame_page) {
                next_occurrence = j;
                break;
            }
        }
        if (next_occurrence > farthest) {
            farthest = next_occurrence;
            victim = i;
        }
    }
    return victim;
}

static void simulate(const Access *accesses, size_t count, Policy policy,
                     unsigned long page_size, size_t frames_count,
                     unsigned long *out_faults, unsigned long *out_accesses) {
    unsigned long *frames = calloc(frames_count, sizeof(unsigned long));
    unsigned long *last_used = calloc(frames_count, sizeof(unsigned long));
    size_t loaded = 0;
    unsigned long faults = 0;
    unsigned long hits = 0;
    size_t fifo_position = 0;

    if (frames == NULL || last_used == NULL) {
        fprintf(stderr, "Erro de alocacao de memoria.\n");
        free(frames);
        free(last_used);
        return;
    }

    for (size_t i = 0; i < frames_count; ++i) {
        frames[i] = (unsigned long)-1;
    }

    for (size_t i = 0; i < count; ++i) {
        unsigned long page = accesses[i].address / page_size;
        size_t frame_index = find_page_in_frames(frames, loaded, page);

        if (frame_index != (size_t)-1) {
            hits++;
            if (policy == POLICY_LRU) {
                last_used[frame_index] = i;
            }
            continue;
        }

        faults++;

        if (loaded < frames_count) {
            frames[loaded] = page;
            last_used[loaded] = i;
            loaded++;
            continue;
        }

        size_t victim;
        if (policy == POLICY_FIFO) {
            victim = select_victim_fifo(&fifo_position, frames_count);
        } else if (policy == POLICY_LRU) {
            victim = select_victim_lru(last_used, frames_count);
        } else {
            victim = select_victim_optimal(accesses, count, i, frames, frames_count, page_size);
        }

        frames[victim] = page;
        last_used[victim] = i;
    }

    free(frames);
    free(last_used);

    *out_faults = faults;
    *out_accesses = count;
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

    long page_size_bytes = parse_kilobytes(argv[3]);
    long physical_size_bytes = parse_kilobytes(argv[4]);
    if (page_size_bytes <= 0 || physical_size_bytes <= 0) {
        fprintf(stderr, "Tamanho de pagina e memoria fisica devem ser inteiros positivos.\n");
        return EXIT_FAILURE;
    }
    if (physical_size_bytes < page_size_bytes) {
        fprintf(stderr, "Memoria fisica deve ser maior ou igual ao tamanho da pagina.\n");
        return EXIT_FAILURE;
    }
    if (physical_size_bytes % page_size_bytes != 0) {
        fprintf(stderr, "Memoria fisica deve ser multiplo do tamanho da pagina.\n");
        return EXIT_FAILURE;
    }

    unsigned long page_size = (unsigned long)page_size_bytes;
    size_t frames_count = (size_t)(physical_size_bytes / page_size_bytes);

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

    printf("Politica: %s\n", argv[1]);
    printf("Arquivo: %s\n", argv[2]);
    printf("Tamanho da pagina: %ld KB\n", page_size_bytes / 1024);
    printf("Memoria fisica: %ld KB\n", physical_size_bytes / 1024);
    printf("Quadros disponiveis: %zu\n\n", frames_count);

    unsigned long faults = 0;
    unsigned long total_accesses = 0;
    simulate(accesses, count, policy, page_size, frames_count, &faults, &total_accesses);
    free(accesses);

    printf("Total de acessos: %lu\n", total_accesses);
    printf("Page faults: %lu\n", faults);
    if (total_accesses > 0) {
        double rate = (double)faults * 100.0 / (double)total_accesses;
        printf("Taxa de page fault: %.2f%%\n", rate);
    }

    return EXIT_SUCCESS;
}
