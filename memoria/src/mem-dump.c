#include "mem-dump.h"

int createFile(char *path)
{
    /*
        1: fue creado con exito
        0: no fue creado porque existe
    */
    int status = 0;
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        //No existe
        file = fopen(path, "w+");
        status = 1;
    }
    return status;
}
void memdump()
{
    char *path_folder_dump = string_new();
    char *timestamp = temporal_get_string_time("Dump_%y%m%d%H%M%S.dmp");
    string_append_with_format(&path_folder_dump, "%s", timestamp);

    createFile(path_folder_dump);
    int fd = open(path_folder_dump, O_RDWR | (O_APPEND | O_CREAT), S_IRUSR | S_IWUSR);

   char *contenido = string_new();
   string_append(&contenido, dumpPaginacion());
   write(fd, contenido, string_length(contenido));
   close(fd);
}


char *dumpPaginacion()
{
    char *texto = string_new();
    char *timestamp = temporal_get_string_time("%d/%m/%y %H:%M:%S");
    string_append_with_format(&texto, "Dump: %s", timestamp);


    

    t_list_iterator *list_iterator = list_iterator_create(tabla_procesos);
    t_tabla_paginas *tablas;
    while (list_iterator_has_next(list_iterator))
    {
        tablas = list_iterator_next(list_iterator);
        texto = cargarTexto(tablas->paginas,tablas->pid);

    }
    return texto;
}
char* cargarTexto(t_list *paginas, int carpincho_id){

    t_list_iterator *list_iterator = list_iterator_create(paginas);
    t_pagina *pagina;
    char* texto = string_new();
    char* estado= string_new();
    int i = 0;
    while (list_iterator_has_next(list_iterator))
    {
        pagina = list_iterator_next(list_iterator);
        if(pagina->tamanio_ocupado == 0){
            estado = "Libre";
        }else{
            estado="Ocupado";
        }
        string_append_with_format(&texto, "Entrada:%d	Estado:%s	Carpincho: %d	Pagina: %d	Marco: %d \n",i,estado, carpincho_id,pagina->numero_pagina, pagina->marco_asignado);
        i++;
    }
    return texto;
}