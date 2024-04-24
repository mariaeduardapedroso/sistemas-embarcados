#include <LiquidCrystal.h>

// Definição dos pinos para o display LCD
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Definição dos pinos para o joystick e botão
const int joyXPin = A0;
const int joyYPin = A1;
const int joyButtonPin = 7;

// Definição do pino para o buzzer
const int buzzerPin = 8;

// Variáveis para armazenar os horários do alarme e atual
int alarmHour = 0;
int alarmMinute = 0;
int currentHour = 0;
int currentMinute = 0;

// Estados para o botão
enum ButtonState {
  Idle,
  Pressed,
  Released
};

// Estados para o joystick
enum JoystickState {
  Neutral,
  Up,
  Down,
  Left,
  Right
};

// Máquina de estado para o botão
class Button {
private:
  int pin;
  ButtonState state;
  unsigned long lastDebounceTime;
  unsigned long debounceDelay;

public:
  Button(int pin) : pin(pin), state(Idle), lastDebounceTime(0), debounceDelay(50) {}

  void update() {
    int buttonState = digitalRead(pin);

    switch (state) {
      case Idle:
        if (buttonState == HIGH) {
          state = Pressed;
          lastDebounceTime = millis();
        }
        break;

      case Pressed:
        if (buttonState == LOW && millis() - lastDebounceTime > debounceDelay) {
          state = Released;
        }
        break;

      case Released:
        if (buttonState == LOW) {
          state = Idle;
        }
        break;
    }
  }

  ButtonState getState() {
    return state;
  }
};

// Máquina de estado para o joystick
class Joystick {
private:
  int xPin;
  int yPin;

public:
  Joystick(int xPin, int yPin) : xPin(xPin), yPin(yPin) {}

  JoystickState getState() {
    int xValue = analogRead(xPin);
    int yValue = analogRead(yPin);

    if (xValue < 100) {
      return Left;
    } else if (xValue > 900) {
      return Right;
    } else if (yValue < 100) {
      return Up;
    } else if (yValue > 900) {
      return Down;
    } else {
      return Neutral;
    }
  }
};

// Máquina de estado para o alarme
class Alarm {
private:
  int hour;
  int minute;

public:
  Alarm(int hour, int minute) : hour(hour), minute(minute) {}

  bool isAlarmTime(int currentHour, int currentMinute) {
    return (hour == currentHour && minute == currentMinute);
  }
};

// Função para acionar o buzzer
void buzz() {
  tone(buzzerPin, 1000); // Frequência do buzzer
  delay(500); // Duração do buzz
  noTone(buzzerPin); // Desliga o buzzer
}

// Função para atualizar o horário atual
void updateCurrentTime() {
  // Aqui você pode implementar a lógica para obter o horário atual
  // Por exemplo, utilizando a biblioteca Time.h ou RTC
  // Por simplicidade, vou apenas incrementar os minutos a cada segundo
  currentMinute++;
  if (currentMinute >= 60) {
    currentMinute = 0;
    currentHour++;
    if (currentHour >= 24) {
      currentHour = 0;
    }
  }
}

// Função para atualizar o display LCD
void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hora: ");
  lcd.print(currentHour);
  lcd.print(":");
  lcd.print(currentMinute);

  lcd.setCursor(0, 1);
  lcd.print("Alarme: ");
  lcd.print(alarmHour);
  lcd.print(":");
  lcd.print(alarmMinute);
}

// Função para ajustar o horário do alarme
void adjustAlarmTime(JoystickState direction) {
  switch (direction) {
    case Up:
      alarmMinute++;
      if (alarmMinute >= 60) {
        alarmMinute = 0;
      }
      break;

    case Down:
      alarmMinute--;
      if (alarmMinute < 0) {
        alarmMinute = 59;
      }
      break;

    case Left:
      alarmHour--;
      if (alarmHour < 0) {
        alarmHour = 23;
      }
      break;

    case Right:
      alarmHour++;
      if (alarmHour >= 24) {
        alarmHour = 0;
      }
      break;

    default:
      break;
  }
}

// Função para ajustar o horário atual
void adjustCurrentTime(JoystickState direction) {
  // Implemente aqui a lógica para ajustar o horário atual
}

// Função principal
void setup() {
  // Inicialização do display LCD
  lcd.begin(16, 2);

  // Inicialização do joystick e botão
  pinMode(joyButtonPin, INPUT_PULLUP);

  // Inicialização do buzzer
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  // Atualiza o estado do botão
  static Button button(joyButtonPin);
  button.update();

  // Atualiza o estado do joystick
  static Joystick joystick(joyXPin, joyYPin);
  JoystickState joyState = joystick.getState();

  // Atualiza o horário atual
  updateCurrentTime();

  // Ajusta o horário do alarme se necessário
  if (button.getState() == Pressed) {
    adjustAlarmTime(joyState);
  }

  // Atualiza o display LCD
  updateLCD();

  // Verifica se é hora do alarme e aciona o buzzer
  static Alarm alarm(alarmHour, alarmMinute);
  if (alarm.isAlarmTime(currentHour, currentMinute)) {
    buzz();
  }

  // Implemente outras lógicas de máquinas de estado, se necessário
  // Por exemplo, para ajustar o horário atual com o joystick
}
