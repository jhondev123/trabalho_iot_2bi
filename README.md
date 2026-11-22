# Armário Inteligente

Trabalho de IoT do 2º bimestre. É um armário de academia que abre pelo celular em vez de chave: o aluno escolhe o armário no site, guarda as coisas e depois só o celular dele (ou a recepção) consegue abrir.

Grupo: Jhonattan Curtarelli, Matheus Augusto e Matheus Mantovani.

## Como funciona

O armário tem três estados:

- Livre (LED verde): pode ser usado.
- Pareado (LED amarelo piscando): o trinco fica aberto um tempo para o aluno guardar as coisas.
- Ocupado (LED vermelho): a porta fechou com algo dentro. Só o dono ou o admin abrem.

Quem decide é o ESP32, então ele continua funcionando sem internet. O sensor ultrassônico (HC-SR04) detecta se tem objeto dentro e, depois de 3 tentativas com token errado, o armário bloqueia por 1 minuto.

A comunicação com o site é por MQTT, usando o broker público `broker.hivemq.com`.

## Pastas

- `firmware/`: código do ESP32, circuito do Wokwi e bibliotecas
- `site/`: site em Next.js com a tela do aluno e o painel do admin

## Rodando o firmware no Wokwi

1. Crie um projeto ESP32 no Wokwi e cole `sketch.ino`, `diagram.json` e `libraries.txt` da pasta `firmware/`.
2. Crie uma aba `secrets.h` com o conteúdo de `firmware/secrets.example.h` (sem esse arquivo não compila).
3. Dê play. O ESP32 conecta no `Wokwi-GUEST` e no broker.

A chave deslizante faz o papel da porta (esquerda fechada, direita aberta) e o controle do HC-SR04 simula o objeto (menos de 26 cm conta como ocupado). O LCD e o Monitor Serial mostram o que está acontecendo.

Bibliotecas: ESP32Servo, PubSubClient, ArduinoJson e LiquidCrystal I2C.

## Rodando o site

```bash
cd site
npm install
npm run dev
```

Logins de teste: `aluno` / `aluno123` e `admin` / `admin123`. Mais detalhes em `site/README.md`.

## Tópicos MQTT

Prefixo: `academia/armario/<id>/` (por exemplo `A07`).

- `cmd`: comandos do site para o armário (parear, abrir)
- `estado`: estado atual do armário (retido)
- `evento`: avisos como token inválido, porta forçada e bloqueio
- `online`: se o armário está conectado (LWT)

## Limitações

- O broker é público e sem senha, então qualquer um que assinar os tópicos vê os comandos.
- O estado fica só na memória. Se o ESP32 reiniciar com algo dentro, só o admin consegue abrir.
- O HC-SR04 não pega objetos muito pequenos ou tecidos macios.
