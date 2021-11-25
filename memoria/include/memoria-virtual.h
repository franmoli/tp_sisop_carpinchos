#ifndef MEMORIA_VIRTUAL_H
#define MEMORIA_VIRTUAL_H

#include "memoria-global.h"


//t_pagina* reemplazarPagina(t_pagina* paginaAgregar, int carpincho_id);
int reemplazarPagina(int nro_pagina, int carpincho_id);
int eliminarPrimerElementoLista(int carpincho_id);
void consultaSwap(int carpincho_id);

//Reemplazo
int reemplazarLRU(int nro_pagina, int carpincho_id);
#endif