#ifndef PROCESO_H
#define PROCESO_H

#include <unistd.h>
#include <time.h>

#include "kernel-global.h"

void *proceso(void *_);
void new();
void ready();
void exec(t_proceso *self);
void blocked();
void bloquear(t_proceso *self);
void desbloquear(t_proceso *self);
void iniciar_semaforo(t_semaforo *semaforo);
void enviar_sem_disponible(int id);
t_semaforo *traer_semaforo(char *nombre_solicitado);
void enviar_error(int socket);
bool solicitar_semaforo(char *nombre_semaforo, int id);
void postear_semaforo(char *nombre_semaforo, int id_del_chabon_que_postea );
t_proceso *traer_proceso_bloqueado(int id);
void *desbloquear_en(void *param);
void devolver_recurso(int id, char *sem_devuelto);
#endif