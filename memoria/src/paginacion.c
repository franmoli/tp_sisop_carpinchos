#include "paginacion.h"

int getPaginaByDireccion(uint32_t direccion){
    log_info(logger_memoria,"Dire:%d", direccion);
    uint32_t inicio = tamanio_memoria;
    log_info(logger_memoria,"inicio:%d", inicio);
    uint32_t resta =  direccion - inicio;
    log_info(logger_memoria,"resta:%d", resta);
    log_info(logger_memoria,"config:%d", config_memoria->TAMANIO_PAGINA);
    return resta/config_memoria->TAMANIO_PAGINA;
}
int getPrimeraPaginaDisponible(int size, t_tabla_paginas *tabla_paginas)
{
    t_list_iterator *list_iterator = list_iterator_create(tabla_paginas->paginas);
    int numeroPagina = -1;
    bool pagainaFueEncontrada = false;
    while (list_iterator_has_next(list_iterator) && !pagainaFueEncontrada)
    {
        t_pagina *paginaLeida = list_iterator_next(list_iterator);
        int a = (config_memoria->TAMANIO_PAGINA - paginaLeida->tamanio_ocupado);
        if (a > 0)
        {
            pagainaFueEncontrada = true;
            numeroPagina = paginaLeida->numero_pagina;
        }
    }
    list_iterator_destroy(list_iterator);
    return numeroPagina;
}
t_contenidos_pagina *getLastContenidoByPagina(t_pagina* pagina){
     t_list_iterator *list_iterator = list_iterator_create(pagina->listado_de_contenido);
     t_contenidos_pagina *contenido;
     while (list_iterator_has_next(list_iterator))
    {
        contenido = list_iterator_next(list_iterator);
    }
    return contenido;
}
t_contenidos_pagina *getLastHeaderContenidoByPagina(t_pagina* pagina){
     t_list_iterator *list_iterator = list_iterator_create(pagina->listado_de_contenido);
     t_contenidos_pagina *contenido;
     t_contenidos_pagina *contenidoFinal;
     while (list_iterator_has_next(list_iterator))
    {
        contenido = list_iterator_next(list_iterator);
        if(contenido->contenido_pagina == HEADER){
            contenidoFinal = contenido;
        }
        
    }
    return contenidoFinal;
}
//////////////////////////////////////////////////////////////////////////////

// Mem Read: Dada una direccion de memoria busco el contenido que se encuentra alli
t_heap_metadata* memRead(t_paquete* paquete) {

    //TODO: Tener en cuenta que la direccion pertenezca a una pagina de la tabla de este proceso
    uint32_t direccion = deserializar_alloc(paquete);
    uint32_t carpincho_id = 1;
    //Traer pagina
        if(!direccionValida(direccion)){
        //MATE FREE FAIL
    }

     t_tabla_paginas* tabla_paginas = buscarTablaPorPID(carpincho_id);

    int nro_pagina = getPaginaByDireccion(direccion);

    //Ir a la tlb
    int nro_marco = getFromTLB(nro_pagina, tabla_paginas);

    //Falta buscar posta la info en memoria


    t_heap_metadata* alloc = traerAllocDeMemoria(direccion);

    log_info(logger_memoria, "Traje alloc de memoria");
    return alloc;

}
int getMarco(){

    int numeroPagina = -1;
    t_list_iterator *list_iterator = list_iterator_create(tabla_marcos->marcos);
    bool marcoDisponible = false;
    while (list_iterator_has_next(list_iterator) && !marcoDisponible)
    {
        t_marco *marco = list_iterator_next(list_iterator);
        if (marco->isFree)
        {
            marcoDisponible = true;
            numeroPagina = marco->numero_marco;
        }
    }
    list_iterator_destroy(list_iterator);
    return numeroPagina;
}
int getMarcoParaPagina(t_tabla_paginas* tabla_paginas){
    
    if(strcmp(config_memoria->TIPO_ASIGNACION, "FIJA") == 0){
        if(tabla_paginas->paginas_en_memoria <=config_memoria->MARCOS_POR_CARPINCHO){
            return getMarco();
        }else{
            
        }
        
    }
    else{
        log_error(logger_memoria,"ERROR EL CARPINCHO NO PUEDE ASIGNAR MAS MARCOS EN MEMORIA");
        //reemplazarPagina();
        return -1;
    }
}

void mostrarPaginas(t_tabla_paginas* tabla_paginas){
    system("clear");
    t_list_iterator *list_iterator = list_iterator_create(tabla_paginas->paginas);
    int i= 0;
    while (list_iterator_has_next(list_iterator))
    {
        t_pagina *paginaLeida = list_iterator_next(list_iterator);
        printf("Entrada: %d PAGINA nro: %d. Marco Asignado:%d Tam Ocupado:%d \n",i,paginaLeida->numero_pagina, paginaLeida->marco_asignado, paginaLeida->tamanio_ocupado);
        i++;
    }
    list_iterator_destroy(list_iterator);
}

t_tabla_paginas* buscarTablaPorPID(int id){

    if(list_size(tabla_procesos)==0){
        //No existen procesos
        return 1000;
    }

    int numeroTabla = -1;
    t_list_iterator *list_iterator = list_iterator_create(tabla_procesos);

    while(list_iterator_has_next(list_iterator)) {

        t_tabla_paginas *tabla = list_iterator_next(list_iterator);
        if(tabla->pid == id){
            list_iterator_destroy(list_iterator);
            return tabla;
        }

    }

    //No se encontro la tabla --> me pego un tiro
    log_info(logger_memoria,"No se encontro la tabla perteneciente al proceso");
    list_iterator_destroy(list_iterator);
    return 1000;
}

int buscarMarcoEnMemoria(int numero_pagina_buscada, int id){

    //Manejar error si no existe tabla
    t_tabla_paginas *tabla = buscarTablaPorPID(id);

    int numeroPagina = -1;
    t_list_iterator *list_iterator = list_iterator_create(tabla->paginas);

    while(list_iterator_has_next(list_iterator)) {

        t_pagina *pagina = list_iterator_next(list_iterator);
        if(pagina->numero_pagina == numero_pagina_buscada){
            list_iterator_destroy(list_iterator);
            return pagina->marco_asignado;
        }

    }

    //No encontre la pagina
    //Voy a swap

}

int solicitarPaginaNueva(uint32_t carpincho_id){

    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(carpincho_id);
    int marco = getMarcoParaPagina(tabla_paginas);   
    if(marco <0){
        log_error(logger_memoria,"No se pudo asignar un marco");
        return;
    }
    int numero_pagina = 0;
    if(list_size(tabla_paginas->paginas) > 0)
        numero_pagina = list_size(tabla_paginas->paginas);

    t_pagina *pagina =malloc(sizeof(t_pagina));
    pagina->listado_de_contenido = list_create();
    pagina->marco_asignado = marco;
    pagina->numero_pagina = numero_pagina;
    pagina->tamanio_ocupado = 0;
    pagina->carpincho_id = carpincho_id;
    pagina->cantidad_contenidos = 0;
    pagina->bit_presencia = true;
    if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "CLOCK-M") == 0) //SOLO CLOCK USA ESTE CAMPO
        pagina->bit_uso=true;

    list_add(tabla_paginas->paginas,pagina);

    t_marco *marcoAsignado = list_get(tabla_marcos->marcos, marco);
    marcoAsignado->isFree = false;
    tabla_paginas->paginas_en_memoria += 1;
    
    return pagina->numero_pagina;
}