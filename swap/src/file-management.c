#include "file-management.h"

/*
    Inicialización de archivos de SWAP
*/
void crear_archivos_swap() {
    for(int i=0; i<list_size(config_swap->ARCHIVOS_SWAP); i++) {
        char *path = list_get(config_swap->ARCHIVOS_SWAP, i);
        char *init_value = '\0';

        FILE *file = fopen(path, "wb");
        fwrite(&init_value, 0, 1, file);

        fclose(file);
    }
}

/*
    Selección del archivo que se escribirá/leerá
*/
void siguiente_archivo() {
    if(archivo_seleccionado == -1) {
        archivo_seleccionado = 0;
    }

    archivo_seleccionado++;

    if(archivo_seleccionado > list_size(config_swap->ARCHIVOS_SWAP)) {
        archivo_seleccionado = -1;
    }
}

/*
    Inserción de página en archivo de SWAP
*/
void insertar_pagina_en_archivo(t_pagina *pagina) {
    bool pagina_guardada = 0;
    struct stat statbuf;

    do {
        //Abro el archivo
        char *path_archivo_actual = list_get(config_swap->ARCHIVOS_SWAP, archivo_seleccionado-1);
        int archivo = open(path_archivo_actual, O_RDWR);
        if(archivo < 0){
            log_error(logger_swap, "No pudo encontrarse el archivo en la ruta especificada (%s). Abortando ejecucion.", path_archivo_actual);
            exit(-1);
        }

        //Obtengo datos del archivo
        int archivo_cargado_correctamente = fstat(archivo, &statbuf);
        if(archivo_cargado_correctamente == -1) {
            log_error(logger_swap, "No pudo cargarse el archivo correctamente, revise el codigo. Abortando ejecucion.");
            exit(-1);
        }
        int offset_actual = statbuf.st_size;

        if(bytes_pagina(*pagina) <= (config_swap->TAMANIO_SWAP - statbuf.st_size)) {
            log_info(logger_swap, "Seleccionando el archivo %s con %dB de espacio disponible", path_archivo_actual, config_swap->TAMANIO_SWAP - statbuf.st_size);
            //Si es la primera pagina a insertar en el archivo, genero el mapeo del archivo correspondiente
            if(statbuf.st_size == 0) {
                void *mapeo_archivo = mmap(NULL, config_swap->TAMANIO_SWAP, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, archivo, 0);
                list_add(lista_mapeos, mapeo_archivo);
            }

            //Obtengo el mapeo correspondiente al archivo
            void *mapping = list_get(lista_mapeos, archivo_seleccionado-1);
            if(mapping == MAP_FAILED){
                log_error(logger_swap, "El mapeado de la pagina fallo, revise el codigo. Abortanto ejecucion.");
                exit(-1);
            }
            
            //Almaceno la página a guardar en el mapeado
            log_info(logger_swap, "Escribiendo la pagina %d en el archivo %s en el offset %d", pagina->numero_pagina, path_archivo_actual, statbuf.st_size);
            memcpy(mapping + statbuf.st_size, &(pagina->numero_pagina), sizeof(pagina->numero_pagina));
	        statbuf.st_size += sizeof(pagina->numero_pagina);
            memcpy(mapping + statbuf.st_size, &(pagina->marco->numero_marco), sizeof(pagina->marco->numero_marco));
	        statbuf.st_size += sizeof(pagina->marco->numero_marco);

            //Almaceno la página mapeada en el archivo
            ssize_t flag_escritura = write(archivo, mapping, statbuf.st_size);
            if(flag_escritura != statbuf.st_size){
                log_error(logger_swap, "No se pudo realizar la escritura en el archivo, revise el codigo. Abortando ejecucion.");
                exit(-1);
            }

            //Añado los datos de la página a la estructura administrativa
            t_pagina_almacenada *pagina_almacenada = malloc(sizeof(t_pagina_almacenada));
            pagina_almacenada->numero_pagina = pagina->numero_pagina;
            pagina_almacenada->base = offset_actual;
            pagina_almacenada->size = bytes_pagina(*pagina);
            pagina_almacenada->file = path_archivo_actual;

            list_add(lista_paginas_almacenadas, pagina_almacenada);

            //Cierro el archivo, libero la memoria de la página y termino el ciclo
            close(archivo);
            pagina_guardada = 1;
        } else {
            close(archivo);
            siguiente_archivo();
        }
    } while(pagina_guardada == 0 && archivo_seleccionado != -1);

    if(pagina_guardada == 0) {
        log_error(logger_swap, "No se ha podido guardar la pagina en ningun archivo dado que no hay espacio suficiente");
    } else {
        log_info(logger_swap, "Pagina %d almacenada en el archivo %d.bin", pagina->numero_pagina, archivo_seleccionado);
        free(pagina);
    }
}

/*
    Lectura de una página de un archivo de SWAP
*/
void leer_pagina_de_archivo(int numero_pagina) {
    //Me fijo si la página solicitada es una de las páginas almacenadas
    t_pagina_almacenada *informacion_almacenamiento = NULL;
    for(int i=0; i<list_size(lista_paginas_almacenadas); i++) {
        t_pagina_almacenada *pagina_almacenada = list_get(lista_paginas_almacenadas, i);
        if(numero_pagina == pagina_almacenada->numero_pagina) {
            informacion_almacenamiento = pagina_almacenada;
            break;
        }
    }

    //Si la página fue encontrada la voy a buscar, sino lanzo mensaje de aviso
    if(informacion_almacenamiento != NULL) {
        //Lectura del archivo
        int archivo = open(informacion_almacenamiento->file, O_RDONLY);

        struct stat statbuf;
        fstat(archivo, &statbuf);

        //Obtengo la página dentro del archivo
        void *paginas_obtenidas = mmap(0, statbuf.st_size, PROT_READ, MAP_PRIVATE, archivo, 0);
        t_pagina pagina_obtenida;
        t_marco marco_pagina;

        int offset_actual = informacion_almacenamiento->base;
        memcpy(&pagina_obtenida.numero_pagina, paginas_obtenidas + offset_actual, sizeof(pagina_obtenida.numero_pagina));
        offset_actual += sizeof(pagina_obtenida.numero_pagina);
        memcpy(&marco_pagina, paginas_obtenidas + offset_actual, sizeof(marco_pagina));
        offset_actual += sizeof(marco_pagina);
        pagina_obtenida.marco = malloc(bytes_marco(marco_pagina));
        pagina_obtenida.marco->numero_marco = marco_pagina.numero_marco;

        printf("Numero de pagina: %d\n", pagina_obtenida.numero_pagina);
        printf("Numero de marco: %d\n", pagina_obtenida.marco->numero_marco);

        close(archivo);
    } else {
        log_error(logger_swap, "La pagina %d no se encuentra almacenada en los archivos de swap", numero_pagina);
    }
}

/*
    Eliminación de archivos de SWAP
    Utilizado cuando finaliza la ejecución
*/
void borrar_archivos_swap() {
    for(int i=0; i<list_size(config_swap->ARCHIVOS_SWAP); i++) {
        char *path = list_get(config_swap->ARCHIVOS_SWAP, i);
        remove(path);
    }
}