# tp-2020-2c-Segundo-Saque

- [ENUNCIADO](https://docs.google.com/document/d/13JCJawPWfL2y6wGTBYykUTvQY5Uso0iYpXSFDAQFbFw)
- [PRUEBAS BÁSICAS](https://docs.google.com/document/d/17-rWQDjldHw7QfLmyZFri0hYw9C9wJfpxHmDpAVLAwI)
- [PRUEBAS FINALES](https://docs.google.com/document/d/1pYgeTd9Nu2LiR4CkVKAMk3W1wSIJoSENVzq2f2gmDVQ)
- [Video 1: Introducción](https://www.youtube.com/watch?v=n4zWjlTwDtw)
- [Video 2: Planificación](https://www.youtube.com/watch?v=SQsC7bwt3_c)
- [Video 3: Memoria](https://www.youtube.com/watch?v=zHn_kmtbtpw)
- [Video 4: File System](https://www.youtube.com/watch?v=f1BXdYVhrdM)

## Cómo deployar

```
git clone https://github.com/sisoputnfrba/so-deploy
cd so-deploy
./deploy.sh -d=cshared -p=app -p=comanda -p=cliente -p=restaurante -p=sindicato tp-2020-2c-Segundo-Saque
```

## Cómo configurar las pruebas

Primero, editar el archivo "pedidosya.config" ingresando los campos IP y Puerto correspondientes a cada módulo. Luego se deberá ejecutar desde el directorio raíz del repo (es decir, la carpeta "tp-2020-2c-Segundo-Saque") el siguiente comando:
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
./cleanlogs.sh
```
También se pueden guardar los logs antes de ser limpiados:
```
./cleanlogs.sh save <subpath>
```
Por defecto, las pruebas se guardan en el directorio "/home/utnso/logs". Se debe usar el campo subpath solo en caso de que se quiera especificar un subdirectorio a partir de éste.
