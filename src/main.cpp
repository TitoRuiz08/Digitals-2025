#include <Arduino.h>

#define bot1 ((PIND >> PD2) & 1) // macros para pulsadores
#define time_40ms 40             // tiempo de comparación para el antirrebote
#define time_3s 3000             // tiempo de comparación para cuando se mantiene pulsado 3 seg
#define time_300ms 300           // tiempo que tarda en hacer la caso_cuenta modo RÁPIDO 300ms
#define time_1s 1000             // tiempo que tarda en contar 1

#define set_bit(sfr, bit) sfr |= (1 << bit)
#define reset_bit(sfr, bit) sfr &= ~(1 << bit)

uint32_t tiempo = 0;

typedef enum
{
  BOT_SUELTO,
  BOT_REBOTANDO_ABAJO,
  BOT_BAJANDO,
  BOT_PULSADO,
  BOT_STILL_PULSADO,
  BOT_SUBIENDO
} estado_bot_t;

typedef enum
{
  ASCENDENTE,
  DESCENDENTE
} direccion_t;

estado_bot_t antirrebote(void); // función para dejar la MEF del antirrebotes adentro+

uint8_t cuenta(uint8_t limite);

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
void conf_timer0(void) // función para configurar el timer0
{
  TCCR0A = (1 << WGM01);              // WGM01 = 1, WGM00 = 0 para modo CTC
  TCCR0B = (1 << CS01) | (1 << CS00); // CS02:0 = 011 para prescaler 64
  OCR0A = 249;                        // 249 porque el contador inicia en 0 (0 a 249 = 250 ticks)
  TIMSK0 |= (1 << OCIE0A);
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
void conf_puertos(void)
{
  DDRB |= ((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3));
  DDRD &= ~(1 << PD2); // entrada (pulsador)
  PORTD |= (1 << PD2); // pull up del pulsador
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

int main(void)
{
  conf_timer0();
  conf_puertos();

  sei();
  estado_bot_t boton;

  while (1)
  {
    asm("nop");

    boton = antirrebote();
    if (boton == BOT_BAJANDO)
    {
      // PORTB = 0x00;

      PORTB = cuenta(15) & 0x0F; // enmascaro el valor de caso_cuenta cada vez que cambia
                                   // con los pines que quiero ver de salida para que se haga en orden.
    }
    // else if (boton == BOT_SUELTO)
    // {
    //   PORTB &= ~(1 << PB1);
    // }
    // else if (boton == BOT_STILL_PULSADO)
    // {
    //   PORTB |= (1 << PB2);
    // }
  }
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
ISR(TIMER0_COMPA_vect)
{

  tiempo++;
}

uint8_t cuenta(uint8_t limite)
{
  static uint8_t cuenta=0;
  static direccion_t direccion_cuenta = ASCENDENTE;

  if (cuenta >= limite) // si contó hasta 15
  {
    direccion_cuenta = DESCENDENTE; // le cambio el estado a direccion_cuenta a DESCENDENTE para que cuente de esa forma
  }
  else if (cuenta == 0) // si contó hasta 0
  {
    direccion_cuenta = ASCENDENTE; // hago que cuente de manera ascendente
  }

  
  
  if (direccion_cuenta == ASCENDENTE)
  { // si el estado de direccion_cuenta es ASCENDENTE
    cuenta++; // aumento el valor de caso_cuenta para que llegue hasta 15
  }

  if (direccion_cuenta == DESCENDENTE) // si direccion_cuenta está en DESCENDENTE
  {
    cuenta--; // decremento caso_cuenta para que cuente hasta 0
  }

  return cuenta;
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

estado_bot_t antirrebote(void)
{
  static estado_bot_t estado_bot1 = BOT_SUELTO;
  static uint32_t timestamp;

  switch (estado_bot1)
  {
  //-------------------------------------------------------------------
  //-------------------------------------------------------------------
  //-------------------------------------------------------------------
  case BOT_SUELTO: // suelto
    if (bot1 == 0) // si pulso, cambio el estado
    {
      timestamp = tiempo + time_40ms;
      estado_bot1 = BOT_REBOTANDO_ABAJO;
    }
    break;
  //-------------------------------------------------------------------
  //-------------------------------------------------------------------
  //-------------------------------------------------------------------
  case BOT_REBOTANDO_ABAJO:


    if (bot1 != 0)
    {
      estado_bot1 = BOT_SUELTO;
    }
    if (bot1 == 0 && tiempo >= timestamp)
    {
      estado_bot1 = BOT_BAJANDO;
      timestamp = tiempo + time_3s;
    }
    break;
  //-------------------------------------------------------------------
  //-------------------------------------------------------------------
  //-------------------------------------------------------------------
  case BOT_BAJANDO: // bajando (detección de pulso bajo)

      estado_bot1 = BOT_PULSADO;

    break;

    //-------------------------------------------------------------------
    //-------------------------------------------------------------------
    //-------------------------------------------------------------------
  case BOT_PULSADO: // pulsado

    if (bot1 == 0 && tiempo > timestamp) // si pulso por 3 segundos, cambio de estado a que sigue pulsado
    {
      estado_bot1 = BOT_STILL_PULSADO;
    }
    else if (bot1 == 1) // sino pasa a subiendo
    {
      estado_bot1 = BOT_SUBIENDO;
    }
    break;
    //-------------------------------------------------------------------
    //-------------------------------------------------------------------
    //-------------------------------------------------------------------
  case BOT_STILL_PULSADO: // sigue abajo
    if (bot1 != 0)        // si sigue abajo se acelera la caso_cuenta y se hace sola hasta que suelte
    {
      estado_bot1 = BOT_PULSADO; // si suelto, vuelve al estado de pulsado
    }
    //-------------------------------------------------------------------
    //-------------------------------------------------------------------
    //-------------------------------------------------------------------
  case BOT_SUBIENDO: // subiendo (detección de pulso alto)
    if (bot1 == 1)   // si detectó el pulso alto, pasa a suelto
    {
      estado_bot1 = BOT_SUELTO;
      timestamp = tiempo + time_40ms;
    }
    else // si no, vuelve a pulsado, detectó un falso pulso alto (rebote)
    {
      estado_bot1 = BOT_PULSADO;
    }
    break;
    //-------------------------------------------------------------------
    //-------------------------------------------------------------------
    //-------------------------------------------------------------------

  default:
    break;
  }

  return estado_bot1;
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

/*

#include <Arduino.h>
#define bot1 ((PIND >> PD2) & 1) // macros para pulsadores
#define time_40ms 40             // tiempo de comparación para el antirrebote
#define time_3s 3000             // tiempo de comparación para cuando se mantiene pulsado 3 seg
#define time_300ms 300           // tiempo que tarda en hacer la caso_cuenta modo RÁPIDO 300ms
#define time_1s 1000             // tiempo que tarda en contar 1

void conf_timer0(void);  // función para configuración del timer0
void conf_puertos(void); // entradas y salidas
void antirrebote(void);  // función para dejar la MEF del antirrebotes adentro
uint8_t cuenta();
uint8_t cuenta(uint8_t limite);

typedef enum
{
  BOT_SUELTO,
  BOT_BAJANDO,
  BOT_PULSADO,
  BOT_STILL_PULSADO,
  BOT_SUBIENDO
} estado_bot_t;
creo una MEF para el antirrebotes de un botón
// BOT_SUELTO (botón no presionado)
// BOT_BAJANDO (detección de pulso bajo)
// BOT_PULSADO (botón presionado)
// BOT_STILL_PULSADO (si se mantiene pulsado por 3 segundos acelera la caso_cuenta)
// BOT_SUBIENDO (detección de pulso alto)

typedef enum
{
  ASCENDENTE,
  DESCENDENTE
} direccion_t;
// MEF para la dirección de la caso_cuenta
// ASCENDENTE (caso_cuenta para arriba)
// DESCENDENTE (caso_cuenta para abajo)

estado_bot_t estado_bot1 = BOT_SUELTO;
direccion_t direccion_cuenta = ASCENDENTE;

volatile uint64_t contador1, contador2, time_bounce, tiempo_cuenta, time_pulsado;
// contadores para antirrebote, para la caso_cuenta y para el tiempo que se mantiene pulsado el bot1

uint8_t caso_cuenta = 0;
// caso_cuenta es una variable que hace la caso_cuenta hasta 1111
// (15 en decimal)

int main()
{
  while (1)
  {
    asm("nop");
    antirrebote();

    if (estado_bot1 == BOT_BAJANDO) // si pulso y suelto
    {
      PORTB = 0x00; // para que no se sume el valor nuevo con el anterior le mando un 0 al puerto B
                    // antes de que se escriba el dato nuevo
                    // se puede hacer de esa manera o así "PORTB &=~ ((1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3))"

      PORTB = (cuenta(15) & 0xF0); // enmascaro el valor de caso_cuenta cada vez que cambia
                                   // con los pines que quiero ver de salida para que se haga en orden.
    }
    else if (estado_bot1 == BOT_STILL_PULSADO) // si mantengo pulsado
    {

      if (tiempo_cuenta >= time_300ms) // y si pasaron 300ms
      {
        PORTB = 0x00; // para que no se sume el valor nuevo con el anterior le mando un 0 al puerto B
                      // se puede hacer de esa manera o así "PORTB &=~ ((1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3))"

        PORTB |= (caso_cuenta & 0x0F); // enmascaro el valor de caso_cuenta cada vez que cambia
        PORTB |= (cuenta(15) & 0x0F);  // enmascaro el valor de caso_cuenta cada vez que cambia
        // con los pines que quiero ver de salida para que se haga en orden.
      }
    }
  }
}

ISR(TIMER0_COMPA_vect)
{
  time_bounce++;   // contador para comparar el tiempo del antirrebote
  tiempo_cuenta++; // contador que caso_cuenta cuanto tarda en hacer la caso_cuenta cuando mantengo pulsado
  time_pulsado++;  // contador para comparar si pulsé durante 3 segundos
}

uint8_t cuenta(uint8_t limite)
{

  if (caso_cuenta == limite) // si contó hasta 15
  {
    direccion_cuenta = DESCENDENTE; // le cambio el estado a direccion_cuenta a DESCENDENTE para que cuente de esa forma
  }
  else if (caso_cuenta == 0) // si contó hasta 0
  {
    direccion_cuenta = ASCENDENTE; // hago que cuente de manera ascendente
  }
  if (direccion_cuenta == ASCENDENTE)
  { // si el estado de direccion_cuenta es ASCENDENTE
    // caso_cuenta de esta forma.

    tiempo_cuenta = 0; // si no reinicio esto, la caso_cuenta no se ve correctamente
    // se prende y apaga rapidisimo sin esto

    caso_cuenta++; // aumento el valor de caso_cuenta para que llegue hasta 15
  }

  if (direccion_cuenta == DESCENDENTE) // si direccion_cuenta está en DESCENDENTE
  {
    tiempo_cuenta = 0; // igual que antes
    caso_cuenta--;     // decremento caso_cuenta para que cuente hasta 0
  }

  return 0;
}

void antirrebote(void)
{
  switch (estado_bot1)
  {
  case BOT_SUELTO:                                        // suelto
    if (bot1 == 0 && (contador2 - contador1) > time_40ms) // si pulso, cambio el estado
    {
      estado_bot1 = BOT_BAJANDO;
      contador1 = contador2; //"reseteo" el delta para que cuente hasta 40ms otra vez en este caso
    }
    break;
  case BOT_BAJANDO: // bajando (detección de pulso bajo)
    if (bot1 == 0)  // si detectó pulso bajo y pasaron 40ms, cambio el estado a pulsado
    {
    //   llamo a una función caso_cuenta por ejemplo, lo que hace es devolverme un valor que valga de 0 a 15
    //   ejemplo: uint8_T caso_cuenta();
    //  la salida de caso_cuenta, PORTB = (caso_cuenta() & 0x0F);


      if (bot1 == 0) // si detectó pulso bajo, cambio el estado a pulsado
      {
        estado_bot1 = BOT_PULSADO;
      }
      else // sino vuelve a suelto, porque fue un falso rebote
      {
        estado_bot1 = BOT_SUELTO;
      }
      break;
    case BOT_PULSADO:                          // pulsado
      if (bot1 == 0 && time_pulsado > time_3s) // si pulso por 3 segundos, cambio de estado a que sigue pulsado
      {

        estado_bot1 = BOT_STILL_PULSADO;
      }
      else if (bot1 == 1) // sino pasa a subiendo
      {
        estado_bot1 = BOT_SUBIENDO;
      }
      break;
    case BOT_STILL_PULSADO: // sigue abajo
      if (bot1 == 0)        // si sigue abajo se acelera la caso_cuenta y se hace sola hasta que suelte
      {
        estado_bot1 = BOT_STILL_PULSADO;
      }
      else
      {
        estado_bot1 = BOT_PULSADO; // si suelto, vuelve al estado de pulsado
      }
    case BOT_SUBIENDO:                                      // subiendo (detección de pulso alto)
      if (bot1 == 1 && (contador2 - contador1) > time_40ms) // si detectó el pulso alto, pasa a suelto
      {
        estado_bot1 = BOT_SUELTO;
        contador1 = contador2; //"reseteo" el delta para que cuente hasta 40ms otra vez en este caso
        time_pulsado = 0;      // reseteo esta variable para que cada vez que mantenga pulsado, cuente 3 segundos de nuevo
      }
      else // si no, vuelve a pulsado, detectó un falso pulso alto (rebote)
      {
        estado_bot1 = BOT_PULSADO;
      }
      break;

    default:
      estado_bot1 = BOT_SUELTO;
      break;
    }
    contador2 = time_bounce; // hago valer contador2 lo que valga el tiempo del antirrebote para poder hacer la comparación y que de 0
  }
}


*/