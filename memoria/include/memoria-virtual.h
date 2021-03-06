#ifndef MEMORIA_VIRTUAL_H
#define MEMORIA_VIRTUAL_H

#include "memoria-global.h"


//t_pagina* reemplazarPagina(t_pagina* paginaAgregar, int carpincho_id);
int reemplazarPagina();
int eliminarPrimerElementoLista(int carpincho_id);
void consultaSwap(int carpincho_id);

//Reemplazo
int reemplazarLRU();
int reemplazarClockM();
void actualizarModificado(uint32_t nro_pagina, uint32_t carpincho_id);
void actualizarReferido(uint32_t nro_pagina, uint32_t carpincho_id);

void actualizarLRU(t_pagina* pagina);

void agregarAsignacion(t_pagina* pagina);
int enviarPaginaSwap(t_pagina* pagina);

void replaceClock(t_pagina *pagina);
int recibirPaginaSwap(t_pagina* pagina);

#endif