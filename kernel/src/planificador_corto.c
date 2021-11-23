
#include "../include/planificador_corto.h"

void estimar(t_proceso *proceso);

void iniciar_planificador_corto(){

    printf("Inicio planificador CORTO \n");
    void *(*planificador)(void*);
    pthread_t hilo_planificador;
    pthread_t hilo_terminar_rafaga;
    pthread_t hilo_liberar_multiprocesamiento_h;
    int *multiprocesamiento = malloc(sizeof(int));
    *multiprocesamiento = config_kernel->GRADO_MULTIPROCESAMIENTO;

    if(!strcmp(config_kernel->ALGORITMO_PLANIFICACION, "SJF")){

        planificador = planificador_corto_plazo_sjf;

    }else if(!strcmp(config_kernel->ALGORITMO_PLANIFICACION, "HRRN")){

        planificador = planificador_corto_plazo_hrrn;

    }else{

        log_error(logger_kernel, "Planificador no soportado/no reconocido");
        return;
    }
     
    pthread_create(&hilo_planificador, NULL, planificador, (void *)multiprocesamiento);
    pthread_create(&hilo_terminar_rafaga, NULL, esperar_salida_exec, (void *)multiprocesamiento);
    pthread_create(&hilo_liberar_multiprocesamiento_h, NULL, hilo_liberar_multiprocesamiento, (void *)multiprocesamiento);
}

void *planificador_corto_plazo_sjf (void *multiprocesamiento_p){

    
    t_proceso *aux;
    int *multiprocesamiento = multiprocesamiento_p;

    while(1){

        //calcular estimaciones
        for(int i = 0; i < list_size(lista_ready); i++){

            aux = list_get(lista_ready, i);
            if(aux->estimar)
                estimar(aux);

        }
        
        if(*multiprocesamiento && list_size(lista_ready)){
            int index = -1;
            int estimacion_aux;

            //se busca la estimacion menor
            for(int i = 0; i < list_size(lista_ready); i++){
                aux = list_get(lista_ready, i);
                if(aux->estimacion < estimacion_aux || i == 0){
                    estimacion_aux = aux->estimacion;
                    index = i;
                }
            }

            //Se saca de ready y se pasa a exec
            mover_proceso_de_lista(lista_ready, lista_exec, index, EXEC);
            sem_wait(&mutex_multiprocesamiento);
            *multiprocesamiento = *multiprocesamiento - 1;
            sem_post(&mutex_multiprocesamiento);

        }
    }


    return NULL;
}

void *planificador_corto_plazo_hrrn (void *multiprocesamiento_p){
    t_proceso *aux;
    int *multiprocesamiento = multiprocesamiento_p;

    //calcular estimaciones
    for(int i = 0; i < list_size(lista_ready); i++){

        aux = list_get(lista_ready, i);
        if(aux->estimar)
            estimar(aux);

    }

    while(1){
        
        if(*multiprocesamiento && list_size(lista_ready)){
            int index = -1;
            int response_ratio = 0;

            //se busca el response ratio mas alto
            for(int i = 0; i < list_size(lista_ready); i++){

                aux = list_get(lista_ready, i);
                
                if(calcular_response_ratio(aux) > response_ratio || i == 0){
                    response_ratio = calcular_response_ratio(aux);
                    index = i;
                }
            }

            //Se saca de ready y se pasa a exec
            mover_proceso_de_lista(lista_ready, lista_exec, index, EXEC);
            sem_wait(&mutex_multiprocesamiento);
            *multiprocesamiento = *multiprocesamiento - 1;
            sem_post(&mutex_multiprocesamiento);

        }
    }


    return NULL;
    return NULL;
}

void estimar(t_proceso *proceso){
    int alfa = config_kernel->ALFA;
    proceso->estimacion = (alfa * proceso->ejecucion_anterior) + (( 1 - alfa) * proceso->estimacion);
    proceso->estimar = false;
    return;
}

int calcular_response_ratio(t_proceso *proceso){

    int tiempo_transcurrido = clock() - proceso->entrada_a_ready;

    return (tiempo_transcurrido + proceso->estimacion)/proceso->estimacion;

}

void *esperar_salida_exec(void *multiprocesamiento_p){

    int *multiprocesamiento = multiprocesamiento_p;

    while(1){

        sem_wait(&salida_exec);
        //printf("#recibida ready\n");
        bool encontrado = false;
        int tamanio_lista_exec = list_size(lista_exec);
        int index = 0;

        while(!encontrado && (index < tamanio_lista_exec)){
            t_proceso *aux = list_get(lista_exec, index);
            if(aux->termino_rafaga){
                if(aux->block){
                    mover_proceso_de_lista(lista_exec, lista_blocked, index, BLOCKED);
                }else{
                    //printf("Saco de exec %d\n", aux->id);
                    mover_proceso_de_lista(lista_exec, lista_ready, index, READY);
                }
                encontrado = true;
            }
            index ++;
        }
        if(encontrado){
            sem_wait(&mutex_multiprocesamiento);
            *multiprocesamiento = *multiprocesamiento + 1;
            sem_post(&mutex_multiprocesamiento);
            index = 0;
            sem_post(&salida_de_exec_recibida);
        }
        if(*multiprocesamiento == 0){
            printf("ME quede sin multiprocesamiento\n");
        }
    }
}

void *esperar_salida_block(void *multiprocesamiento_p){

    while(1){

        sem_wait(&salida_block);

        bool encontrado = false;
        int tamanio_lista_blocked = list_size(lista_blocked);
        int index = 0;

        while(!encontrado && (index < tamanio_lista_blocked)){
            t_proceso *aux = list_get(lista_blocked, index);
            if(aux->termino_rafaga){
                printf("saco por block\n");
                    mover_proceso_de_lista(lista_blocked, lista_ready, index, READY);
                encontrado = true;
            }
            index ++;
        }
    }
}

void *hilo_liberar_multiprocesamiento(void *multiprocesamiento_p){
    int *multiprocesamiento = multiprocesamiento_p;

    while(1){

        sem_wait(&liberar_multiprocesamiento);
        
        sem_wait(&mutex_multiprocesamiento);
        *multiprocesamiento = *multiprocesamiento + 1;
        sem_post(&mutex_multiprocesamiento);
        
    }
    return NULL;
}

void *esperar_bloqueo(void *multiprocesamiento_p){
    int *multiprocesamiento = multiprocesamiento_p;

    while(1){

        sem_wait(&solicitar_block);
        bool encontrado = false;
        int tamanio_lista_blocked = list_size(lista_blocked);
        int index = 0;

        while(!encontrado && (index < tamanio_lista_blocked)){
            t_proceso *aux = list_get(lista_blocked, index);
            
                if(aux->block){
                    printf("Bloqueando %d", aux->id);
                    mover_proceso_de_lista(lista_exec, lista_blocked, index, BLOCKED);
                    encontrado = true;
                }
            index ++;
        }
        if(encontrado){

            sem_wait(&mutex_multiprocesamiento);
            *multiprocesamiento = *multiprocesamiento + 1;
            sem_post(&mutex_multiprocesamiento);
            index = 0;

        }
        if(*multiprocesamiento == 0){
            printf("ME quede sin multiprocesamiento\n");
        }
    }
}