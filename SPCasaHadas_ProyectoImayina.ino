/**
 * @file CasaHadas_ProyectoImayina.ino
 * @brief Firmware Principal - Proyecto Casas de Hadas (Mundo Imayina)
 * @microcontroller ESP32-S3 Super Mini
 * 
 * Descripción:
 * Este programa controla la iluminación, expulsión asíncrona de purpurina
 * y reproducción de audio temática para las Casas de Hadas mecánicas.
 */

#include <Arduino.h>
#include <esp_task_wdt.h>
#include <Botones.hpp>

#include "src/Config.h"
#include "src/ILightingController.h"
#include "src/FastLEDController.h"
#include "src/ServoManager.h"
#include "src/BlowerControl.h"
#include "src/SPControladorDFPlayerMini.hpp"

// ==========================================
// ESTADOS DE LA MÁQUINA DE ESTADOS FINITA (FSM)
// ==========================================
enum SystemState {
    STATE_IDLE,          // Reposó: Iluminación tenue ("hada dentro"), espera de botón
    STATE_ACTIVATED,     // Activación: Inicia audio y prepara purpurina
    STATE_SHOW_RUNNING,  // Espectáculo: Mantiene show, ejecuta secuencia de purpurina (también podría monitorear pin BUSY)
    STATE_CLOSING,       // Cierre: Limpia estados y regresa a IDLE
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
ControladorDFRobotDFPlayerMini audioPlayer(PIN_DFPLAYER_RX, PIN_DFPLAYER_TX, PIN_DFPLAYER_BUSY);
Controlador Ctrl;

uint32_t purpurinaStageStartTime = 0;
uint32_t showRunningStartTime = 0;
bool purpurinaCompleted = false;
bool teclado = true;
int showChoosen = 0;
int trackChoosen = 0;
const uint32_t INTERVALO_MS = 60UL * 60UL * 1000UL; // 1 hora
uint32_t ultimoEvento = 0;
uint8_t audioRandom; // pista para el show secundario
uint8_t luzRandom = random8(0,1); // animación para el show secundario

// ==========================================
// PROTOTIPOS DE FUNCIONES
// ==========================================
void setupWatchdog();
void activateShow();
void updateFSM();
void processPurpurinaSequence();
void readKeyboard();

// ==========================================
// SETUP PRINCIPAL
// ==========================================
void setup() {
    Serial.begin(115200);
    Serial.println("\n=============================================");
    Serial.println("  INICIALIZANDO CASAS DE HADAS - MUNDO IMAYINA");
    Serial.println("=============================================");

    // Inicializar componentes
    lighting->begin();
    servos.begin();
    blower.begin();
    audioPlayer.Inicializar();
    audioPlayer.EstablecerVolumen(5);

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

// ==========================================
// EVENTO DE BOTÓN
// ==========================================
void activateShow() {
    if (currentState != STATE_IDLE) {
        return;
    }

    Serial.println("[EVENTO] Activación recibida -> Activando Show");
    showChoosen = (showChoosen + 1) % 2;
    trackChoosen = (trackChoosen % 3) + 1;
    currentState = STATE_ACTIVATED;
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
            Serial.println("[EVENTO] Tecla R recibida");
            activateShow();
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
            FastLED.clear(); // Limpiar los LEDs
            FastLED.show();
            break;

        case STATE_ACTIVATED:
            Serial.println("[FSM] Estado: ACTIVATED -> Iniciando Audio y Mecanismo");

            // 1. Abrir la guillotina e iniciar su temporización.
            purpurinaStage = PURPURINA_OPEN_GUILLOTINE;
            purpurinaStageStartTime = millis();
            showRunningStartTime = millis();
            purpurinaCompleted = false;
            servos.openGuillotine();

            // 2. Iniciar reproducción de audio y cambiar brillo de luces.
            Serial.print("[AUDIO] Reproduciendo pista: ");
            Serial.println(trackChoosen);
            audioPlayer.ReproducirPista(trackChoosen);
            lighting->setBrightness(BRIGHTNESS_SHOW);

            currentState = STATE_SHOW_RUNNING;
            break;

        case STATE_SHOW_RUNNING:
            // 1. Alternar el patrón de iluminación en cada activación.
            if (showChoosen == 1) {
                lighting->updateShowEffect();
            } else {
                lighting->updateAmberSequenceEffect2();
            }

            // 2. Procesar secuencia asíncrona de purpurina
            if (!purpurinaCompleted) {
                processPurpurinaSequence();
            }

            // 3. Finalizar el show después del tiempo configurado
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

            // Restablecer brillo de luces para reposo
            lighting->setBrightness(BRIGHTNESS_IDLE);

            // Regresar a reposo
            currentState = STATE_IDLE;
            Serial.println("[FSM] Retorno a STATE_IDLE completado");
            break;
        case STATE_SHOW_AUTO:
            if (luzRandom == 1) {
                lighting->updateShowEffect();
            } else {
                lighting->updateAmberSequenceEffect2();
            }

            if (millis() - showRunningStartTime >= SHOW_RUNNING_DURATION_MS) {
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
                purpurinaStage = PURPURINA_BLOWER;
                purpurinaStageStartTime = millis();
            }
            break;

        case PURPURINA_BLOWER:
            // Paso 3: Mantener el blower encendido durante 2 segundos.
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
    audioPlayer.ReproducirPista(audioRandom);
    lighting->setBrightness(BRIGHTNESS_SHOW);
}