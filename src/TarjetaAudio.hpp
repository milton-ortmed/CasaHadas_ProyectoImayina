#pragma once

#include <Arduino.h>
#include <SoftwareSerial.h>

// #define HABILITAR_SALIDA_DEPURACION    // Descomentar si se desea imprimir el contenido de los buffers.

/**
 * @brief Controlador para las tarjetas de audio de un solo canal.
 */
class ControladorTarjetaAudio {
  protected:
    // Indica si el controlador está o no silenciado.
    bool silenciado = false;
    // Nivel general de volumen actualmente establecido.
    int volumen = 12;
    // Número de pista que se repite inmediatemente al terminar de reproducirse.
    // Se establece en el método ReproducirPista() cuando la bandera "repetir" se establece en "true".
    int pistaEnRepeticion = 0;
    // Cantidad de niveles de volumen
    static const int NUM_NIVELES_VOLUMEN = 21;
  public:
    virtual ~ControladorTarjetaAudio();
    // Efectúa el proceso de inicialización del controlador.
    virtual void Inicializar() = 0;
    /**
     * @brief Reproduce la pista en la salida tanto central como lateral.
     * @param pista El número de pista a reproducir.
     * @param  repetir Establecer en "true" para reiniciar automáticamente la pista cuando ésta termine, 
     * "false" para reproducirla sólo una vez. "false" por defecto.
     */
    virtual void ReproducirPista(int pista, bool repetir = false) = 0;    
    // Detiene todas las pistas en ejecución
    virtual void Detener() = 0;
    /**
     * @brief Establece un nivel de volumen general para la salida de audio de la tarjeta.
     * @param nivel El nivel de volumen. El rango permitido es entre 0 (sin volumen) y 9. El valor por defecto es 5.
     */
    virtual void EstablecerVolumen(int nivel) = 0;
    /**
     * @brief Habilita o inhabilita la reproducción de sonido.
     * @param silenciar "True" para inhabilitar el sonido, "false" para habilitarlo.
     */
    virtual void Silenciar(bool silenciar);
    // Retorna "true" si el dispositivo está actualmente silenciado y "false" en caso contrario. 
    bool Silenciado();
    // Retorna el nivel de volumen actualmente establecido en el dispositivo. 
    int ObtenerVolumen();
};

/**
 * @brief Interfaz para el control con el dispositivo de audio auxiliar
 */
class ControladorAuxiliar {
  public:
    // Habilita la funcionalidad del dispositivo auxiliar.
    virtual void ActivarControladorAuxiliar() = 0;
    // Deshabilita la funcionalidad del dispositivo auxiliar.
    virtual void DesactivarControladorAuxiliar() = 0;
    // Retorna "true" si el dispositivo auxiliar está activado, y "false" en caso contrario.
    virtual bool ControladorAuxiliarActivado() = 0;
};

/**
 * @brief Controlador para las tarjetas que cuentan con más de un canal de salida de audio.
 */
class ControladorTarjetaAudioMulticanal : public ControladorTarjetaAudio, public ControladorAuxiliar {
  protected:
    // Volumen para la salida central de audio.
    int volumenSalidasCentrales = 12;
    // Volumen para la salida lateral de audio.
    int volumenSalidasLaterales = 12;
    // Volumen actual para la salida lateral 1
    int volumenLateral1 = 12;
    // Volumen actual para la salida lateral 2
    int volumenLateral2 = 12;
  public:
    // Efectúa el proceso de inicialización del controlador.
    virtual void Inicializar() = 0;
    /**
     * @brief Reproduce la pista en la salida general de audio.
     * @param pista El número de pista a reproducir.
     * @param  repetir Establecer en "true" para reiniciar automáticamente la pista cuando ésta termine, 
     * "false" para reproducirla sólo una vez. "false" por defecto.
     */
    virtual void ReproducirPista(int pista, bool repetir = false) = 0;
    // Detiene todas las pistas en ejecución
    virtual void Detener() = 0;
    /**
     * @brief Detiene la voz que se esté reproduciendo en el canal especificado
     * @param canal El canal en el que está la voz a detener.
     * @param voz El índice de voz a detener. Los valores válidos depeden de la implementación del controlador.
     */
    virtual void Detener(int canal, int voz) = 0;
    /**
     * @brief Establece un nivel de volumen general para la salida de audio de la tarjeta.
     * @param nivel El nivel de volumen. El rango permitido es entre 0 (sin volumen) y 20. El valor por defecto es 5.
     */
    virtual void EstablecerVolumen(int nivel) = 0;
    /**
     * @brief Establece el nivel de volumen para la pista en la salida especificada.
     * @param nivel El nivel de volumen. El rango permitido es entre 0 (sin volumen) y 20.
     * @param salida La salida o canal de audio en la que reproducir la pista.
     */
    virtual void EstablecerVolumen(int nivel, int salida) = 0;
    /**
     * @brief Reproduce la pista de audio en la salida central del dispositivo; suelen etiquetarse como "canal 1".
     * @param pista El número de pista a reproducir.
     */
    virtual void ReproducirPistaSalidasCentrales(int pista, bool aplicarVolumenPrincipal = false) = 0;
    /**
     * @brief Repruduce la pista de audio en las bocinas laterales del dispositivo; suelen etiquetarse como "canal 2".
     * (NOTA: tratándose del Sensy Magic, corresponden a las bocinas en las nubecitas; si es el TOi Robot, entonces son las bocinas en sus manitas).
     * @param pista El número de pista a reproducir.
     */
    virtual void ReproducirPistaSalidasLaterales(int pista, bool aplicarVolumenPrincipal = false) = 0;
    /**
     * @brief Reproduce la pista especificada solamente en una de las bocinas laterales (bocina opuesta a la empleada por ReproducirPistaSalidaLateral2()).
     * @param pista El número de pista a reproducir
     */
    virtual void ReproducirPistaSalidaLateral1(int pista) = 0;
    /**
     * @brief Reproduce la pista especificada solamente en una de las bocinas laterales (bocina opuesta a la empleada por ReproducirPistaSalidaLateral1()).
     * @param pista El número de pista a reproducir
     */
    virtual void ReproducirPistaSalidaLateral2(int pista) = 0;
    /**
     * @brief Establece el volumen para las salidas centrales.
     * @param nivel El nivel de volumen. El rango permitido es entre 0 (sin volumen) y 20.
     */
    virtual void EstablecerVolumenSalidasCentrales(int nivel) = 0;
    /**
     * @brief Establece el volumen para las salidas laterales.
     * @param nivel El nivel de volumen. El rango permitido es entre 0 (sin volumen) y 20.
     */
    virtual void EstablecerVolumenSalidasLaterales(int nivel) = 0;
    /**
     * @brief Establece el volumen para la bocina lateral 1.
     * @param nivel El nivel de volumen. El rango permitido es entre 0 (sin volumen) y 20.
     */
    virtual void EstablecerVolumenSalidaLateral1(int nivel) = 0;
    /**
     * @brief Establece el volumen para la bocina lateral 1.
     * @param nivel El nivel de volumen. El rango permitido es entre 0 (sin volumen) y 20.
     */
    virtual void EstablecerVolumenSalidaLateral2(int nivel) = 0;
    // Detiene la pista que se esté ejecutando en las salidas centrales.
    virtual void DetenerPistaSalidasCentrales() = 0;
    // Detiene la pista que se esté ejecutando en las salidas laterales.
    virtual void DetenerPistaSalidasLaterales() = 0;

    /**
     * @brief Reproduce una pista en la salida especificada. El contexto que adquiere el número de salida es dependiente de la implementación;
     * consulta la documentación de las subclases para más información sobre la misma.
     * @param pista Número de pista a reproducir. Si la pista no existe no se reproducirá nada.
     * @param salida La salida o canal de audio en la que reproducir la pista. El contexto de este número puede variar entre implementaciones.
     * @param  repetir Establecer en "true" para reiniciar automáticamente la pista cuando ésta termine, 
     * "false" para reproducirla sólo una vez. "false" por defecto.
     */
    virtual void ReproducirPista(int pista, int salida, bool repetir = false) = 0;    
    // Habilita la funcionalidad del dispositivo auxiliar.
    virtual void ActivarControladorAuxiliar() = 0;
    // Deshabilita la funcionalidad del dispositivo auxiliar.
    virtual void DesactivarControladorAuxiliar() = 0;
    // Retorna "true" si el dispositivo auxiliar está activado, y "false" en caso contrario.
    virtual bool ControladorAuxiliarActivado() = 0;
};

/**
 * @brief Tipos de UART que puede usar el controlador de la OrtMed SoundBoard.
 */
enum TipoUART {
  HARDWARE_SERIAL,
  SOFTWARE_SERIAL
};
/**
 * @brief Controlador para la tarjeta de audio diseñada en OrtMed por el Ing. Yael Narváez para introducir mejoras y simplificaciones.
 * Al igual que la versión anterior, esta SoundBoard reproduce archivos de audio en stereo (muestreados a 22KHz y codificados a 16 bits) que pueden ser reproducidos 
 * en uno o dos canales, identificados como canal 0 y canal 1. Ambos canales tienen polifonía, con la capacidad de reproducir 3 diferentes "voces" cada uno.
 * Los archivos de audio son leídos desde una memoria MicroSD; el nombre de éstos debe corresponder a su número de pista (por ejemplo: 1.wav para la pista 1, 123.wav para la pista 123, etcétera).
 * La tarjeta es controlada mediante comunicación serial por hardware empleando algún UART, el cual se especifica desde el constructor.
 */
class ControladorOrtMedSoundBoard_v2 : public ControladorTarjetaAudioMulticanal {
  protected:
    TipoUART tipoUart = HARDWARE_SERIAL;
    // Puntero al receptor-transmisor serial (UART) empleado para la comunicación con la tarjeta.
    Stream *uart = NULL;
    // Bytes que marcan el inicio de un comando a serializar.
    static const uint8_t INI1 = 0xF0;
    static const uint8_t INI2 = 0xAA;
    static const uint8_t INI3 = 0xD8;
    static const uint8_t INI4 = 0x8B;
    // Bytes que marcan el final del comando a serializar.
    static const uint8_t FIN1 = 0x55;
    static const uint8_t FIN2 = 0xBB;
    static const uint8_t FIN3 = 0x77;
    static const uint8_t FIN4 = 0x11;
    // Banderas para conformar el contenido de los comandos.
    static const uint8_t CHANNEL_0 = 0x00;
    static const uint8_t CHANNEL_1 = 0x01;
    // Comandos actualmente implementados en la versión 2.
    static const uint8_t CMD_REPRODUCIR_PISTA = 0x01;   
    static const uint8_t CMD_DETENER = 0x02; 
    static const uint8_t CMD_VOLUMEN = 0x03;
    static const uint8_t CMD_BLUETOOTH = 0x04;
    static const uint8_t CMD_GUARDAR_AUDIO = 0x05;
    static const uint8_t CMD_ELIMINAR_AUDIO = 0x06;
    static const uint8_t CMD_LONGITUD_MENSAJE = 0xA0;
    static const uint8_t CMD_LONGITUD_MENSAJE_POR_DEFECTO = 0xA1;
    static const uint8_t CMD_TERMINACION_ANTICIPADA = 0x99;
    // Offset existente entre números de pistas cargadas hacia la derecha y pistas cargadas hacia la izquierda.
    // Ejemplo: si una pista con audio cargado hacia la derecha tiene un número de pista igual a 200, el controlador esperará
    // que la pista análoga cargada hacia la izquierda tenga un número de pista igual a 200 + offsetAudios.
    int offsetAudios = 1000;
    // Mapeo entre nivel de volumen especificado y el nivel de volumen real.
    // La OrtMed SoundBoard acepta actualmente 21 niveles de volumen [0..20].
    int nivelesVolumen[NUM_NIVELES_VOLUMEN] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                                                11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
    // Bandera para indicar si se está reproduciendo audio en una única salida lateral o central.
    bool esUnicaSalida = false;
    // Bandera para indicar que la última pista se reprodujo en la salida 1 (lateral o central).
    bool esSalida1 = false;
    // Indica si la funcionalidad Bluetooth está o no habilitada
    bool btHabilitado = false;
    // Imprime un arreglo.
    void ImprimirBuffer(uint8_t* buffer, int tam);
  public:
    /**
     * @brief Construye un controlador para la OrtMed SoundBoard que usará comunicación serial por hardware.
     * @param uart Indica la instancia de HardwareSerial que debe usarse para la comunicación. Los valores válidos son 1 (SERIAL1) y 2 (SERIAL2).
     * Valores no válidos se toman como 2. 
     */
    ControladorOrtMedSoundBoard_v2(
    #if defined(ESP8266) 
    int uart = 1
    #elif defined(ESP32) 
    int uart = 2
    #endif
    );
    /**
     * @brief Construye un controlador para la OrtMed SoundBoard que usará comunicación serial emulada por software.
     * @param pinRx El pin de recepción de datos para la comunicación serial.
     * @param pinTx El pin de transmisión de datos para la comunicación serial.
     */
    ControladorOrtMedSoundBoard_v2(uint8_t pinRx, uint8_t pinTx);
    ~ControladorOrtMedSoundBoard_v2();
    // Inicia la comunicación serial y establece el volumen por defecto.
    virtual void Inicializar() override;
    /**
     * @brief Reproduce una pista en ambos canales stéreo.
     * @param pista El número de pista a reproducir. 
     * @param repetir Establecer en "true" para reiniciar automáticamente la pista cuando ésta termine, 
     * "false" para reproducirla sólo una vez. "false" por defecto.
     * NOTA: para detener la repetición, es necesario invocar al método Detener(), o bien invocar a ReproducirPista() con este
     * parámetro en "false".
     */
    virtual void ReproducirPista(int pista, bool repetir = false) override;
    /**
     * @brief Reproduce hasta dos pistas en la salida polifónica de la tarjeta (canal 0).
     * @param pista El número de pista a reproducir.
     * @param salida Este parámetro especifica el "número de voz" en la cual se reproducirá el audio. El rango válido es [0..6].
     * * @param repetir Establecer en "true" para reiniciar automáticamente la pista cuando ésta termine, 
     * "false" para reproducirla sólo una vez. "false" por defecto.
     */
    virtual void ReproducirPista(int pista, int salida, bool repetir = false) override;  
    /**
     * @brief Reproduce una pista en el canal 0.
     * @param pista El número de pista a reproducir.
     * @param aplicarVolumenPrincipal Indica si debe usarse el volumen general en lugar del establecido para las salidas centrales.
     */
    virtual void ReproducirPistaSalidasCentrales(int pista, bool aplicarVolumenPrincipal = false) override;
    /**
     * @brief Reproduce una pista en el canal 1.
     * @param pista El número de pista a reproducir.
     * * @param aplicarVolumenPrincipal Indica si debe usarse el volumen general en lugar del establecido para las salidas laterales.
     */
    virtual void ReproducirPistaSalidasLaterales(int pista, bool aplicarVolumenPrincipal = false) override;
    void ReproducirPistaSalidaLateral1(int pista) override;
    void ReproducirPistaSalidaLateral2(int pista) override;
    void EstablecerVolumenSalidasCentrales(int nivel) override;
    void EstablecerVolumenSalidasLaterales(int nivel) override;
    void EstablecerVolumenSalidaLateral1(int nivel) override;
    void EstablecerVolumenSalidaLateral2(int nivel) override;
    void DetenerPistaSalidasCentrales() override;
    void DetenerPistaSalidasLaterales() override;
    // Detiene todas las pistas en ejecución
    void Detener() override;
    /**
     * @brief Detiene la voz que se esté reproduciendo en el canal especificado
     * @param canal El canal en el que está la voz a detener.
     * @param voz El índice de voz a detener. Los valores válidos depeden de la implementación del controlador.
     */
    void Detener(int canal, int voz) override;
    /**
     * @brief Establece el nivel de volumen en ambos canales stéreo
     * @param nivel El nivel de volumen. El rango válido es [0..9].
     */
    void EstablecerVolumen(int nivel) override;
    /**
     * @brief Establece el nivel de volumen en uno de los canales stereo.
     * @param nivel El nivel de volumen. El rango válido es [0..9].
     * @param salida El canal al cual asignar el volumen. Puede ser 0 ó 1.
     */
    void EstablecerVolumen(int nivel, int salida) override;
    // Habilita la funcionalidad Bluetooth.
    void ActivarControladorAuxiliar() override;
    // Deshabilita la funcionalidad Bluetooth.
    void DesactivarControladorAuxiliar() override;
    // Retorna "true" si el Bluetooth está habilitado, y "false" en caso contrario.
    bool ControladorAuxiliarActivado() override;
};


