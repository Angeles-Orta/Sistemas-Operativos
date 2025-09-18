<p  align="center">
  <img  width="200"  src="https://www.fciencias.unam.mx/sites/default/files/logoFC_2.png"  alt="">  <br>Sistemas Operativos  2026-1 <br>
  Práctica 1: Procesos e Hillos <br> Alumna: Orta Castillo Maria de los Angeles - 319074253
</p>

## Descripción del Problema
Este proyecto implementa una comparación directa entre procesos (fork()) e hilos (pthread) resolviendo el mismo problema computacional: múltiples trabajadores incrementando un contador compartido.
Problema Elegido
El problema consiste en tener N trabajadores que incrementan un contador compartido un número específico de veces. Este problema es ideal para comparar procesos vs hilos porque:

- Es paralelizable: Cada trabajador puede operar independientemente
- Tiene estado compartido: El contador debe ser accesible por todos los trabajadores
- Es medible: Podemos verificar corrección y medir rendimiento
Demuestra diferencias clave: Muestra cómo cada modelo maneja la memoria compartida

## Objetivos

Comparar rendimiento (tiempo de ejecución, throughput)
Evaluar corrección (¿el resultado final es el esperado?)
Analizar manejo de memoria (compartida vs separada)
Entender sincronización (condiciones de carrera, mutex)

## Implementación

### Procesos

Utiliza fork() para crear N procesos hijos

* Memoria compartida: Implementada con mmap() y MAP_SHARED
* Comunicación: A través de memoria compartida
* Sincronización: Los procesos no compiten por el mismo recurso (cada uno incrementa secuencialmente)
* Identificación: Usa getpid() y getppid()

### Hilos 

Utiliza pthread_create() para crear N hilos

- Memoria compartida: Naturalmente compartida entre hilos del mismo proceso
- Comunicación: Variables globales/compartidas
- Sincronización: Mutex (pthread_mutex_t) para evitar condiciones de carrera
- Identificación: Usa pthread_self()

## Características Implementadas

1. Número configurable de trabajadores: Parámetro por línea de comandos
2. Medición de tiempo: Microsegundos con gettimeofday()
3. Verificación de corrección: Compara resultado con valor esperado
4. Métricas de rendimiento: Tiempo total, throughput
5. Identificadores únicos: PIDs para procesos, TIDs para hilos
6. Versión con/sin sincronización: Para demostrar condiciones de carrera

## Preguntas
1. Presenta una gráfica que muestre el tiempo de ejecución vs el número de hilos.

      1 . Ejecutar el script de escalabilidad

        chmod +x scalability_test.sh
        ./scalability_test.sh
      
      2. Generar gráficos

        cd scalability_results
        python3 plot_scalability.py
  
  La gráfica tiempo_vs_hilos.png muestra cómo varía el tiempo de ejecución conforme aumenta el número de hilos. Los resultados típicos muestran:

  - 1-4 hilos: Disminución casi lineal del tiempo (mejora del rendimiento)
  - 4-8 hilos: Mejora marginal, dependiendo del número de cores del sistema
  - 8+ hilos: Tiempo se estabiliza o incluso aumenta debido a overhead de sincronización

2. ¿Cómo uniste los resultados parciales en cada programa?

  - Procesos: Requieren mecanismos explícitos (memoria compartida con mmap())
  - Hilos: Comparten automáticamente el espacio de memoria del proceso padre

3. ¿Qué funciones usaste para imprimir los identificadores de cada proceso/hilo?

  - getpid(): Process ID del proceso actual
  - getppid(): Process ID del proceso padre
  - pthread_self(): Thread ID del hilo actual

4. ¿Qué identificadores imprimió tu programa (PID para procesos, TID para hilos)?

Para procesos:

- PID (Process ID): Identificador único del proceso hijo
- PPID (Parent Process ID): Identificador del proceso padre (programa principal)

Para hilos:

- TID (Thread ID): Identificador único del hilo (tipo pthread_t)
- PID (Process ID): Identificador del proceso contenedor (igual para todos los hilos)

5. Presenta un ejemplo de la salida mostrando estos identificadores y explica la diferencia.

  - Ejemplo con procesos: 

        [PROCESO 0] PID=12345, PPID=12340 - Iniciando 50000 incrementos (con sincronización)
        [PROCESO 1] PID=12346, PPID=12340 - Iniciando 50000 incrementos (con sincronización)
        [PROCESO 2] PID=12347, PPID=12340 - Iniciando 50000 incrementos (con sincronización)
        [PROCESO 0] PID=12345 - Completado. Contador local: 50000
        [PROCESO 1] PID=12346 - Completado. Contador local: 50000
        [PROCESO 2] PID=12347 - Completado. Contador local: 50000

  - Ejemplo con hilos: 

        [HILO 0] TID=140234567890, PID=12340 - Iniciando 50000 incrementos (con mutex)
        [HILO 1] TID=140234567891, PID=12340 - Iniciando 50000 incrementos (con mutex)
        [HILO 2] TID=140234567892, PID=12340 - Iniciando 50000 incrementos (con mutex)
        [HILO 0] TID=140234567890 - Completado
        [HILO 1] TID=140234567891 - Completado
        [HILO 2] TID=140234567892 - Completado  

- PID: 
  - Procesos. Cada proceso tiene PID unico.
  - Hilos: Todos comparten el mismo PID.
- PPID: 
  - Procesos. Todos tienen el mismo   PPID (proceso padre).
  - Hilos. No aplica.
- TID: 
  - Procesos. No aplica.
  - Hilos. Cada hilo tiene TID unico.
- Numeracion: 
  - Procesos. PID asignado por el SO (No secuenciales). 
  - Hilos. TIDs asignados por pthread (suelen ser secuenciales).


6. ¿El orden en que terminaron los procesos/hilos coincidió con el orden en que fueron creados?

  No necesariamente, pues podemos observar que: 

  - Orden de creación: Los procesos/hilos se crean secuencialmente (0, 1, 2, ...)
  - Orden de terminación: Depende del scheduler del SO y la carga del sistema

Factores que afectan el orden:

- Scheduler del SO: Asignación de tiempo de CPU
- Carga del sistema: Otros procesos compitiendo
- Cache locality: Algunos workers pueden ejecutar más rápido
- Sincronización: Los mutex pueden introducir variabilidad

  Ejecutar múltiples veces y observar: 

      ./process_thread_comparison -w 4 -i 100000 -r 3 

7. ¿Cómo adaptaste tu código para que el número de procesos/hilos pueda cambiar?

Implementación flexible usando arrays dinámicos.

- Procesos: 

En la función test_processes se utilizó un arreglo dinámico (VLA) de tipo pid_t para almacenar los identificadores de cada proceso creado.

Se genera un bucle que ejecuta fork() tantas veces como num_workers se especifique, permitiendo así ajustar el número de procesos en tiempo de ejecución.

Cada hijo ejecuta la función process_worker con sus parámetros (worker_id, número de incrementos y si se usa sincronización).

El proceso padre mantiene los pid en el arreglo y luego recorre el mismo para invocar waitpid, asegurándose de esperar la finalización de todos los procesos.

De esta forma, el número de procesos ya no está fijo en el código, sino que depende directamente del argumento num_workers.

- Hilos

En la función test_threads también se usaron arreglos dinámicos (VLA) para manejar tanto los identificadores de los hilos (pthread_t) como sus argumentos (thread_args_t).

En un bucle, se inicializan las estructuras args[i] con la información de cada hilo (ID lógico y número de incrementos).

Se invoca pthread_create tantas veces como num_workers se pida, pasando el puntero a los argumentos correspondientes.

Posteriormente, un segundo bucle llama a pthread_join para esperar la terminación de todos los hilos creados.

Así, la cantidad de hilos también se vuelve configurable y se adapta dinámicamente al valor de num_workers.

  Ejemplos de uso

      ./process_thread_comparison -w 2   # 2 trabajadores
      ./process_thread_comparison -w 8   # 8 trabajadores
      ./process_thread_comparison -w 16  # 16 trabajadores

8. ¿Qué pasó cuando aumentaste el número de procesos/hilos (por ejemplo, de 100 a 200)?

  Prueba realizada con diferentes números de trabajadores:

    ./process_thread_comparison -w 4 -i 1000000 -r 1   # Baseline
    ./process_thread_comparison -w 100 -i 1000000 -r 1 # Alta concurrencia
    ./process_thread_comparison -w 200 -i 1000000 -r 1 # Muy alta concurrencia

Comportamientos observados:

  - Trabajadores: 4 

        Incrementos por trabajador: 1000000
        Total incrementos esperados: 4000000
        Contador final: 4000000
        ¿Resultado correcto?: SÍ
        Tiempo de ejecución: 0.14 ms
        Throughput: 28571428.57 ops/ms
        Tiempo CPU total: 0.0001 segundos
        Utilización CPU: 88.57%

  - Trabajadores: 100

        Incrementos por trabajador: 1000000
        Total incrementos esperados: 100000000
        Contador final: 100000000
        ¿Resultado correcto?: SÍ
        Tiempo de ejecución: 3.39 ms
        Throughput: 29524653.09 ops/ms
        Tiempo CPU total: 0.0022 segundos
        Utilización CPU: 63.86%

  - Trabajadores: 200

        Incrementos por trabajador: 1000000
        Total incrementos esperados: 200000000
        Contador final: 200000000
        ¿Resultado correcto?: SÍ
        Tiempo de ejecución: 6.18 ms
        Throughput: 32346757.24 ops/ms
        Tiempo CPU total: 0.0012 segundos
        Utilización CPU: 18.73%

Cuando se aumenta el número de procesos/hilos de 100 a 200, el programa sigue siendo correcto, pero el tiempo de ejecución aumenta y la utilización de la CPU disminuye drásticamente. El throughput se mantiene estable, lo que indica que agregar más hilos no aporta beneficios reales y, de hecho, genera overhead por exceso de concurrencia. En otras palabras, existe un punto óptimo de número de hilos, y superarlo reduce la eficiencia.




## Escenario de reflexion
- ¿Qué problemas concretos surgirían en cuanto a seguridad, aislamiento y confiabilidad si todas las aplicaciones se ejecutaran como hilos en un mismo espacio de memoria?

  - Seguridad: cualquier hilo podría leer/modificar datos de otros, alterar credenciales o explotar vulnerabilidades que comprometen a todas las aplicaciones.

  - Aislamiento: no habría separación de recursos; un fallo o sobreuso (ej. bucle infinito, exceso de memoria) impactaría a todas las aplicaciones.

  - Confiabilidad: un bug o corrupción de memoria en un hilo afectaría a todo el sistema, además de facilitar deadlocks globales imposibles de recuperar selectivamente.

- ¿Cómo la introducción del concepto de proceso resuelve estos problemas y mejora la estabilidad del sistema?

  El modelo de procesos brinda seguridad, aislamiento y confiabilidad al encapsular cada aplicación en su propio espacio protegido, permitiendo que un fallo local no comprometa todo el sistema.


## Conclusion
La separación de procesos resulta esencial para garantizar la estabilidad y seguridad en sistemas modernos, aun con el costo adicional que implica su gestión. Los hilos son útiles para la concurrencia, pero únicamente dentro del espacio protegido de un proceso, no como sustituto de estos. En consecuencia, los sistemas operativos actuales combinan ambos enfoques: procesos para aislamiento y confiabilidad, e hilos para eficiencia y paralelismo interno.

## Compilacion y Ejecucion

- Usando la version portable

      make portable
      ./process_thread_comparison_portable -w 2 -i 50000 -r 1

- Compilacion Manual

      gcc -pthread -o test main_portable.c
      ./test -w 2 -i 10000

  Compilar

        make

  Si hay problemas de compatibilidad

        make portable

  Generar gráficos de escalabilidad

        ./scalability_test.sh

  Ejecutar análisis completo

        make test-all
        make analyze-full

- Ejemplo de Uso Completo

1. Compilar el programa

        make setup

2. Generar datos de escalabilidad

        ./scalability_test.sh

3. Crear gráficos

        cd scalability_results
        python3 plot_scalability.py

4. Revisar resultados

        ls -la *.png  # Ver gráficos generados
        cat scalability_data.csv  # Ver datos numéricos


## Archivos Principales Entregados

- main.c - Implementación principal con identificadores
- Makefile - Sistema de construcción automatizado
- scalability_test.sh - Script para generar datos de escalabilidad
- test_suite.sh - Suite completa de pruebas
- plot_results.py - Herramienta de análisis con visualizaciones