#include "grupos_led.h"
#include "config.h"

namespace {

struct DefinicaoGrupo {
  const uint8_t* pinos;  // MAX_PINOS_POR_GRUPO posições; SEM_PINO nas que sobram
  const char* nome;
};

// Mesma ordem do enum GrupoLed.
const DefinicaoGrupo GRUPOS[] = {
  { PINOS_OLHOS_ANEL,           "OLHOS_ANEL" },
  { PINOS_OLHOS_MEIO_LATERAL,   "OLHOS_MEIO_LATERAL" },
  { PINOS_OLHOS_MEIO_CENTRO,    "OLHOS_MEIO_CENTRO" },
  { PINOS_SOBRANCELHAS_BRAVAS,  "SOBRANCELHAS_BRAVAS" },
  { PINOS_SOBRANCELHAS_TRISTES, "SOBRANCELHAS_TRISTES" },
  { PINOS_BOCA_RETA,            "BOCA_RETA" },
  { PINOS_BOCA_SORRISO,         "BOCA_SORRISO" },
  { PINOS_BOCA_TRISTE,          "BOCA_TRISTE" },
};

constexpr uint8_t TOTAL_GRUPOS = uint8_t(GrupoLed::QUANTIDADE);

static_assert(sizeof(GRUPOS) / sizeof(GRUPOS[0]) == TOTAL_GRUPOS,
              "GRUPOS precisa de uma linha para cada valor de GrupoLed");
static_assert(TOTAL_GRUPOS <= sizeof(MascaraLeds) * 8,
              "MascaraLeds não tem bits para todos os grupos");

constexpr uint8_t NIVEL_LED_APAGADO = (NIVEL_LED_ACESO == HIGH) ? LOW : HIGH;

MascaraLeds estadoAtual = 0;

void escrever(uint8_t indice, bool aceso) {
  for (uint8_t i = 0; i < MAX_PINOS_POR_GRUPO; i++) {
    const uint8_t pino = GRUPOS[indice].pinos[i];
    if (pino != SEM_PINO) {
      digitalWrite(pino, aceso ? NIVEL_LED_ACESO : NIVEL_LED_APAGADO);
    }
  }
}

}  // namespace

void gruposIniciar() {
  for (uint8_t g = 0; g < TOTAL_GRUPOS; g++) {
    for (uint8_t i = 0; i < MAX_PINOS_POR_GRUPO; i++) {
      if (GRUPOS[g].pinos[i] != SEM_PINO) {
        pinMode(GRUPOS[g].pinos[i], OUTPUT);
      }
    }
    escrever(g, false);
  }
  estadoAtual = 0;
}

void gruposAplicar(MascaraLeds acesos) {
  const MascaraLeds mudaram = acesos ^ estadoAtual;
  for (uint8_t i = 0; i < TOTAL_GRUPOS; i++) {
    const MascaraLeds bit = MascaraLeds(1u << i);
    if (mudaram & bit) {
      escrever(i, acesos & bit);
    }
  }
  estadoAtual = acesos;
}

const char* grupoNome(GrupoLed grupo) {
  return GRUPOS[uint8_t(grupo)].nome;
}
