#ifndef KERNEL_GLOBAL_H
#define KERNEL_GLOBAL_H

// Acá estan todas las declaraciones y librerias usadas por todos los archivos .c
#include <pthread.h>
#include <semaphore.h>
#include <commons/log.h>

#include "config_utils.h"



//tipos propios para listas
typedef enum {
    NEW = 1,
    READY = 2,
    EXEC = 3,
    BLOCKED = 4,
    S_BLOCKED = 5,
    S_READY = 6
} t_status;

typedef struct {
    int id;
    t_status status;
    int estimacion;
    int ejecucion_anterior;
    bool estimar;
    bool termino_rafaga;
    bool block;
    // t_task_list *task_list; ????? cuales son las tareas que ejecuta el proceso
} t_proceso;

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
//Semaforos
sem_t mutex_listas;
sem_t proceso_finalizo_o_suspended;
sem_t salida_exec;
sem_t actualizacion_de_listas_1;
sem_t actualizacion_de_listas_2;
sem_t actualizacion_de_listas_1_recibido;
sem_t proceso_inicializado;
sem_t libre_para_inicializar_proceso;
sem_t mutex_multiprocesamiento;
sem_t mutex_cant_procesos;
//Auxiliares
int cantidad_de_procesos;
bool salida_de_exec;

//funciones
void mover_proceso_de_lista(t_list *origen, t_list *destino, int index, int status);
void avisar_cambio();

#endif