#include "memoria.h"

int main(int argc, char **argv)
{
    system("clear");
    logger_memoria = log_create("./cfg/memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);
    log_info(logger_memoria, "Programa inicializado correctamente");
   
    t_config *config = leer_config_file("./cfg/memoria.cfg");
    config_memoria = generarConfigMemoria(config);
    log_info(logger_memoria, "Configuración cargada correctamente");
    tamanio_memoria = malloc(config_memoria->TAMANIO);
    socket_server = iniciar_servidor(config_memoria->IP, (config_memoria->PUERTO), logger_memoria);
    
    socket_cliente_swap = crear_conexion(config_memoria->IP_SWAP, "5002");
    if (socket_cliente_swap == -1) {
        log_info(logger_memoria, "Fallo en la conexion a swap");
    }

   

        tabla_tlb = malloc(sizeof(t_tabla_tlb));
        tabla_tlb->tlb = list_create();
        tabla_tlb->hit_totales = 0;
        tabla_tlb->miss_totales = 0;

        tlb_LRU = list_create();
        tlb_FIFO = queue_create();
        entradas_tlb = config_memoria->CANTIDAD_ENTRADAS_TLB;

        if(strcmp(config_memoria->TIPO_ASIGNACION, "FIJA") != 0){
            if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "LRU") == 0) {
                reemplazo_LRU = list_create();
            }else 
            {
                t_lista_clock *reemplazo_CLOCK = malloc(sizeof(t_lista_clock));
                reemplazo_CLOCK->Clock = list_create();
                reemplazo_CLOCK->ultimo = 0;
            }
        }

        tabla_marcos_memoria = malloc(sizeof(t_tabla_marcos));
        tabla_marcos_memoria->marcos = list_create();

        tabla_procesos = list_create();

        int i = 0;
        uint32_t cant_marcos = config_memoria->TAMANIO / config_memoria->TAMANIO_PAGINA;;
        while (i < cant_marcos)
        {
            t_marco *tlb = malloc(sizeof(t_marco));
            tlb->numero_marco = i;
            list_add(tabla_marcos_memoria->marcos, tlb);
            i++;
        }
        signal(SIGINT, imprimirMetricas);
        signal(SIGUSR1, generarDump);
        signal(SIGUSR2, limpiarTlb);

    indice = 0;
    sem_init(&mutex_memoria,0,1);
    while (1)
    {   
        printf("Esperando cliente\n");
        int socket_client = esperar_cliente(socket_server, logger_memoria);
        int *socekt = malloc(sizeof(int));
        *socekt = socket_client;
        if (socket_client != -1)
        {
            //inicializarCarpincho(socket_client);
            pthread_t hilo_client;
            pthread_create(&hilo_client, NULL, (void *)ejecutar_operacion, (void *)socekt);
        }
    }
    log_info(logger_memoria, "Programa finalizado con éxito");
    log_destroy(logger_memoria);
    liberar_config(config);
    free(config_memoria);
}

static void *ejecutar_operacion(int *socket_client_p)
{
    int socket_client = *socket_client_p;
    while (1)
    {   
        log_info(logger_memoria, "Esperando recibir paquete por parte de kernel %d", socket_client);        
        t_paquete *paquete = recibir_paquete(socket_client);
        log_info(logger_memoria, "Paquete recibido codigo de op %d", paquete->codigo_operacion);

        //Analizo el código de operación recibido y ejecuto acciones según corresponda
        switch (paquete->codigo_operacion)
        {
        case CLIENTE_TEST:
            log_info(logger_memoria, "Mensaje de prueba recibido correctamente por el cliente %d", socket_client);
            break;
        case MEMALLOC:
        sem_wait(&mutex_memoria);
            log_info(logger_memoria, "MEMALLOC %d", socket_client);
            
            int dire_logica =memAlloc(paquete);
            
            if(dire_logica <0){
                //NO HAY MEMORIA Y SWAP NO PUDO GUARDAR
                t_paquete* paquete_enviar = serializar(DIRECCION_LOGICA_INVALIDA,2,INT,dire_logica);
                log_info(logger_memoria,"No hay memoria suficiente ni en swap. No se pudo guardar lo pedido.");
                enviar_paquete(paquete_enviar,socket_client);
            }
            else{
                //DIRECCION_LOGICA VALIDA
                t_paquete* paquete_enviar = serializar(MEMALLOC,2,INT,dire_logica);
                log_info(logger_memoria,"Enviando paquete con direccion logica");
                enviar_paquete(paquete_enviar,socket_client);
                log_info(logger_memoria,"Paquete enviado");
            }
            sem_post(&mutex_memoria);
            break;
        case MEMFREE:
            sem_wait(&mutex_memoria);
            log_info(logger_memoria, "recibi orden de memfree del cliente %d", socket_client);
   
            int resultado_free = freeAlloc(paquete);

            if(resultado_free < 0){
                //NO SE PUDO LIBERAR
                t_paquete* paquete_enviar = serializar(DIRECCION_LOGICA_INVALIDA,2,INT,0);
                log_error(logger_memoria,"No se pudo liberar la direccion logica de memoria solicitada.");
                enviar_paquete(paquete_enviar,socket_client);
            }
            else{
                //SI SE PUDO LIBERAR
                t_paquete* paquete_enviar = serializar(MEMFREE,2,INT,0);
                log_info(logger_memoria,"Se pudo liberar el espacio de memoria en la direccion logica solicitada.");
                enviar_paquete(paquete_enviar,socket_client);
                log_info(logger_memoria,"Paquete enviado");
            }
             sem_post(&mutex_memoria);
            break;

        case MEMWRITE:
            sem_wait(&mutex_memoria);
            log_warning(logger_memoria, "MEMWRITE %d", socket_client);
            
            int resultado_write = memWrite(paquete);

            if(resultado_write< 0){
                //NO SE PUDO ESCRIBIR
                t_paquete* paquete_enviar = serializar(DIRECCION_LOGICA_INVALIDA,2,INT,0);
                log_error(logger_memoria,"No se pudo escribir en la direccion logica solicitada.");
                enviar_paquete(paquete_enviar,socket_client);
            }
            else{
                //SI SE PUDO ESCRIBIR
                t_paquete* paquete_enviar = serializar(MEMWRITE,2,INT,0);
                log_info(logger_memoria,"Enviando la direccion logica solicitada.");
                enviar_paquete(paquete_enviar,socket_client);
                log_info(logger_memoria,"Paquete enviado");
            }
             sem_post(&mutex_memoria);
            break;
        case MEMREAD:
            sem_wait(&mutex_memoria);
            log_warning(logger_memoria, "MEMREAD %d", socket_client);
            char *data = memRead(paquete);

            if(string_equals_ignore_case(data,"FAIL")){
                //NO SE PUDO LEER
                t_paquete* paquete_enviar = serializar(DIRECCION_LOGICA_INVALIDA,2,INT,0);
                log_info(logger_memoria,"No se pudo leer de la direccion logica solicitada.");
                enviar_paquete(paquete_enviar,socket_client);
            }
            else{
                //SI SE PUDO LEER
                t_paquete* paquete_enviar = serializar(MEMREAD,6,CHAR_PTR,data,INT,strlen(data),INT,0);
                log_info(logger_memoria,"Se pudo leer en la direccion logica solicitada.");
                enviar_paquete(paquete_enviar,socket_client);
                log_info(logger_memoria,"Paquete enviado");
            }
            sem_post(&mutex_memoria);
            break;

        case NUEVO_CARPINCHO:
            log_warning(logger_memoria,"NUEVO CARPINCHO %d", socket_client);
            t_paquete *paquete = malloc(sizeof(t_paquete));
            paquete->buffer = malloc(sizeof(t_buffer));
            paquete->buffer->size = 0;
            paquete->buffer->stream = NULL;
            paquete->codigo_operacion = OP_CONFIRMADA;
           
            enviar_paquete(paquete, socket_client);
            break;
            
        default:
            log_error(logger_memoria, "Codigo de operacion desconocido. Codigo operacion recibida: %d",paquete->codigo_operacion);
            t_paquete *paquete_enviado_error = serializar(DIRECCION_LOGICA_INVALIDA,2,INT,0);
            enviar_paquete(paquete_enviado_error, socket_client);
            
            break;
        }

        free(paquete);
    }

    close(socket_client);
    log_info(logger_memoria, "Se desconecto el cliente [%d]", socket_client);
    return NULL;
}
void limpiarTlb()
{
    log_info(logger_memoria, "SEÑAL RECIBIDA LIMPIANDO TLB");
    list_clean(tabla_tlb->tlb);
    log_info(logger_memoria, "TLB VACIA");
}
void generarDump()
{
   log_info(logger_memoria, "SEÑAL RECIBIDA GENERANDO DUMP");
   memdump();
   log_info(logger_memoria, "DUMP GENERADO");
}
void imprimirMetricas()
{
    metricas();
    log_info(logger_memoria, "SEÑAL RECIBIDA");
    exit(EXIT_SUCCESS);
}

void metricas(){
    t_list_iterator *list_iterator = list_iterator_create(tabla_procesos);
    t_tabla_paginas *tablas;

    log_info("HITS TOTALES: %d \n MISS TOTALES: %d \n", tabla_tlb->hit_totales, tabla_tlb->miss_totales);
    while (list_iterator_has_next(list_iterator))
    {
        tablas = list_iterator_next(list_iterator);
        log_info("CARPINCHO: %d HITS POR CARPINCHO: %d MISS POR CARPINCHO: %d \n", tablas->pid, tablas->hit, tablas->miss);
    }
}

void inicializarCarpincho(int socket_cliente)
{

    //t_mateinit_serializado *mateInit_deserializado = deserializar_mate_init(paquete);
    printf("Inicializo carpincho\n");
    uint32_t id = socket_cliente;

    t_tabla_paginas *nuevaTabla = malloc(sizeof(t_tabla_paginas));

    nuevaTabla->pid = id;
    nuevaTabla->paginas = list_create();
    nuevaTabla->Lru = list_create();
    nuevaTabla->Clock = list_create();
    nuevaTabla->paginas_en_memoria = 0;
    nuevaTabla->paginas_totales_maximas = config_memoria->TAMANIO / config_memoria->TAMANIO_PAGINA;
    nuevaTabla->hit = 0;
    nuevaTabla->miss = 0;
    list_add(tabla_procesos, nuevaTabla);
    tabla_paginas = nuevaTabla;
    return;
}