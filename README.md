# biot

Actualizado el fichero biodata.py a version 1.2

* Se han agrupado todos los datos que deben ser configurados por el usuario para mayor comodidad.

* (Si ejecutas esta nueva version recuerda utilizar el comando /start en tu bot para reiniciarlo y poder ver las nuevas opciones en el menu. en cualquier caso, escribiendolas estaran disponibles).
* Añadidos tres nuevos comandos al menu del bot:
  * /txt  envia el fichero de datos como mensaje de telegram.
  * /deleteOld  borra los primeros 15 datos de la serie, de manera que se pueden ir eliminando los datos erroneos de los primeros minutos.
  * /deleteNew  borra los ultimos 15 datos de la serie, de manera que se pueden ir eliminando posibles datos erroneos de los ultimos minutos.
* Se ha suprimido la representacion de dos variables (luz roja y luz azul) ya que no se conectaron y solo mostraban ruido.
* Se han separado las graficas de temperatura y humedad para que se muestren mejor en sus respectivas escalas.
* Modificada la funcion de representacion grafica para que solo represente como maximo 1440 muestras. Es decir las ultimas 24 horas, si se toma una muestra por minuto. Pero la toma de datos si conserva todos y seran enviados cuando se pida el txt o el email
