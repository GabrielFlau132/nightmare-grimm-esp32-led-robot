"""Converte um arquivo MIDI em notas para os 5 buzzers do Nightmare Grimm.

Uso:
    python ferramentas/midi_para_buzzer.py ARQUIVO.mid NOME_DA_VARIAVEL "Nome da música"

Gera code/NightmareGrimm/musica_<nome_da_variavel em minúsculas>.h com um array
de EventoNota {início ms, duração ms, frequência Hz, voz 0..4}, no mesmo formato
de musica_troupe_grimm.h. Depois, acrescente a música na lista MUSICAS em
code/NightmareGrimm/musica.cpp.

Regras (cada buzzer toca uma nota por vez):
  - Cada nota vai para um buzzer livre. Duas notas nunca dividem um buzzer.
  - Com mais de 5 notas soando, ficam a mais aguda (melodia) e a mais grave
    (baixo): uma nota nova do meio do acorde é descartada; uma nova melodia ou
    um novo baixo corta a nota mais do meio que estiver soando.
  - Notas seguidas no mesmo buzzer ganham uma pausa de PAUSA_MS entre elas,
    senão duas notas iguais em sequência viram um som contínuo.
  - Notas mais curtas que DURACAO_MINIMA_MS e a bateria (canal 10) ficam de fora.
Precisa de: pip install mido
"""
import os
import sys

import mido

TOTAL_BUZZERS = 5
PAUSA_MS = 12
DURACAO_MINIMA_MS = 20
CANAL_BATERIA = 9  # canal 10 no MIDI


def ler_notas(caminho):
    """[(início ms, fim ms, nota MIDI)] em ordem de início (mais aguda primeiro)."""
    t, abertas, notas = 0.0, {}, []
    for msg in mido.MidiFile(caminho):  # já converte ticks em segundos pelo andamento
        t += msg.time
        if msg.type not in ("note_on", "note_off") or msg.channel == CANAL_BATERIA:
            continue
        chave = (msg.channel, msg.note)
        if msg.type == "note_on" and msg.velocity > 0:
            abertas.setdefault(chave, []).append(t)
        elif abertas.get(chave):
            inicio = abertas[chave].pop(0)
            notas.append((round(inicio * 1000), round(t * 1000), msg.note))
    notas.sort(key=lambda n: (n[0], -n[2]))
    return notas


def distribuir(notas):
    eventos = []                    # [início, fim, nota, voz]
    vozes = [None] * TOTAL_BUZZERS  # índice do evento soando em cada buzzer
    curtas = descartadas = cortadas = 0
    for inicio, fim, nota in notas:
        if fim - inicio < DURACAO_MINIMA_MS:
            curtas += 1
            continue
        livres = [v for v in range(TOTAL_BUZZERS) if vozes[v] is None or eventos[vozes[v]][1] <= inicio]
        if livres:
            voz = livres[0]
        else:
            soando = [eventos[vozes[v]][2] for v in range(TOTAL_BUZZERS)]
            if min(soando) < nota < max(soando):
                descartadas += 1
                continue
            # nova melodia ou novo baixo: corta a nota mais do meio
            meio = sorted(soando)[len(soando) // 2]
            voz = min(range(TOTAL_BUZZERS), key=lambda v: abs(eventos[vozes[v]][2] - meio))
            eventos[vozes[voz]][1] = inicio
            cortadas += 1
        eventos.append([inicio, fim, nota, voz])
        vozes[voz] = len(eventos) - 1

    # pausa entre notas seguidas do mesmo buzzer
    for voz in range(TOTAL_BUZZERS):
        da_voz = [e for e in eventos if e[3] == voz]
        for atual, proxima in zip(da_voz, da_voz[1:]):
            if proxima[0] - atual[1] < PAUSA_MS:
                novo_fim = proxima[0] - PAUSA_MS
                # se não sobra tempo nem para a pausa, a nota sai (fim = início)
                atual[1] = novo_fim if novo_fim - atual[0] >= DURACAO_MINIMA_MS // 2 else atual[0]

    removidas = sum(1 for e in eventos if e[1] <= e[0])
    eventos = [e for e in eventos if e[1] > e[0]]
    eventos.sort(key=lambda e: (e[0], e[3]))
    return eventos, curtas, descartadas + removidas, cortadas - removidas


def conferir(eventos):
    for voz in range(TOTAL_BUZZERS):
        da_voz = sorted(e for e in eventos if e[3] == voz)
        for a, b in zip(da_voz, da_voz[1:]):
            assert b[0] >= a[1], f"voz {voz}: notas sobrepostas em {a} e {b}"
    assert all(e[1] - e[0] <= 65535 for e in eventos), "nota longa demais para uint16"


def frequencia(nota):
    return round(440 * 2 ** ((nota - 69) / 12))


def main():
    if len(sys.argv) != 4:
        sys.exit(__doc__)
    caminho, variavel, titulo = sys.argv[1], sys.argv[2].upper(), sys.argv[3]
    notas = ler_notas(caminho)
    eventos, curtas, descartadas, cortadas = distribuir(notas)
    conferir(eventos)

    raiz = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    saida = os.path.join(raiz, "code", "NightmareGrimm", f"musica_{variavel.lower()}.h")
    duracao = max(e[1] for e in eventos)
    linhas = [f"  {{{e[0]}UL, {e[1] - e[0]}, {frequencia(e[2])}, {e[3]}}}," for e in eventos]
    with open(saida, "w", encoding="utf-8", newline="\n") as f:
        f.write(f"// {titulo}, em 5 vozes, para os 5 buzzers.\n")
        f.write(f"// Gerado por ferramentas/midi_para_buzzer.py a partir de {os.path.basename(caminho)}.\n")
        f.write(f"// {len(eventos)} notas, {duracao / 1000:.1f} s. Cada evento: {{início ms, duração ms, frequência Hz, voz}}.\n")
        f.write("#pragma once\n#include \"musica_eventos.h\"\n\n")
        f.write(f"const EventoNota {variavel}[] = {{\n" + "\n".join(linhas) + "\n};\n")

    print(f"{titulo}: {len(notas)} notas no MIDI -> {len(eventos)} no buzzer, {duracao / 1000:.1f} s")
    print(f"  fora: {curtas} curtas demais (< {DURACAO_MINIMA_MS} ms) e {descartadas} do meio de acordes com mais de "
          f"{TOTAL_BUZZERS} notas; {cortadas} notas do meio encurtadas para caber melodia/baixo")
    print(f"  frequências: {min(frequencia(e[2]) for e in eventos)} a {max(frequencia(e[2]) for e in eventos)} Hz")
    print(f"  arquivo: {saida} ({len(eventos) * 12 / 1024:.1f} KB na flash)")


if __name__ == "__main__":
    main()
