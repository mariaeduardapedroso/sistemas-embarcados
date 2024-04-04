/****************************************************************************************************************************
  TimerInterruptTest.ino
  For Arduino and Adadruit AVR 328(P) and 32u4 boards
  Written by Khoi Hoang

  Built by Khoi Hoang https://github.com/khoih-prog/TimerInterrupt
  Licensed under MIT license

  Now we can use these new 16 ISR-based timers, while consuming only 1 hardware Timer.
  Their independently-selected, maximum interval is practically unlimited (limited only by unsigned long miliseconds)
  The accuracy is nearly perfect compared to software timers. The most important feature is they're ISR-based timers
  Therefore, their executions are not blocked by bad-behaving functions / tasks.
  This important feature is absolutely necessary for mission-critical tasks.

  Notes:
  Special design is necessary to share data between interrupt code and the rest of your program.
  Variables usually need to be "volatile" types. Volatile tells the compiler to avoid optimizations that assume
  variable can not spontaneously change. Because your function may change variables while your program is using them,
  the compiler needs this hint. But volatile alone is often not enough.
  When accessing shared variables, usually interrupts must be disabled. Even with volatile,
  if the interrupt changes a multi-byte variable between a sequence of instructions, it can be read incorrectly.
  If your data is multiple variables, such as an array and a count, usually interrupts need to be disabled
  or the entire sequence of your code which accesses the data.
 *****************************************************************************************************************************/

// These define's must be placed at the beginning before #include "TimerInterrupt.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG         2
#define _TIMERINTERRUPT_LOGLEVEL_     0
#define USE_TIMER_1     true

#if ( defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)  || \
        defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_MINI) ||    defined(ARDUINO_AVR_ETHERNET) || \
        defined(ARDUINO_AVR_FIO) || defined(ARDUINO_AVR_BT)   || defined(ARDUINO_AVR_LILYPAD) || defined(ARDUINO_AVR_PRO)      || \
        defined(ARDUINO_AVR_NG) || defined(ARDUINO_AVR_UNO_WIFI_DEV_ED) || defined(ARDUINO_AVR_DUEMILANOVE) || defined(ARDUINO_AVR_FEATHER328P) || \
        defined(ARDUINO_AVR_METRO) || defined(ARDUINO_AVR_PROTRINKET5) || defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_AVR_PROTRINKET5FTDI) || \
        defined(ARDUINO_AVR_PROTRINKET3FTDI) )
  #define USE_TIMER_2     true
  #warning Using Timer1, Timer2
#else          
  #define USE_TIMER_3     true
  #warning Using Timer1, Timer3
#endif

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "TimerInterrupt.h"

#if !defined(LED_BUILTIN)
  #define LED_BUILTIN     13
#endif

#if !defined(OUT_PORT)
  #define OUT_PORT     7
#endif

#if USE_TIMER_1

bool messageReady = false;

void TimerHandler1(unsigned int outputPin = OUT_PORT)
{
  static bool toggle1 = false;

#if (TIMER_INTERRUPT_DEBUG > 1)
  Serial.print("ITimer1 called, millis() = "); Serial.println(millis());
#endif

  //timer interrupt toggles pin LED_BUILTIN
  if (messageReady){
    toggle1 = !toggle1;
    // sendBits(incomingMessage);
    // messageReady = false;
  }
  // else{
  //   toggle1 = false;
  // }
  digitalWrite(outputPin, toggle1);
  Serial.print("output menssage "); Serial.println(toggle1);
  
}

#endif


unsigned int outputPin1 = LED_BUILTIN;
unsigned int sendMessagePin = OUT_PORT;
unsigned int outputPin  = A0;

#define USING_LOOP_TEST       false

#define TIMER1_INTERVAL_MS    1000
#define TIMER1_FREQUENCY      (float) (1000.0f / TIMER1_INTERVAL_MS)

#define TIMER_INTERVAL_MS     2000
#define TIMER_FREQUENCY       (float) (1000.0f / TIMER_INTERVAL_MS)


#if USING_LOOP_TEST
  #define TIMER1_DURATION_MS    (10UL * TIMER1_INTERVAL_MS)
  #define TIMER_DURATION_MS     (20UL * TIMER_INTERVAL_MS)
#else
  #define TIMER1_DURATION_MS    0
  #define TIMER_DURATION_MS     0
#endif


#define SERIAL_BAUD_RATE 9600
#define HEADER "10101"
#define TRAILER "10101"
#define MESSAGE_SIZE 20

char incomingMessage[MESSAGE_SIZE + 1]; // +1 for null terminator

volatile int messageIndex = 0;

void setup()
{ 
  pinMode(outputPin1, OUTPUT);
  pinMode(outputPin,  OUTPUT);
  pinMode(sendMessagePin,  OUTPUT);
  
  Serial.begin(115200);
  while (!Serial);

  Serial.print(F("\nStarting TimerInterruptTest on "));
  Serial.println(BOARD_TYPE);
  Serial.println(TIMER_INTERRUPT_VERSION);
  Serial.print(F("CPU Frequency = ")); Serial.print(F_CPU / 1000000); Serial.println(F(" MHz"));

#if USE_TIMER_1

  // Timer0 is used for micros(), millis(), delay(), etc and can't be used
  // Select Timer 1-2 for UNO, 0-5 for MEGA
  // Timer 2 is 8-bit timer, only for higher frequency

  ITimer1.init();

  // Using ATmega328 used in UNO => 16MHz CPU clock ,

  if (ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS, TimerHandler1, sendMessagePin, TIMER1_DURATION_MS))
  {
    Serial.print(F("Starting  ITimer1 OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer1. Select another freq. or timer"));

#endif

}

void loop() {
  if (messageReady) {
    sendBits(incomingMessage);
    messageReady = false;
  }
}

void sendBits(char* message) {
  Serial.print(HEADER);

  for (int i = 0; i < MESSAGE_SIZE; i++) {
    char c = message[i];
    for (int j = 7; j >= 0; j--) {
      Serial.print((c >> j) & 1);
    }
  }

  Serial.print(TRAILER);
}
