#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Config.h>
#include <SPGestorComandosSeriales.hpp>

// --- PINES LORA (ESP32-S3 Super Mini) ---
#define SCK_PIN   7
#define MISO_PIN  5
#define MOSI_PIN  6
#define NSS_PIN   11
#define RESET_PIN 10
#define DIO0_PIN  9 // Fundamental para interrupciones RX
// DIO1 (GPIO 8) no es estrictamente necesario para la recepción básica con la librería de Sandeep Mistry.

// --- ESTRUCTURA DE LORA ---
struct __attribute__((packed)) ComandoLoRa {
  uint8_t id_destino;
  uint8_t comando; // Ahora el comando recibido dictará directamente qué pista reproducir
};

ComandoLoRa datosRecibidos;

// --- DEFINICIONES DEL AUDIO AMBIENTAL ---
#define MI_ID_DISPOSITIVO 5   // ID asignado a este receptor

// --- INSTANCIA DEL GESTOR (Patrón Singleton) ---
GestorComandosSeriales& gestor = GestorComandosSeriales::ObtenerInstancia();

// --- CONTRATO DE COMANDOS (IDs alineados a OrtMedSoundBoard) ---
const uint16_t CMD_REPRODUCIR_PISTA       = 0x0001;
const uint16_t CMD_DETENER                = 0x0002;
const uint16_t CMD_VOLUMEN                = 0x0003;

const uint8_t  CMD_TERMINACION_ANTICIPADA = 0x99;

// --- CONFIGURACIÓN DE HARDWARE Y UART ---
static const int UART_RX = 44; 
static const int UART_TX = 43; 
static const uint32_t UART_BAUD = 115200;

// --- DATOS DE ESTADO ---
static const uint8_t CANAL_AUDIO   = 0;
static const uint8_t VOLUMEN_MAXIMO = 20;

// --- VARIABLES PARA INTERRUPCIONES Y COOLDOWN ---
volatile bool paqueteRecibido = false;
volatile int tamanoPaqueteRecibido = 0;
unsigned long tiempoUltimoComando = 0;
const unsigned long COOLDOWN_RECEPCION = 100;

// --- RUTINA DE SERVICIO DE INTERRUPCIÓN (ISR) ---
// IRAM_ATTR asegura que la función se cargue en la RAM rápida para evitar cuelgues durante la interrupción
void IRAM_ATTR onReceiveLoRa(int packetSize) {
  if (packetSize > 0) {
    tamanoPaqueteRecibido = packetSize;
    paqueteRecibido = true;
  }
}

// --- ENVOLTORIOS (WRAPPERS) DE COMANDOS ---
void enviarVolumen(uint8_t volumen) {
  uint8_t payload[2] = {volumen, CMD_TERMINACION_ANTICIPADA};
  gestor.EnviarComandoSerial(CMD_VOLUMEN, payload, sizeof(payload));
  Serial.print("Volumen establecido al MAXIMO: ");
  Serial.println(volumen);
  delay(120);
}

void enviarStop() {
  uint8_t payload[1] = {CMD_TERMINACION_ANTICIPADA};
  gestor.EnviarComandoSerial(CMD_DETENER, payload, sizeof(payload));
  delay(120);
  Serial.println("Detener audio general");
}

void reproducirPista(uint32_t pista, uint8_t canal = 0, uint8_t voz = 0, bool repetir = true) {
  if (pista == 0) return;
  
  uint8_t payload[6];
  payload[0] = static_cast<uint8_t>((pista >> 16) & 0xFF);
  payload[1] = static_cast<uint8_t>((pista >> 8) & 0xFF);
  payload[2] = static_cast<uint8_t>(pista & 0xFF);
  payload[3] = canal;
  payload[4] = voz;
  payload[5] = repetir ? 1 : 0;
  gestor.EnviarComandoSerial(CMD_REPRODUCIR_PISTA, payload, sizeof(payload));
  Serial.print("Reproduciendo pista ");
  Serial.println(pista);
  delay(120);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // 1. Inicializar bus SPI
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, NSS_PIN);
  LoRa.setPins(NSS_PIN, RESET_PIN, DIO0_PIN);

  // 2. Inicializar la UART hacia la SoundBoard
  gestor.Inicializar(UART_RX, UART_TX, UART_BAUD);
  delay(1000); 
  
  // Establecer el volumen al máximo (20) de forma fija
  enviarVolumen(VOLUMEN_MAXIMO);

  // 3. Inicializar módulo LoRa
  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("Error Critico: Fallo al iniciar la radio LoRa");
    while (true);
  }

  LoRa.setSignalBandwidth(LORA_BANDWIDTH);
  LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
  LoRa.setCodingRate4(LORA_CODING_RATE);
  LoRa.setSyncWord(LORA_SYNC_WORD);

  // 4. Configurar la interrupción de recepción
  LoRa.onReceive(onReceiveLoRa);
  
  // Poner el módulo en modo de recepción continua
  LoRa.receive();

  Serial.println("Receptor por interrupciones listo (ESP32-S3). Volumen al maximo.");
  Serial.print("LoRa listo a ");
  Serial.print(LORA_FREQUENCY / 1E6);
  Serial.println(" MHz. Esperando paquetes...");
}

void loop() {
  // Mantener actualizado el gestor serial
  gestor.Actualizar();

  // Procesar solo si la interrupción levantó la bandera
  if (paqueteRecibido) {
    paqueteRecibido = false; // Bajar bandera
    int packetSize = tamanoPaqueteRecibido;

    Serial.print("[LORA RX] Interrupcion detectada - Tamano: ");
    Serial.print(packetSize);
    Serial.print(" bytes | RSSI: ");
    Serial.print(LoRa.packetRssi());
    Serial.print(" dBm | SNR: ");
    Serial.println(LoRa.packetSnr());

    // Validar el tamaño esperado de la estructura
    if (packetSize == sizeof(ComandoLoRa)) {
      uint8_t buffer[sizeof(ComandoLoRa)];
      LoRa.readBytes(buffer, sizeof(ComandoLoRa));
      memcpy(&datosRecibidos, buffer, sizeof(ComandoLoRa));

      Serial.print("[LORA RX] ID Destino: ");
      Serial.print(datosRecibidos.id_destino);
      Serial.print(" | Comando/Pista: ");
      Serial.println(datosRecibidos.comando);

      if (datosRecibidos.id_destino == MI_ID_DISPOSITIVO) {
        if (millis() - tiempoUltimoComando > COOLDOWN_RECEPCION) {
          tiempoUltimoComando = millis();

          // El receptor confía en que la placa emisora mandará directamente el ID de la pista (1, 2, 3...)
          // o mandará un 0 para detener.
          if (datosRecibidos.comando == 0) {
            Serial.println("[AUDIO] Comando STOP recibido");
            enviarStop();
          } else {
            Serial.print("[AUDIO] Reproduciendo pista por comando LoRa: ");
            Serial.println(datosRecibidos.comando);
            // Detenemos el audio actual antes de mandar el nuevo por seguridad
            enviarStop();
            reproducirPista(datosRecibidos.comando, CANAL_AUDIO, 0, true);
          }
        } else {
          Serial.println("[LORA RX] Rafaga bloqueada por cooldown.");
        }
      } else {
        Serial.print("[LORA RX] Paquete descartado (Destino ");
        Serial.print(datosRecibidos.id_destino);
        Serial.print(" != ");
        Serial.print(MI_ID_DISPOSITIVO);
        Serial.println(")");
      }
    } else {
      Serial.print("[LORA RX] Advertencia: Tamano de paquete inesperado: ");
      Serial.println(packetSize);
    }
    
    // Limpiar cualquier byte residual del FIFO y rearmar la escucha continua
    while (LoRa.available()) {
      LoRa.read();
    }
    LoRa.receive(); 
  }
}