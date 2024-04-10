#define TIMER_INTERRUPT_DEBUG 0
#define _TIMERINTERRUPT_LOGLEVEL_ 0
#define USE_TIMER_1 true
#define HEADER "10101"
#define TRAILER "10101"
#define TAMHEADER 4
#define TAMTRAILER 4
#define MESSAGE_SIZE 20
#define USING_LOOP_TEST false
#define TIMER1_INTERVAL_MS 100
#define TIMER1_FREQUENCY (float)(1000.0f / TIMER1_INTERVAL_MS)
#define SERIAL_BAUD_RATE 9600

#include "TimerInterrupt.h" // Inclui a biblioteca TimerInterrupt.h

#if !defined(LED_BUILTIN) // Define o pino do LED embutido se não estiver definido
#define LED_BUILTIN 13
#endif

#if !defined(OUT_PORT) // Define o pino de saída se não estiver definido
#define OUT_PORT 7
#endif

#if USE_TIMER_1 // Se estiver usando o Timer 1, define variáveis e funções relacionadas

static bool messageReady = false; // Indica se uma mensagem está pronta para ser enviada
volatile int messageHeader = 0; // Índice para o cabeçalho da mensagem
volatile int messageTrailer = 0; // Índice para o trailer da mensagem
volatile int message = 0; // Índice para os bits da mensagem
char bitsIncomingMessage[MESSAGE_SIZE * 8 + 8]; // Array para armazenar os bits da mensagem recebida
unsigned int outputPin1 = LED_BUILTIN; // Pino para o LED embutido
unsigned int sendMessagePin = OUT_PORT; // Pino para enviar a mensagem
unsigned int outputPin = A0; // Pino de saída
char incomingMessage[MESSAGE_SIZE + 1]; // Array para armazenar a mensagem recebida
volatile int messageIndex = 0; // Índice para a mensagem
volatile int messageIndexBits = 0; // Índice para os bits da mensagem

// Função de tratamento de interrupção do Timer 1
void TimerHandler1(unsigned int outputPin = OUT_PORT) {
  static bool toggle1 = false; // Variável para alternar entre ligado/desligado

#if (TIMER_INTERRUPT_DEBUG > 1) // Se o debug estiver ativado
  Serial.print("ITimer1 called, millis() = ");
  Serial.println(millis());
#endif

  if (messageReady) { // Se uma mensagem estiver pronta para ser enviada
    if (messageHeader <= TAMHEADER) { // Se ainda não enviou todo o cabeçalho da mensagem
      // Envia o próximo bit do cabeçalho
      if (HEADER[messageHeader] == '1') {
        toggle1 = true;
        Serial.println("\nHEADER 1");
      } else {
        toggle1 = false;
        Serial.println("\nHEADER 0");
      }
      messageHeader++; // Incrementa o índice do cabeçalho
    } else { // Se o cabeçalho já foi enviado
      if (message < (MESSAGE_SIZE * 8)) { // Se ainda não enviou toda a mensagem
        // Envia o próximo bit da mensagem
        if (bitsIncomingMessage[message] == 0) {
          toggle1 = false;
          Serial.println("\nMENSAGEM 0");
        } else {
          toggle1 = true;
           Serial.println("\nMENSAGEM 1");
        }
        message++; // Incrementa o índice da mensagem
      } else { // Se a mensagem foi enviada completamente
        if (messageTrailer <= TAMTRAILER) { // Se ainda não enviou todo o trailer da mensagem
          // Envia o próximo bit do trailer
          if (TRAILER[messageTrailer] == '1') {
            toggle1 = true;
            Serial.println("\TRAILER 1");
          } else {
            toggle1 = false;
            Serial.println("\TRAILER 0");
          }
          messageTrailer++; // Incrementa o índice do trailer
        } else { // Se o trailer já foi enviado completamente
          // Finaliza o envio da mensagem
          messageReady = false;
          toggle1 = false;
          messageHeader = 0;
          messageTrailer = 0;
          message = 0;
        }
      }
    }
  } else { // Se nenhuma mensagem está pronta para ser enviada
    toggle1 = false;
  }

  digitalWrite(outputPin, toggle1); // Define o pino de saída conforme o valor de toggle1
  Serial.print("output menssage ");
  Serial.println(toggle1);
}

#endif


#if USING_LOOP_TEST
#define TIMER1_DURATION_MS (10UL * TIMER1_INTERVAL_MS)
#else
#define TIMER1_DURATION_MS 0
#endif



void charToBits(char character) {
  Serial.print("\nEm bits");
  for (int i = 7; i >= 0; i--) { // Converte um caractere para bits
    Serial.print((character >> i) & 1); // Shifta o caractere e extrai o bit
    bitsIncomingMessage[messageIndexBits++] = (character >> i) & 1; // Armazena o bit na mensagem recebida
  }
}

void setup() {
  pinMode(outputPin1, OUTPUT); // Define o pino do LED embutido como saída
  pinMode(outputPin, OUTPUT); // Define o pino de saída como saída
  pinMode(sendMessagePin, OUTPUT); // Define o pino de envio como saída

  Serial.begin(9600); // Inicializa a comunicação serial
  while (!Serial); // Aguarda a inicialização da comunicação serial

  // Exibe informações sobre o sistema
  Serial.print(F("\nStarting TimerInterruptTest on "));
  Serial.println(BOARD_TYPE);
  Serial.println(TIMER_INTERRUPT_VERSION);
  Serial.print(F("CPU Frequency = "));
  Serial.print(F_CPU / 1000000);
  Serial.println(F(" MHz"));

#if USE_TIMER_1
  ITimer1.init(); // Inicializa o Timer 1

  // Anexa a interrupção ao Timer 1
  if (ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS, TimerHandler1, sendMessagePin, TIMER1_DURATION_MS)) {
    Serial.print(F("Starting  ITimer1 OK, millis() = "));
    Serial.println(millis());
  } else {
    Serial.println(F("Can't set ITimer1. Select another freq. or timer"));
  }
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
        incomingMessage[messageIndex++] = incomingChar; // Armazena o caractere na mensagem recebida
        charToBits(incomingChar); // Converte o caractere para bits
      }
      // Se encontrar um caractere de nova linha, a mensagem está pronta
      if (incomingChar == '\n') {
        int falta = MESSAGE_SIZE - messageIndex; // Calcula quantos caracteres faltam para completar a mensagem
        Serial.print("\nFALTA");
        Serial.println(falta);
        for (int i = 0; i < falta; i++) {
          incomingMessage[messageIndex++] = ' '; // Completa a mensagem com espaços em branco
          charToBits(' '); // Converte o espaço em branco para bits
        }
        messageIndex = 0; // Reseta o índice para a próxima mensagem
        messageIndexBits = 0; // Reseta o índice para a próxima mensagem
        messageReady = true; // Indica que uma nova mensagem está pronta
        break; // Sai do loop de leitura
      }
    }
  }
}
