// Definición de pines según la tabla del ESP32-S3 Super Mini
const int BUTTON_PIN = 7; // Botón (un pin a GPIO 7, el otro a GND)
const int BLOWER_PIN = 3; // Control hacia el MOSFET o Transistor del Blower

void setup() {
  Serial.begin(115200);

  // Usa la resistencia de pull-up interna del ESP32.
  // El pin estará en HIGH por defecto, y pasará a LOW al presionar el botón.
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Configura la salida hacia la etapa de potencia del Blower
  pinMode(BLOWER_PIN, OUTPUT);
  digitalWrite(BLOWER_PIN, LOW); // Asegurar que inicie apagado

  Serial.println("--- Prueba de Blower con Botón Lista ---");
}

void loop() {
  // Leer el estado del botón (LOW = Presionado, HIGH = Suelto)
  int buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW) {
    digitalWrite(BLOWER_PIN, HIGH); // Enciende el Blower
    Serial.println("Estado: Botón PRESIONADO -> Blower ENCENDIDO");
  } else {
    digitalWrite(BLOWER_PIN, LOW);  // Apaga el Blower
  }

  delay(50); // Filtro básico antirrebote (debounce)
}