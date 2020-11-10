#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <Stepper.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Credenciales de la red
const char* ssid = "MERCUSYS_4056";
const char* password = "12345678";

// Inicializar el bot de telegram
#define BOTtoken "1463070330:AAEpGOD_67ajFVfYbOnUsGF5wjr0TpDuNyI"  // your Bot Token (Get from Botfather)

//Motor puertos
#define IN1   D1
#define IN2   D2
#define IN3   D3
#define IN4   D4

//Sensor puertos
#define PIN_TRIG D5
#define PIN_ECHO D6

float tiempo;
float distancia;
float porcentaje;

// ID del chat
#define CHAT_ID "1491918875"

// Motor
const float PASOS_POR_REVOLUCION = 32;
const float MARCHA_RED = 64;
int direccion;
const float PASOS = PASOS_POR_REVOLUCION * MARCHA_RED;
int pasosRequeridos;
Stepper motorcito (PASOS_POR_REVOLUCION, IN4, IN2, IN3, IN1);


WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Verificar mensaje nuevo cada segundo
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

void setup() {
  Serial.begin(115200);
  #ifdef ESP8266
    client.setInsecure();
  #endif

  // Conexion a Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a la red Wi-Fi..");
  }
  // Para confirmar se imprime la ip local
  Serial.println(WiFi.localIP());

  //motor RPM
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  motorcito.setSpeed(700);

  //Sensor Puertos
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
}

void restante(){
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
   
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
   
  tiempo = pulseIn(PIN_ECHO, HIGH);
  distancia = tiempo*0.034/2;
  porcentaje = (100 - ((100 / 26) * distancia));

  Serial.println(distancia);
  Serial.println("Comida restante:\t");
  Serial.println(tiempo);
  delay(1000);
}

// Nuevos mensajes
void handleNewMessages(int numNewMessages){
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Recibir el Id del que escribio
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Lo siento no tienes los permisos para dar ordenes a este bot", "");
      continue;
    }

    // Guarda el texto del mensaje y el solicitante
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Bienvenido, " + from_name + ".\n";
      welcome += "Utilice los siguientes comandos para dar ordenes a PetCare.\n\n";
      welcome += "/limpiar : Para vaciar por completo el recipiente \n";
      welcome += "/alimentar : Para liberar una racion de comida \n";
      welcome += "/estado : Para cosultar la cantidad de comida que queda en el recipiente \n";
      welcome += "/start : Para ver los comandos disponibles \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/limpiar") {
      //direccion = 2000;
      bot.sendMessage(chat_id, "Contenedor limpiado", "");
      Serial.println("Contenedor limpiado");
    }

    if (text == "/alimentar") {
      direccion = 1024;
      delay(500);
      bot.sendMessage(chat_id, "Mascota alimentada", "");
      Serial.println("Mascota alimentada");
    }
    
    if (text == "/estado") {
      restante();
      bot.sendMessage(chat_id, "Comida restante: " + String(porcentaje) + " % (Distancia medida: " + String(distancia) + " cm).", "");
      Serial.println("Realizado");
    }
    
  }
}

void alimentar() {
  pasosRequeridos = PASOS / 2;
  motorcito.setSpeed(100);
  motorcito.step(512);
  delay(5000);
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("Respuesta obtenida");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  motorcito.step(direccion);
  delay(1000);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  direccion = 0;
}
