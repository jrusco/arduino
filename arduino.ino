#include <Arduino.h>

// Asignación de pines para motores y sensores
const int ACTIVATE_MOTOR_RIGHT = 12 ;
const int MOTOR_RIGHT_FORWARD = 9 ;
const int MOTOR_RIGHT_BACKWARD = 8;
const int ACTIVATE_MOTOR_LEFT = 13 ;
const int MOTOR_LEFT_FORWARD = 10;
const int MOTOR_LEFT_BACKWARD = 11;

const int TRIG_FRONT = 5;
const int ECHO_FRONT = 4;
const int TRIG_LEFT = 3;
const int ECHO_LEFT = 2;
const int TRIG_RIGHT = 7;
const int ECHO_RIGHT = 6;

// Variables para guardar las distancias (en centímetros)
int dist_front = 0;
int dist_left = 0;
int dist_right = 0;

// Lee la distancia de un sensor ultrasónico (en centímetros)
long readUltrasonicDistance(int triggerPin, int echoPin)
{
    pinMode(triggerPin, OUTPUT);
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);
    pinMode(echoPin, INPUT);
    long duration = pulseIn(echoPin, HIGH);
    // Convierte el tiempo en distancia (cm): velocidad del sonido = 343 m/s
    return 0.01723 * duration;
}

// Funciones para controlar los motores
void moveForward()
{
    // Hace que el robot avance
  	digitalWrite(ACTIVATE_MOTOR_RIGHT, HIGH);
    digitalWrite(MOTOR_RIGHT_FORWARD, HIGH);
    digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
 	digitalWrite(ACTIVATE_MOTOR_LEFT, HIGH);
    digitalWrite(MOTOR_LEFT_FORWARD, HIGH);
    digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
}

void stopMotors()
{
    // Detiene todos los motores
  	digitalWrite(ACTIVATE_MOTOR_RIGHT, LOW);
    digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
    digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
  	digitalWrite(ACTIVATE_MOTOR_LEFT, LOW);
    digitalWrite(MOTOR_LEFT_FORWARD, LOW);
    digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
}

void turnLeft()
{
    // Gira a la izquierda: la rueda izquierda va para atrás y la derecha para adelante
    digitalWrite(ACTIVATE_MOTOR_LEFT, HIGH);
  	digitalWrite(MOTOR_LEFT_FORWARD, LOW);
    digitalWrite(MOTOR_LEFT_BACKWARD, HIGH);
   	digitalWrite(ACTIVATE_MOTOR_RIGHT, HIGH);
    digitalWrite(MOTOR_RIGHT_FORWARD, HIGH);
    digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
}

void turnRight()
{
    // Gira a la derecha: la rueda derecha va para atrás y la izquierda para adelante
    digitalWrite(ACTIVATE_MOTOR_LEFT, HIGH);
  	digitalWrite(MOTOR_LEFT_FORWARD, HIGH);
    digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
   	digitalWrite(ACTIVATE_MOTOR_RIGHT, HIGH);
    digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
    digitalWrite(MOTOR_RIGHT_BACKWARD, HIGH);
}

void setup()
{
    // Configura los pines de los motores como salida
    pinMode(ACTIVATE_MOTOR_RIGHT, OUTPUT);
    pinMode(MOTOR_RIGHT_FORWARD, OUTPUT);
    pinMode(MOTOR_RIGHT_BACKWARD, OUTPUT);
    pinMode(ACTIVATE_MOTOR_LEFT, OUTPUT);
    pinMode(MOTOR_LEFT_FORWARD, OUTPUT);
    pinMode(MOTOR_LEFT_BACKWARD, OUTPUT);

    Serial.begin(9600);
}

void loop()
{
    // Mide las distancias con los sensores ultrasónicos
    dist_front = readUltrasonicDistance(TRIG_FRONT, ECHO_FRONT);
    dist_left = readUltrasonicDistance(TRIG_LEFT, ECHO_LEFT);
    dist_right = readUltrasonicDistance(TRIG_RIGHT, ECHO_RIGHT);

    // Muestra las distancias por el monitor serie al usuario
    Serial.print("Frente: ");
    Serial.print(dist_front);
    Serial.print(" cm, Izquierda: ");
    Serial.print(dist_left);
    Serial.print(" cm, Derecha: ");
    Serial.println(dist_right);

    // Lógica para evitar obstáculos
    if (dist_front > 45)
    {
        // Si el camino de adelante está libre, avanza
        moveForward();
    }
    else
    {
        // Si hay un obstáculo adelante, revisa los costados
        stopMotors();
        delay(100); // Hace una pausa corta antes de girar

        if (dist_left > 45)
        {
            // Si la izquierda está libre, gira a la izquierda
            turnLeft();
            delay(400); // Ajustá este valor para cambiar el ángulo de giro
            stopMotors();
        }
        else if (dist_right > 45)
        {
            // Si la derecha está libre, gira a la derecha
            turnRight();
            delay(400); // Ajustá este valor para cambiar el ángulo de giro
            stopMotors();
        }
        else
        {
            // Si todos los lados están bloqueados, se detiene
            stopMotors();
        }
    }

    delay(100); // Pequeña pausa antes de repetir el ciclo
}
