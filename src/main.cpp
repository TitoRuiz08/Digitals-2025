#include <Arduino.h>
#define BOT1 ((PINB >> PB0) & 1)
#define BOT2 ((PINB >> PB1) & 1)

void IN_OUT(void);

int main()
{
  int16_t dato;
  uint8_t bit_dato1, bit_dato2;
  volatile uint8_t envio_dato1, envio_dato2;
  init();
  IN_OUT(); // función in_out

  Serial.begin(38400);
  PORTD |= (1 << PB3);

  while (1)
  {
    // entro entradas y salidas
    bit_dato1 = BOT1; // el bit1 vale el estado del bot1
    bit_dato2 = BOT2; // el bit2 vale el estado del bot2

    if (bit_dato1 == 0) // si bot1
    {
      envio_dato1 = 0;
    }
    else
    {
      envio_dato1 = 1;
    }

    if (bit_dato2 == 0)
    {
      envio_dato2 = 0;
    }
    else
    {
      envio_dato2 = 1;
    }

    if (Serial.available())// gestión de comunicación serie
    {
      dato = Serial.read();
      if (dato == '&')
      {
        Serial.write(envio_dato1);
        Serial.write(envio_dato2);
      }
      if (dato == '%')
      {
        PORTB ^= (1 << PB3); // invierto el estado de salida PB0 que es la led
      }
    }
  }
}

void IN_OUT(void)
{
  DDRB = (1 << PB3);
  DDRB &= ~((1 << PB0) | (1 << PB1));
  PORTB |= ((1 << PB1) | (1 << PB0));
}
