#include "matelib.h"
/*
static void *realizar_operacion(t_paquete* paquete){
    t_paquete* paquete_a_enviar;
	//Armar switch para analizar dependiendo del codigo_operacion
    enviar_paquete(paquete_a_enviar,socket_kernel);
    log_info(logger_matelib,"Se envio el paquete a kernel");
    return NULL;
}*/

int32_t obtenerIDRandom(){
    srand(0);
    return rand();
}

t_config_matelib* obtenerConfig(char* config){
    t_config *cfg;
    t_config_matelib* config_mate;
    cfg = leer_config_file(config);
    config_mate = generar_config_matelib(cfg);
    return config_mate;
}

//-----------------------------------Instanciacion -----------------------------------

int mate_init(mate_instance *lib_ref, char *config){
    char *string = malloc(sizeof(char)*50);

    lib_ref = malloc(sizeof(mate_instance));
    config_matelib = obtenerConfig(config);
    
    lib_ref->id             = obtenerIDRandom();
    lib_ref->config         = config_matelib;
    lib_ref->group_info = malloc(sizeof(mate_inner_structure));

    sprintf(string,"%d",lib_ref->id);
    strcpy(string,strcat(string,".cfg"));
    
    //TODO Fijarse como usar el log_level_debug para instanciarlo desde config (string to enum)
    lib_ref->logger = log_create(strcat("./cfg/",string),"MATELIB",0,LOG_LEVEL_DEBUG);
    log_info(lib_ref->logger,"Acabo de instanciarme");

    free(string);
    return 0;
}

int mate_close(mate_instance *lib_ref){
    free(lib_ref->group_info);
    free(lib_ref);
    return 0;
}

//-----------------------------------Semaforos-----------------------------------

int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value){

    return 1;
}

int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem){

    return 1;
}

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem){

    return 1;
}

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem){

    return 1;
}

//-----------------------------------Funcion Entrada/Salida-----------------------------------

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg){

    return 1;
}

//-----------------------------------Funciones Modulo Memoria-----------------------------------
/*
mate_pointer mate_memalloc(mate_instance *lib_ref, int size){

    mate_pointer p;

    return p;
}
*/
int mate_memfree(mate_instance *lib_ref, mate_pointer addr){

    return 1;
}

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size){

    return 1;
}

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size){

    return 1;
}