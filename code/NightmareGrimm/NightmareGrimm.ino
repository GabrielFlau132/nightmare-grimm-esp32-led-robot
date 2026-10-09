// Nightmare Grimm — rosto de LEDs controlado por ESP32.
//
// Organização:
//   config.h        pinos e tempos (o que normalmente se ajusta)
//   grupos_led.*    grupos de LEDs e escrita nos pinos
//   expressoes.*    tabela das cinco expressões
//   rosto.*         expressão ativa + olhos, traduzidos em grupos de LEDs
//   piscada.*       piscada automática não bloqueante
//   comandos.*      serial, modo demonstração e teste de montagem
//
// Montagem e ligação elétrica: README.md na raiz do repositório.

#include "config.h"
#include "comandos.h"
#include "grupos_led.h"
#include "piscada.h"
#include "rosto.h"

void setup() {
  Serial.begin(SERIAL_VELOCIDADE);

  gruposIniciar();
  rostoIniciar(EXPRESSAO_INICIAL);

  const uint32_t agora = millis();
  piscadaIniciar(agora);
  comandosIniciar(agora);
}

void loop() {
  const uint32_t agora = millis();

  comandosAtualizar(agora);
  if (!comandosTesteAtivo()) {
    piscadaAtualizar(agora);
  }
}
