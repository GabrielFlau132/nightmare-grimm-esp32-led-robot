"""Layout dos LEDs do Nightmare Grimm (frente de 152 x 72 mm, LEDs de 4,8 mm).

Os desenhos de Models/ são o esboço: definem quantos LEDs há, em que grupo
cada um fica e o formato de cada parte do rosto. Este script define a posição
final de cada LED, alinhada e com espaçamento uniforme, e confere que nenhum
LED fica a menos de FOLGA_MIN de outro nem da borda.

Uso:  python docs/gerar_layout.py
Gera em docs/: leds.csv, mapa-leds.svg, corte-frente.svg, expressoes.svg
"""
import csv
import html
import itertools
import math
import os
import sys

# ---------------------------------------------------------------------------
# Medidas gerais (mm). Origem no canto superior esquerdo da frente, y para baixo.
# ---------------------------------------------------------------------------
LARGURA, ALTURA = 152.0, 72.0
DIAMETRO_LED = 4.8
FOLGA_MIN = 2.0        # entre as bordas de dois LEDs quaisquer
MARGEM_MIN = 2.0       # entre a borda de um LED e a borda da frente
DIAMETRO_FURO = DIAMETRO_LED  # ajuste depois de testar o corte num retalho

DIST_MIN = DIAMETRO_LED + FOLGA_MIN  # entre centros
MEIO = LARGURA / 2

# ---------------------------------------------------------------------------
# Parâmetros do rosto (lado esquerdo; o direito é espelhado)
# ---------------------------------------------------------------------------
# Olho: 4 colunas com passo OLHO_PASSO (coluna 0 = de fora, 3 = de dentro).
#   Colunas 0 e 3: laterais, 3 LEDs cada, com passo vertical OLHO_PASSO_LATERAL.
#   Colunas 1 e 2: 1 LED no topo, 1 embaixo e 1 no meio (barra da piscada).
OLHO_X, OLHO_Y = 22.8, 33.6   # centro do olho esquerdo
OLHO_PASSO = 7.3
OLHO_PASSO_LATERAL = 8.3
OLHO_TOPO = 12.7              # do centro do olho até o LED do topo/base

# Sobrancelhas: LEDs igualmente espaçados, nas colunas do olho.
#   (coluna da ponta de fora, y da ponta de fora, quanto desce a cada LED)
#   Coluna -1 fica um passo para fora do olho. Descer positivo = em direção
#   ao centro do rosto.
SOBRANCELHA_BRAVA = (0, 4.4, 2.7)      # 4 LEDs, colunas 0..3, desce para o centro
SOBRANCELHA_TRISTE = (-1, 19.4, -2.7)  # 3 LEDs, colunas -1..1, sobe para o centro

# Boca: colunas com passo BOCA_PASSO, centradas no meio da frente.
#   Linha vermelha (reta) em BOCA_Y; linha azul (triste) BOCA_LINHAS abaixo.
#   Cada linha tem BOCA_LEDS_LINHA LEDs. Cada canto tem 2 LEDs nas 2 colunas
#   seguintes à ponta da linha, um degrau acima/abaixo dela, e
#   1 LED na ponta, deslocado BOCA_PONTA (dx, dy) do último LED do degrau.
BOCA_PASSO = 7.0
BOCA_LEDS_LINHA = 4
BOCA_Y = 53.9
BOCA_LINHAS = 7.0
BOCA_DEGRAU = 3.3
BOCA_PONTA = (6.0, 3.3)

# ---------------------------------------------------------------------------
# Grupos (mesma ordem e números do firmware: code/NightmareGrimm/grupos_led.h)
# ---------------------------------------------------------------------------
GRUPOS = [
    # n, nome, cor, pino
    (1, "OLHOS_ANEL", "amarelo", 25),
    (2, "OLHOS_MEIO_LATERAL", "amarelo", 26),
    (3, "OLHOS_MEIO_CENTRO", "amarelo", 27),
    (4, "SOBRANCELHAS_BRAVAS", "vermelho", 32),
    (5, "SOBRANCELHAS_TRISTES", "azul", 33),
    (6, "BOCA_RETA", "vermelho", 23),
    (7, "BOCA_SORRISO", "vermelho", 19),
    (8, "BOCA_TRISTE", "azul", 18),
]
QUANTIDADE_ESPERADA = {1: 16, 2: 4, 3: 4, 4: 8, 5: 6, 6: 4, 7: 6, 8: 10}

# Grupos acesos em cada expressão: espelha expressoes.cpp + rosto.cpp.
OLHOS_ABERTOS, OLHOS_FECHADOS = {1, 2}, {2, 3}
EXPRESSOES = [
    ("FELIZ", ":)", OLHOS_ABERTOS | {6, 7}),
    ("FELIZ_BRAVO", ">:)", OLHOS_ABERTOS | {4, 6, 7}),
    ("BRAVO", ">:|", OLHOS_ABERTOS | {4, 6}),
    ("NEUTRO", ":|", OLHOS_ABERTOS | {6}),
    ("TRISTE", "<:(", OLHOS_ABERTOS | {5, 8}),
    ("piscada (em NEUTRO)", ":|", OLHOS_FECHADOS | {6}),
]


def coluna_olho(k):
    return OLHO_X + (k - 1.5) * OLHO_PASSO


def sobrancelha(grupo, coluna, y, desce, quantidade):
    return [(grupo, coluna_olho(coluna + i), y + i * desce) for i in range(quantidade)]


def lado_esquerdo():
    """LEDs do lado esquerdo da frente (o olho direito do robô)."""
    leds = []
    y = OLHO_Y
    for k in (0, 3):  # laterais
        leds.append((1, coluna_olho(k), y - OLHO_PASSO_LATERAL))
        leds.append((2, coluna_olho(k), y))
        leds.append((1, coluna_olho(k), y + OLHO_PASSO_LATERAL))
    for k in (1, 2):  # topo, barra do meio, base
        leds.append((1, coluna_olho(k), y - OLHO_TOPO))
        leds.append((3, coluna_olho(k), y))
        leds.append((1, coluna_olho(k), y + OLHO_TOPO))
    leds += sobrancelha(4, *SOBRANCELHA_BRAVA, 4)
    leds += sobrancelha(5, *SOBRANCELHA_TRISTE, 3)
    # cantos da boca deste lado: sorriso sobe a partir da linha vermelha,
    # tristeza desce a partir da linha azul
    p, d = BOCA_PASSO, BOCA_DEGRAU
    pdx, pdy = BOCA_PONTA
    y_azul = BOCA_Y + BOCA_LINHAS
    ponta = (BOCA_LEDS_LINHA - 1) / 2 * p  # da linha central até seu último LED
    for dx, dy in ((ponta + p, d), (ponta + 2 * p, d), (ponta + 2 * p + pdx, d + pdy)):
        leds.append((7, MEIO - dx, BOCA_Y - dy))
        leds.append((8, MEIO - dx, y_azul + dy))
    return leds


def gerar():
    esquerdo = lado_esquerdo()
    leds = esquerdo + [(g, LARGURA - x, y) for g, x, y in esquerdo]
    for k in range(BOCA_LEDS_LINHA):  # linhas centrais da boca
        x = MEIO + (k - (BOCA_LEDS_LINHA - 1) / 2) * BOCA_PASSO
        leds.append((6, x, BOCA_Y))
        leds.append((8, x, BOCA_Y + BOCA_LINHAS))
    leds.sort(key=lambda l: (l[0], round(l[2], 3), round(l[1], 3)))
    return [(f"L{i + 1:02d}", g, round(x, 2), round(y, 2)) for i, (g, x, y) in enumerate(leds)]


def conferir(leds):
    erros = []
    contagem = {g: sum(1 for l in leds if l[1] == g) for g in QUANTIDADE_ESPERADA}
    if contagem != QUANTIDADE_ESPERADA:
        erros.append(f"quantidades {contagem} != {QUANTIDADE_ESPERADA}")
    r = DIAMETRO_LED / 2
    for id_, g, x, y in leds:
        borda = min(x, y, LARGURA - x, ALTURA - y) - r
        if borda < MARGEM_MIN - 1e-9:
            erros.append(f"{id_} (grupo {g}) a {borda:.2f} mm da borda")
    pares = []
    for a, b in itertools.combinations(leds, 2):
        d = math.dist(a[2:], b[2:])
        pares.append((d, a, b))
        if d < DIST_MIN - 1e-9:
            erros.append(f"{a[0]} (g{a[1]}) e {b[0]} (g{b[1]}) a {d - DIAMETRO_LED:.2f} mm (borda a borda)")
    pares.sort(key=lambda p: p[0])
    return erros, pares


# ---------------------------------------------------------------------------
# Saídas
# ---------------------------------------------------------------------------
COR_HEX = {"amarelo": "#f2c200", "vermelho": "#d7262f", "azul": "#1f8fe0"}
INFO = {n: (nome, cor, pino) for n, nome, cor, pino in GRUPOS}


def escrever_csv(leds, caminho):
    with open(caminho, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["led", "grupo", "nome_grupo", "cor", "pino_esp32", "x_mm", "y_mm"])
        for id_, g, x, y in leds:
            nome, cor, pino = INFO[g]
            w.writerow([id_, g, nome, cor, pino, f"{x:.2f}", f"{y:.2f}"])


def svg_led(x, y, g, aceso=True, numero=True):
    cor = COR_HEX[INFO[g][1]]
    r = DIAMETRO_LED / 2
    if not aceso:
        return f'<circle cx="{x:.2f}" cy="{y:.2f}" r="{r}" fill="#e6e6e6" stroke="#b5b5b5" stroke-width="0.15"/>'
    s = f'<circle cx="{x:.2f}" cy="{y:.2f}" r="{r}" fill="{cor}" stroke="#000" stroke-width="0.12"/>'
    if numero:
        texto = "#000" if INFO[g][1] == "amarelo" else "#fff"
        s += (f'<text x="{x:.2f}" y="{y + 0.62:.2f}" font-size="1.8" text-anchor="middle" '
              f'font-weight="bold" fill="{texto}">{g}</text>')
    return s


def escrever_mapa(leds, pares, caminho):
    m = 5
    w, h = LARGURA + 2 * m, ALTURA + 64
    s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}mm" height="{h}mm" viewBox="0 0 {w} {h}" '
         'font-family="Arial, Helvetica, sans-serif">',
         f'<rect width="{w}" height="{h}" fill="#fff"/>',
         f'<text x="{m}" y="3.6" font-size="2.6" font-weight="bold">Nightmare Grimm: mapa dos LEDs (frente, escala 1:1)</text>',
         f'<g transform="translate({m} {m})">',
         f'<rect width="{LARGURA}" height="{ALTURA}" fill="none" stroke="#000" stroke-width="0.25"/>']
    s += [svg_led(x, y, g) for _, g, x, y in leds]
    s.append('</g>')
    s.append(f'<text x="{m + LARGURA}" y="{m - 0.8}" font-size="1.8" text-anchor="end" fill="#555">'
             f'{LARGURA:g} x {ALTURA:g} mm</text>')
    ry = m + ALTURA + 4
    s.append(f'<path d="M{m} {ry - 1}v2M{m} {ry}h10M{m + 10} {ry - 1}v2" stroke="#000" stroke-width="0.3" fill="none"/>')
    s.append(f'<text x="{m + 12}" y="{ry + 0.7}" font-size="2">10 mm (confira a régua: imprima em 100%, sem ajustar à página)</text>')
    ly = ry + 6
    for xx, t in ((m, "n"), (m + 6, "grupo"), (m + 44, "cor"), (m + 64, "LEDs"), (m + 76, "pino")):
        s.append(f'<text x="{xx}" y="{ly}" font-size="2.2" font-weight="bold">{t}</text>')
    for i, (n, nome, cor, pino) in enumerate(GRUPOS):
        y = ly + 4.6 + i * 5
        s.append(svg_led(m + 2.4, y - 0.7, n))
        for xx, t in ((m + 6, nome), (m + 44, cor), (m + 64, QUANTIDADE_ESPERADA[n]), (m + 76, f"GPIO {pino}")):
            s.append(f'<text x="{xx}" y="{y}" font-size="2.2">{t}</text>')
    menor = pares[0][0]
    s.append(f'<text x="{m}" y="{h - 2}" font-size="1.8" fill="#555">LEDs de {DIAMETRO_LED:g} mm. '
             f'Menor distância entre centros: {menor:.2f} mm ({menor - DIAMETRO_LED:.2f} mm entre bordas). '
             'Coordenadas exatas em docs/leds.csv.</text>')
    s.append('</svg>')
    with open(caminho, "w", encoding="utf-8") as f:
        f.write("\n".join(s) + "\n")


def escrever_furos(leds, caminho):
    # Tudo em vermelho = corte: contorno da placa e furos dos LEDs.
    s = ['<?xml version="1.0" encoding="UTF-8"?>',
         f'<svg xmlns="http://www.w3.org/2000/svg" version="1.1" width="{LARGURA}mm" height="{ALTURA}mm" '
         f'viewBox="0 0 {LARGURA} {ALTURA}">',
         f'<rect width="{LARGURA}" height="{ALTURA}" fill="none" stroke="#ff0000" stroke-width="0.1"/>']
    s += [f'<circle cx="{x:.2f}" cy="{y:.2f}" r="{DIAMETRO_FURO / 2:.3f}" fill="none" stroke="#ff0000" stroke-width="0.1"/>'
          for _, _, x, y in leds]
    s.append('</svg>')
    with open(caminho, "w", encoding="utf-8") as f:
        f.write("\n".join(s) + "\n")


def escrever_expressoes(leds, caminho):
    colunas, esc, m, legenda = 2, 0.75, 4, 6  # esc: prévia, não é 1:1
    cw, ch = LARGURA + 2 * m, ALTURA + 2 * m + legenda
    linhas = math.ceil(len(EXPRESSOES) / colunas)
    w, h = colunas * cw, linhas * ch
    s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{w * esc}mm" height="{h * esc}mm" viewBox="0 0 {w} {h}" '
         'font-family="Arial, Helvetica, sans-serif">', f'<rect width="{w}" height="{h}" fill="#fff"/>']
    for i, (nome, simbolo, acesos) in enumerate(EXPRESSOES):
        ox, oy = (i % colunas) * cw + m, (i // colunas) * ch + m
        s.append(f'<text x="{ox}" y="{oy + 3}" font-size="3.6" font-weight="bold">'
                 f'{html.escape(nome)}  {html.escape(simbolo)}</text>')
        s.append(f'<g transform="translate({ox} {oy + legenda})">')
        s.append(f'<rect width="{LARGURA}" height="{ALTURA}" rx="1.5" fill="#1d1d1f"/>')
        s += [svg_led(x, y, g, aceso=g in acesos, numero=False) for _, g, x, y in leds]
        s.append('</g>')
    s.append('</svg>')
    with open(caminho, "w", encoding="utf-8") as f:
        f.write("\n".join(s).replace('fill="#e6e6e6" stroke="#b5b5b5"', 'fill="#3a3a3c" stroke="#55555a"') + "\n")


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    leds = gerar()
    erros, pares = conferir(leds)
    print(f"{len(leds)} LEDs. Pares mais próximos (centro a centro):")
    for d, a, b in pares[:8]:
        print(f"  {d:5.2f} mm  {a[0]} g{a[1]} ({a[2]:.1f},{a[3]:.1f})  {b[0]} g{b[1]} ({b[2]:.1f},{b[3]:.1f})")
    if erros:
        print("ERROS:\n  " + "\n  ".join(erros))
        sys.exit(1)
    pasta = os.path.dirname(os.path.abspath(__file__))
    escrever_csv(leds, os.path.join(pasta, "leds.csv"))
    escrever_mapa(leds, pares, os.path.join(pasta, "mapa-leds.svg"))
    escrever_furos(leds, os.path.join(pasta, "corte-frente.svg"))
    escrever_expressoes(leds, os.path.join(pasta, "expressoes.svg"))
    print(f"OK: nenhum par a menos de {FOLGA_MIN:g} mm entre bordas. Arquivos gerados em {pasta}")


if __name__ == "__main__":
    main()
