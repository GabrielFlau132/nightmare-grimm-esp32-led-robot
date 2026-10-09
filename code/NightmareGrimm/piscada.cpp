#include "piscada.h"
#include "config.h"
#include "rosto.h"

namespace {

enum class Fase : uint8_t {
  ESPERANDO,     // olhos abertos até a próxima piscada
  FECHADO,       // olhos fechados
  PAUSA_DUPLA,   // olhos abertos por um instante antes da segunda piscada
};

Fase fase = Fase::ESPERANDO;
uint32_t inicioFase = 0;
uint32_t duracaoFase = 0;
bool faltaSegunda = false;  // piscada dupla em andamento

void entrar(Fase nova, uint32_t agora, uint32_t duracao) {
  fase = nova;
  inicioFase = agora;
  duracaoFase = duracao;
}

uint32_t sortearEspera() {
  return random(PISCADA_INTERVALO_MIN_MS, PISCADA_INTERVALO_MAX_MS + 1);
}

void fechar(uint32_t agora) {
  rostoDefinirOlhos(Olhos::FECHADOS);
  entrar(Fase::FECHADO, agora, PISCADA_FECHADO_MS);
}

}  // namespace

void piscadaIniciar(uint32_t agora) {
  rostoDefinirOlhos(Olhos::ABERTOS);
  faltaSegunda = false;
  entrar(Fase::ESPERANDO, agora, sortearEspera());
}

void piscadaAgora(uint32_t agora) {
  if (fase == Fase::ESPERANDO) {
    faltaSegunda = false;
    fechar(agora);
  }
}

void piscadaAtualizar(uint32_t agora) {
  // Subtração sem sinal: continua correta quando o millis() dá a volta.
  if (agora - inicioFase < duracaoFase) {
    return;
  }

  switch (fase) {
    case Fase::ESPERANDO:
      faltaSegunda = random(100) < PISCADA_DUPLA_CHANCE;
      fechar(agora);
      break;

    case Fase::FECHADO:
      rostoDefinirOlhos(Olhos::ABERTOS);
      if (faltaSegunda) {
        faltaSegunda = false;
        entrar(Fase::PAUSA_DUPLA, agora, PISCADA_DUPLA_PAUSA_MS);
      } else {
        entrar(Fase::ESPERANDO, agora, sortearEspera());
      }
      break;

    case Fase::PAUSA_DUPLA:
      fechar(agora);
      break;
  }
}
