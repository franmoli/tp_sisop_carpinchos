#ifndef ALLOC_H
#define ALLOC_H

#include "memoria-global.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "paginacion.h"
#include <math.h>

t_heap_metadata* traerAllocDeMemoria(uint32_t);
void guardarAlloc(t_heap_metadata* data, uint32_t direccion);

int memAlloc(t_paquete *paquete);
int freeAlloc(t_paquete *paquete);

bool direccionValida(uint32_t direccion);
t_heap_metadata* getLastHeapFromPagina(int pagina, int carpincho_id);
void mostrarAllocs(int carpincho_id);

int agregarPagina(t_pagina* pagina, t_heap_metadata* data, uint32_t nextAnterior, uint32_t size,bool ultimo, int index);
void crearPrimerHeader(t_pagina* pagina, uint32_t size);
uint32_t asignarFooterSeparado(t_pagina* pagina,t_heap_metadata* data,uint32_t size, uint32_t nextAnterior);
t_pagina* asignarFooterSeparadoSubContenido(t_contenido subcontenido, t_pagina* pagina, uint32_t nextAnterior, t_heap_metadata* data);
void eliminarcontenidoBydireccion(uint32_t direccion,t_pagina *pagina);

//GENERALES
void restarTamanioaPagina(t_pagina* pagina_alloc_actual,t_heap_metadata* alloc,uint32_t direccion_logica_alloc_actual);
#endif