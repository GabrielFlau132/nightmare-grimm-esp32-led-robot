// Nightmare Grimm — rosto de LEDs controlado por ESP32.
//
// Organização:
//   config.h          pinos, tempos e Wi-Fi (o que normalmente se ajusta)
//   grupos_led.*      grupos de LEDs e escrita nos pinos
//   expressoes.*      tabela das cinco expressões
//   rosto.*           expressão ativa + olhos, traduzidos em grupos de LEDs
//   piscada.*         piscada automática não bloqueante (PISCADA_ATIVA)
//   comandos.*        serial, modo demonstração e teste de montagem
//   controle_wifi.*   página no celular para escolher a expressão
//   musica.*          Troupe Master Grimm nos 5 buzzers
//
// Montagem e ligação elétrica: README.md na raiz do repositório.

#include "config.h"
#include "comandos.h"
#include "controle_wifi.h"
#include "grupos_led.h"
#include "musica.h"
#include "piscada.h"
#include "rosto.h"

// Motivo do último reinício, para diagnosticar quedas de energia e travamentos.
const char* motivoReinicio() {
  switch (esp_reset_reason()) {
    case ESP_RST_POWERON:  return "ligou na energia (ou botao EN)";
    case ESP_RST_SW:       return "reinicio pelo software (ex.: depois de gravar)";
    case ESP_RST_PANIC:    return "TRAVOU (erro no codigo)";
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:      return "TRAVOU (watchdog)";
    case ESP_RST_BROWNOUT: return "QUEDA DE TENSAO (fonte, bateria ou cabo fraco)";
    default:               return "outro";
  }
}

void setup() {
  Serial.begin(SERIAL_VELOCIDADE);
  Serial.printf("\nReiniciou: %s\n", motivoReinicio());

  gruposIniciar();
  musicaIniciar();
  rostoIniciar(EXPRESSAO_INICIAL);

  const uint32_t agora = millis();
  piscadaIniciar(agora);
  comandosIniciar(agora);
  controleWifiIniciar();
}

void loop() {
  const uint32_t agora = millis();

  comandosAtualizar(agora);
  controleWifiAtualizar();
  musicaAtualizar(agora);
  if (PISCADA_ATIVA && !comandosTesteAtivo()) {
    piscadaAtualizar(agora);
  }
}
