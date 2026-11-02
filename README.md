# Armário Inteligente – Trabalho de IoT (2º bimestre)

Armário de academia que abre pelo celular, sem chave. O aluno escolhe o armário no site, o armário fica "pareado" com o celular dele e só esse celular (ou a recepção) consegue abrir depois.

**Integrantes:** Jhonattan Curtarelli, Matheus Augusto, Matheus Mantovani
**Simulação no Wokwi:** https://wokwi.com/projects/479606793513063425

## Como funciona

| Estado | LED | O que acontece |
|---|---|---|
| Livre | verde | aceita pareamento pelo site |
| Pareado | amarelo piscando | trinco aberto por 30 s (2 min na versão final) para guardar as coisas |
| Ocupado | vermelho | porta fechou com objeto dentro; só o dono (token) ou o admin abrem |

- O ESP32 decide tudo sozinho (funciona sem internet): trava ao fechar a porta, usa o HC-SR04 para saber se tem objeto, volta para Livre se a janela expirar, religa o trinco se ninguém abrir a porta em 10 s e bloqueia comandos por 1 min depois de 3 tokens errados.
- A comunicação é por **MQTT** no broker público `broker.hivemq.com`, com mensagens em JSON.
- O site mostra o estado ao vivo para o aluno e para o admin.

## Estrutura

```
firmware/   código do ESP32 (sketch.ino), circuito do Wokwi (diagram.json) e bibliotecas
site/       site em Next.js: tela do aluno, painel do admin e registros em JSON
docs/       proposta, relatório de testes, teste ponta a ponta e roteiro de testes
```

## Firmware (Wokwi)

1. Abra o link do Wokwi acima (ou crie um projeto ESP32 e cole os 3 arquivos de `firmware/`).
2. Clique em play. O ESP32 conecta no Wi-Fi `Wokwi-GUEST` e no broker.
3. A chave deslizante simula a porta (reed switch) e o controle do HC-SR04 simula o objeto (menos de 26 cm = tem objeto).

Bibliotecas: ESP32Servo, PubSubClient, ArduinoJson.

## Site

Veja `site/README.md`. Resumo:

```bash
cd site
npm install
npm run dev
```

Logins de teste: `aluno` / `aluno123` e `admin` / `admin123`.

## Tópicos MQTT

Prefixo `academia/armario/<id>/` (ex.: `A07`).

| Tópico | Direção | Exemplo |
|---|---|---|
| `cmd` | site → ESP32 | `{"acao":"parear","token":"…"}`, `{"acao":"abrir","token":"…"}`, `{"acao":"abrir","admin":"…"}` |
| `estado` (retido) | ESP32 → site | `{"id":"A07","estado":"ocupado","porta":"fechada","travado":true,"distancia_cm":10}` |
| `evento` | ESP32 → site | `{"id":"A07","evento":"token_invalido","tentativas":2}`, `porta_forcada`, `bloqueado` |
| `online` (retido, LWT) | ESP32 → site | `{"online":false}` |

## Documentos

- `docs/proposta_iot.pdf`: proposta entregue ao professor.
- `docs/relatorio_testes.pdf`: 11 testes do firmware v0 no Wokwi, com os bugs encontrados.
- `docs/teste_ponta_a_ponta.pdf`: teste do site + broker + Wokwi com o firmware v1 (corrigido).
- `docs/roteiro_testes.md`: passo a passo para repetir os testes.

## Limitações conhecidas

- Broker público e sem senha (projeto acadêmico): quem assinar os tópicos vê os comandos.
- O estado do armário fica só na memória: se o ESP32 reiniciar, ele volta como Livre.
- O HC-SR04 não detecta objetos muito pequenos nem tecidos macios.
- O armário não fecha a porta sozinho.
