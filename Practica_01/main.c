#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/resource.h>
#include <getopt.h>
#ifndef MAP_ANONYMOUS
#ifdef MAP_ANON
#define MAP_ANONYMOUS MAP_ANON
#else
#define MAP_ANONYMOUS 0x20
#endif
#endif

// Configuración por defecto
#define DEFAULT_NUM_WORKERS 4
#define DEFAULT_INCREMENTS_PER_WORKER 1000000
#define DEFAULT_TEST_ITERATIONS 3

// Estructura para datos compartidos entre procesos
typedef struct {
    int counter;
    pthread_mutex_t mutex;
} shared_data_t;

// Estructura para argumentos de hilos
typedef struct {
    int worker_id;
    int increments;
    shared_data_t* shared_data;
    int use_mutex;
} thread_args_t;

// Variables globales para hilos
shared_data_t thread_shared;
int thread_counter_no_sync = 0;

// Función para obtener tiempo en microsegundos
long long get_time_microseconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000LL + tv.tv_usec;
}

// Función para obtener uso de CPU
void get_cpu_usage(struct rusage* usage) {
    getrusage(RUSAGE_CHILDREN, usage);
}

// Función ejecutada por cada proceso hijo
void process_worker(shared_data_t* shared_data, int worker_id, int increments, int use_sync) {
    pid_t pid = getpid();
    pid_t ppid = getppid();
    
    printf("[PROCESO %d] PID=%d, PPID=%d - Iniciando %d incrementos%s\n", 
           worker_id, pid, ppid, increments, use_sync ? " (con sincronización)" : " (sin sincronización)");
    
    for (int i = 0; i < increments; i++) {
        if (use_sync) {
            pthread_mutex_lock(&shared_data->mutex);
            shared_data->counter++;
            pthread_mutex_unlock(&shared_data->mutex);
        } else {
            // Sin sincronización - cada proceso maneja su propia copia
            shared_data->counter++;
        }
    }
    
    printf("[PROCESO %d] PID=%d - Completado. Contador local: %d\n", worker_id, pid, shared_data->counter);
}

// Función ejecutada por cada hilo
void* thread_worker(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    pthread_t tid = pthread_self();
    pid_t pid = getpid();
    
    printf("[HILO %d] TID=%lu, PID=%d - Iniciando %d incrementos%s\n", 
           args->worker_id, (unsigned long)tid, pid, args->increments, 
           args->use_mutex ? " (con mutex)" : " (sin sincronización)");
    
    if (args->use_mutex) {
        // Con sincronización
        for (int i = 0; i < args->increments; i++) {
            pthread_mutex_lock(&args->shared_data->mutex);
            args->shared_data->counter++;
            pthread_mutex_unlock(&args->shared_data->mutex);
        }
    } else {
        // Sin sincronización
        for (int i = 0; i < args->increments; i++) {
            thread_counter_no_sync++;
        }
    }
    
    printf("[HILO %d] TID=%lu - Completado\n", args->worker_id, (unsigned long)tid);
    return NULL;
}

// Función para ejecutar prueba con procesos
void test_processes(int num_workers, int increments_per_worker, int use_sync) {
    printf("\n=== PRUEBA CON PROCESOS (%s) ===\n", 
           use_sync ? "CON SINCRONIZACIÓN" : "SIN SINCRONIZACIÓN");
    
    // Crear memoria compartida
    shared_data_t* shared_data = mmap(NULL, sizeof(shared_data_t), 
                                    PROT_READ | PROT_WRITE, 
                                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    if (shared_data == MAP_FAILED) {
        perror("mmap failed");
        return;
    }
    
    // Inicializar datos compartidos
    shared_data->counter = 0;
    
    // Inicializar mutex para memoria compartida
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shared_data->mutex, &mutex_attr);
    
    struct rusage usage_start, usage_end;
    getrusage(RUSAGE_CHILDREN, &usage_start);
    long long start_time = get_time_microseconds();
    
    // Crear procesos hijo
    pid_t pids[num_workers];
    
    for (int i = 0; i < num_workers; i++) {
        pids[i] = fork();
        
        if (pids[i] == 0) {
            // Código del proceso hijo
            process_worker(shared_data, i, increments_per_worker, use_sync);
            exit(0);
        } else if (pids[i] < 0) {
            perror("fork failed");
            return;
        }
    }
    
    // Esperar a que terminen todos los procesos hijo
    for (int i = 0; i < num_workers; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }
    
    long long end_time = get_time_microseconds();
    getrusage(RUSAGE_CHILDREN, &usage_end);
    
    // Calcular métricas
    double execution_time = (end_time - start_time) / 1000.0; // ms
    long total_increments = (long)num_workers * increments_per_worker;
    
    double cpu_time = (usage_end.ru_utime.tv_sec - usage_start.ru_utime.tv_sec) +
                     (usage_end.ru_utime.tv_usec - usage_start.ru_utime.tv_usec) / 1000000.0;
    
    // Mostrar resultados
    printf("Trabajadores: %d\n", num_workers);
    printf("Incrementos por trabajador: %d\n", increments_per_worker);
    printf("Total incrementos esperados: %ld\n", total_increments);
    printf("Contador final: %d\n", shared_data->counter);
    printf("¿Resultado correcto?: %s\n", 
           (shared_data->counter == total_increments) ? "SÍ" : "NO");
    printf("Tiempo de ejecución: %.2f ms\n", execution_time);
    printf("Throughput: %.2f ops/ms\n", total_increments / execution_time);
    printf("Tiempo CPU total: %.4f segundos\n", cpu_time);
    printf("Utilización CPU: %.2f%%\n", (cpu_time * 1000 / execution_time) * 100);
    
    // Limpiar recursos
    pthread_mutex_destroy(&shared_data->mutex);
    pthread_mutexattr_destroy(&mutex_attr);
    munmap(shared_data, sizeof(shared_data_t));
}

// Función para ejecutar prueba con hilos
void test_threads(int num_workers, int increments_per_worker, int use_sync) {
    printf("\n=== PRUEBA CON HILOS (%s) ===\n", 
           use_sync ? "CON SINCRONIZACIÓN" : "SIN SINCRONIZACIÓN");
    
    // Inicializar datos compartidos
    thread_shared.counter = 0;
    thread_counter_no_sync = 0;
    
    if (use_sync) {
        pthread_mutex_init(&thread_shared.mutex, NULL);
    }
    
    struct rusage usage_start, usage_end;
    getrusage(RUSAGE_SELF, &usage_start);
    long long start_time = get_time_microseconds();
    
    // Crear hilos
    pthread_t threads[num_workers];
    thread_args_t args[num_workers];
    
    for (int i = 0; i < num_workers; i++) {
        args[i].worker_id = i;
        args[i].increments = increments_per_worker;
        args[i].shared_data = &thread_shared;
        args[i].use_mutex = use_sync;
        
        if (pthread_create(&threads[i], NULL, thread_worker, &args[i]) != 0) {
            perror("pthread_create failed");
            return;
        }
    }
    
    // Esperar a que terminen todos los hilos
    for (int i = 0; i < num_workers; i++) {
        pthread_join(threads[i], NULL);
    }
    
    long long end_time = get_time_microseconds();
    getrusage(RUSAGE_SELF, &usage_end);
    
    // Calcular métricas
    double execution_time = (end_time - start_time) / 1000.0; // ms
    long total_increments = (long)num_workers * increments_per_worker;
    
    double cpu_time = (usage_end.ru_utime.tv_sec - usage_start.ru_utime.tv_sec) +
                     (usage_end.ru_utime.tv_usec - usage_start.ru_utime.tv_usec) / 1000000.0;
    
    int final_counter = use_sync ? thread_shared.counter : thread_counter_no_sync;
    
    // Mostrar resultados
    printf("Trabajadores: %d\n", num_workers);
    printf("Incrementos por trabajador: %d\n", increments_per_worker);
    printf("Total incrementos esperados: %ld\n", total_increments);
    printf("Contador final: %d\n", final_counter);
    printf("¿Resultado correcto?: %s\n", 
           (final_counter == total_increments) ? "SÍ" : "NO");
    printf("Tiempo de ejecución: %.2f ms\n", execution_time);
    printf("Throughput: %.2f ops/ms\n", total_increments / execution_time);
    printf("Tiempo CPU total: %.4f segundos\n", cpu_time);
    printf("Utilización CPU: %.2f%%\n", (cpu_time * 1000 / execution_time) * 100);
    
    if (use_sync) {
        pthread_mutex_destroy(&thread_shared.mutex);
    }
}

void print_usage(const char* program_name) {
    printf("Uso: %s [opciones]\n", program_name);
    printf("Opciones:\n");
    printf("  -w <num>     Número de trabajadores (por defecto: %d)\n", DEFAULT_NUM_WORKERS);
    printf("  -i <num>     Incrementos por trabajador (por defecto: %d)\n", DEFAULT_INCREMENTS_PER_WORKER);
    printf("  -r <num>     Iteraciones de prueba (por defecto: %d)\n", DEFAULT_TEST_ITERATIONS);
    printf("  -p           Solo ejecutar pruebas de procesos\n");
    printf("  -t           Solo ejecutar pruebas de hilos\n");
    printf("  -s           Solo ejecutar pruebas con sincronización\n");
    printf("  -n           Solo ejecutar pruebas sin sincronización\n");
    printf("  -h           Mostrar esta ayuda\n");
}

int main(int argc, char* argv[]) {
    int num_workers = DEFAULT_NUM_WORKERS;
    int increments_per_worker = DEFAULT_INCREMENTS_PER_WORKER;
    int test_iterations = DEFAULT_TEST_ITERATIONS;
    int test_processes_only = 0;
    int test_threads_only = 0;
    int test_sync_only = 0;
    int test_no_sync_only = 0;
    
    // Procesar argumentos de línea de comandos
    int opt;
    while ((opt = getopt(argc, argv, "w:i:r:ptsnnh")) != -1) {
        switch (opt) {
            case 'w':
                num_workers = atoi(optarg);
                if (num_workers <= 0) {
                    fprintf(stderr, "Error: Número de trabajadores debe ser positivo\n");
                    return 1;
                }
                break;
            case 'i':
                increments_per_worker = atoi(optarg);
                if (increments_per_worker <= 0) {
                    fprintf(stderr, "Error: Incrementos por trabajador debe ser positivo\n");
                    return 1;
                }
                break;
            case 'r':
                test_iterations = atoi(optarg);
                if (test_iterations <= 0) {
                    fprintf(stderr, "Error: Iteraciones de prueba debe ser positivo\n");
                    return 1;
                }
                break;
            case 'p':
                test_processes_only = 1;
                break;
            case 't':
                test_threads_only = 1;
                break;
            case 's':
                test_sync_only = 1;
                break;
            case 'n':
                test_no_sync_only = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    printf("=== COMPARACIÓN PROCESOS vs HILOS ===\n");
    printf("Configuración:\n");
    printf("- Trabajadores: %d\n", num_workers);
    printf("- Incrementos por trabajador: %d\n", increments_per_worker);
    printf("- Iteraciones de prueba: %d\n\n", test_iterations);
    
    for (int iteration = 1; iteration <= test_iterations; iteration++) {
        printf(">>> ITERACIÓN %d/%d <<<\n", iteration, test_iterations);
        
        // Ejecutar pruebas según las opciones
        if (!test_threads_only) {
            if (!test_no_sync_only) {
                test_processes(num_workers, increments_per_worker, 1); // Con sincronización
            }
            if (!test_sync_only) {
                test_processes(num_workers, increments_per_worker, 0); // Sin sincronización
            }
        }
        
        if (!test_processes_only) {
            if (!test_no_sync_only) {
                test_threads(num_workers, increments_per_worker, 1); // Con sincronización
            }
            if (!test_sync_only) {
                test_threads(num_workers, increments_per_worker, 0); // Sin sincronización
            }
        }
        
        if (iteration < test_iterations) {
            printf("\n");
            for (int i = 0; i < 50; i++) printf("=");
            printf("\n");
        }
    }
    
    return 0;
}