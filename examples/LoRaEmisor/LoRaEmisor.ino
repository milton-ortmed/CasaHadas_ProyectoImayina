#include <Botones.hpp>
#include <Config.h>
#include <SPI.h>
#include <LoRa.h>

// Define los pines que vas a usar en tu ESP32-S3
// Estos pines corresponden a la placa TOi Board con LoRa
#define LORA_SCK_PIN   9  // Reloj SPI
#define LORA_MISO_PIN  15  // Master In Slave Out
#define LORA_MOSI_PIN  43  // Master Out Slave In
#define LORA_SS_PIN    8  // Slave Select (NSS/CS)
#define LORA_RST_PIN   18  // Reset
#define LORA_DIO0_PIN  17  // Interrupción (DIO0)

int contador = 0;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Bandera: programa iniciado");

  SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_SS_PIN);
  Serial.println("Bandera: SPI iniciado");
  LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);

  Serial.println("Iniciando Transmisor LoRa");

  // Inicia el módulo en la frecuencia de 915 MHz
  if (!LoRa.begin(915E6)) {
    Serial.println("¡Error al iniciar LoRa!");
    while (1); // Si falla, se queda atrapado aquí
  }
}

void loop() {
  Serial.print("Enviando paquete: ");
  Serial.println(contador);

  // Comienza a crear el paquete
  LoRa.beginPacket();
  
  // Escribe el mensaje (funciona igual que Serial.print)
  LoRa.print("Hola LoRa ");
  LoRa.print(contador);
  
  // Cierra y envía el paquete por radiofrecuencia
  LoRa.endPacket();

  contador++;
  delay(5000); // Espera 5 segundos antes del siguiente envío
}