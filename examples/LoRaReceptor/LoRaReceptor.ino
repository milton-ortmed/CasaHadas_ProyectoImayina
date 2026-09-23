#include <Botones.hpp>
#include <Config.h>
#include <SPI.h>
#include <LoRa.h>

// Define los pines que vas a usar en tu ESP32-S3
// Estos pines corresponden a la placa Balancin con LoRa
#define LORA_SCK_PIN   14  // Reloj SPI
#define LORA_MISO_PIN  32  // Master In Slave Out
#define LORA_MOSI_PIN  23  // Master Out Slave In
#define LORA_SS_PIN    27  // Slave Select (NSS/CS)
#define LORA_RST_PIN   26  // Reset
#define LORA_DIO0_PIN  25  // Interrupción (DIO0)

void setup() {
  Serial.begin(115200);
  delay(100);

  SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_SS_PIN);
  LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);

  Serial.println("Iniciando Receptor LoRa con Interrupciones");

  if (!LoRa.begin(915E6)) {
    Serial.println("¡Error al iniciar LoRa!");
    while (1);
  }

  // 1. Asignamos la función que se ejecutará cuando llegue un paquete
  LoRa.onReceive(alRecibirPaquete);

  // 2. Ponemos el módulo de radio en modo de recepción continua
  LoRa.receive();
}

void loop() {
  // ¡El loop ahora está completamente libre!
  // Aquí puedes poner código para leer sensores, actualizar una pantalla, etc.
  // No necesitas consultar a LoRa constantemente.
}

// 3. Esta es la función "Callback" (Rutina de Servicio de Interrupción)
// Se ejecuta AUTOMÁTICAMENTE cuando el pin DIO0 detecta un nuevo paquete
void alRecibirPaquete(int packetSize) {
  // Si el tamaño es 0, no hay nada que leer
  if (packetSize == 0) return;

  Serial.print("Paquete recibido: '");

  // Leemos los datos mientras haya bytes disponibles
  while (LoRa.available()) {
    Serial.print((char)LoRa.read());
  }

  // Imprimimos la fuerza de la señal (RSSI)
  Serial.print("' con RSSI ");
  Serial.println(LoRa.packetRssi());
}