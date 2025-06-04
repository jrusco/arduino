#include <Arduino.h>

// C++ code
//
int dist_delante = 0;

int dis_izq = 0;

int dist_der = 0;

long readUltrasonicDistance(int triggerPin, int echoPin)
{
    pinMode(triggerPin, OUTPUT); // Clear the trigger
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    // Sets the trigger pin to HIGH state for 10 microseconds
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);
    pinMode(echoPin, INPUT);
    // Reads the echo pin, and returns the sound wave travel time in microseconds
    return pulseIn(echoPin, HIGH);
}

void setup()
{
    pinMode(12, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(13, OUTPUT);
    pinMode(10, OUTPUT);
    Serial.begin(9600);

    dist_delante = 0;
    dist_der = 0;
    dis_izq = 0;
}

void loop()
{
    // medir distancia delantera
    dist_delante = 0.01723 * readUltrasonicDistance(5, 4);
    // medir distancia izquierda
    dis_izq = 0.01723 * readUltrasonicDistance(3, 2);
    // medir distancia derecha
    dist_der = 0.01723 * readUltrasonicDistance(7, 6);
    // avanzar
    if (dist_delante > 45)
    {
        // avanzar motor derecho
        digitalWrite(12, HIGH);
        digitalWrite(9, HIGH);
        // avanzar motor izquierdo
        digitalWrite(13, HIGH);
        digitalWrite(10, HIGH);
    }
    // mostrar en monitor
    Serial.print("dist delante_");
    Serial.print(dist_delante);
    Serial.print("      dist izq_");
    Serial.print(dis_izq);
    Serial.print("     dist der_");
    Serial.println(dist_der);
    delay(10); // Wait for 10 millisecond(s)
}
