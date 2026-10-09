#include "comandos.h"
#include "config.h"
#include "grupos_led.h"
#include "piscada.h"
#include "rosto.h"

namespace {

constexpr uint8_t TOTAL_EXPRESSOES = uint8_t(Expressao::QUANTIDADE);
constexpr uint8_t TOTAL_GRUPOS = uint8_t(GrupoLed::QUANTIDADE);

bool demoAtivo = DEMO_AO_LIGAR;
uint32_t ultimaTrocaDemo = 0;

// Teste de montagem: acende um grupo por vez, na ordem do enum GrupoLed.
bool testeAtivo = false;
uint8_t testeGrupo = 0;
uint32_t testeInicioGrupo = 0;

void mostrarAjuda() {
  Serial.println();
  Serial.println(F("Nightmare Grimm - comandos:"));
  for (uint8_t i = 0; i < TOTAL_EXPRESSOES; i++) {
    const DefinicaoExpressao& definicao = expressaoDefinicao(Expressao(i));
    Serial.printf("  %u  %-12s %s\n", i + 1, definicao.nome, definicao.simbolo);
  }
  Serial.println(F("  p  piscar agora"));
  Serial.println(F("  d  liga/desliga o modo demonstração"));
  Serial.println(F("  t  teste de montagem (um grupo de LEDs por vez)"));
  Serial.println(F("  ?  esta ajuda"));
}

void mudarExpressao(Expressao expressao) {
  rostoDefinirExpressao(expressao);
  const DefinicaoExpressao& definicao = expressaoDefinicao(expressao);
  Serial.printf("Expressão: %s %s\n", definicao.nome, definicao.simbolo);
}

void acenderGrupoDoTeste(uint32_t agora) {
  const GrupoLed grupo = GrupoLed(testeGrupo);
  gruposAplicar(mascara(grupo));
  testeInicioGrupo = agora;
  Serial.printf("Teste %u/%u: %s\n", testeGrupo + 1, TOTAL_GRUPOS, grupoNome(grupo));
}

void iniciarTeste(uint32_t agora) {
  testeAtivo = true;
  testeGrupo = 0;
  acenderGrupoDoTeste(agora);
}

void pararTeste(uint32_t agora) {
  if (!testeAtivo) {
    return;
  }
  testeAtivo = false;
  rostoReaplicar();
  piscadaIniciar(agora);
  ultimaTrocaDemo = agora;
}

void atualizarTeste(uint32_t agora) {
  if (agora - testeInicioGrupo < TESTE_GRUPO_MS) {
    return;
  }
  testeGrupo++;
  if (testeGrupo < TOTAL_GRUPOS) {
    acenderGrupoDoTeste(agora);
  } else {
    Serial.println(F("Teste concluído."));
    pararTeste(agora);
  }
}

void atualizarDemo(uint32_t agora) {
  if (!demoAtivo || agora - ultimaTrocaDemo < DEMO_INTERVALO_MS) {
    return;
  }
  ultimaTrocaDemo = agora;
  const uint8_t proxima = (uint8_t(rostoExpressao()) + 1) % TOTAL_EXPRESSOES;
  mudarExpressao(Expressao(proxima));
}

void tratarComando(char comando, uint32_t agora) {
  // '1'..'5' escolhem a expressão e desligam a demonstração.
  if (comando >= '1' && comando < char('1' + TOTAL_EXPRESSOES)) {
    pararTeste(agora);
    demoAtivo = false;
    mudarExpressao(Expressao(comando - '1'));
    return;
  }

  switch (comando) {
    case 'p':
    case 'P':
      if (!testeAtivo) {
        piscadaAgora(agora);
      }
      break;

    case 'd':
    case 'D':
      demoAtivo = !demoAtivo;
      ultimaTrocaDemo = agora;
      Serial.println(demoAtivo ? F("Demonstração ligada.") : F("Demonstração desligada."));
      break;

    case 't':
    case 'T':
      iniciarTeste(agora);
      break;

    case '?':
    case 'h':
    case 'H':
      mostrarAjuda();
      break;

    case '\r':
    case '\n':
    case ' ':
      break;

    default:
      Serial.printf("Comando desconhecido: '%c' (envie ? para ajuda)\n", comando);
      break;
  }
}

}  // namespace

void comandosIniciar(uint32_t agora) {
  ultimaTrocaDemo = agora;
  mostrarAjuda();
  mudarExpressao(rostoExpressao());
}

void comandosAtualizar(uint32_t agora) {
  while (Serial.available() > 0) {
    tratarComando(char(Serial.read()), agora);
  }

  if (testeAtivo) {
    atualizarTeste(agora);
  } else {
    atualizarDemo(agora);
  }
}

bool comandosTesteAtivo() {
  return testeAtivo;
}
