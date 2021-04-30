# TP Sistemas Operativos - 2do Cuatrimestre 2020

Simulación de un sistema de delivery de comidas similar a Pedidos Ya, como analogía para explicar de una forma práctica los conceptos teóricos dictados en la cursada de Sistemas Operativos. 

Incluye: Programación en C. Tests unitarios en C. Biblioteca compartida en C. Sockets. Serialización. POSIX Threads. Concurrencia. Semáforos. Gestión de memoria. Segmentación Paginada. Planificación de procesos. Multiprogramación. Multiprocesamiento. Sistema de archivos. Bash Scripting. Automatización de deploy.

## Grupo "Segundo Saque"

| Apellido y Nombre | GitHub user | Módulos a cargo |
|-------------------|-------------|-----------------|
| BRUN, Maximiliano | [@BMaxi6](https://github.com/BMaxi6) | Sindicato |
| COLLAZO, Cecilia  | [@cecicollazo](https://github.com/cecicollazo) | App |
| OVIEDO, Facundo   | [@foviedo](https://github.com/foviedo) | Comanda |
| RANIERI, Agustín  | [@RaniAgus](https://github.com/RaniAgus) | Restaurante / App | 

## Enunciado

| Documentos | Videos |
| -----------| -------|
| [Enunciado](https://docs.google.com/document/d/13JCJawPWfL2y6wGTBYykUTvQY5Uso0iYpXSFDAQFbFw) | [Video 1: Introducción](https://www.youtube.com/watch?v=n4zWjlTwDtw) |
| [Pruebas básicas](https://docs.google.com/document/d/17-rWQDjldHw7QfLmyZFri0hYw9C9wJfpxHmDpAVLAwI) | [Video 2: Planificación](https://www.youtube.com/watch?v=SQsC7bwt3_c) |
| [Pruebas avanzadas](https://docs.google.com/document/d/1pYgeTd9Nu2LiR4CkVKAMk3W1wSIJoSENVzq2f2gmDVQ) | [Video 3: Memoria](https://www.youtube.com/watch?v=zHn_kmtbtpw) |
| | [Video 4: File System](https://www.youtube.com/watch?v=f1BXdYVhrdM) |

## Cómo deployar

Para deployar instalando las dependencias, clonar el repo y utilizar el script `deploy.sh`:

```
git clone https://github.com/RaniAgus/so-tp-2020-2c-pedidOS-ya.git
cd so-tp-2020-2c-pedidOS-ya
./deploy.sh
```

## Cómo configurar las pruebas

Primero, editar el archivo [pedidosya.config](https://github.com/RaniAgus/so-tp-2020-2c-pedidOS-ya/blob/master/pedidosya.config) ingresando los campos IP y Puerto correspondientes a cada módulo. Luego se deberá ejecutar desde el directorio raíz del repo (es decir, la carpeta "tp-2020-2c-Segundo-Saque") el siguiente comando:
```
./pruebas/<documento>/<prueba>.sh
```

## Cómo configurar cada módulo

Para configurar un parámetro de cada módulo, se deberá ejecutar desde el directorio raíz del repo (es decir, la carpeta "tp-2020-2c-Segundo-Saque") el siguiente comando:
```
./<modulo>/config.sh <parametros>
```
Se puede listar todos los campos configurables de cada módulo ejecutando el script sin parámetros.

## Cómo limpiar y guardar los logs de cada prueba

Para limpiar todos los logs:
```
./clean.sh
```
Para guardar los logs en otra ubicación:
```
./save.sh <subpath>
```
Por defecto, las pruebas se guardan en el directorio "/home/utnso/logs". Se debe usar el campo subpath solo en caso de que se quiera especificar un subdirectorio a partir de éste (ej: /home/utnso/logs/**basicas/app**):
