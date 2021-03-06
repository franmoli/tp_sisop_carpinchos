# Interfaz para Biblioteca TP 2C2021 馃

La propuesta de este repo es poder dejar delineada una interfaz de la
biblioteca planteada en el en el TP de Sistemas Operativos de la UTN FRBA
para el 2do cuatrimestre de 2021.

Por un lado, tenemos el archivo `matelib.h`, que es un archivo de Headers para
para usar, en el que cada equipo tiene que implementar la funcionalidad de
cada funci贸n definida all铆.

## Conceptos

### 驴Qu茅 es un archivo de Headers?
Hay muchas definiciones dando vueltas, su utilidad y detalles de su
implementaci贸n. Pero, en pocas palabras, es una buena forma de poner
definiciones y valores constantes que va a utilizar un programa en C.

Est谩 bueno para definir `los prototipos` de las funciones (prototipo,
interfaz, interface, elija el nombre que m谩s guste) de un m贸dulo,
en este caso la biblioteca que van a usar los distintos procesos para
interactuar con el resto del sistema.

### 驴Una biblioteca?
La forma que tiene C (y la mayor铆a de los lenguajes de programaci贸n) para poder
compartir c贸digo entre distintos componentes, es crear una biblioteca.

Dado que en el TP, los puntos de entrada son programas fuera de nuestro
control, la interacci贸n de esos programas con el sistema va a ser mediante una
biblioteca que nos va a permitir:
- Planificar su ejecuci贸n.
- Gestionar memoria en un sistema centralizado.
- Hacer uso de recursos compartidos, como sem谩foros.

Pero **haciendo uso de lenguajes de programaci贸n existentes**, en vez de
alg煤n lenguage propio que requiera un parser especial.

## Uso y funcionamiento esperado de la lib implementada
Antes de empezar a utilizar las funcionalidades de la lib, es necesario
inicializar las distintas estructuras administrativas que la misma va a
usar (como definiciones, conexiones, etc) mediante `mate_init`.

```c
void main() {
    mate_instance* lib;
    char* config_path = "...";
    int status = mate_init(lib, config_path);
    if(status != 0) {
        // Algo fall贸
    };
    // Ahora podemos usar lib como referencia.
};
```

Esta funci贸n recibe el path **relativo** del archivo de config a usar y un
puntero al la instancia de la "matelib" que vamos a usar para interactuar.
La especificaci贸n de la configuraci贸n ser谩 an谩loga a la de los otros
componentes del TP en forma, delegando al grupo la implementaci贸n para leer
el archivo e interpretar las distintas configuraciones.
**Como m铆nimo, el archivo de config debe contener**:
- IP de Conexi贸n al Backend
- Puerto de Conexi贸n al Backend

#### Backends
Un backend puede ser tanto un proceso Kernel, como un proceso Memoria.
Cada uno ser谩 identificado a la hora de realizar el hanshake de inicio,
permitiendo m谩s o menos operaciones dependiendo de cual de los 2 sea.

En el caso del Kernel, se permitir谩n todas las operaciones, realizando un
"pasamanos" este 煤ltimo de los comandos al proceso Memoria.
Sin embargo, en el caso de la Memoria, solo los m茅todos de inicializaci贸n y
cierre; y de memoria, se encontrar谩n disponibles, *fallando el resto al tratar de usarse*.

*Es importante recalcar*, que en las operaciones de memoria, el grupo genere
la misma interfaz para tanto el m贸dulo de Kernel, como para el de Memoria,
a fin de simplificar la implementaci贸n de la interacci贸n.


#### mate_instance
La estructura de `mate_instance` en la especificaci贸n del TP se encuentra
vac铆a, con solo un puntero a alg煤n tipo de estructura.
Es responsabilidad del grupo llenar dentro de la entidad todas las
referencias necesarias para mantener vivas las conexiones y estructuras
necesarias para operar con la misma.

El instancia ser谩 煤til para referenciar distintos inits que se pueda hacer
en un mismo proceso.

Para identificar una instanciaci贸n, cada grupo deber谩 elegir alguna forma de
identificar cada una de estas instancias mediante alg煤n medio (sean UUIDv4,
combinaciones de IPs, pids, n煤meros autogenerados, etc).


Como todo lo que se abre tiene que cerrarse, al finalizar de usar la lib, es
necesario llamar a `mate_close` usando la instancia de la lib como par谩metro,
en el que la instancia de `mate_instance` deber谩 ser usada para librar los
recursos que referencia.

### Limitaciones de Implementaci贸n
Existen determinadas limitaciones y recomendaciones constructivas al usar
la lib en un proceso propio (y utilizables a la hora de testearla).

1. Esta lib puede no ser fork-safe, porque **se permiten File
Descriptors abiertos** y no se espera que se realice un manejo de los mismos
ante un escenario de fork.

> Sin embargo, cada grupo puede optar por evitar esta limitaci贸n mediante su
propia implementaci贸n. Pero no es un requerimiento obligatorio.

2. Esta lib debe soportar ser usada entre **varios threads**, cada uno con
realizando su propio `mate_init`. Es decir, la implementaci贸n de los grupos
**no puede hacer uso de estructuras y variables globales que puedan
generar conflictos entre varias llamadas a dicha funci贸n**.


## 驴C贸mo hago para implementar la lib para mi TP?
En este repo, en la carpeta `examples/lib_implementation`, hay un ejemplo de
implementaci贸n de la lib.
All铆 hacemos uso del `void*` de la estructura para usar una nuestra,
que manejaremos de forma interna con la informaci贸n necearia para cada uno de
los m贸dulos del TP.

Para este ejemplo, por simplicidad, solo va a responder las direcciones de
memoria `0`, el IO `PRINTER` y el sem谩foro `SEM1`, pero el esp铆ritu es el mismo.

En la carpeta `examples/lib_usage` poseemos un ejemplo de programa
que hace uso de la lib que creamos y de las funcionalidades expuestas.

Para poder compilar todo, tenemos un Makefile que tiene 3 directivas, una para
compilar la lib, una para compilar al ejemplo que la usa, y finalmente, una
para correrlo (4 en realidad, con la de limpieza).

Para ver el ejemplo funcionar, ejecuta `make run`.


> **Nota**:
> Si revisan el ejemplo pueden ver como la referencia de la lib puede viajar
> entre hilos! Esto fue necesario porque solo implementamos el sem谩foro, de
> "forma local"; pero cada hilo podr铆a hacer uso de su propio init!

## Requerimientos no funcionales
- Permitir un nivel de loggeo a modo de "debug", dado por config, de las
acciones de la biblioteca.
- Se deben utilizar las funciones de Kernel llamadas *bloqueantes*, para
poder planificar un proceso, no generando esperas activas.
- Las direcciones de memoria de la lib a utilizar, van a ser definidas como
int32_t (Enteros de 32bits con signo), para evitar problemas de tipado contra punteros locales.
