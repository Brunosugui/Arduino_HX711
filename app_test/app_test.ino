/*
      Teste de função em C para funcionamento do CI HX711

      Conversor AD para célula de carga

      Modificações por Bruno Suguimoto Iwami para funcionamento seguindo estrutura de máquina de estados

      Adicionada interface serial para dados de calibração


      Eng. Wagner Rambo
      Outubro de 2016
*/

// --- Mapeamento de Hardware ---
#define  ADDO  7    //Data Out
#define  ADSK  6    //SCK

#define FSM_PERIOD_MS           128

#define MAX_SERIAL_STRING_SIZE          16

// --- Protótipo das Funções Auxiliares ---
void fsm_procedure(void);
void keyboard_procedure(void);


// --- Variáveis Globais ---
unsigned long time_now = 0;

// --- Configurações Iniciais ---
void setup()
{
  pinMode(ADDO, INPUT_PULLUP);   //entrada para receber os dados
  pinMode(ADSK, OUTPUT);         //saída para SCK

  Serial.begin(115200);

} //end setup


// --- Loop Infinito ---
void loop()
{
  time_now = millis();
  
  fsm_procedure();
  
  keyboard_procedure();

  //guarantee cycle has fixed cycle time
  if (millis() < time_now + FSM_PERIOD_MS)
    while (millis() < time_now + FSM_PERIOD_MS) delay(1);
  else
    Serial.println("Loop exceed fsm period!!!");
    
} //end loop
