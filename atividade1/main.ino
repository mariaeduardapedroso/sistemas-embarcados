#define TIMER_INTERRUPT_DEBUG 0
#define _TIMERINTERRUPT_LOGLEVEL_ 0
#define USE_TIMER_1 true

#include "TimerInterrupt.h"

#if !defined(LED_BUILTIN)
#define LED_BUILTIN 13
#endif

#if !defined(OUT_PORT)
#define OUT_PORT 7
#endif

#if USE_TIMER_1

static bool messageReady = false;
#define HEADER "10101"
#define TRAILER "10101"
#define TAMHEADER 4
#define TAMTRAILER 4
volatile int messageHeader = 0;
volatile int messageTrailer = 0;
volatile int message = 0;

void TimerHandler1(unsigned int outputPin = OUT_PORT) {
  static bool toggle1 = false;

#if (TIMER_INTERRUPT_DEBUG > 1)
  Serial.print("ITimer1 called, millis() = ");
  Serial.println(millis());
#endif

  //timer interrupt toggles pin LED_BUILTIN
  if (messageReady) {
    if (messageHeader <= TAMHEADER) {
      Serial.print("\nHEADER ");
      Serial.println(HEADER[messageHeader]);
      if (HEADER[messageHeader] == '1') {
        toggle1 = true;
      }
      else {
        toggle1 = false;
      }

      messageHeader++;
    } else {
      if (message <= 0) {

        message++;
      } else {
        if (messageTrailer <= TAMTRAILER) {
          Serial.print("\nTRAILER ");
          Serial.println(TRAILER[messageTrailer]);
          if (TRAILER[messageTrailer] == '1') {
            toggle1 = true;
          }
          else {
            toggle1 = false;
          }

          messageTrailer++;
        } else {
          //sendBits(incomingMessage);
          messageReady = false;
          messageHeader = 0;
          messageTrailer = 0;
        }
      }
    }
  } else {
    toggle1 = false;
  }

  digitalWrite(outputPin, toggle1);
  Serial.print("output menssage ");
  Serial.println(toggle1);
  //Serial.print("ready menssage ");
  //Serial.println(messageReady);
}

#endif


unsigned int outputPin1 = LED_BUILTIN;
unsigned int sendMessagePin = OUT_PORT;
unsigned int outputPin = A0;

#define USING_LOOP_TEST false

#define TIMER1_INTERVAL_MS 1000
#define TIMER1_FREQUENCY (float)(1000.0f / TIMER1_INTERVAL_MS)

#define TIMER_INTERVAL_MS 2000
#define TIMER_FREQUENCY (float)(1000.0f / TIMER_INTERVAL_MS)


#if USING_LOOP_TEST
#define TIMER1_DURATION_MS (10UL * TIMER1_INTERVAL_MS)
#define TIMER_DURATION_MS (20UL * TIMER_INTERVAL_MS)
#else
#define TIMER1_DURATION_MS 0
#define TIMER_DURATION_MS 0
#endif


#define SERIAL_BAUD_RATE 9600
#define MESSAGE_SIZE 20

char incomingMessage[MESSAGE_SIZE + 1];  // +1 for null terminator
char bitsIncomingMessage[MESSAGE_SIZE * 8 + 8];

volatile int messageIndex = 0;
volatile int messageIndexBits = 0;

void charToBits(char character) {
  Serial.print("\nEm bits");
  for (int i = 7; i >= 0; i--) { // Itera sobre cada bit do caractere, começando pelo bit mais significativo
    Serial.print((character >> i) & 1); // Shifta o caractere para a direita por i bits e então aplica uma máscara para obter o bit na posição i
    bitsIncomingMessage[messageIndexBits++] = (character >> i) & 1;
  }
}
void setup() {
  pinMode(outputPin1, OUTPUT);
  pinMode(outputPin, OUTPUT);
  pinMode(sendMessagePin, OUTPUT);

  Serial.begin(9600);
  while (!Serial)
    ;

  Serial.print(F("\nStarting TimerInterruptTest on "));
  Serial.println(BOARD_TYPE);
  Serial.println(TIMER_INTERRUPT_VERSION);
  Serial.print(F("CPU Frequency = "));
  Serial.print(F_CPU / 1000000);
  Serial.println(F(" MHz"));

#if USE_TIMER_1
  ITimer1.init();

  if (ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS, TimerHandler1, sendMessagePin, TIMER1_DURATION_MS)) {
    Serial.print(F("Starting  ITimer1 OK, millis() = "));
    Serial.println(millis());
  } else
    Serial.println(F("Can't set ITimer1. Select another freq. or timer"));

#endif
}

void loop() {
  // Verifica se há dados disponíveis na porta serial
  if (!messageReady) {
    while (Serial.available() > 0) {
      char incomingChar = Serial.read(); // Lê o próximo caractere disponível
      Serial.print("\nCaractere recebido: ");
      Serial.print(incomingChar);
      if (messageIndex < MESSAGE_SIZE) {
        incomingMessage[messageIndex++] = incomingChar;
        charToBits(incomingChar);
      }
      // Se encontrar um caractere de nova linha, a mensagem está pronta
      if (incomingChar == '\n') {
        incomingMessage[messageIndex] = '\0'; // Adiciona o terminador de string
        charToBits('\0');
        messageIndex = 0; // Reseta o índice para a próxima mensagem
        messageIndexBits = 0; // Reseta o índice para a próxima mensagem
        messageReady = true; // Indica que uma nova mensagem está pronta
        break; // Sai do loop de leitura
      }
    }
  }
}
