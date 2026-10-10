"""Layout dos LEDs do Nightmare Grimm (frente de 152 x 72 mm, LEDs de 4,8 mm).

Os desenhos de Models/ são o esboço: definem quantos LEDs há, em que grupo
cada um fica e o formato de cada parte do rosto. Este script define a posição
final de cada LED, alinhada e com espaçamento uniforme, e confere que nenhum
LED fica a menos de FOLGA_MIN de outro nem da borda.

Uso:  python docs/gerar_layout.py
Gera em docs/: leds.csv, mapa-leds.svg, corte-frente.svg, expressoes.svg, ligacoes.svg
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
# Pinos: mesmos de config.h. Com 2 pinos, o primeiro liga a metade esquerda do
# rosto (vista de frente) e o segundo, a direita.
GRUPOS = [
    # n, nome, cor, pinos
    (1, "OLHOS_ANEL", "amarelo", (25, 13)),
    (2, "OLHOS_MEIO_LATERAL", "amarelo", (26,)),
    (3, "OLHOS_MEIO_CENTRO", "amarelo", (27,)),
    (4, "SOBRANCELHAS_BRAVAS", "vermelho", (32,)),
    (5, "SOBRANCELHAS_TRISTES", "azul", (33,)),
    (6, "BOCA_RETA", "vermelho", (23,)),
    (7, "BOCA_SORRISO", "vermelho", (19,)),
    (8, "BOCA_TRISTE", "azul", (18, 4)),
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
INFO = {n: (nome, cor, pinos) for n, nome, cor, pinos in GRUPOS}

# LEDs ligados direto no ESP32: pino -> resistor do pino -> anodo; catodo no GND.
# Um resistor por pino, dividido por todos os LEDs daquele pino, para ~2 mA
# por LED (conta no README).
TENSAO = 3.2  # pino em HIGH fornecendo ~10-15 mA fica um pouco abaixo de 3,3 V
VF = {"vermelho": 1.85, "amarelo": 1.95, "azul": 2.8}  # típico a ~2 mA; confirmar
CORRENTE_MA = 2.0
CORRENTE_MAX_PINO_MA = 20
# Resistor de cada pino, escolhido entre os valores disponíveis (100, 150, 200
# e 47 ohm) para ficar perto de CORRENTE_MA por LED. O main() confere que
# nenhum pino passa de CORRENTE_MAX_PINO_MA.
RESISTOR_PINO = {25: 100, 13: 100, 32: 100,
                 26: 200, 27: 200, 23: 200,
                 19: 150,
                 33: 47, 18: 47, 4: 47}


def resistor_do_pino(pino):
    return RESISTOR_PINO[pino]


def corrente_do_pino_ma(cor, resistor):
    return (TENSAO - VF[cor]) / resistor * 1000


def pino_do_led(g, x):
    pinos = INFO[g][2]
    return pinos[0] if len(pinos) == 1 or x < MEIO else pinos[1]


def ligacoes_por_pino(leds):
    """[(pino, grupo, lado, [ids])] na ordem dos grupos."""
    saida = []
    for n, _, _, pinos in GRUPOS:
        for i, pino in enumerate(pinos):
            lado = "" if len(pinos) == 1 else ("metade esquerda", "metade direita")[i]
            ids = [l[0] for l in leds if l[1] == n and pino_do_led(n, l[2]) == pino]
            saida.append((pino, n, lado, ids))
    return saida


def escrever_csv(leds, caminho):
    with open(caminho, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["led", "grupo", "nome_grupo", "cor", "pino_esp32", "resistor_do_pino_ohm", "x_mm", "y_mm"])
        resistores = {pino: resistor_do_pino(pino) for pino, _, _, _ in ligacoes_por_pino(leds)}
        for id_, g, x, y in leds:
            nome, cor, _ = INFO[g]
            pino = pino_do_led(g, x)
            w.writerow([id_, g, nome, cor, pino, resistores[pino], f"{x:.2f}", f"{y:.2f}"])


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
    for i, (n, nome, cor, pinos) in enumerate(GRUPOS):
        y = ly + 4.6 + i * 5
        s.append(svg_led(m + 2.4, y - 0.7, n))
        gpio = "GPIO " + " e ".join(map(str, pinos))
        for xx, t in ((m + 6, nome), (m + 44, cor), (m + 64, QUANTIDADE_ESPERADA[n]), (m + 76, gpio)):
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


# Cor do fio de cada pino na folha de ligações (só para o desenho).
COR_FIO = {25: "#ef6c00", 13: "#6d4c41", 26: "#00897b", 27: "#5e35b1", 32: "#c2185b",
           33: "#1565c0", 23: "#7cb342", 19: "#9e9d24", 18: "#00acc1", 4: "#8e8e8e"}
COR_GND = "#111111"
# Shield de expansão do ESP32 (barras S/V/G), vista de cima, antena para cima.
SHIELD_ESQUERDA = ["EN", "VP", "VN", "D34", "D35", "D32", "D33", "D25", "D26", "D27", "D14", "D12", "D13"]
SHIELD_DIREITA = ["D23", "D22", "TX0", "RX0", "D21", "D19", "D18", "D5", "D17", "D16", "D4", "D2", "D15"]


def arvore(pontos):
    """Arestas da árvore geradora mínima (Prim): o caminho de fio mais curto."""
    if len(pontos) < 2:
        return []
    dentro, arestas = {0}, []
    while len(dentro) < len(pontos):
        i, j = min(((i, j) for i in dentro for j in range(len(pontos)) if j not in dentro),
                   key=lambda e: math.dist(pontos[e[0]], pontos[e[1]]))
        dentro.add(j)
        arestas.append((i, j))
    return arestas


def espalhar(xs, minimo, x_min, x_max):
    """Posições dos rótulos perto de xs (ordenados), com pelo menos `minimo` entre eles."""
    pos = []
    for x in xs:
        pos.append(max(x, pos[-1] + minimo) if pos else max(x, x_min))
    for i in range(len(pos) - 1, -1, -1):
        pos[i] = min(pos[i], x_max if i == len(pos) - 1 else pos[i + 1] - minimo)
    return pos


def escrever_ligacoes(leds, caminho):
    """Folha de ligações: shield, fios entre os LEDs (vista de trás) e tabela."""
    w = 200
    s = []

    def texto(x, y, t, tam=2.6, extra=""):
        s.append(f'<text x="{x:.2f}" y="{y:.2f}" font-size="{tam}" {extra}>{html.escape(str(t))}</text>')

    def fio(x1, y1, x2, y2, cor, larg=0.7, curva=0.0):
        # Fios longos viram curvas, cada pino com uma curvatura, para não se sobreporem.
        comp = math.hypot(x2 - x1, y2 - y1)
        if curva and comp > 15:
            mx, my = (x1 + x2) / 2 - (y2 - y1) / comp * curva, (y1 + y2) / 2 + (x2 - x1) / comp * curva
            d = f"M{x1:.2f} {y1:.2f}Q{mx:.2f} {my:.2f} {x2:.2f} {y2:.2f}"
        else:
            d = f"M{x1:.2f} {y1:.2f}L{x2:.2f} {y2:.2f}"
        s.append(f'<path d="{d}" fill="none" stroke="{cor}" stroke-width="{larg}" stroke-linecap="round"/>')

    por_pino = ligacoes_por_pino(leds)
    info_pino = {pino: (n, lado, ids) for pino, n, lado, ids in por_pino}
    texto(5, 8, "Nightmare Grimm: ligações no shield do ESP32", 5, 'font-weight="bold"')
    texto(5, 13, "LEDs direto no ESP32: GPIO (pino S) -> resistor -> anodo (perna longa); catodo (perna curta) "
          "no GND comum. Pino em HIGH acende.", 2.3, 'fill="#444"')

    # --- 1. Shield --------------------------------------------------------
    # Cada pino tem três terminais: S (sinal), V (tensão) e G (GND). Os fios
    # dos LEDs usam o S; o GND comum vai num G qualquer; o V não é usado.
    texto(5, 21, "1. Shield", 3.4, 'font-weight="bold"')
    p, cx, y0 = 4.2, w / 2, 33
    yf = [y0 + i * p for i in range(13)]
    curto = {1: "olhos contorno", 2: "olhos meio lat.", 3: "olhos meio centro", 4: "sobr. bravas",
             5: "sobr. tristes", 6: "boca reta", 7: "boca sorriso", 8: "boca triste"}
    terminais = (("S", 37, "#f2c200"), ("V", 32.8, "#d32f2f"), ("G", 28.6, "#9e9e9e"))
    s.append(f'<rect x="{cx - 45}" y="{y0 - 10}" width="90" height="{yf[-1] - y0 + 22:.2f}" rx="3" fill="#2b2b2b"/>')
    s.append(f'<rect x="{cx - 13}" y="{y0 - 6}" width="26" height="{yf[-1] - y0 + 12:.2f}" rx="1.5" fill="#3d4a6b"/>')
    texto(cx, y0 - 1.5, "ESP32", 2.8, 'text-anchor="middle" fill="#fff" font-weight="bold"')
    texto(cx, yf[-1] + 3, "USB", 2.2, 'text-anchor="middle" fill="#fff"')
    for nomes, lado in ((SHIELD_ESQUERDA, -1), (SHIELD_DIREITA, 1)):
        for letra, dx, cor_t in terminais:
            texto(cx + lado * dx, y0 - 4.5, letra, 2, f'text-anchor="middle" fill="{cor_t}" font-weight="bold"')
        for nome, y in zip(nomes, yf):
            for _, dx, cor_t in terminais:
                s.append(f'<rect x="{cx + lado * dx - 1:.2f}" y="{y - 1:.2f}" width="2" height="2" fill="{cor_t}" '
                         'stroke="#555" stroke-width="0.15"/>')
            texto(cx + lado * 26.4, y + 0.7, nome, 1.9,
                  f'text-anchor="{"start" if lado < 0 else "end"}" fill="#ddd"')
            gpio = int(nome[1:]) if nome[0] == "D" and nome[1:].isdigit() else None
            if gpio not in info_pino:
                continue
            n, lado_rosto, ids = info_pino[gpio]
            cor = COR_FIO[gpio]
            metade = " esq." if lado_rosto == "metade esquerda" else " dir." if lado_rosto else ""
            rotulo = f"GPIO {gpio}: {curto[n]}{metade} ({len(ids)})"
            x_s = cx + lado * 37
            x_fim = x_s + lado * 20
            fio(x_s, y, x_fim, y, cor, 0.9)
            s.append(f'<circle cx="{x_s:.2f}" cy="{y:.2f}" r="0.9" fill="{cor}"/>')
            # resistor soldado no meio do fio, coberto com termo-retrátil
            r = resistor_do_pino(gpio)
            xr = min(x_s + lado * 6, x_s + lado * 14)
            s.append(f'<rect x="{xr:.2f}" y="{y - 1.1:.2f}" width="8" height="2.2" rx="0.6" '
                     'fill="#e8d3a9" stroke="#333" stroke-width="0.25"/>')
            texto(xr + 4, y + 0.6, f"{r} Ω", 1.6, 'text-anchor="middle" fill="#222" font-weight="bold"')
            texto(x_fim + lado * 1.5, y + 0.8, rotulo, 2.1,
                  f'text-anchor="{"end" if lado < 0 else "start"}" fill="{cor}" font-weight="bold"')
    # GND comum: qualquer pino G serve
    x_g, y_g = cx - 28.6, yf[-1]
    y_baixo = yf[-1] + 7
    x_fim = cx - 57
    s.append(f'<path d="M{x_g:.2f} {y_g:.2f}V{y_baixo:.2f}H{x_fim:.2f}" fill="none" stroke="{COR_GND}" '
             'stroke-width="0.9"/>')
    s.append(f'<circle cx="{x_g:.2f}" cy="{y_g:.2f}" r="0.9" fill="{COR_GND}" stroke="#fff" stroke-width="0.2"/>')
    texto(x_fim - 1.5, y_baixo + 0.8, f"GND: perna curta dos {len(leds)} LEDs", 2.1,
          f'text-anchor="end" fill="{COR_GND}" font-weight="bold"')
    y_b = yf[-1] + 18
    texto(5, y_b, "Cada fio de GPIO encaixa no pino S (amarelo) e tem o resistor do pino soldado no meio, coberto "
          "com termo-retrátil (valor no desenho e na tabela).", 2.2, 'fill="#444"')
    texto(5, y_b + 3.6, "O fio preto do GND pode ir em qualquer pino G. Os pinos V não são usados, então a posição "
          "do jumper 5V/3.3V não importa.", 2.2, 'fill="#444"')

    # --- 2. Fios entre os LEDs (vista de trás) ---------------------------
    k = 1.1
    vx, vy = (w - LARGURA * k) / 2, y_b + 24
    texto(5, vy - 14, "2. Fios entre os LEDs: vista de trás (espelhada: a esquerda aqui é a direita do rosto)", 3.4,
          'font-weight="bold"')
    s.append(f'<rect x="{vx:.2f}" y="{vy:.2f}" width="{LARGURA * k:.2f}" height="{ALTURA * k:.2f}" '
             'fill="#f7efe3" stroke="#000" stroke-width="0.3"/>')
    perna = 1.27 * k
    centro = {id_: (vx + (LARGURA - x) * k, vy + y * k) for id_, _, x, y in leds}
    anodos = {i: (c[0], c[1] - perna) for i, c in centro.items()}
    catodos = {i: (c[0], c[1] + perna) for i, c in centro.items()}
    for id_, g, x, y in leds:
        c = centro[id_]
        s.append(f'<circle cx="{c[0]:.2f}" cy="{c[1]:.2f}" r="{DIAMETRO_LED / 2 * k:.2f}" '
                 f'fill="{COR_HEX[INFO[g][1]]}" fill-opacity="0.35" stroke="#555" stroke-width="0.15"/>')
    # GND entre os catodos
    ids_todos = [l[0] for l in leds]
    pts = [catodos[i] for i in ids_todos]
    for i, j in arvore(pts):
        fio(*pts[i], *pts[j], COR_GND, 0.45)
    # um fio por pino ligando os anodos (pernas longas) dos LEDs daquele pino
    y_base = vy + ALTURA * k
    saidas = []
    for indice, (pino, n, lado, ids) in enumerate(por_pino):
        pts = [anodos[i] for i in ids]
        curva = (indice % 3 + 1) * 2.5 * (1 if indice % 2 else -1)
        for i, j in arvore(pts):
            fio(*pts[i], *pts[j], COR_FIO[pino], 0.7, curva)
        saidas.append((pts, COR_FIO[pino], f"GPIO {pino}"))
    saidas.append((list(catodos.values()), COR_GND, "GND"))
    # cada fio sai pela borda (de cima ou de baixo) mais perto de um dos seus LEDs
    em_cima, embaixo = [], []
    for pts, cor, rotulo in saidas:
        x, y = min(pts, key=lambda q: min(q[1] - vy, y_base - q[1]))
        (em_cima if y - vy < y_base - y else embaixo).append((x, y, cor, rotulo))
    for grupo, sinal in ((em_cima, -1), (embaixo, 1)):
        grupo.sort(key=lambda sd: sd[0])
        borda = vy if sinal < 0 else y_base
        xs_rotulo = espalhar([g[0] for g in grupo], 12, vx + 5, vx + LARGURA * k - 5)
        y_txt = borda + sinal * (1.5 + len(grupo) * 1.1 + 2.2) + (2 if sinal > 0 else 0)
        for i, ((x, y, cor, rotulo), xr) in enumerate(zip(grupo, xs_rotulo)):
            y_nivel = borda + sinal * (1.5 + i * 1.1)
            y_ponta = borda + sinal * (1.5 + len(grupo) * 1.1 + 0.6)
            s.append(f'<path d="M{x:.2f} {y:.2f}V{y_nivel:.2f}H{xr:.2f}V{y_ponta:.2f}" fill="none" stroke="{cor}" '
                     f'stroke-width="{0.7 if rotulo != "GND" else 0.45}" stroke-linejoin="round"/>')
            texto(xr, y_txt, rotulo, 2.1, f'text-anchor="middle" fill="{cor}" font-weight="bold"')
    pino_de = {i: pino for pino, _, _, ids in por_pino for i in ids}
    for id_ in ids_todos:
        s.append(f'<circle cx="{anodos[id_][0]:.2f}" cy="{anodos[id_][1]:.2f}" r="0.55" '
                 f'fill="{COR_FIO[pino_de[id_]]}"/>')
        s.append(f'<circle cx="{catodos[id_][0]:.2f}" cy="{catodos[id_][1]:.2f}" r="0.55" fill="{COR_GND}"/>')
    y_nota = y_base + 15.5
    texto(5, y_nota, "Perna longa (anodo, ponto colorido) para cima: os LEDs do mesmo pino ficam ligados pelo fio "
          "da cor do pino, que vai até o resistor e o pino S.", 2.2, 'fill="#444"')
    texto(5, y_nota + 3.6, "Perna curta (catodo, ponto preto) no fio preto do GND, comum a todos os LEDs. "
          "Atrás da placa não vai nenhum resistor.", 2.2, 'fill="#444"')

    # --- 3. Tabela --------------------------------------------------------
    ty = y_nota + 15
    texto(5, ty - 5, "3. Tabela por pino", 3.4, 'font-weight="bold"')
    colunas = [(5, "Pino"), (22, "Grupo"), (66, "Cor"), (82, "Resistor"), (98, "LEDs"), (110, "mA"),
               (120, "LEDs (docs/leds.csv)")]
    for x, t in colunas:
        texto(x, ty, t, 2.4, 'font-weight="bold"')
    for i, (pino, n, lado, ids) in enumerate(por_pino):
        nome, cor, _ = INFO[n]
        y = ty + 5.5 + i * 5
        if i % 2 == 0:
            s.append(f'<rect x="4" y="{y - 3.6}" width="{w - 8}" height="5" fill="#f2f2f2"/>')
        s.append(f'<rect x="5" y="{y - 2.6}" width="2.6" height="2.6" fill="{COR_FIO[pino]}"/>')
        grupo = f"{n} {nome}" + (" (esq.)" if lado == "metade esquerda" else " (dir.)" if lado else "")
        r = resistor_do_pino(pino)
        valores = [f"GPIO {pino}", grupo, cor, f"{r} Ω", len(ids),
                   f"{corrente_do_pino_ma(cor, r):.0f}", " ".join(ids)]
        for (x, _), v in zip(colunas, valores):
            texto(x + (3.5 if x == 5 else 0), y, v, 2.1 if x == 120 else 2.4)
    total = len(leds)
    texto(5, ty + 5.5 + len(por_pino) * 5, f"Total: {total} LEDs, {len(por_pino)} resistores (um por pino), {len(por_pino)} pinos + GND. "
          "Esq./dir. = metade do rosto vista de frente.", 2.3, 'fill="#444"')
    h = ty + 5.5 + len(por_pino) * 5 + 6

    cab = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}mm" height="{h:.1f}mm" viewBox="0 0 {w} {h:.1f}" '
           'font-family="Arial, Helvetica, sans-serif">', f'<rect width="{w}" height="{h:.1f}" fill="#fff"/>']
    with open(caminho, "w", encoding="utf-8") as f:
        f.write("\n".join(cab + s + ["</svg>"]) + "\n")


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    leds = gerar()
    erros, pares = conferir(leds)
    print(f"{len(leds)} LEDs. Pares mais próximos (centro a centro):")
    for d, a, b in pares[:8]:
        print(f"  {d:5.2f} mm  {a[0]} g{a[1]} ({a[2]:.1f},{a[3]:.1f})  {b[0]} g{b[1]} ({b[2]:.1f},{b[3]:.1f})")
    for pino, n, _, ids in ligacoes_por_pino(leds):
        cor = INFO[n][1]
        r = resistor_do_pino(pino)
        ma = corrente_do_pino_ma(cor, r)
        print(f"  GPIO {pino:2d}: {len(ids):2d} LEDs {cor:8s} -> {r:4d} ohm, {ma:4.1f} mA ({ma / len(ids):.1f} mA/LED)")
        if ma > CORRENTE_MAX_PINO_MA:
            erros.append(f"GPIO {pino} (grupo {n}) passa de {CORRENTE_MAX_PINO_MA} mA")
    if erros:
        print("ERROS:\n  " + "\n  ".join(erros))
        sys.exit(1)
    pasta = os.path.dirname(os.path.abspath(__file__))
    escrever_csv(leds, os.path.join(pasta, "leds.csv"))
    escrever_mapa(leds, pares, os.path.join(pasta, "mapa-leds.svg"))
    escrever_furos(leds, os.path.join(pasta, "corte-frente.svg"))
    escrever_expressoes(leds, os.path.join(pasta, "expressoes.svg"))
    escrever_ligacoes(leds, os.path.join(pasta, "ligacoes.svg"))
    print(f"OK: nenhum par a menos de {FOLGA_MIN:g} mm entre bordas. Arquivos gerados em {pasta}")


if __name__ == "__main__":
    main()
