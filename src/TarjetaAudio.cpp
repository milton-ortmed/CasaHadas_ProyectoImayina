#include "TarjetaAudio.hpp"

/*
  ControladorTarjetaAudio
*/

ControladorTarjetaAudio::~ControladorTarjetaAudio() { }

void ControladorTarjetaAudio::Silenciar(bool silenciar) {
  silenciado = silenciar;
  if (silenciado) {
    Detener();
  }
}

bool ControladorTarjetaAudio::Silenciado() {
  return silenciado;
}

int ControladorTarjetaAudio::ObtenerVolumen() {
  return volumen;
}

/*
  ControladorOrtMedSoundBoard_v2
*/

ControladorOrtMedSoundBoard_v2::ControladorOrtMedSoundBoard_v2(int uart) {
  tipoUart = HARDWARE_SERIAL;
  switch(uart) {
    case 0:
      this->uart = &Serial;
      break;
    case 1:
      this->uart = &Serial1;
      #ifdef ESP32
      Serial1.setPins(GPIO_NUM_26, GPIO_NUM_27);
      #endif
      break;
    case 2:
      #if defined(ESP32)
      this->uart = &Serial2;
      Serial2.setPins(GPIO_NUM_16, GPIO_NUM_17);
      #else
      this->uart = &Serial1;
      #endif
      break;
    default:
      this->uart = &Serial1;
      #ifdef ESP32
      Serial1.setPins(GPIO_NUM_26, GPIO_NUM_27);
      #endif
  }
}

ControladorOrtMedSoundBoard_v2::ControladorOrtMedSoundBoard_v2(uint8_t pinRx, uint8_t pinTx) {
  tipoUart = SOFTWARE_SERIAL;
  uart = new SoftwareSerial(pinRx, pinTx);
}

ControladorOrtMedSoundBoard_v2::~ControladorOrtMedSoundBoard_v2() {     
  Detener();
}

void ControladorOrtMedSoundBoard_v2::Inicializar() {
  #ifdef ESP8266
  switch (tipoUart) {
    case HARDWARE_SERIAL:
      (static_cast<HardwareSerial*>(uart))->begin(115200);
      break;
    case SOFTWARE_SERIAL:
      break;
      (static_cast<SoftwareSerial*>(uart))->begin(9600);
    default:
      (static_cast<HardwareSerial*>(uart))->begin(115200);
      break;
  }
  #endif
  #ifdef ESP32
  switch (tipoUart) {
    case HARDWARE_SERIAL:
      (static_cast<HardwareSerial*>(uart))->begin(921600);
      break;
    case SOFTWARE_SERIAL:
      (static_cast<SoftwareSerial*>(uart))->begin(9600);
      break;
    default:
      (static_cast<HardwareSerial*>(uart))->begin(921600);
      break;
  }
  #endif
  EstablecerVolumen(volumen);
  Detener();
}

void ControladorOrtMedSoundBoard_v2::ReproducirPista(int pista, bool repetir) {
  if (pista <= 0 || silenciado) return;
  const size_t NBYTES = 16;
  uint8_t buffer[NBYTES];
  buffer[0] = INI1;
  buffer[1] = INI2;
  buffer[2] = INI3;
  buffer[3] = INI4;
  buffer[4] = 0;
  buffer[5] = CMD_REPRODUCIR_PISTA;
  buffer[6] = (pista >> 16) & 0xFF;
  buffer[7] = (pista >> 8) & 0xFF;
  buffer[8] = pista & 0xFF;
  buffer[9] = CHANNEL_0;
  buffer[10] = 0;
  buffer[11] = repetir;
  buffer[12] = FIN1;
  buffer[13] = FIN2;
  buffer[14] = FIN3;
  buffer[15] = FIN4;
  #ifdef HABILITAR_SALIDA_DEPURACION
  ImprimirBuffer(buffer, NBYTES);
  #endif
  uart->write(buffer, NBYTES);
}

void ControladorOrtMedSoundBoard_v2::ReproducirPista(int pista, int salida, bool repetir) {
  Serial.println("Rep. pista: " + String(pista) + ", salida: " + String(salida));
  if (pista <= 0 || silenciado) return;
  // Polifonía en el canal 0 de la SoundBoard
  const size_t NBYTES = 16;
  uint8_t buffer[NBYTES];
  buffer[0] = INI1;
  buffer[1] = INI2;
  buffer[2] = INI3;
  buffer[3] = INI4;
  buffer[4] = 0;
  buffer[5] = CMD_REPRODUCIR_PISTA;
  buffer[6] = (pista >> 16) & 0xFF;
  buffer[7] = (pista >> 8) & 0xFF;
  buffer[8] = pista & 0xFF;
  buffer[9] = CHANNEL_0;
  buffer[10] = (uint8_t)salida;
  buffer[11] = repetir;
  buffer[12] = FIN1;
  buffer[13] = FIN2;
  buffer[14] = FIN3;
  buffer[15] = FIN4;
  #ifdef HABILITAR_SALIDA_DEPURACION
  ImprimirBuffer(buffer, NBYTES);
  #endif
  uart->write(buffer, NBYTES);
}

void ControladorOrtMedSoundBoard_v2::ReproducirPistaSalidasCentrales(int pista, bool aplicarVolumenPrincipal) {
  if (pista <= 0 || silenciado) return; 
  if (!esUnicaSalida && !aplicarVolumenPrincipal) {
    // Restaurando el volumen que otros métodos pudieran haber modificado.
    EstablecerVolumenSalidasCentrales(volumenSalidasCentrales);
  }
  const size_t NBYTES = 16;
  uint8_t buffer[NBYTES];
  buffer[0] = INI1;
  buffer[1] = INI2;
  buffer[2] = INI3;
  buffer[3] = INI4;
  buffer[4] = 0;
  buffer[5] = CMD_REPRODUCIR_PISTA;
  buffer[6] = (pista >> 16) & 0xFF;
  buffer[7] = (pista >> 8) & 0xFF;
  buffer[8] = pista & 0xFF;
  buffer[9] = CHANNEL_0;
  buffer[10] = 0;
  buffer[11] = false;
  buffer[12] = FIN1;
  buffer[13] = FIN2;
  buffer[14] = FIN3;
  buffer[15] = FIN4;
  #ifdef HABILITAR_SALIDA_DEPURACION
  ImprimirBuffer(buffer, NBYTES);
  #endif
  uart->write(buffer, NBYTES);
}

void ControladorOrtMedSoundBoard_v2::ReproducirPistaSalidasLaterales(int pista, bool aplicarVolumenPrincipal) {
  if (pista <= 0 || silenciado) return;
  if (!esUnicaSalida && !aplicarVolumenPrincipal) {
    // Restaurando el volumen que otros métodos como ReproducirPistaSalidaLateral1() o ReproducirPistaSalidaLateral2() pudieran haber modificado.
    EstablecerVolumenSalidasLaterales(volumenSalidasLaterales);
  }
  const size_t NBYTES = 16;
  uint8_t buffer[NBYTES];
  buffer[0] = INI1;
  buffer[1] = INI2;
  buffer[2] = INI3;
  buffer[3] = INI4;
  buffer[4] = 0;
  buffer[5] = CMD_REPRODUCIR_PISTA;
  buffer[6] = (pista >> 16) & 0xFF;
  buffer[7] = (pista >> 8) & 0xFF;
  buffer[8] = pista & 0xFF;
  buffer[9] = CHANNEL_1;
  buffer[10] = 0;
  buffer[11] = false;
  buffer[12] = FIN1;
  buffer[13] = FIN2;
  buffer[14] = FIN3;
  buffer[15] = FIN4;
  #ifdef HABILITAR_SALIDA_DEPURACION
  ImprimirBuffer(buffer, NBYTES);
  #endif
  uart->write(buffer, NBYTES);
}

void ControladorOrtMedSoundBoard_v2::ReproducirPistaSalidaLateral1(int pista) {
  esSalida1 = true;
  EstablecerVolumenSalidaLateral1(volumenLateral1);
  esUnicaSalida = true;
  ReproducirPistaSalidasLaterales(pista + offsetAudios);
  esUnicaSalida = false;
}

void ControladorOrtMedSoundBoard_v2::ReproducirPistaSalidaLateral2(int pista) {
  esSalida1 = false;
  EstablecerVolumenSalidaLateral2(volumenLateral2);
  esUnicaSalida = true;
  ReproducirPistaSalidasLaterales(pista);
  esUnicaSalida = false;
}

void ControladorOrtMedSoundBoard_v2::EstablecerVolumen(int nivel) {
  volumen = constrain(nivel, 0, NUM_NIVELES_VOLUMEN);  
  const size_t NBYTES = 12;
  uint8_t buffer[NBYTES];
  buffer[0] = INI1;
  buffer[1] = INI2;
  buffer[2] = INI3;
  buffer[3] = INI4;
  buffer[4] = 0;
  buffer[5] = CMD_VOLUMEN;
  buffer[6] = (uint8_t)nivelesVolumen[nivel];
  buffer[7] = CMD_TERMINACION_ANTICIPADA;
  buffer[8] = FIN1;
  buffer[9] = FIN2;
  buffer[10] = FIN3;
  buffer[11] = FIN4;
  #ifdef HABILITAR_SALIDA_DEPURACION
  ImprimirBuffer(buffer, NBYTES);
  #endif
  uart->write(buffer, NBYTES);
};

void ControladorOrtMedSoundBoard_v2::EstablecerVolumenSalidasCentrales(int nivel) {
  volumenSalidasCentrales = constrain(nivel, 0, NUM_NIVELES_VOLUMEN);
  const size_t NBYTES = 12;
  uint8_t buffer[NBYTES];
  buffer[0] = INI1;
  buffer[1] = INI2;
  buffer[2] = INI3;
  buffer[3] = INI4;
  buffer[4] = 0;
  buffer[5] = CMD_VOLUMEN;
  buffer[6] = (uint8_t)nivelesVolumen[nivel];
  buffer[7] = CHANNEL_0;
  buffer[8] = FIN1;
  buffer[9] = FIN2;
  buffer[10] = FIN3;
  buffer[11] = FIN4;
  #ifdef HABILITAR_SALIDA_DEPURACION
  ImprimirBuffer(buffer, NBYTES);
  #endif
  uart->write(buffer, NBYTES);
}

void ControladorOrtMedSoundBoard_v2::EstablecerVolumenSalidasLaterales(int nivel) {
  volumenSalidasLaterales = constrain(nivel, 0, NUM_NIVELES_VOLUMEN);  
  const size_t NBYTES = 12;
  uint8_t buffer[NBYTES];
  buffer[0] = INI1;
  buffer[1] = INI2;
  buffer[2] = INI3;
  buffer[3] = INI4;
  buffer[4] = 0;
  buffer[5] = CMD_VOLUMEN;
  buffer[6] = (uint8_t)nivelesVolumen[nivel];
  buffer[7] = CHANNEL_1;
  buffer[8] = FIN1;
  buffer[9] = FIN2;
  buffer[10] = FIN3;
  buffer[11] = FIN4;
  #ifdef HABILITAR_SALIDA_DEPURACION
  ImprimirBuffer(buffer, NBYTES);
  #endif
  uart->write(buffer, NBYTES);
}

void ControladorOrtMedSoundBoard_v2::EstablecerVolumenSalidaLateral1(int nivel) {
  volumenLateral1 = constrain(nivel, 0, NUM_NIVELES_VOLUMEN); 
  if (esSalida1)  {
    const size_t NBYTES = 12;
    uint8_t buffer[NBYTES];
    buffer[0] = INI1;
    buffer[1] = INI2;
    buffer[2] = INI3;
    buffer[3] = INI4;
    buffer[4] = 0;
    buffer[5] = CMD_VOLUMEN;
    buffer[6] = (uint8_t)nivelesVolumen[nivel];
    buffer[7] = CHANNEL_1;
    buffer[8] = FIN1;
    buffer[9] = FIN2;
    buffer[10] = FIN3;
    buffer[11] = FIN4;
    #ifdef HABILITAR_SALIDA_DEPURACION
    ImprimirBuffer(buffer, NBYTES);
    #endif
    uart->write(buffer, NBYTES);
  }
}

void ControladorOrtMedSoundBoard_v2::EstablecerVolumenSalidaLateral2(int nivel) {
  volumenLateral2 = constrain(nivel, 0, NUM_NIVELES_VOLUMEN); 
  if (!esSalida1) {
    const size_t NBYTES = 12;
    uint8_t buffer[NBYTES];
    buffer[0] = INI1;
    buffer[1] = INI2;
    buffer[2] = INI3;
    buffer[3] = INI4;
    buffer[4] = 0;
    buffer[5] = CMD_VOLUMEN;
    buffer[6] = (uint8_t)nivelesVolumen[nivel];
    buffer[7] = CHANNEL_1;
    buffer[8] = FIN1;
    buffer[9] = FIN2;
    buffer[10] = FIN3;
    buffer[11] = FIN4;
    #ifdef HABILITAR_SALIDA_DEPURACION
    ImprimirBuffer(buffer, NBYTES);
    #endif
    uart->write(buffer, NBYTES);
  }
}

void ControladorOrtMedSoundBoard_v2::DetenerPistaSalidasCentrales() {
  const size_t NBYTES = 12;
  uint8_t buffer[NBYTES];
  buffer[0] = INI1;
  buffer[1] = INI2;
  buffer[2] = INI3;
  buffer[3] = INI4;
  buffer[4] = 0;
  buffer[5] = CMD_DETENER;
  buffer[6] = CHANNEL_0;
  buffer[7] = CMD_TERMINACION_ANTICIPADA;
  buffer[8] = FIN1;
  buffer[9] = FIN2;
  buffer[10] = FIN3;
  buffer[11] = FIN4;
  #ifdef HABILITAR_SALIDA_DEPURACION
  ImprimirBuffer(buffer, NBYTES);
  #endif
  uart->write(buffer, NBYTES);
}

void ControladorOrtMedSoundBoard_v2::DetenerPistaSalidasLaterales() {
  const size_t NBYTES = 12;
  uint8_t buffer[NBYTES];
  buffer[0] = INI1;
  buffer[1] = INI2;
  buffer[2] = INI3;
  buffer[3] = INI4;
  buffer[4] = 0;
  buffer[5] = CMD_DETENER;
  buffer[6] = CHANNEL_1;
  buffer[7] = CMD_TERMINACION_ANTICIPADA;
  buffer[8] = FIN1;
  buffer[9] = FIN2;
  buffer[10] = FIN3;
  buffer[11] = FIN4;
  #ifdef HABILITAR_SALIDA_DEPURACION
  ImprimirBuffer(buffer, NBYTES);
  #endif
  uart->write(buffer, NBYTES);
}

void ControladorOrtMedSoundBoard_v2::Detener() {
  const size_t NBYTES = 11;
  uint8_t buffer[NBYTES];
  buffer[0] = INI1;
  buffer[1] = INI2;
  buffer[2] = INI3;
  buffer[3] = INI4;
  buffer[4] = 0;
  buffer[5] = CMD_DETENER;
  buffer[6] = CMD_TERMINACION_ANTICIPADA;
  buffer[7] = FIN1;
  buffer[8] = FIN2;
  buffer[9] = FIN3;
  buffer[10] = FIN4;
  #ifdef HABILITAR_SALIDA_DEPURACION
  ImprimirBuffer(buffer, NBYTES);
  #endif
  uart->write(buffer, NBYTES);
}

void ControladorOrtMedSoundBoard_v2::Detener(int canal, int voz) {
  const size_t NBYTES = 12;
  uint8_t buffer[NBYTES];
  buffer[0] = INI1;
  buffer[1] = INI2;
  buffer[2] = INI3;
  buffer[3] = INI4;
  buffer[4] = 0;
  buffer[5] = CMD_DETENER;
  buffer[6] = canal;
  buffer[7] = voz;
  buffer[8] = FIN1;
  buffer[9] = FIN2;
  buffer[10] = FIN3;
  buffer[11] = FIN4;
  #ifdef HABILITAR_SALIDA_DEPURACION
  ImprimirBuffer(buffer, NBYTES);
  #endif
  uart->write(buffer, NBYTES);
}

void ControladorOrtMedSoundBoard_v2::EstablecerVolumen(int nivel, int salida) {
  // TBD
}

void ControladorOrtMedSoundBoard_v2::ImprimirBuffer(uint8_t* buffer, int tam) {
  uint8_t valor;
  for (int i = 0; i < tam; i++) {
    valor = buffer[i];
    Serial.print(valor, HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void ControladorOrtMedSoundBoard_v2::ActivarControladorAuxiliar() {  
  const size_t NBYTES = 11;
  uint8_t buffer[NBYTES];
  buffer[0] = INI1;
  buffer[1] = INI2;
  buffer[2] = INI3;
  buffer[3] = INI4;
  buffer[4] = 0;
  buffer[5] = CMD_BLUETOOTH;;
  buffer[6] = 1;
  buffer[7] = FIN1;
  buffer[8] = FIN2;
  buffer[9] = FIN3;
  buffer[10] = FIN4;
  #ifdef HABILITAR_SALIDA_DEPURACION
  ImprimirBuffer(buffer, NBYTES);
  #endif
  uart->write(buffer, NBYTES);
  btHabilitado = true;
}

void ControladorOrtMedSoundBoard_v2::DesactivarControladorAuxiliar() {  
  const size_t NBYTES = 11;
  uint8_t buffer[NBYTES];
  buffer[0] = INI1;
  buffer[1] = INI2;
  buffer[2] = INI3;
  buffer[3] = INI4;
  buffer[4] = 0;
  buffer[5] = CMD_BLUETOOTH;;
  buffer[6] = 0;
  buffer[7] = FIN1;
  buffer[8] = FIN2;
  buffer[9] = FIN3;
  buffer[10] = FIN4;
  #ifdef HABILITAR_SALIDA_DEPURACION
  ImprimirBuffer(buffer, NBYTES);
  #endif
  uart->write(buffer, NBYTES);
  btHabilitado = false;
}

bool ControladorOrtMedSoundBoard_v2::ControladorAuxiliarActivado() {
  return btHabilitado;
}