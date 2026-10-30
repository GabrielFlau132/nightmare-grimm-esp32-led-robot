#include "comandos.h"
#include "config.h"
#include "controle_wifi.h"
#include "grupos_led.h"
#include "musica.h"
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
  if (PISCADA_ATIVA) {
    Serial.println(F("  p  piscar agora"));
  }
  Serial.println(F("  d  liga/desliga o modo demonstração"));
  Serial.println(F("  m  toca/para a música"));
  Serial.println(F("  n  próxima música"));
  Serial.println(F("  b  teste dos buzzers (um por vez, depois os 5 juntos)"));
  Serial.println(F("  w  estado do Wi-Fi"));
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

// Escolha feita por uma pessoa (serial ou celular): para o teste e a demonstração.
void escolherExpressao(Expressao expressao, uint32_t agora) {
  pararTeste(agora);
  demoAtivo = false;
  mudarExpressao(expressao);
}

void definirDemo(bool ligada, uint32_t agora) {
  demoAtivo = ligada;
  ultimaTrocaDemo = agora;
  Serial.println(demoAtivo ? F("Demonstração ligada.") : F("Demonstração desligada."));
}

void tratarComando(char comando, uint32_t agora) {
  // '1'..'5' escolhem a expressão e desligam a demonstração.
  if (comando >= '1' && comando < char('1' + TOTAL_EXPRESSOES)) {
    escolherExpressao(Expressao(comando - '1'), agora);
    return;
  }

  switch (comando) {
    case 'p':
    case 'P':
      if (PISCADA_ATIVA && !testeAtivo) {
        piscadaAgora(agora);
      }
      break;

    case 'd':
    case 'D':
      definirDemo(!demoAtivo, agora);
      break;

    case 'm':
    case 'M':
      if (musicaTocando()) {
        musicaParar();
      } else {
        musicaTocar(musicaSelecionada(), agora);
      }
      break;

    case 'n':
    case 'N':
      musicaTocar((musicaSelecionada() + 1) % musicaQuantidade(), agora);
      break;

    case 'w':
    case 'W':
      controleWifiMostrarEstado();
      break;

    case 'b':
    case 'B':
      musicaTestarBuzzers(agora);
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

void comandosEscolherExpressao(Expressao expressao) {
  escolherExpressao(expressao, millis());
}

void comandosDefinirDemo(bool ligada) {
  definirDemo(ligada, millis());
}

bool comandosDemoAtiva() {
  return demoAtivo;
}
