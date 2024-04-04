#include <TimerInterrupt.h>

#define SERIAL_BAUD_RATE 9600
#define HEADER "10101"
#define TRAILER "10101"
#define MESSAGE_SIZE 20

char incomingMessage[MESSAGE_SIZE + 1]; // +1 for null terminator
bool messageReady = false;
volatile int messageIndex = 0;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  TimerInterrupt_Init();
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

ISR(TIMER3_COMPA_vect) {
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c != '\n' && messageIndex < MESSAGE_SIZE) {
      incomingMessage[messageIndex++] = c;
    } else {
      incomingMessage[messageIndex] = '\0';
      messageIndex = 0;
      messageReady = true;
    }
  }
}
