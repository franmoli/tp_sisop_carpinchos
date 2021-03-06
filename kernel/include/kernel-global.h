#ifndef KERNEL_GLOBAL_H
#define KERNEL_GLOBAL_H

// Acá estan todas las declaraciones y librerias usadas por todos los archivos .c
#include <pthread.h>
#include <semaphore.h>
#include <commons/log.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
//#include "matelib.h"
#include "server.h"
#include "config_utils.h"



//tipos propios para listas
typedef enum {
    NEW = 1,
    READY = 2,
    EXEC = 3,
    BLOCKED = 4,
    S_BLOCKED = 5,
    S_READY = 6,
    EXIT = 7
} t_status;

typedef struct {
    int id;
    t_status status;
    t_list *task_list;
    float estimacion;
    int ejecucion_anterior;
    int entrada_a_ready;
    bool estimar;
    bool termino_rafaga;
    bool block;
    bool salida_exit;
    bool salida_block;
    int socket_carpincho;
} t_proceso;

typedef struct {
    op_code id;
    void *datos_tarea;
} t_task;

typedef struct {
    char *nombre_semaforo;
    int value;
    t_list *solicitantes;
}t_semaforo;

typedef struct {
    char *nombre;
    char *mensaje;
    int duracion;
    t_proceso *proceso_solicitante;
    int id;
}t_io;

typedef struct {
    char *nombre_recurso;
    int id_asignado;
}t_recurso_asignado;



//Configuración
t_config_kernel *config_kernel;
//Logs
t_log *logger_kernel;
//listas
t_list *lista_new;
t_list *lista_ready;
t_list *lista_exec;
t_list *lista_blocked;
t_list *lista_s_blocked;
t_list *lista_s_ready;
t_list *lista_semaforos;
t_list *lista_exit;
t_list *lista_recursos_asignados;

//Hilos
pthread_t hilo_planificador;
pthread_t hilo_esperar_bloqueo;

//Semaforos
sem_t mutex_listas;
sem_t proceso_finalizo_o_suspended;
sem_t salida_exec;
sem_t salida_block;
sem_t actualizacion_de_listas_1;
sem_t actualizacion_de_listas_2;
sem_t actualizacion_de_listas_1_recibido;
sem_t proceso_inicializado;
sem_t libre_para_inicializar_proceso;
sem_t mutex_multiprocesamiento;
sem_t mutex_multiprogramacion;
sem_t mutex_cant_procesos;
sem_t salida_a_exit;
sem_t liberar_multiprocesamiento;
sem_t salida_a_exit_recibida;
sem_t salida_de_exec_recibida;
sem_t cambio_de_listas;
sem_t cambio_de_listas_largo;
sem_t cambio_de_listas_mediano;
sem_t cambio_de_listas_corto;
sem_t pedir_salida_de_block;
sem_t solicitar_block;
sem_t mutex_semaforos;
sem_t mutex_recursos_asignados;
sem_t *io_libre;


//Auxiliares
int cantidad_de_procesos;
bool salida_de_exec;
int multiprogramacion_disponible;
int socket_cliente_memoria;
int socket_servidor_kernel;
int multiprocesamiento;
int procesos_esperando_bloqueo;
bool terminar_kernel;

//Hilos
pthread_t hilo_deteccion_deadlock;

//funciones
void mover_proceso_de_lista(t_list *origen, t_list *destino, int index, int status);
void avisar_cambio();


#endif