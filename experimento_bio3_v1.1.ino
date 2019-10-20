/*
#       _\|/_   A ver..., ¿que tenemos por aqui?
#       (O-O)        
# ---oOO-(_)-OOo---------------------------------
 
 
##########################################################
# ****************************************************** #
# *          LABORATORIO PARA PRINCIPIANTES            * #
# *     Experimentos para el Biotecnoencuentro 3       * #
# *          Autor:  Eulogio López Cayuela             * #
# *                                                    * #
# *     Experimento Bio v1.1     Fecha: 18/10/2019     * #
# ****************************************************** #
##########################################################
*/


#define __VERSION__ "Experimento Biotecnoencuentro 3 v1.1\n"


/*
      ===== NOTAS DE LA VERSION "ZANGANO" ===== 
 - Version minimalista que pasa todo el trabajo al software python
   y que se limita a servir de interfaz para la adquisicion de datos

 - se han renombrado algunas cosas para mas claridad

   >> Fecha: 11/05/2019
 - Calibrado del sensor de gases MQ135 para usarlo con CO2
   Se realiza la calibracion ante un entorno cpn concentracion conocida.
   (Usaremos alrededor de  415ppm que es lo que se estima de media ahora mismpo en el planeta)
   
   Tamaño actual compilado 7864 bytes de programa y 444 bytes de uso de RAM

    a = 5.5973021420;
    b = -0.365425824;
    Ro=(Rs/a*(ppm)pow(b))
    Rs = Rsensor_media
    Rsensor_media= (Sumatorio(n) Rs )/n   // n muestras durante 5 minutos cada 1 segundos
    Rs = 1024 * (RL/acd)-RL  <<<  RL = 20k(20000)


  CONEXIONES:
 =======================
  ARDUINO UNO      MQ-135
 =======================
  GND               GND
  5V                Vcc
  A0                A0


 =======================
  ARDUINO UNO      GY-21
 =======================
  GND               GND
  5V                Vin
  SCL (A5)          SCL
  SDA (A4)          SDA


  CONEXIONES:
 =======================
  ARDUINO UNO   SENSOR PH
 =======================
  GND               GND
  5V                Vcc
  A1                P0


 =======================
  ARDUINO        LEDS
 =======================
  GND         GND de los leds

  A2          led AZUL
  A3          led ROJO

*/




/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        IMPORTACION DE LIBRERIAS 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/
#include <Wire.h>
#include <Adafruit_HTU21DF.h>  //ojo esta libreria tal cual falla(fallo gordo), la he retocado
//queda pendiente escribir una propia

/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        CREACCION DE OBJETOS
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

Adafruit_HTU21DF Higrometro_GY_21 = Adafruit_HTU21DF();


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        SECCION DE DECLARACION DE CONSTANTES  Y  VARIABLES GLOBALES
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//------------------------------------------------------------------------
// Algunas definiciones personales para mi comodidad al escribir codigo
//------------------------------------------------------------------------
#define AND &&
#define OR ||
#define NOT !
#define ANDbit &
#define ORbit |
#define NOTbit ~
#define XORbit ^


//------------------------------------------------------
//Otras definiciones para pines y variables
//------------------------------------------------------
#define PIN_LED_OnBoard    13   // Led on Board

#define PIN_MQ135          A0
#define PIN_SENSOR_PH      A1

#define PIN_led_azul       A2 
#define PIN_led_rojo       A3

 
float TEMPERATURA = -100;   //inicializado fuera de rango
float HUMEDAD = -100;       //inicializado fuera de rango
int PPM = -100;             //inicializado fuera de rango
float PH = -100;            //inicializado fuera de rango
int LUZ_AZUL = -100;        //inicializado fuera de rango
int LUZ_ROJA = -100;        //inicializado fuera de rango
  
unsigned long momento_para_adquirir_datos = 0;  //para controlar el momento de tomar muestras

boolean FLAG_calibracion = true;

float a = 5.5973021420;         //a y b   constantes del sensor para medidas de CO2
float b = -0.365425824;         //obtenidas de su hoja de caracteristicas

unsigned long Ro = 1467930;      //Ro  del sensor que se ha de calcular calibrandolo. 
                                //Se usa este valor si no se activa la calibracion
#define PPM_REFERENCIA  410     //ppm del ambiente de calibracion



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
   ****************************************************************************************************** 
                                    FUNCION DE CONFIGURACION
   ****************************************************************************************************** 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

void setup()
{
  Serial.begin(115200);               //#9600, 19200, 38400, 57600, 115200
  delay(10000);
  Serial.println (F(__VERSION__));    // mostramos la version por consola, DEBUG

  if (!Higrometro_GY_21.begin()) {
    Serial.println(F("Sensor HTU no presente"));
    //while (true);  //saltamos este punto porque sensores clon no responden correctamente
  }
  
  pinMode(PIN_LED_OnBoard, OUTPUT);   // PIN_LED_OnBoard como salida
  digitalWrite(PIN_LED_OnBoard, LOW); // apagamos el led 'On Board'

  if (FLAG_calibracion == false){
    Ro = calibrarCO2();
  }
  
  leerSensores();     //refresco inicial de las variables
}



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
   ****************************************************************************************************** 
                                  BUCLE PRINCIPAL DEL PROGRAMA
   ****************************************************************************************************** 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

void loop()
{ 
  /* mostrar medidas resales en funcion de la calibracion */
  unsigned long momento_actual = millis();
  if (momento_actual > momento_para_adquirir_datos){
    momento_para_adquirir_datos = momento_actual + 5000;    //una muestra cada 5 segundos
    leerSensores();
    //mostarDatosEnPython(); //si deseamos monitorizar solo en puerto serie
  }
  leerPuertoSerie();  //mostraremos los datos solo si hay peticiones por puerto serie
}



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
   ###################################################################################################### 
        BLOQUE DE FUNCIONES: LECTURAS DE SENSORES, COMUNICACION SERIE, CONTROL LCD...
   ###################################################################################################### 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//    RECOPILAR DATOS DE LOS SENSORES
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//========================================================
//  FUNCION OBTENER DATOS DESDE LOS SENSORES
//========================================================

void leerSensores()
{ 
  /* leer datos desde el sensor de humedad y temperatura  GY-21 - HTU21DF*/
  TEMPERATURA = Higrometro_GY_21.readTemperature();
  HUMEDAD = Higrometro_GY_21.readHumidity();
  
  /* medir valor de PH */
  PH = medir_ph();
  
  /* lectura el CO2 desde el MQ135*/
  int adc = lecturaSensorMQ(PIN_MQ135);
  float Rsensor = (1024*(20000.0/adc))-20000;
  PPM = pow((Rsensor/Ro)/a,(1/b)); // 'a' y 'b', constantes del sensor. 'Ro',  valor calculado previamente

  /* medir luminosidad con los led*/
  LUZ_AZUL = analogRead(PIN_led_azul);
  LUZ_ROJA = analogRead(PIN_led_rojo);
}


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//    MQ135
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//========================================================
//  FUNCION PARA CALIBRAR EL MQ135 PARA CO2
//========================================================

unsigned long calibrarCO2()
{          
  float Rsensor_media = 0;   //variable usada para la calibracion
  int iteraciones  = 0;
  unsigned long inicio = millis();
  
  Serial.println(F("Calibrando Sensor..."));
  
  while(millis()-inicio < 300000){  //5 minutos aproximadamente
    int adc = lecturaSensorMQ(PIN_MQ135);
    float Rsensor = (1024*(20000.0/adc))-20000;
    Rsensor_media += Rsensor;
    iteraciones++;
    delay(1000);
    Serial.print(F("."));
    if(iteraciones%30==0){
      Serial.println();
    }
  }
  Serial.println(F("  OK"));

  /* calculo de Ro en funcion de la Rs media durante 5 minutos a una concentracion de CO2 conocida */
  Rsensor_media /= iteraciones;

  unsigned long ro = Rsensor_media/(a*pow(PPM_REFERENCIA, b));  //ppm=410 ( ¿la media mundial actual? )

  /* mostrar los datos calculados */
  Serial.print(F("Ro = "));Serial.println(ro);
  Serial.println("");
  return ro;
}


//========================================================
//  FUNCION PARA OBTENER LECTURAS DEL ADC DE UN MQxxx
//========================================================

int lecturaSensorMQ(uint8_t pin)
{
  /* hacemos el promedio de 8 muestras consecutivas */
  int analogica_MQ = 0;
  for(int i=0;i<8;i++){  //leemos 8 veces y hacemos la media
    analogica_MQ += analogRead(pin);
    delay(2);
  }
  analogica_MQ = analogica_MQ >> 3; // division por 8 (con desplazamiento de 3 bits)        
  return analogica_MQ; 
}


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//    SENSOR DE PH
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//========================================================
//  OBTENER DATOS DESDE EL SENSOR ANALOGICO DE PH
//========================================================

float medir_ph() 
{
  int buffer_lecturas[10];
  int temp;
  
  /* toma de muestras */
  for(int i=0;i<10;i++) { 
    buffer_lecturas[i] = analogRead(PIN_SENSOR_PH);
    delay(100);
  }
  
  /* ordenar las muestas de menor a mayor */
  for(int i=0;i<9;i++){
    for(int j=i+1;j<10;j++){
      if(buffer_lecturas[i]>buffer_lecturas[j]){
        temp=buffer_lecturas[i];
        buffer_lecturas[i]=buffer_lecturas[j];
        buffer_lecturas[j]=temp;
      }
    }
  }

  /* calcular el valor medio despreciando los dos valores menores y los dos mayores */
  float valor_medio = 0; 
  for(int i=2;i<8;i++){
    valor_medio+=buffer_lecturas[i];
  }

  valor_medio = valor_medio/6;

  /* obtener el voltage asociado a la lectura del pin analogico (sin utilidad por ahora) */
  //float Vph = valor_medio*5.0/1023;  

  /* ---  calculo del PH usando dos medidas de ph conocidas para calibrar --- */
  float ph_bajo = 3.4;        // Vinagre, ph = 3.4
  float adc_ph_bajo = 615;    // valor ADC para ph = 3.4          (ph obtenidos con papel tornasol)

  
  float ph_alto = 8;          // Agua del grifo, ph = 8
  float adc_ph_alto = 463;    // valor ADC para ph = 8  
  
  float PH2 = (valor_medio - adc_ph_alto ) * ((ph_alto-ph_bajo) / ( adc_ph_alto - adc_ph_bajo )) + ph_alto;

  if(PH2>=0 AND PH2<=14){
    //Serial.print(F("PH = "));
    //Serial.println(PH2, 2);
  }
  else{
    //Serial.println(F("ERROR - Revise la sonda"));
    PH2 = -100;
  }
  return PH2;
}

/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//    PUERTO SERIE
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//========================================================
// FUNCION PARA ACCESO Y LECTURA DEL PUERTO SERIE
//========================================================

int leerPuertoSerie()
  /*
  Funcion para atender ordenes a través del puerto serie
  Lee los comandos/parametros que envia python al puerto serie:
  
  - Si el comandos es '*' se realiza una muestra unica de datos
  - Si el comandos es 'x' ordena que cancele el proceso de muestreo y envio de datos
  ---- (Ampliable en un futuro para mas cosas
  */
  
{
  int lectura = 'x';                //por defecto pensamos que no se nos pide nada
  if (Serial.available() > 0) {
     // leer byte
    lectura = Serial.read();
    if (lectura == '*' OR lectura == '+'){  // indicador para mandar una muestra unica al puerto serie
      mostarDatosEnPython();
      return 0;  //ok, peticion de datos
     } 
  }
  return -1;    //sin peticion por parte del usuario
}


//========================================================
//  ENVIAR DATOS POR PUERTO SERIE A PYTHON
//========================================================

void mostarDatosEnPython()
{ 
  Serial.flush();
  
  /* FORMATEO de datos y ENVIO al puerto SERIE */
  Serial.print(TEMPERATURA);
  Serial.print(F("**"));
  Serial.print(HUMEDAD);
  Serial.print(F("**"));
  Serial.print(PH);
  Serial.print(F("**"));
  Serial.print(PPM);
  Serial.print(F("**"));
  Serial.print(LUZ_AZUL);
  Serial.print(F("**"));
  Serial.println(LUZ_ROJA); 
}



//*******************************************************
//                    FIN DE PROGRAMA
//*******************************************************
