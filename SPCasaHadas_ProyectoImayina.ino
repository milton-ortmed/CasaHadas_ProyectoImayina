/**
 * @file CasaHadas_ProyectoImayina.ino
 * @brief Firmware Principal - Proyecto Casas de Hadas (Mundo Imayina)
 * @microcontroller ESP32-S3 Super Mini
 * 
 * Descripción:
 * Este programa controla la iluminación, expulsión asíncrona de purpurina, movimiento del
 * hada y reproducción de audio temática para las Casas de Hadas mecánicas.
 */

#include <Arduino.h>
#include <esp_task_wdt.h>
#include <SPI.h>
#include <LoRa.h>
#include <Botones.hpp>

#include <Config.h>
#include <ILightingController.h>
#include <FastLEDController.h>
#include <ServoManager.h>
#include <BlowerControl.h>
#include <ActuadorLinealControl.h>

// LA ESTRUCTURA DEBE SER EXACTAMENTE IGUAL A LA DEL RECEPTOR
struct __attribute__((packed)) ComandoLoRa {
  uint8_t id_destino;
  uint8_t comando;
};

// ==========================================
// ESTADOS DE LA MÁQUINA DE ESTADOS FINITA (FSM)
// ==========================================
enum SystemState {
    STATE_IDLE,          // Reposó: Iluminación tenue ("hada dentro"), espera de botón
    STATE_PRESHOW,       // Luces y audio de presentación
    STATE_FAIRY_OUT,     // Inicia movimiento del hada hacia hacia afuera de la casa con la luz de la puerta
    STATE_ACTIVATED,     // Activación: Inicia audio y prepara purpurina
    STATE_SHOW_RUNNING,  // Espectáculo: Mantiene show, ejecuta secuencia de purpurina (también podría monitorear pin BUSY)
    STATE_CLOSING,       // Cierre: Limpia estados
    STATE_FAIRY_IN,      // Secuencia de movimiento del hada hacia adentro y regresa a IDLE
    STATE_SHOW_AUTO      // Espectáculo automático: Mantiene show secundario de luz y sonido
};

// Sub-estados de la secuencia asíncrona de purpurina (previene atascos)
enum PurpurinaStage {
    PURPURINA_IDLE,             // Estado inicial de reposo
    PURPURINA_OPEN_GUILLOTINE,  // Apertura de Guillotina para liberación de mica
    PURPURINA_WAIT_BLOWER,      // Espera antes de encender el blower
    PURPURINA_BLOWER,           // Blower encendido después del cierre
    PURPURINA_DONE              // Secuencia de purpurina finalizada
};

// ==========================================
// INSTANCIAS DE COMPONENTES DE SOFTWARE
// ==========================================
SystemState currentState = STATE_IDLE;
PurpurinaStage purpurinaStage = PURPURINA_IDLE;

// Objetos de los controladores de hardware
ILightingController* lighting = new FastLEDController();
ServoManager servos;
BlowerControl blower;
Controlador Ctrl;
ActuadorLinealControl actuadorLineal;

uint32_t purpurinaStageStartTime = 0;
uint32_t showRunningStartTime = 0;
bool purpurinaCompleted = false;
bool teclado = true;
const uint32_t INTERVALO_MS = 60UL * 60UL * 1000UL; // 1 hora
uint32_t ultimoEvento = 0;
uint8_t audioRandom; // pista para el show secundario
uint8_t luzRandom = random8(0,1); // animación para el show secundario

const uint8_t track1 = 10; // pista de efecto del preludio al show (5 segundos)
const uint8_t track2 = 11; // pista del show cuando el hada sale (4 segundos)
const uint8_t track3 = 12; // pista de efecto cuando se sopla la brillantina (6 segundos)
const uint8_t track4 = 13; // pista del show cuando el hada regresa a la casa (9 segundos)
const uint8_t ID_RECEPTOR_AUDIO = 5; // El ID de la placa conectada a la SoundBoard
uint8_t numPista = 0;
uint8_t numPaqueteLora = 0;
uint8_t startSendPackage = 0;

// ==========================================
// PROTOTIPOS DE FUNCIONES
// ==========================================
void setupWatchdog();
void activateShow();
void updateFSM();
void processPurpurinaSequence();
void readKeyboard();
void enviarComandoPista(uint8_t numeroPista);
void enviarReproduccionAudio();

// ==========================================
// SETUP PRINCIPAL
// ==========================================
void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n=============================================");
    Serial.println("  INICIALIZANDO CASAS DE HADAS - MUNDO IMAYINA");
    Serial.println("=============================================");

    // Inicializar componentes
    lighting->begin();
    servos.begin();
    blower.begin();
    actuadorLineal.begin();

    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, PIN_LORA_CS);
    LoRa.setPins(PIN_LORA_CS, PIN_LORA_RST, PIN_LORA_DIO0);
    Serial.println("Iniciando Transmisor LoRa");

    // Inicia el módulo en la frecuencia de 915 MHz
    if (!LoRa.begin(LORA_FREQUENCY)) {
        Serial.println("¡Error al iniciar LoRa!");
        while (1); // Si falla, se queda atrapado aquí
    }

    LoRa.setSignalBandwidth(LORA_BANDWIDTH);
    LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
    LoRa.setCodingRate4(LORA_CODING_RATE);
    LoRa.setSyncWord(LORA_SYNC_WORD);
    Serial.println("LoRa configurado: 915MHz, SF9, BW125kHz, CR4/5");

    Ctrl.RegistrarAccion(PIN_BUTTON, EventoBoton::Pulsar, activateShow);
    Ctrl.InicializarCtrl();

    // Configurar Watchdog Timer de seguridad
    setupWatchdog();

    Serial.println("Sistema iniciado correctamente. Estado: IDLE");
}

// ==========================================
// BUCLE PRINCIPAL (LOOP)
// ==========================================
void loop() {
    // Lectura de tiempo para activar el show secundario
    uint32_t ahora = millis();
    if (ahora - ultimoEvento >= INTERVALO_MS) {
        ultimoEvento = ahora;
        activateShowAuto();
    }

    // Alimenta el Watchdog Timer para evitar reinicios por falso colgado
    esp_task_wdt_reset();

    // 1. Actualización del controlador de botones con antirrebote
    Ctrl.ActualizarCtrl(DEBOUNCE_DELAY_MS);

    // 2. Lectura opcional del teclado por el monitor serial
    readKeyboard();

    // 3. Despachador de la Máquina de Estados Finita (FSM)
    updateFSM();

    // 4. Revisar si hay comandos de reproducción de audio por enviar
    enviarReproduccionAudio();

    // Cede el procesador para que las tareas idle del ESP32 se ejecuten.
    delay(1);
}

// ==========================================
// CONFIGURACIÓN DEL WATCHDOG TIMER (WDT)
// ==========================================
void setupWatchdog() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = WDT_TIMEOUT_SECONDS * 1000,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = true
    };
    esp_task_wdt_reconfigure(&wdt_config);
    esp_task_wdt_add(NULL);
#else
    esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true);
    esp_task_wdt_add(NULL);
#endif
}

// ---------------------------------
// ENVIO DE COMANDOS LORA EN RAFAGA
// ---------------------------------

// FUNCIÓN PARA ENVIAR LA PISTA
void enviarComandoPista(uint8_t numeroPista) {
  ComandoLoRa paqueteEnvio;
  
  // Llenamos los datos
  paqueteEnvio.id_destino = ID_RECEPTOR_AUDIO;
  paqueteEnvio.comando = numeroPista; // Aquí viaja el 15 (o cualquier otra pista)

  // Iniciamos la transmisión LoRa
  LoRa.beginPacket();
  
  // Enviamos la estructura completa traduciéndola a bytes
  LoRa.write((uint8_t*)&paqueteEnvio, sizeof(ComandoLoRa));
  
  // Cerramos y disparamos el paquete al aire
  LoRa.endPacket();
}

void banderaReproduccionPista(uint8_t numeroPista){
    numPista = numeroPista;
    numPaqueteLora = 1;
    startSendPackage = millis();
}

void enviarReproduccionAudio(){
    if (numPista == 0) return;

    if (numPaqueteLora <= 3 && millis() - startSendPackage > 20){
        enviarComandoPista(numPista);
        startSendPackage = millis();
        numPaqueteLora ++;
    }

    if (numPaqueteLora > 3) {
        Serial.print("Mensaje LoRa enviado. Reproduciendo pista: ");
        Serial.println(numPista);
        numPista = 0;
    }

}

// ==========================================
// EVENTO DE BOTÓN
// ==========================================
void activateShow() {
    if (currentState != STATE_IDLE) {
        return;
    }

    Serial.println("[EVENTO] Activación recibida -> Activando Show");
    currentState = STATE_PRESHOW;
}

// ==========================================
// EVENTO DE TECLADO (FUNCIÓN AUXILIAR PARA PRUEBAS)
// ==========================================
void readKeyboard() {
    if (!teclado) {
        return;
    }

    while (Serial.available() > 0) {
        char command = Serial.read();
        if (command == 'r' || command == 'R') {
            Serial.println("[EVENTO] Tecla R recibida -> Activando Show completo");
            activateShow();
        } else if (command == 't' || command == 'T') {
            Serial.println("[PRUEBA] Tecla T recibida -> Enviando comando de audio de prueba (Pista 10)");
            banderaReproduccionPista(track1);
        }
    }
}

// ==========================================
// LÓGICA DE LA MÁQUINA DE ESTADOS (FSM)
// ==========================================
void updateFSM() {
    switch (currentState) {

        case STATE_IDLE:
            // Iluminación detenida
            FastLED.clear();
            FastLED.show();
            break;

        case STATE_PRESHOW:
            Serial.println("[FSM] Estado: PRESHOW -> Iniciando Audio y luces de presentación");
            showRunningStartTime = millis();
            lighting->setBrightness(BRIGHTNESS_SHOW);
            banderaReproduccionPista(track1);
            currentState = STATE_FAIRY_OUT;
            break;

        case STATE_FAIRY_OUT:
            // Patrón de iluminación de recorrido en color ámbar.
            lighting->updateAmberSequenceEffect(4, NUM_LEDS - 5);
            if (millis() - showRunningStartTime >= PRE_SHOW_RUNNING_DURATION) {
                Serial.println("[FSM] Estado: FAIRY_OUT -> Iniciando luces de la puerta y sacando el hada");
                FastLED.clear(); // Limpiar los LEDs
                FastLED.show();
                banderaReproduccionPista(track2);
                Serial.print("[AUDIO] Reproduciendo pista: ");
                Serial.print(track2);
                actuadorLineal.forward();
                showRunningStartTime = millis();
                currentState = STATE_ACTIVATED;
            }
            break;

        case STATE_ACTIVATED:
            // encender luces de la puerta en su correspondiente animación
            lighting->updateSolidEffect(0, 3);
            lighting->updateSolidEffect(NUM_LEDS - 4, NUM_LEDS - 1);
            if (millis() - showRunningStartTime >= LINEAR_ACTUATOR_DURATION_MS - GUILLOTINE_OPEN_TIME_MS - BLOWER_START_DELAY_MS) {
                Serial.println("[FSM] Estado: ACTIVATED -> Iniciando Audio y Mecanismo");

                // Abrir la guillotina e iniciar su temporización.
                purpurinaStage = PURPURINA_OPEN_GUILLOTINE;
                purpurinaStageStartTime = millis();
                showRunningStartTime = millis();
                purpurinaCompleted = false;
                servos.openGuillotine();

                currentState = STATE_SHOW_RUNNING;
            }
            break;

        case STATE_SHOW_RUNNING:
            // mantener luces de la puerta en su correspondiente animación

            // Procesar secuencia asíncrona de purpurina
            if (!purpurinaCompleted) {
                processPurpurinaSequence();
            }

            // Finalizar el show después del tiempo configurado
            if (millis() - showRunningStartTime >= SHOW_RUNNING_DURATION_MS) {
                Serial.println("[FSM] Duración de SHOW_RUNNING completada");
                currentState = STATE_CLOSING;
            }
            break;

        case STATE_CLOSING:
            Serial.println("[FSM] Estado: CLOSING -> Limpiando");
            // Asegurar que la guillotina esté cerrada y el blower apagado
            servos.closeGuillotine();
            blower.turnOff();
            lighting->clearLED(4, NUM_LEDS - 5);
            actuadorLineal.reverse();
            banderaReproduccionPista(track4);

            showRunningStartTime = millis();
            currentState = STATE_FAIRY_IN;
            Serial.println("[FSM] Retorno del hada");
            break;

        case STATE_FAIRY_IN:
            // mantener luces de la puerta en su correspondiente animación
            lighting->updateSolidEffect(0, 3);
            lighting->updateSolidEffect(NUM_LEDS - 4, NUM_LEDS - 1);
            if (millis() - showRunningStartTime >= LINEAR_ACTUATOR_DURATION_MS) {
                // Restablecer brillo de luces para reposo
                lighting->setBrightness(BRIGHTNESS_IDLE);
                actuadorLineal.turnOff();

                // Regresar a reposo
                currentState = STATE_IDLE;
                Serial.println("[FSM] Retorno a STATE_IDLE completado");
            }
            break;

        case STATE_SHOW_AUTO:
            if (luzRandom == 1) {
                lighting->updateShowEffect(4, NUM_LEDS - 5);
            } else {
                lighting->updateAmberSequenceEffect2(4, NUM_LEDS - 5);
            }

            if (millis() - showRunningStartTime >= AUTO_SHOW_RUNNING_DURATION_MS) {
                Serial.println("[FSM] Duración de SHOW_RUNNING completada");
                lighting->setBrightness(BRIGHTNESS_IDLE);
                currentState = STATE_IDLE;
                Serial.println("[FSM] Retorno a STATE_IDLE completado");
            }
            break;
    }
}

// ==========================================
// SECUENCIA ASÍNCRONA ANTI-ATASCOS DE PURPURINA
// ==========================================
void processPurpurinaSequence() {
    uint32_t elapsedTime = millis() - purpurinaStageStartTime;

    switch (purpurinaStage) {

        case PURPURINA_OPEN_GUILLOTINE:
            // Paso 1: Guillotina abierta durante el tiempo configurado.
            if (elapsedTime >= GUILLOTINE_OPEN_TIME_MS) {
                Serial.println("[PURPURINA] Cerrando Guillotina");
                servos.closeGuillotine();
                purpurinaStage = PURPURINA_WAIT_BLOWER;
                purpurinaStageStartTime = millis();
            }
            break;

        case PURPURINA_WAIT_BLOWER:
            // Paso 2: Esperar antes de encender el blower.
            if (elapsedTime >= BLOWER_START_DELAY_MS) {
                Serial.println("[PURPURINA] Encendiendo Blower");
                blower.turnOn();
                actuadorLineal.turnOff();
                banderaReproduccionPista(track3);
                purpurinaStage = PURPURINA_BLOWER;
                purpurinaStageStartTime = millis();
            }
            break;

        case PURPURINA_BLOWER:
            // Paso 3: Mantener el blower encendido durante 6 segundos.
            lighting->updateShowEffect(4, NUM_LEDS - 5);
            if (elapsedTime >= BLOWER_DURATION_MS) {
                Serial.println("[PURPURINA] Tiempo del Blower cumplido -> Apagando Blower");
                blower.turnOff();
                purpurinaStage = PURPURINA_DONE;
                purpurinaCompleted = true;
            }
            break;

        case PURPURINA_DONE:
        case PURPURINA_IDLE:
            break;
    }
}

// ==========================================
// EVENTO DE TIEMPO
// ==========================================
void activateShowAuto() {
    if (currentState != STATE_IDLE) {
        return;
    }

    Serial.println("[EVENTO] Tiempo alcanzado -> Activando Show Secundario");

    audioRandom = random8(1, 3);
    luzRandom = random8(0,1);

    currentState = STATE_SHOW_AUTO;

    Serial.println("[FSM] Estado: SHOW_AUTO -> Iniciando Audio y Luz Aleatoria");
    showRunningStartTime = millis();
    Serial.print("[AUDIO] Reproduciendo pista: ");
    Serial.println(audioRandom);
    banderaReproduccionPista(audioRandom);
    lighting->setBrightness(BRIGHTNESS_SHOW);
}