#include <ESP32Servo.h>

// Pin de señal asignado en el ESP32S3 Super Mini para cada servomotor
#define SERVO1_PIN 1 // Pin de señal para el servomotor que controla la puerta de la casa
#define SERVO2_PIN 2 // Pin de señal para el servomotor que controla la entrada de la purpurina

// Posición calibrada de los servomotores
const int servo1Position_0 = 0;
const int servo1Position_1 = 90;
const long int servo1Duration = 3000;  // Duración del servomotor en la posición 1 (en milisegundos)

const int servo2Position_0 = 0;
const int servo2Position_1 = 90;
const long int servo2Duration = 500;  // Duración del servomotor en la posición 1 (en milisegundos)

Servo miServo1;
Servo miServo2;

void setup() {
  Serial.begin(115200);

  // Asignar temporizador de PWM para el ESP32
  ESP32PWM::allocateTimer(0);
  
  // La frecuencia estándar para el MG90S es de 50 Hz
  miServo1.setPeriodHertz(50);
  miServo2.setPeriodHertz(50);

  // Vincula el servo al pin con los anchos de pulso habituales el MG90S (500us - 2400us)
  miServo1.attach(SERVO1_PIN, 500, 2400);
  miServo2.attach(SERVO2_PIN, 500, 2400);

  if (servo1Duration < servo2Duration) {
    Serial.println("Advertencia: La duración del servomotor 1 es menor que la del servomotor 2.");
    return; // Detener la ejecución si la duración del servomotor 1 es mayor
  }

  Serial.println("Posición inicial");
  miServo1.write(servo1Position_0);
  miServo2.write(servo2Position_0);

  Serial.println("--- Inicio de prueba del servo MG90S ---");
}

void loop() {

  delay(1000); // Espera 1 segundo antes de iniciar la secuencia

  // Posición de espectáculo: puerta y guillotina abiertas
  Serial.println("Posición1: espectáculo");
  miServo1.write(servo1Position_1);
  miServo2.write(servo2Position_1);
  delay(servo2Duration); // Espera hasta que la purpurina se termine de liberar

  miServo1.write(servo2Position_0); // Cierra la purpurina

  if (servo1Duration > servo2Duration) {
    delay(servo1Duration - servo2Duration); // Espera adicional si el show dura más que la purpurina
  }

  // Posición inicial: puerta y guillotina cerradas
  Serial.println("Posición2: reposo");
  miServo2.write(servo1Position_0);

}