#include <TimerInterrupt.h>

// Definições dos pinos de entrada e saída
#define PINO_ENTRADA 2
#define PINO_SAIDA 3

// Definição do intervalo de tempo para as interrupções (em microssegundos)
#define INTERVALO_ENVIO 1000000 // 1 segundo
#define INTERVALO_RECEBIMENTO 500000 // 0.5 segundos

// Classe para gerenciar a comunicação
class Comunicacao {
public:
  Comunicacao() {}

  // Método para inicializar a comunicação
  void iniciar() {
    Serial.begin(9600); // Inicializa a comunicação serial
    pinMode(PINO_ENTRADA, INPUT);
    pinMode(PINO_SAIDA, OUTPUT);
  }

  // Método para enviar informações
  void enviarInformacao(byte info) {
    Serial.write(info); // Envia a informação pela porta serial
  }

  // Método para receber informações
  byte receberInformacao() {
    if (Serial.available() > 0) {
      return Serial.read(); // Lê a informação disponível na porta serial
    }
    return 0; // Retorna 0 se não houver informação disponível
  }

  // Método para processar o envio de informações
  void processarEnvio() {
    // Aqui você pode implementar o código para enviar informações
    // Este método será chamado pela interrupção do timer de envio
  }

  // Método para processar o recebimento de informações
  void processarRecebimento() {
    // Aqui você pode implementar o código para receber informações
    // Este método será chamado pela interrupção do timer de recebimento
  }
};

// Instância da classe de comunicação
Comunicacao comunicacao;

// Função de callback para a interrupção do timer de envio
void envioCallback() {
  comunicacao.processarEnvio();
}

// Função de callback para a interrupção do timer de recebimento
void recebimentoCallback() {
  comunicacao.processarRecebimento();
}

void setup() {
  comunicacao.iniciar(); // Inicializa a comunicação

  // Configuração dos timers de envio e recebimento
  Timer3.initialize(INTERVALO_ENVIO);
  Timer3.attachInterrupt(envioCallback);

  Timer4.initialize(INTERVALO_RECEBIMENTO);
  Timer4.attachInterrupt(recebimentoCallback);
}

void loop() {
  // Exemplo de envio de informação
  comunicacao.enviarInformacao(42);

  // Exemplo de recebimento de informação
  byte infoRecebida = comunicacao.receberInformacao();
  if (infoRecebida != 0) {
    // Faça algo com a informação recebida
  }

  // Outras tarefas podem ser realizadas aqui
}
