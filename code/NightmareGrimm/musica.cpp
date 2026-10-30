#include "musica.h"
#include "config.h"
#include "musica_giornos_theme.h"
#include "musica_giornos_theme_reduzida.h"
#include "musica_tetris.h"
#include "musica_winner_takes_it_all.h"
#include "musica_troupe_grimm.h"

namespace {

struct Musica {
  const char* nome;
  const EventoNota* eventos;
  uint16_t totalEventos;
};

template <size_t N>
constexpr uint16_t tamanho(const EventoNota (&)[N]) {
  return N;
}

// Para acrescentar uma música: gere o .h com ferramentas/midi_para_buzzer.py,
// inclua-o acima e acrescente uma linha aqui.
const Musica MUSICAS[] = {
  { "Troupe Master Grimm", TROUPE_MASTER_GRIMM, tamanho(TROUPE_MASTER_GRIMM) },
  { "Jojo",                GIORNOS_THEME,       tamanho(GIORNOS_THEME) },
  { "Jojo (reduzida)",     GIORNOS_THEME_REDUZIDA, tamanho(GIORNOS_THEME_REDUZIDA) },
  { "Tetris",              TETRIS,              tamanho(TETRIS) },
  { "The Winner Takes It All", WINNER_TAKES_IT_ALL, tamanho(WINNER_TAKES_IT_ALL) },
};
constexpr uint8_t TOTAL_MUSICAS = sizeof(MUSICAS) / sizeof(MUSICAS[0]);

// Teste dos buzzers: um passo por linha, com a frequência de cada buzzer (0 = mudo).
struct PassoTeste {
  uint16_t frequencias[TOTAL_BUZZERS];
  uint16_t duracaoMs;
};

const PassoTeste PASSOS_TESTE[] = {
  { { 262, 0, 0, 0, 0 }, 600 }, { { 0, 0, 0, 0, 0 }, 200 },
  { { 0, 330, 0, 0, 0 }, 600 }, { { 0, 0, 0, 0, 0 }, 200 },
  { { 0, 0, 392, 0, 0 }, 600 }, { { 0, 0, 0, 0, 0 }, 200 },
  { { 0, 0, 0, 523, 0 }, 600 }, { { 0, 0, 0, 0, 0 }, 200 },
  { { 0, 0, 0, 0, 659 }, 600 }, { { 0, 0, 0, 0, 0 }, 400 },
  { { 262, 330, 392, 523, 659 }, 2000 },  // os 5 juntos: acorde de Dó maior
};
constexpr uint8_t TOTAL_PASSOS_TESTE = sizeof(PASSOS_TESTE) / sizeof(PASSOS_TESTE[0]);

enum class Modo : uint8_t { PARADO, MUSICA, TESTE };

Modo modo = Modo::PARADO;
uint32_t inicio = 0;

// música
uint8_t musicaAtual = 0;
uint16_t proximoEvento = 0;
bool vozTocando[TOTAL_BUZZERS] = {};
uint32_t fimDaVoz[TOTAL_BUZZERS] = {};  // em ms desde o início da música

// teste
uint8_t passoTeste = 0;

// volume em % do máximo: o máximo é a onda quadrada com metade do período
// ligada (duty 512 de 1024); um duty menor deixa o som mais fraco.
void tocarVoz(uint8_t voz, uint16_t frequencia, uint8_t volume = 100) {
  ledcWriteTone(CANAIS_BUZZERS[voz], frequencia);  // 0 = silêncio
  if (frequencia != 0 && volume < 100) {
    ledcWrite(CANAIS_BUZZERS[voz], 512UL * volume / 100);
  }
}

// Leva a nota para a faixa em que o piezo soa bem, subindo de oitava em oitava.
// Devolve true se a nota era grave (ficou abaixo do mínimo depois de MUSICA_OITAVAS_ACIMA).
bool ajustarParaPiezo(uint16_t original, uint16_t& ajustada) {
  uint32_t frequencia = uint32_t(original) << MUSICA_OITAVAS_ACIMA;
  const bool eraGrave = frequencia < MUSICA_FREQ_MINIMA_HZ;
  while (frequencia < MUSICA_FREQ_MINIMA_HZ) {
    frequencia <<= 1;
  }
  ajustada = frequencia;
  return eraGrave;
}

void silenciarTudo() {
  for (uint8_t voz = 0; voz < TOTAL_BUZZERS; voz++) {
    tocarVoz(voz, 0);
    vozTocando[voz] = false;
  }
}

void atualizarMusica(uint32_t agora) {
  const uint32_t tempo = agora - inicio;

  // Primeiro encerra as notas que acabaram, depois começa as novas: assim uma
  // nota que começa no mesmo ms em que a anterior da mesma voz termina não é
  // cortada.
  for (uint8_t voz = 0; voz < TOTAL_BUZZERS; voz++) {
    if (vozTocando[voz] && tempo >= fimDaVoz[voz]) {
      tocarVoz(voz, 0);
      vozTocando[voz] = false;
    }
  }

  const Musica& musica = MUSICAS[musicaAtual];
  while (proximoEvento < musica.totalEventos && musica.eventos[proximoEvento].inicioMs <= tempo) {
    const EventoNota& evento = musica.eventos[proximoEvento++];
    const uint32_t fim = evento.inicioMs + evento.duracaoMs;
    if (fim <= tempo) {
      continue;  // o loop atrasou e a nota inteira já passou
    }
    // Cada buzzer só toca uma frequência: se a voz já tivesse uma nota soando,
    // a nova substituiria a anterior. Nos dados atuais isso não acontece.
    uint16_t frequencia;
    const bool eraGrave = ajustarParaPiezo(evento.frequencia, frequencia);
    tocarVoz(evento.voz, frequencia, eraGrave ? MUSICA_VOLUME_GRAVES : 100);
    vozTocando[evento.voz] = true;
    fimDaVoz[evento.voz] = fim;
  }

  if (proximoEvento >= musica.totalEventos) {
    bool algumaVoz = false;
    for (uint8_t voz = 0; voz < TOTAL_BUZZERS; voz++) {
      algumaVoz = algumaVoz || vozTocando[voz];
    }
    if (!algumaVoz) {
      modo = Modo::PARADO;
      Serial.println(F("Música terminou."));
    }
  }
}

void aplicarPassoTeste() {
  const PassoTeste& passo = PASSOS_TESTE[passoTeste];
  for (uint8_t voz = 0; voz < TOTAL_BUZZERS; voz++) {
    tocarVoz(voz, passo.frequencias[voz]);
    if (passo.frequencias[voz]) {
      Serial.printf("Buzzer %u (GPIO %u): %u Hz\n", voz + 1, PINOS_BUZZERS[voz], passo.frequencias[voz]);
    }
  }
}

void atualizarTeste(uint32_t agora) {
  if (agora - inicio < PASSOS_TESTE[passoTeste].duracaoMs) {
    return;
  }
  inicio = agora;
  passoTeste++;
  if (passoTeste < TOTAL_PASSOS_TESTE) {
    aplicarPassoTeste();
  } else {
    silenciarTudo();
    modo = Modo::PARADO;
    Serial.println(F("Teste dos buzzers concluído."));
  }
}

}  // namespace

void musicaIniciar() {
  for (uint8_t voz = 0; voz < TOTAL_BUZZERS; voz++) {
    ledcSetup(CANAIS_BUZZERS[voz], 1000, 10);
    ledcAttachPin(PINOS_BUZZERS[voz], CANAIS_BUZZERS[voz]);
  }
  silenciarTudo();
}

void musicaAtualizar(uint32_t agora) {
  // O loop lê o millis() no começo da volta; se a música foi iniciada depois
  // disso (pelo celular, no meio da volta), "agora" é anterior ao início. Sem
  // este teste, agora - inicio daria um número enorme e a música acabaria na hora.
  if (int32_t(agora - inicio) < 0) {
    return;
  }
  if (modo == Modo::MUSICA) {
    atualizarMusica(agora);
  } else if (modo == Modo::TESTE) {
    atualizarTeste(agora);
  }
}

void musicaTocar(uint8_t indice, uint32_t agora) {
  if (indice >= TOTAL_MUSICAS) {
    return;
  }
  silenciarTudo();
  modo = Modo::MUSICA;
  musicaAtual = indice;
  inicio = agora;
  proximoEvento = 0;
  Serial.printf("Música %u/%u: %s\n", indice + 1, TOTAL_MUSICAS, MUSICAS[indice].nome);
  atualizarMusica(agora);
}

void musicaParar() {
  if (modo != Modo::PARADO) {
    silenciarTudo();
    modo = Modo::PARADO;
    Serial.println(F("Música parada."));
  }
}

bool musicaTocando() {
  return modo == Modo::MUSICA;
}

uint8_t musicaSelecionada() {
  return musicaAtual;
}

uint8_t musicaQuantidade() {
  return TOTAL_MUSICAS;
}

const char* musicaNome(uint8_t indice) {
  return indice < TOTAL_MUSICAS ? MUSICAS[indice].nome : "";
}

void musicaTestarBuzzers(uint32_t agora) {
  silenciarTudo();
  modo = Modo::TESTE;
  inicio = agora;
  passoTeste = 0;
  Serial.println(F("Teste dos buzzers: um por vez, depois os 5 juntos."));
  aplicarPassoTeste();
}
