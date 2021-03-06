#include "../include/kernel.h"

int main(int argc, char **argv) {
    //Inicio del programa
    system("clear");
    logger_kernel = log_create("./cfg/kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    log_info(logger_kernel, "Programa inicializado correctamente ");
    terminar_kernel = false;

    //Se carga la configuración
    log_info(logger_kernel, "Iniciando carga del archivo de configuración");
    config_file = leer_config_file("./cfg/kernel.cfg");
    config_kernel = generar_config_kernel(config_file);
    log_info(logger_kernel, "Configuración cargada correctamente");

    //Iniciar semaforos de uso general
    iniciar_semaforos_generales();

    //Inicio variables globales generales
    iniciar_variables_generales();
    

    //Iniciar listas de procesos
    iniciar_listas();
    
    //Iniciar planificador de largo plazo
    iniciar_planificador_largo();
    //Iniciar planificador de corto plazo
    iniciar_planificador_corto();
    //Iniciar planificador de mediano plazo
    iniciar_planificador_mediano();
    
    //Iniciar el análisis de deadlock
    iniciar_deadlock();

    //Iniciar consola para debug
    iniciar_debug_console();
    
    
    //Conectar a memoria (datos temporales hardcodeados)
    socket_cliente_memoria = crear_conexion(config_kernel->IP_MEMORIA, config_kernel->PUERTO_MEMORIA);
    if(socket_cliente_memoria == -1){
        log_error(logger_kernel, "Fallo en la conexion a memoria");
    }

    //bloquear con algo
    while(!terminar_kernel);

    sleep(config_kernel->TIEMPO_DEADLOCK/1000);

    pthread_join(hilo_deteccion_deadlock,NULL);

    //Fin del programa
    liberar_memoria_y_finalizar(config_kernel, logger_kernel, config_file);
    return 0;
}



void liberar_memoria_y_finalizar(t_config_kernel *config_kernel, t_log *logger_kernel, t_config *config_file){
    config_destroy(config_file);
    destruir_semaforos();
    destruir_listas();
    list_destroy_and_destroy_elements(config_kernel->DISPOSITIVOS_IO, destruir_cosas);
    list_destroy_and_destroy_elements(config_kernel->DURACIONES_IO, destruir_cosas);
    free(config_kernel);
    log_info(logger_kernel, "Programa finalizado con éxito");
    log_destroy(logger_kernel);
}

void element_destroyer(void* elemento){
    t_proceso *procesou = elemento;
    printf("Proceso %d size %d\n", procesou->id, list_size(procesou->task_list));
    list_destroy_and_destroy_elements(procesou->task_list, destruir_cosas);
    free(elemento);
}
void destruir_cosas(void *elemento){
    free(elemento);
}
void iniciar_listas(){
    
    lista_new = list_create();
    lista_ready = list_create();
    lista_exec = list_create();
    lista_blocked = list_create();
    lista_s_blocked = list_create();
    lista_s_ready = list_create();
    lista_semaforos = list_create();
    lista_exit = list_create();
    lista_recursos_asignados = list_create();

    sem_init(&mutex_listas, 0, 1);
    
    cantidad_de_procesos = 0;
    log_info(logger_kernel, "Listas inicializadas correctamente");
    
    return;
}

void iniciar_semaforos_generales(){
    sem_init(&proceso_finalizo_o_suspended, 0, 0);
    sem_init(&salida_exec, 0, 0);
    sem_init(&salida_block, 0, 0);
    sem_init(&actualizacion_de_listas_1, 0, 0);
    sem_init(&actualizacion_de_listas_2, 0, 0);
    sem_init(&actualizacion_de_listas_1_recibido, 0, 0);
    sem_init(&proceso_inicializado, 0, 0);
    sem_init(&libre_para_inicializar_proceso, 0, 1);
    sem_init(&mutex_multiprocesamiento, 0, 1);
    sem_init(&mutex_cant_procesos, 0, 1);
    sem_init(&mutex_multiprogramacion, 0, 1);
    sem_init(&salida_a_exit, 0, 0);
    sem_init(&liberar_multiprocesamiento, 0, 0);
    sem_init(&salida_de_exec_recibida, 0, 0);
    sem_init(&salida_a_exit_recibida, 0, 0);
    sem_init(&cambio_de_listas, 0, 0);
    sem_init(&cambio_de_listas_largo, 0, 0);
    sem_init(&cambio_de_listas_mediano, 0, 0);
    sem_init(&cambio_de_listas_corto, 0, 0);
    sem_init(&pedir_salida_de_block, 0, 0);
    sem_init(&solicitar_block, 0, 0);
    sem_init(&mutex_semaforos, 0, 1);
    sem_init(&mutex_recursos_asignados, 0, 1);
    io_libre = calloc(&io_libre,list_size(config_kernel->DISPOSITIVOS_IO));
    for(int i = 0; i<list_size(config_kernel->DISPOSITIVOS_IO); i++){
        sem_init(&io_libre[i], 0, 1);
    }
    return;
}

void mover_proceso_de_lista(t_list *origen, t_list *destino, int index, int status){

    t_proceso *aux;

    aux = list_remove(origen, index);
    printf("Moviendo proceso - %d | to: %d\n", aux->id , status);
    
    aux->status =  status;
    aux->termino_rafaga = false;

    if(status == READY)
        aux->entrada_a_ready = clock();
    
    if(status == EXIT){
        avisar_cambio();
        cantidad_de_procesos--;
    }
    if(status == NEW)
        cantidad_de_procesos++;

    if(status == BLOCKED)
        multiprocesamiento++;

    if(status == EXEC)
        multiprocesamiento--;

    list_add(destino, aux);

    avisar_cambio();
    return;
}

void avisar_cambio(){
    sem_wait(&mutex_cant_procesos);
    //Aviso que hubo un cambio de listas
    //printf("CANTIDAD DE PROCESOS: %d\n", cantidad_de_procesos);
    for(int i = 0; i < cantidad_de_procesos; i++){
        //printf("Un post\n");
        sem_post(&actualizacion_de_listas_1);
    }

    //Espero que todos los procesos hayan recibido el aviso y ejecutado
    for(int i = 0; i < cantidad_de_procesos; i++){
        //printf("Un wait\n");        
        sem_wait(&actualizacion_de_listas_1_recibido);
    }
    //Habilito que vuelvan a esperar una vez ya resuelto todo lo que tengan que hacer con su nuevo estado
    for(int i = 0; i < cantidad_de_procesos; i++){
        sem_post(&actualizacion_de_listas_2);
    }

    //Habilito planificadores
    sem_post(&cambio_de_listas_largo);
    sem_post(&cambio_de_listas_corto);
    sem_post(&cambio_de_listas_mediano);

    sem_post(&mutex_cant_procesos);
}

void iniciar_debug_console(){
    pthread_t hilo_console;
    pthread_create(&hilo_console, NULL, debug_console, (void *)NULL);
    pthread_detach(hilo_console);
    return;
}

//funciones de debug
void *debug_console(void *_ ){
    log_info(logger_kernel, "Debug console active");
    char input[100] = {0};

    while(!terminar_kernel){
        fgets(input, 100, stdin);

        if(string_contains(input, "Texto")){
            printf("Comando de prueba\n");
        }
        if(string_contains(input, "init")){
            print_inicializacion(config_kernel);
        }
        if(string_contains(input, "sem")){
            print_semaforos();
        }
        if(string_contains(input, "task")){
            print_task_lists();
        }
        if(string_contains(input, "exit")){
            void cerrar_conexion(void *elemento){
                t_proceso *carpincho = elemento;
                carpincho->status = EXIT;
                close(carpincho->id);
            };
            list_iterate(lista_blocked, cerrar_conexion);
            list_iterate(lista_ready, cerrar_conexion);
            list_iterate(lista_exec, cerrar_conexion);
            list_iterate(lista_new, cerrar_conexion);
            list_iterate(lista_s_ready, cerrar_conexion);
            list_iterate(lista_s_blocked, cerrar_conexion);
            shutdown(socket_servidor_kernel, SHUT_RDWR);
            sleep(2);
            terminar_kernel = true;
            avisar_cambio();
            sem_post(&salida_a_exit);
            sem_post(&solicitar_block);
            sem_post(&salida_block);
            sleep(1);
        }
        if(string_contains(input, "list")){
            print_lists();
        }
        if(string_contains(input, "multiprog")){
            printf("Multiprogramacion disponible %d\n", multiprogramacion_disponible);
        }
        if(string_contains(input, "process")){
            printf("Procesos activos %d\n", cantidad_de_procesos);
        }
        if(string_contains(input, "asign")){
            print_sem_asignados();
        }
        if(string_contains(input, "sem_t")){
            print_sem_tipe();
        }
        if(string_contains(input, "blocking")){
            printf("Procesos esperando bloqueo %d\n", procesos_esperando_bloqueo);
        }

    }

    printf("Termino el debug console\n");
    return NULL;
}

void print_semaforos(){
    int index = 0;
    int index2 = 0;
    t_semaforo *aux = NULL;
    int *solicitante = NULL;
    printf("Printing semaphores %d:\n", list_size(lista_semaforos));

    while(index < list_size(lista_semaforos)){
        aux = list_get(lista_semaforos, index);
        printf("Semaforo \"%s\" - Value %d  - Solicitantes:\n",aux->nombre_semaforo, aux->value, list_size(aux->solicitantes));
        
        if(list_size(aux->solicitantes)){
            while(index2 < list_size(aux->solicitantes)){
                solicitante = list_get(aux->solicitantes, index2);
                printf("Solicitante del semaforo :%d\n", *solicitante);
                index2++;
            }
        }
        printf("\n");
        index2 = 0;
        index++;
    }
}

void print_inicializacion (t_config_kernel *config_kernel){
    
    // se printea la informacion cargada
    printf("IP MEMORIA: %s\n", config_kernel->IP_MEMORIA);
    printf("PUERTO_MEMORIA: %s\n", config_kernel->PUERTO_MEMORIA);
    printf("ALGORITMO_PLANIFICACION: %s\n", config_kernel->ALGORITMO_PLANIFICACION);
    printf("RETARDO_CPU: %d\n", config_kernel->RETARDO_CPU);
    printf("GRADO_MULTIPROGRAMACION: %d\n", config_kernel->GRADO_MULTIPROGRAMACION);
    printf("GRADO_MULTIPROCESAMIENTO: %d\n", config_kernel->GRADO_MULTIPROCESAMIENTO);
    printf("ESTIMACION_INICIAL: %d\n", config_kernel->ESTIMACION_INICIAL);
    printf("ALFA: %d\n", config_kernel->ALFA);
    printf("DISPOSITIVOS IO:\n");

    t_list_iterator *io_devices_iterator = list_iterator_create(config_kernel->DISPOSITIVOS_IO);
    while(list_iterator_has_next(io_devices_iterator)) {
        printf("- %s\n", (char*)list_iterator_next(io_devices_iterator));
    }

    list_iterator_destroy(io_devices_iterator);
    printf("DURACIONES IO:\n");
    char* duraciones_itoa = string_itoa(list_size(config_kernel->DURACIONES_IO));
    log_info(logger_kernel, duraciones_itoa);
    t_list_iterator *io_durations_iterator = list_iterator_create(config_kernel->DURACIONES_IO);
    while(list_iterator_has_next(io_durations_iterator)) {
        printf("- %s\n", (char*) list_iterator_next(io_durations_iterator));
    }
    list_iterator_destroy(io_durations_iterator);
    free(duraciones_itoa);
}

void print_task_lists(){
    int index =  0;
    int index2 =  0;
    t_task *aux_task;
    t_proceso *aux_proceso;
    t_semaforo *op_semaforo = NULL;

    while(index <= list_size(lista_ready)-1){
        aux_proceso = list_get(lista_ready, index);
        
        
        while(index2 <= list_size(aux_proceso->task_list)-1){

            aux_task = list_get(aux_proceso->task_list, index2);
            if(aux_task != NULL)
                printf("\bTask - Id : %d\n", aux_task->id);
                op_semaforo = aux_task->datos_tarea;
                printf("Nombre-sem %s\n", op_semaforo->nombre_semaforo);
            
            aux_task = NULL;
            index2++;
        }

        index++;
    }
    printf("Pass\n");
}

void print_lists(){
    int index = 0;
    t_proceso *aux = NULL;

    printf("Printeando gente en new %d\n", list_size(lista_new));
    while(index < list_size(lista_new)){
        aux = list_get(lista_new, index);
        printf("Carpincho %d en new \n", aux->id);
        index++;
    }
    printf("\n");
    index = 0;

    printf("Printeando gente en blocked %d\n\n", list_size(lista_blocked));
    while(index < list_size(lista_blocked)){
        aux = list_get(lista_blocked, index);
        printf("Carpincho %d en blocked \n", aux->id);
        index++;
    }

    printf("\n");
    index = 0;

    printf("Printeando gente en ready %d\n\n", list_size(lista_ready));
    while(index < list_size(lista_ready)){
        aux = list_get(lista_ready, index);
        printf("Carpincho %d en ready \n", aux->id);
        index++;
    }

    printf("\n");
    index = 0;

    printf("Printeando gente en exec %d\n\n", list_size(lista_exec));
    while(index < list_size(lista_exec)){
        aux = list_get(lista_exec, index);
        printf("Carpincho %d en exec \n", aux->id);
        index++;
    }

    printf("\n");
    index = 0;

    printf("Printeando gente en suspended block %d\n\n", list_size(lista_s_blocked));
    while(index < list_size(lista_s_blocked)){
        aux = list_get(lista_s_blocked, index);
        printf("Carpincho %d en s_block \n", aux->id);
        index++;
    }

    printf("\n");
    index = 0;

    printf("Printeando gente en suspended ready %d\n\n", list_size(lista_s_ready));
    while(index < list_size(lista_s_ready)){
        aux = list_get(lista_s_ready, index);
        printf("Carpincho %d en s_ready \n", aux->id);
        index++;
    }

    printf("\n");
    index = 0;

    printf("Printeando gente en exit %d\n\n", list_size(lista_exit));
    while(index < list_size(lista_exit)){
        aux = list_get(lista_exit, index);
        printf("Carpincho %d en exit \n", aux->id);
        index++;
    }

    printf("\n");

}

void print_sem_asignados(){
    int index = 0;
    t_recurso_asignado *aux = NULL;
    printf("Printing semaphores %d:\n", list_size(lista_recursos_asignados));

    while(index < list_size(lista_recursos_asignados)){
        aux = list_get(lista_recursos_asignados, index);

        printf("Semaforo \"%s\" - Asignado a %d:\n",aux->nombre_recurso, aux->id_asignado);
        printf("\n");
        index++;
    }
}

void print_sem_tipe(){
    int aux = -100;
    sem_getvalue(&mutex_listas, &aux);
    printf("Sem mutex_listas : %d\n", aux);
    sem_getvalue(&proceso_finalizo_o_suspended, &aux);
    printf("Sem proceso_finalizo_o_suspended : %d\n", aux);
    sem_getvalue(&salida_exec, &aux);
    printf("Sem salida_exec : %d\n", aux);
    sem_getvalue(&salida_block, &aux);
    printf("Sem salida_block : %d\n", aux);
    sem_getvalue(&actualizacion_de_listas_1, &aux);
    printf("Sem actualizacion_de_listas_1 : %d\n", aux);
    sem_getvalue(&actualizacion_de_listas_2, &aux);
    printf("Sem actualizacion_de_listas_2 : %d\n", aux);
    sem_getvalue(&actualizacion_de_listas_1_recibido, &aux);
    printf("Sem actualizacion_de_listas_1_recibido : %d\n", aux);
    sem_getvalue(&proceso_inicializado, &aux);
    printf("Sem proceso_inicializado : %d\n", aux);
    sem_getvalue(&libre_para_inicializar_proceso, &aux);
    printf("Sem libre_para_inicializar_proceso : %d\n", aux);
    sem_getvalue(&mutex_multiprocesamiento, &aux);
    printf("Sem mutex_multiprocesamiento : %d\n", aux);
    sem_getvalue(&mutex_multiprogramacion, &aux);
    printf("Sem mutex_multiprogramacion : %d\n", aux);
    sem_getvalue(&mutex_cant_procesos, &aux);
    printf("Sem mutex_cant_procesos : %d\n", aux);
    sem_getvalue(&salida_a_exit, &aux);
    printf("Sem salida_a_exit : %d\n", aux);
    sem_getvalue(&liberar_multiprocesamiento, &aux);
    printf("Sem liberar_multiprocesamiento : %d\n", aux);
    sem_getvalue(&salida_a_exit_recibida, &aux);
    printf("Sem salida_a_exit_recibida : %d\n", aux);
    sem_getvalue(&salida_de_exec_recibida, &aux);
    printf("Sem salida_de_exec_recibida : %d\n", aux);
    sem_getvalue(&cambio_de_listas, &aux);
    printf("Sem cambio_de_listas : %d\n", aux);
    sem_getvalue(&cambio_de_listas_largo, &aux);
    printf("Sem cambio_de_listas_largo : %d\n", aux);
    sem_getvalue(&cambio_de_listas_mediano, &aux);
    printf("Sem cambio_de_listas_mediano : %d\n", aux);
    sem_getvalue(&cambio_de_listas_corto, &aux);
    printf("Sem cambio_de_listas_corto : %d\n", aux);
    sem_getvalue(&pedir_salida_de_block, &aux);
    printf("Sem pedir_salida_de_block : %d\n", aux);
    sem_getvalue(&solicitar_block, &aux);
    printf("Sem solicitar_block : %d\n", aux);
    
}

void destruir_semaforos(){
    sem_destroy(&proceso_finalizo_o_suspended);
    sem_destroy(&salida_exec);
    sem_destroy(&salida_block);
    sem_destroy(&actualizacion_de_listas_1);
    sem_destroy(&actualizacion_de_listas_2);
    sem_destroy(&actualizacion_de_listas_1_recibido);
    sem_destroy(&proceso_inicializado);
    sem_destroy(&libre_para_inicializar_proceso);
    sem_destroy(&mutex_multiprocesamiento);
    sem_destroy(&mutex_cant_procesos);
    sem_destroy(&mutex_multiprogramacion);
    sem_destroy(&salida_a_exit);
    sem_destroy(&liberar_multiprocesamiento);
    sem_destroy(&salida_de_exec_recibida);
    sem_destroy(&salida_a_exit_recibida);
    sem_destroy(&cambio_de_listas);
    sem_destroy(&cambio_de_listas_largo);
    sem_destroy(&cambio_de_listas_mediano);
    sem_destroy(&cambio_de_listas_corto);
    sem_destroy(&pedir_salida_de_block);
    sem_destroy(&solicitar_block);
    sem_destroy(&mutex_semaforos);
    sem_destroy(&mutex_recursos_asignados);
    sem_destroy(&mutex_listas);
    for(int i = 0; i<list_size(config_kernel->DISPOSITIVOS_IO); i++){
        sem_destroy(&io_libre[i]);
    }
    free(io_libre);
    return;
}

void destruir_listas(){
    list_destroy_and_destroy_elements(lista_new,element_destroyer);
    list_destroy_and_destroy_elements(lista_ready,element_destroyer);
    list_destroy_and_destroy_elements(lista_exec,element_destroyer);
    list_destroy_and_destroy_elements(lista_blocked,element_destroyer);
    list_destroy_and_destroy_elements(lista_s_blocked,element_destroyer);
    list_destroy_and_destroy_elements(lista_s_ready,element_destroyer);
    list_destroy_and_destroy_elements(lista_semaforos,element_destroyer);
    list_destroy_and_destroy_elements(lista_exit,element_destroyer);
    list_destroy_and_destroy_elements(lista_recursos_asignados,destruir_cosas);
    return;
}

void iniciar_variables_generales(){
    multiprogramacion_disponible = config_kernel->GRADO_MULTIPROGRAMACION;
    multiprocesamiento = config_kernel->GRADO_MULTIPROCESAMIENTO;
    procesos_esperando_bloqueo = 0;
    return;
}