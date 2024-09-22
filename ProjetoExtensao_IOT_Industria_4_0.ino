#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>

#define TRIG_PIN D1  // Pino Trigger
#define ECHO_PIN D2  // Pino Echo

const char* ssid = "Tricolornet Wifi";
const char* password = "lanasabata2021";
const String serverUrl = "http://localhost:3000/update"; // URL do servidor

// Distâncias em cm
const int ALTURA_SENSOR = 78;  // Altura total do sensor até o fundo
const int ALTURA_MAXIMA_AGUA = 42;  // Altura máxima da água

WiFiClient client;

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Conectar ao WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Configurar OTA
  ArduinoOTA.setHostname("ESP8266-LevelMonitor"); // Nome do dispositivo para OTA
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    // Reiniciar
    Serial.println("Iniciando OTA: " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA Finalizado.");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progresso: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Erro [%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Falha na autenticação.");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Erro ao iniciar.");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Erro de conexão.");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Erro de recepção.");
    } else if (error == OTA_END_ERROR) {
      Serial.println("Erro no fim.");
    }
  });
  ArduinoOTA.begin();  // Inicializa OTA

  Serial.println("Pronto para OTA!");
}

void loop() {
  // Processa OTA
  ArduinoOTA.handle();  // Verifica se há atualização OTA

  long duration, distance;

  // Gera um pulso no pino Trigger
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Calcula a duração do pulso no pino Echo
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calcula a distância em centímetros
  distance = (duration / 2) / 27.0;

  // Limita a leitura à faixa válida
  if (distance > ALTURA_SENSOR) {
    distance = ALTURA_SENSOR;
  }

  // Calcula o nível da água como percentual
  int nivel_percentual = ((ALTURA_SENSOR - distance) * 100) / ALTURA_MAXIMA_AGUA;

  // Captura o IP do ESP8266
  String ipAddress = WiFi.localIP().toString();

  // Envia os dados para o servidor
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Enviar dados para 6 casas
    for (int casa = 1; casa <= 6; casa++) {
      String url = serverUrl + "/" + String(casa);  // URL para cada casa
      http.begin(client, url);
      http.addHeader("Content-Type", "application/json");

      // Dados JSON para enviar
      String payload = "{\"level\":" + String(nivel_percentual) + ", \"ip\":\"" + ipAddress + "\"}";
      int httpResponseCode = http.POST(payload);

      if (httpResponseCode > 0) {
        Serial.print("Dados enviados para Casa ");
        Serial.print(casa);
        Serial.print(". HTTP Response code: ");
        Serial.println(httpResponseCode);
      } else {
        Serial.print("Erro ao enviar dados para Casa ");
        Serial.print(casa);
        Serial.print(". Código: ");
        Serial.println(httpResponseCode);
      }

      http.end();
      delay(1000);  // Pequeno atraso para evitar sobrecarga
    }
  }

  // Exibe a distância e o nível calculado no Serial Monitor
  Serial.print("Distancia: ");
  Serial.print(distance);
  Serial.println(" cm");

  Serial.print("Nível da água: ");
  Serial.print(nivel_percentual);
  Serial.println(" %");

  delay(5000);  // Espera 5 segundos antes de medir novamente
}
