#include <Arduino.h>

// Asignación de pines
const int ACTIVAR_MOTOR_DERECHO = 12;
const int MOTOR_DERECHO_ADELANTE = 9;
const int MOTOR_DERECHO_ATRAS = 8;
const int ACTIVAR_MOTOR_IZQUIERDO = 13;
const int MOTOR_IZQUIERDO_ADELANTE = 10;
const int MOTOR_IZQUIERDO_ATRAS = 11;

const int TRIG_FRENTE = 5;
const int ECHO_FRENTE = 4;
const int TRIG_IZQUIERDA = 3;
const int ECHO_IZQUIERDA = 2;
const int TRIG_DERECHA = 7;
const int ECHO_DERECHA = 6;

// Constantes de navegación
const int DISTANCIA_SEGUIR_PARED = 12;    // Distancia óptima para mantener de la pared derecha
const int ANCHO_MINIMO_PASILLO = 8;       // Espacio mínimo necesario para avanzar
const int DISTANCIA_MAXIMA_PARED = 25;    // Distancia máxima para considerar una pared presente
const int UMBRAL_INTERSECCION = 30;       // Distancia que indica una intersección
const int UMBRAL_CALLEJON_SIN_SALIDA = 8; // Distancia que indica un callejón sin salida

// Constantes de tiempo de movimiento
const int RETRASO_GIRO_90 = 450;        // Duración calibrada de giro de 90 grados
const int RETRASO_GIRO_180 = 900;       // Duración calibrada de giro de 180 grados
const int VELOCIDAD_SEGUIR_PARED = 150; // Velocidad PWM para seguir pared
const int VELOCIDAD_NORMAL = 200;       // Velocidad PWM para movimiento normal
const int RETRASO_SENSOR = 50;          // Retraso entre lecturas de sensores

// Definiciones de la máquina de estados
enum EstadoRobot
{
    SIGUIENDO_PARED,
    MANEJANDO_INTERSECCION,
    RECUPERACION_CALLEJON,
    GIRANDO_ESQUINA,
    CENTRANDO_PASILLO,
    BUSCANDO_PARED
};

EstadoRobot estadoActual = BUSCANDO_PARED;
unsigned long tiempoInicioEstado = 0;
unsigned long ultimoTiempoMovimiento = 0;
bool paredALaDerecha = false;
int contadorAtascado = 0;

// Estructura de datos del sensor
struct LecturasSensores
{
    long frente;
    long izquierda;
    long derecha;
    bool frenteValido;
    bool izquierdaValida;
    bool derechaValida;
};

// Obtener lecturas filtradas de sensores
LecturasSensores leerSensores()
{
    LecturasSensores lecturas;

    // Tomar múltiples lecturas y filtrar
    long sumaFrente = 0, sumaIzquierda = 0, sumaDerecha = 0;
    int frenteValido = 0, izquierdaValida = 0, derechaValida = 0;

    for (int i = 0; i < 3; i++)
    {
        // Sensor frontal
        long distFrente = leerDistanciaUltrasonico(TRIG_FRENTE, ECHO_FRENTE);
        if (distFrente > 2 && distFrente < 400)
        {
            sumaFrente += distFrente;
            frenteValido++;
        }

        delay(20);

        // Sensor izquierdo
        long distIzquierda = leerDistanciaUltrasonico(TRIG_IZQUIERDA, ECHO_IZQUIERDA);
        if (distIzquierda > 2 && distIzquierda < 400)
        {
            sumaIzquierda += distIzquierda;
            izquierdaValida++;
        }

        delay(20);

        // Sensor derecho
        long distDerecha = leerDistanciaUltrasonico(TRIG_DERECHA, ECHO_DERECHA);
        if (distDerecha > 2 && distDerecha < 400)
        {
            sumaDerecha += distDerecha;
            derechaValida++;
        }

        delay(20);
    }

    lecturas.frente = frenteValido > 0 ? sumaFrente / frenteValido : 400;
    lecturas.izquierda = izquierdaValida > 0 ? sumaIzquierda / izquierdaValida : 400;
    lecturas.derecha = derechaValida > 0 ? sumaDerecha / derechaValida : 400;

    lecturas.frenteValido = frenteValido >= 2;
    lecturas.izquierdaValida = izquierdaValida >= 2;
    lecturas.derechaValida = derechaValida >= 2;

    return lecturas;
}

// Función de lectura de distancia ultrasónica
long leerDistanciaUltrasonico(int pinTrigger, int pinEcho)
{
    pinMode(pinTrigger, OUTPUT);
    digitalWrite(pinTrigger, LOW);
    delayMicroseconds(2);
    digitalWrite(pinTrigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(pinTrigger, LOW);
    pinMode(pinEcho, INPUT);

    long duracion = pulseIn(pinEcho, HIGH, 30000);
    if (duracion == 0)
        return 400;

    return (duracion * 0.034) / 2;
}

// Funciones de control de motor con control de velocidad
void avanzar(int velocidad = VELOCIDAD_NORMAL)
{
    analogWrite(ACTIVAR_MOTOR_DERECHO, velocidad);
    digitalWrite(MOTOR_DERECHO_ADELANTE, HIGH);
    digitalWrite(MOTOR_DERECHO_ATRAS, LOW);
    analogWrite(ACTIVAR_MOTOR_IZQUIERDO, velocidad);
    digitalWrite(MOTOR_IZQUIERDO_ADELANTE, HIGH);
    digitalWrite(MOTOR_IZQUIERDO_ATRAS, LOW);
}

void girarDerecha()
{
    digitalWrite(ACTIVAR_MOTOR_DERECHO, HIGH);
    digitalWrite(MOTOR_DERECHO_ADELANTE, LOW);
    digitalWrite(MOTOR_DERECHO_ATRAS, HIGH);
    digitalWrite(ACTIVAR_MOTOR_IZQUIERDO, HIGH);
    digitalWrite(MOTOR_IZQUIERDO_ADELANTE, HIGH);
    digitalWrite(MOTOR_IZQUIERDO_ATRAS, LOW);
}

void girarIzquierda()
{
    digitalWrite(ACTIVAR_MOTOR_DERECHO, HIGH);
    digitalWrite(MOTOR_DERECHO_ADELANTE, HIGH);
    digitalWrite(MOTOR_DERECHO_ATRAS, LOW);
    digitalWrite(ACTIVAR_MOTOR_IZQUIERDO, HIGH);
    digitalWrite(MOTOR_IZQUIERDO_ADELANTE, LOW);
    digitalWrite(MOTOR_IZQUIERDO_ATRAS, HIGH);
}

void pararMotores()
{
    digitalWrite(ACTIVAR_MOTOR_DERECHO, LOW);
    digitalWrite(MOTOR_DERECHO_ADELANTE, LOW);
    digitalWrite(MOTOR_DERECHO_ATRAS, LOW);
    digitalWrite(ACTIVAR_MOTOR_IZQUIERDO, LOW);
    digitalWrite(MOTOR_IZQUIERDO_ADELANTE, LOW);
    digitalWrite(MOTOR_IZQUIERDO_ATRAS, LOW);
}

// Funciones de giro calibradas
void girar90Derecha()
{
    girarDerecha();
    delay(RETRASO_GIRO_90);
    pararMotores();
    delay(100);
}

void girar90Izquierda()
{
    girarIzquierda();
    delay(RETRASO_GIRO_90);
    pararMotores();
    delay(100);
}

void girar180()
{
    girarDerecha();
    delay(RETRASO_GIRO_180);
    pararMotores();
    delay(200);
}

// Corrección de distancia de pared
void ajustarDistanciaPared(LecturasSensores &lecturas) {
    if (!lecturas.derechaValida){
        return;
    }

    int errorDistancia = lecturas.derecha - DISTANCIA_SEGUIR_PARED;

    if (abs(errorDistancia) > 3)
    {
        if (errorDistancia > 0)
        {
            // Muy lejos de la pared, girar ligeramente a la derecha
            girarDerecha();
            delay(50);
            pararMotores();
        }
        else
        {
            // Muy cerca de la pared, girar ligeramente a la izquierda
            girarIzquierda();
            delay(50);
            pararMotores();
        }
    }
}

// Implementación de la máquina de estados
void maquinaEstados() {
    LecturasSensores lecturas = leerSensores();

    // Salida de depuración
    Serial.print("Estado: ");
    Serial.print(estadoActual);
    Serial.print(" | F:");
    Serial.print(lecturas.frente);
    Serial.print(" I:");
    Serial.print(lecturas.izquierda);
    Serial.print(" D:");
    Serial.print(lecturas.derecha);
    Serial.print(" | Pared a la derecha: ");
    Serial.println(paredALaDerecha);

    switch (estadoActual){

        case BUSCANDO_PARED:
            // Estado inicial: buscar una pared para seguir
            if (lecturas.derechaValida && lecturas.derecha <= DISTANCIA_MAXIMA_PARED){
                paredALaDerecha = true;
                estadoActual = SIGUIENDO_PARED;
                Serial.println("Pared encontrada - cambiando a seguimiento de pared");
            } else {
                // Girar a la derecha para buscar pared
                girarDerecha();
                delay(100);
                pararMotores();
            }
            break;

        case SIGUIENDO_PARED:
            // Estado principal de navegación usando regla de mano derecha
            if (!lecturas.frenteValido || lecturas.frente <= ANCHO_MINIMO_PASILLO) {
                // Frente bloqueado - necesita girar
                estadoActual = GIRANDO_ESQUINA;
                tiempoInicioEstado = millis();
            } else if (!lecturas.derechaValida || lecturas.derecha > UMBRAL_INTERSECCION) {
                // No hay pared a la derecha - intersección detectada
                estadoActual = MANEJANDO_INTERSECCION;
                tiempoInicioEstado = millis();
            } else {
                // Continuar siguiendo la pared
                ajustarDistanciaPared(lecturas);
                avanzar(VELOCIDAD_SEGUIR_PARED);
                paredALaDerecha = true;
            }
            break;

        case MANEJANDO_INTERSECCION:
            // Manejar intersecciones en T y cruces usando regla de mano derecha
            pararMotores();
            delay(200);

            // Regla de mano derecha: siempre girar a la derecha cuando sea posible
            if (!lecturas.derechaValida || lecturas.derecha > UMBRAL_INTERSECCION) {
                Serial.println("Intersección: Girando a la derecha");
                girar90Derecha();
                estadoActual = SIGUIENDO_PARED;
            } else if (lecturas.frenteValido && lecturas.frente > ANCHO_MINIMO_PASILLO) {
                Serial.println("Intersección: Yendo recto");
                avanzar();
                delay(200);
                estadoActual = SIGUIENDO_PARED;
            } else {
                Serial.println("Intersección: Girando a la izquierda");
                girar90Izquierda();
                estadoActual = SIGUIENDO_PARED;
            }
            break;

        case GIRANDO_ESQUINA:
            // Manejar esquinas y callejones sin salida
            pararMotores();
            delay(200);

            if (lecturas.derechaValida && lecturas.derecha <= DISTANCIA_MAXIMA_PARED) {
                // Pared aún a la derecha, girar a la izquierda alrededor de la esquina
                Serial.println("Esquina: Girando a la izquierda");
                girar90Izquierda();
                estadoActual = SIGUIENDO_PARED;
            } else {
                // No hay pared a la derecha, girar a la derecha (regla de mano derecha)
                Serial.println("Esquina: Girando a la derecha");
                girar90Derecha();
                estadoActual = SIGUIENDO_PARED;
            }
            break;

        case RECUPERACION_CALLEJON:
            // Manejar callejones sin salida completos
            Serial.println("Callejón sin salida detectado - realizando giro en U");
            pararMotores();
            delay(500);
            girar180();
            estadoActual = SIGUIENDO_PARED;
            contadorAtascado = 0;
            break;

        case CENTRANDO_PASILLO:
            // Centrar robot en pasillo cuando hay paredes en ambos lados
            if (lecturas.izquierdaValida && lecturas.derechaValida) {
                int diferencia_izq_der = lecturas.izquierda - lecturas.derecha;
                if (abs(diferencia_izq_der) > 5) {
                    if (diferencia_izq_der > 0) {
                        girarDerecha();
                        delay(30);
                    } else {
                        girarIzquierda();
                        delay(30);
                    }
                    pararMotores();
                }
            }
            avanzar(VELOCIDAD_SEGUIR_PARED);
            estadoActual = SIGUIENDO_PARED;
            break;
    }

    // Detección de atasco
    if (millis() - ultimoTiempoMovimiento > 5000) {
        contadorAtascado++;
        if (contadorAtascado > 3) {
            Serial.println("Robot parece atascado - iniciando recuperación");
            estadoActual = RECUPERACION_CALLEJON;
        }
        ultimoTiempoMovimiento = millis();
    }
}

void setup() {
    // Inicializar pines de motor
    pinMode(ACTIVAR_MOTOR_DERECHO, OUTPUT);
    pinMode(MOTOR_DERECHO_ADELANTE, OUTPUT);
    pinMode(MOTOR_DERECHO_ATRAS, OUTPUT);
    pinMode(ACTIVAR_MOTOR_IZQUIERDO, OUTPUT);
    pinMode(MOTOR_IZQUIERDO_ADELANTE, OUTPUT);
    pinMode(MOTOR_IZQUIERDO_ATRAS, OUTPUT);

    Serial.begin(9600);
    Serial.println("Solucionador Avanzado de Laberintos - Implementación Regla Mano Derecha");
    Serial.println("Buscando pared para seguir...");

    ultimoTiempoMovimiento = millis();
}

void loop() {
    maquinaEstados();
    delay(RETRASO_SENSOR);
}
