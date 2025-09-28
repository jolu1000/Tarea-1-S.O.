# Mishell

Esta es una implementación de una **shell simple en C para Linux, llamada `mishell`.  
Permite ejecutar comandos en foreground, soporta pipes y cuenta con un comando interno llamado `miprof` para medir rendimiento (tiempo y memoria).

---

## Compilación y ejecución

Compila el código con:

gcc -o shell shell.c 

Esto genera el ejecutable `shell`. Para iniciar la shell:

./shell

Desde aquí puedes ejecutar cualquier comando de Linux, por ejemplo:

ls -l
echo "Hola mundo"

Para salir de la shell:

exit

---

## Comando interno `miprof`

El comando `miprof` permite medir métricas de rendimiento de un proceso.  
Cuenta con tres variantes:

1. Ejecución simple (ejec)  
   Mide tiempo real, de usuario, de sistema y memoria máxima utilizada:

   miprof ejec <comando> [args...]

   Ejemplo:  
   miprof ejec sort archivo.txt

2. Ejecución con guardado (ejecsave)  
   Guarda los resultados en un archivo (si existe, agrega al final):

   miprof ejecsave <archivo> <comando> [args...]

   Ejemplo:  
   miprof ejecsave resultados.txt sort archivo.txt

3. Ejecución con límite de tiempo (maxtiempo)  
   Ejecuta un comando con un límite de tiempo en segundos.  
   Si lo supera, el proceso se termina automáticamente:

   miprof maxtiempo <segundos> <comando> [args...]

   Ejemplo:  
   miprof maxtiempo 5 sort texto_grande.txt



