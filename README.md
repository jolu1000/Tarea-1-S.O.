# Mishell

Esta es una implementación de una **shell simple en C** para Linux, llamada `mishell`, que permite ejecutar comandos en foreground, soporta pipes y cuenta con un comando interno llamado `miprof` para medir el rendimiento de procesos.

---

## Compilación

Para compilar `mishell`, abre una terminal en el directorio donde está el código y ejecuta:

```bash
gcc -o shell shell.c 

## Comandos
Ejecucion
./shell
Desde aquí puedes ejecutar cualquier comando de Linux.
Para salir de la shell
exit
Comando interno miprof
1. Ejecución simple (ejec)
Mide el tiempo real, tiempo de usuario, tiempo de sistema y memoria máxima utilizada por el comando.

mishell:$ miprof ejec <comando> [args...]
Ejemplo:
mishell:$ miprof ejec sort archivo.txt

2. Ejecución con guardado (ejecsave)
Guarda los resultados en un archivo. Si el archivo existe, los resultados se agregan al final.

mishell:$ miprof ejecsave <archivo> <comando> [args...]
Ejemplo:
mishell:$ miprof ejecsave resultados.txt sort archivo.txt

3. Ejecución con límite de tiempo (maxtiempo)
Ejecuta un comando con un límite de tiempo en segundos. Si el comando supera este tiempo, se termina automáticamente.

mishell:$ miprof maxtiempo <segundos> <comando> [args...]





