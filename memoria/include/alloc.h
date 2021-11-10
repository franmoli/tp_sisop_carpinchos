#ifndef ALLOC_H
#define ALLOC_H

#include "memoria-global.h"
#include <commons/collections/list.h>
#include "paginacion.h"

t_heap_metadata* traerAllocDeMemoria(uint32_t);
void crearPrimerAlloc(t_pagina* primeraPagina,int size);
void guardarAlloc(t_heap_metadata* data, uint32_t direccion);
void memAlloc(t_paquete *paquete);
void freeAlloc(uint32_t direccion);

void crearFooterAlloc(t_pagina *primeraPagina, int inicio,int size);
void crearHeaderAlloc(t_pagina *primeraPagina,int size);
t_heap_metadata* getLastHeapFromPagina(int pagina);
#endif