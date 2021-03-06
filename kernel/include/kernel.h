#ifndef KERNEL_H
#define KERNEL_H
// Acá se están todas las declaraciones de funciones y librerias necesitadas Exclusivamente por kernel.c

#include "server.h"
#include "kernel-global.h"
#include "planificador_corto.h"
#include "planificador_mediano.h"
#include "planificador_largo.h"

void element_destroyer(void* elemento);
void liberar_memoria_y_finalizar(t_config_kernel *config_kernel, t_log *logger_kernel, t_config *config_file);
void print_inicializacion (t_config_kernel *config_kernel);
void iniciar_listas();
void iniciar_semaforos_generales();
void iniciar_debug_console();
void *debug_console(void *_ );
void print_task_lists();
void print_sem_asignados();
void print_lists();
void destruir_semaforos();
void destruir_listas();
void destruir_cosas(void *elemento);

t_config *config_file;

#endif