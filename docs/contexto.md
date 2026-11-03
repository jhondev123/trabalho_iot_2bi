# Contexto do projeto: Armário Inteligente (Trabalho Final de IoT)

Atualizado em 02/11/2026. Este arquivo serve para quem for continuar o projeto sem ter acompanhado o desenvolvimento. Leia na ordem: links, o que falta, depois o desenho técnico.

**Prazo final: 23/11/2026** (entrega + apresentação ao vivo). Sem tolerância: atraso = nota zero.

---

## 1. Links e onde está cada coisa

| O quê | Onde |
|---|---|
| Projeto no Wokwi (firmware v1 + diagrama) | https://wokwi.com/projects/479606793513063425 |
| Repositório GitHub (público) | https://github.com/jhondev123/trabalho_iot_2bi |
| Apresentação do desenho (para o grupo) | https://claude.ai/artifact/2PA1zNpDiFiBzd3W3xBgJk |
| Broker MQTT usado | `broker.hivemq.com` (público, sem senha). TCP 1883 no ESP32; WebSocket seguro `wss://broker.hivemq.com:8884/mqtt` no navegador |
| Cliente web para testar MQTT | https://www.hivemq.com/demos/websocket-client/ |

O repositório é público e tem `docs/`, `firmware/`, `site/`, `README.md` e `.gitignore`. O `firmware/sketch.ino` do repo **é a v1** (corrigida depois dos testes da v0), e o `site/` do repo é a versão mais atual.

O link do Wokwi precisa estar com **compartilhamento ativo** (a disciplina exige). Abra em janela anônima para confirmar que seu colega consegue ver.

**Site na Vercel:** o deploy estava planejado, mas não há registro de que tenha sido feito. Se existir URL, adicione aqui.

**Integrantes:** Jhonattan Curtarelli, Matheus Augusto, Matheus Mantovani.
**Disciplina:** IoT, Prof. Marcos Augusto Campagnaro Mucelini, 2º semestre/2026.

### Arquivos do repositório

| Arquivo | O que é |
|---|---|
| `README.md` | Visão geral, estados, tópicos MQTT e como rodar |
| `firmware/sketch.ino` | Firmware v1 do ESP32 (corrige parte dos bugs; ver seção 3) |
| `firmware/diagram.json`, `firmware/libraries.txt` | Circuito e bibliotecas do Wokwi |
| `site/` | Site em Next.js (ver `site/README.md`) |
| `docs/proposta_iot.pdf` | Proposta do projeto (2 páginas) |
| `docs/roteiro_testes.md` | Roteiro dos 9 testes manuais via MQTT |
| `docs/relatorio_testes.pdf` | 11 casos de teste no Wokwi com o firmware v0 (achou os bugs B1–B8) |
| `docs/teste_ponta_a_ponta.pdf` | Site + broker + Wokwi v1, 9 passos, todos OK |
| `docs/revisao_tecnica.md` | Revisão técnica de 02/11 (lista de problemas com a correção sugerida) |
| `docs/contexto.md` | Este arquivo |

Na `revisao_tecnica.md`, os caminhos e números de linha são os da pasta de trabalho da época (`work/sketch_corrigido.ino`, `armario-site/...`). No repo, o firmware é `firmware/sketch.ino` e o site é `site/`. Alguns itens do site já foram corrigidos (ver seção 3).

**Só com o Jhonattan (fora do repo):** `proposta_iot.docx` (a proposta editável) e `testes_video.mp4` (vídeo de testes de 92 s; não serve como vídeo final).

---

## 2. Requisitos da disciplina (resumo)

O sistema precisa passar pelas 4 camadas (Percepção → Rede → Processamento → Aplicação).

- **R1** ≥ 1 sensor e ≥ 1 atuador; o atuador reage a uma leitura, não só a comando manual.
- **R2** Regra de decisão executada no próprio dispositivo, que continua funcionando sem internet.
- **R3** Comunicação sem fio (Wi-Fi/Bluetooth/MQTT), justificando alcance, consumo, taxa de dados e custo.
- **R4** Payload estruturado, documentado no relatório com exemplo **real**.
- **R5** Estado visível sem Monitor Serial.
- **Montagem:** Wokwi vale igual ao físico. Tem que rodar ao vivo na apresentação.
- **Avaliação:** funcionamento 35%, arquitetura 20%, código 20%, interface 10%, apresentação 15%.
- **Defesa individual:** qualquer integrante pode ser perguntado sobre qualquer parte do código. Todos precisam entender tudo.
- **Adicional de segurança (até +10%)**, em seção própria do relatório:
  - credenciais fora do repo, com arquivo de exemplo: até 3%;
  - checklist OWASP IoT Top 10: até 3%;
  - pelo menos uma correção feita a partir do checklist: até 2%;
  - autenticação ou criptografia na comunicação: até 2%.
- **Penalidade:** não funcionar na demo limita "Funcionamento" a 10,5/35.
- **Uso de IA:** permitido, mas o relatório declara o que foi feito com IA e como o grupo verificou.

---

## 3. Estado atual

### Feito

- **Proposta** escrita (`proposta_iot.pdf`). A especificação pede 1 página e a nossa tem 2. Não encontrei registro de aprovação do professor; **confirme isso**, porque a aprovação deveria vir antes da montagem.
- **Decisões:** só Wokwi (sem hardware físico), broker público, cenário de academia (ver seção 4).
- **Firmware v0** completo, testado no Wokwi (11 casos, `docs/relatorio_testes.pdf`). A v0 não está no repo; o relatório descreve os bugs que ela tinha.
- **Firmware v1** (`firmware/sketch.ino`, também salvo no Wokwi). Compila e corrige os bugs B1, B2, B3, B5 e B8 do relatório de testes. O teste ponta a ponta foi feito com ele.
- **Site** em Next.js (`site/`) funcionando: login fixo, painel do aluno, painel do admin, registros em JSON, botão "abrir" do admin publicado pelo servidor. `tsc` e `next build` passaram na revisão.
- **Teste ponta a ponta** (site + broker + Wokwi v1): 9 passos, todos OK.
- **Revisão técnica** (`docs/revisao_tecnica.md`). Do que ela apontou no site, já estão aplicados em `site/`: `SESSION_SECRET` obrigatório em produção, ignorar mensagem MQTT que não seja objeto JSON, esconder "Abrir" com armário offline, `try/finally` no login, `.env` e `*.tsbuildinfo` no `.gitignore`.

### Falta (ordem sugerida)

**Obrigatório para 23/11**

1. **Terminar o firmware v1** em `firmware/sketch.ino` (poucas linhas cada):
   - **B6:** ao reiniciar com algo dentro, o ESP volta LIVRE e qualquer um consegue parear. Em `setup()`, depois de ler a distância: `if (distancia < LIMITE_CM) estado = OCUPADO;`.
   - **B4 parcial:** `{"acao":"abrir"}` sem token ainda conta como token inválido. Trocar a condição por `else if (estado == OCUPADO && (strlen(tk) > 0 || strlen(adm) > 0))`.
   - **Admin barrado pelo bloqueio:** `if (millis() < bloqueadoAte) return;` também barra o admin. Calcular `ehAdmin` antes e usar `if (millis() < bloqueadoAte && !ehAdmin) return;`.
   - Parear com a porta já aberta não marca `portaAbriuNaJanela`. Correção: `portaAbriuNaJanela = !portaFechada;` (prioridade baixa).
   - O evento `bloqueado` sai com `tentativas:0` (detalhe).
2. **Colar o v1 corrigido no Wokwi, compilar e repetir os testes T1–T11.** Isso gera a evidência de "falha e correção" que o relatório exige e faz o link do Wokwi apontar para a versão final.
3. **Demonstração de R2 (funciona sem internet):** copiar o projeto Wokwi com o SSID errado e mostrar que a regra local continua (travar, LEDs, janela de pareamento).
4. **Relatório PDF (8–15 páginas)** com: capa; problema e solução; arquitetura (diagrama das 4 camadas); hardware (esquemático, lista de componentes, tabela de pinos); software; comunicação (R3 justificado por alcance, consumo, taxa de dados e custo, mais **payload real capturado**); testes (incluindo uma falha e sua correção); conclusão; uso de IA; referências. Incluir a seção **"Mudanças desde a proposta"** (ver seção 6).
5. **Repositório único:** a estrutura já existe (`firmware/`, `site/`, `docs/` e README na raiz). Falta deixar o README final: incluir o `firmware/secrets.example.h` quando o firmware passar a usar o `secrets.h` (ele já está no `.gitignore`), e o link do vídeo.
6. **Vídeo de 3–5 min** com o sistema funcionando (o existente tem 92 s e só mostra testes).
7. **Slides** para até 10 min.
8. **Entrega:** um integrante envia no Google Classroom com o nome de todos.

**Bônus de segurança (barato)**

- **Checklist OWASP IoT Top 10** com os riscos do próprio projeto (é só um documento). Registrar com honestidade que autenticação e criptografia ficam parciais por causa do broker público (o navegador já usa `wss`/TLS).
- **Credenciais fora do código:** o firmware tem `ADMIN_SENHA` escrita no sketch; criar `firmware/secrets.h` (já está no `.gitignore` da raiz) + `firmware/secrets.example.h`. No site, `lib/usuarios.ts` tem os logins fixos no código; ler de variável de ambiente. Usar senhas **diferentes** no site e no ESP32 (hoje as duas são `admin123`, e a do ESP trafega aberta no broker).
- Isso serve também como a "correção feita a partir do checklist".

**Opcional / baixa prioridade**

- O servidor conecta ao broker por `mqtt://...:1883` (sem TLS). Sugestão: `mqtts://broker.hivemq.com:8883`. Que funcione a partir da Vercel é suposição, ninguém testou.
- `lib/registros.ts` lê, altera e grava sem trava (duas gravações simultâneas podem perder dados).
- Registros na Vercel ficam em `/tmp` e não são permanentes (já documentado no README do site).
- Em `docs/roteiro_testes.md`, o teste 9 diz "15–30 s" para o armário aparecer offline; com `setKeepAlive(30)` o broker só marca offline em cerca de 45 s.

**Cortado de propósito:** leitura de QR pela câmera, persistência real dos registros na Vercel, token na flash do ESP32, HiveMQ Cloud com senha/TLS.

---

## 4. Desenho técnico

### Cenário

Armário de academia, uma pessoa só (o aluno guarda e retira). O aluno abre o site, digita o código do armário (ex.: `A07`) e o celular fica **pareado**. Só o celular pareado abre, ou o admin (recepção).

### Estados e LEDs

| Estado | LED | Comportamento |
|---|---|---|
| `livre` | verde | Aceita pareamento |
| `pareado` | amarelo piscando | Destravado, janela de 2 min (30 s na demo). Fechou com objeto → `ocupado`. Fechou vazio ou 2 min sem abrir a porta → trava e volta a `livre` (apaga o token) |
| `ocupado` | vermelho | Trancado, só abre com o token do dono ou a senha do admin. Abriu, esvaziou e fechou → `livre` (apaga o token). Abriu e fechou com objeto → continua `ocupado` |

### Hardware (simulado no Wokwi)

ESP32 DevKit C v4, sensor ultrassônico HC-SR04 (no teto, mede a distância até o fundo e decide se há objeto), servo SG90 (trinco), chave deslizante (simula o reed switch da porta), 3 LEDs com resistor de 220 Ω.

| Função | Pino |
|---|---|
| Servo | 13 |
| Porta (chave, `INPUT_PULLUP`, LOW = fechada) | 27 |
| HC-SR04 TRIG / ECHO | 32 / 33 |
| LED verde / amarelo / vermelho | 25 / 26 / 19 |

No Wokwi: a chave abre e fecha a porta; o campo de distância do HC-SR04 (clicar no sensor) simula o objeto. Menos de 26 cm = há objeto; 30 cm = vazio.

Bibliotecas: `ESP32Servo`, `PubSubClient`, `ArduinoJson`. Rede do Wokwi: SSID `Wokwi-GUEST`, sem senha.

### Regras locais no ESP32 (R2)

- Porta fechou → trava sempre. Ao fechar: distância < 26 cm → `ocupado`; fundo vazio → `livre`.
- `pareado` por 2 min sem abrir a porta → trava e `livre`.
- Destravou (dono/admin) e a porta não abriu em 10 s → trava de novo.
- 3 tokens errados → ignora comandos de abrir por 1 min.
- Reconexão Wi-Fi/MQTT não bloqueante (a cada 5 s); as regras continuam rodando sem rede.
- Na demo os tempos são curtos (30 s no lugar de 2 min). Eles contam no relógio da simulação.

### MQTT (prefixo `academia/armario/A07/`)

| Tópico | Sentido | Retain | Exemplo de payload |
|---|---|---|---|
| `cmd` | site/admin → ESP | não | `{"acao":"parear","token":"…"}`, `{"acao":"abrir","token":"…"}`, `{"acao":"abrir","admin":"…"}` |
| `estado` | ESP → site e painel | sim | `{"id":"A07","estado":"ocupado","porta":"fechada","travado":true,"distancia_cm":11.8}` |
| `evento` | ESP → site e painel | não | `{"id":"A07","evento":"token_invalido","tentativas":2}` (também `bloqueado` e `porta_forcada`) |
| `online` | ESP → site | sim | `{"online":true}`; `{"online":false}` é a última vontade (LWT) |

O painel admin assina `academia/armario/+/estado`. O ESP publica o estado a cada 10 s e sempre que algo muda. JSON inválido é ignorado.

**Atenção:** o broker é público. Qualquer pessoa que assine `.../cmd` vê o token e a senha de admin, e o prefixo `academia/armario` pode colidir com outros grupos. Defina um prefixo único do grupo (variável `NEXT_PUBLIC_MQTT_PREFIXO` no site e o prefixo no firmware, que precisam ser iguais).

### Pareamento

Sem servidor. O token é gerado no navegador (`crypto.randomUUID()`) e salvo no `localStorage` (`armario:A07`); o ESP guarda o mesmo token só na RAM. Perdeu o token → o admin abre. Se a página vê o armário `livre`, apaga o token velho.

### Site (Next.js)

- **Aluno** (`/`): escolhe o código, pareia, abre.
- **Admin** (`/admin`): tabela dos armários ao vivo, eventos (token inválido, bloqueio), botão de abrir. O "abrir" do admin é publicado **pelo servidor**, para a senha do ESP não ir ao navegador.
- **Registros** (login, parear, abrir, admin_abrir) em `data/registros.json`. Eventos ao vivo não são salvos.
- **Logins fixos de demonstração:** `aluno` / `aluno123` e `admin` / `admin123`.
- O navegador fala direto com o broker por WebSocket; o servidor só cuida de login, registros e "abrir" do admin.
- Sessão: cookie assinado com HMAC (`httpOnly`, `SameSite=Lax`), 8 h.

---

## 5. Como rodar e testar

**Site (local)**

```bash
cd site
npm install
npm run dev        # http://localhost:3000
```

No `npm run dev`, `SESSION_SECRET` é opcional. Em produção (`npm run build` + `npm start`, ou Vercel) ele é **obrigatório**: copie `.env.example` para `.env.local` e defina um texto longo e aleatório.

**Firmware:** abra o projeto no Wokwi (link na seção 1), clique em play e espere o ESP32 conectar. O armário `A07` aparece sozinho no site se o prefixo for o mesmo.

**Testes manuais:** siga `docs/roteiro_testes.md`. Resumo da configuração do cliente HiveMQ: host `broker.hivemq.com`, porta `8884`, SSL marcado, usuário e senha em branco, assinar `academia/armario/A07/#`, publicar comandos em `academia/armario/A07/cmd` (precisa ser JSON). Estado inicial esperado: `livre`, LED verde.

**Deploy na Vercel:** importar o repositório e definir o **Root Directory** como `site` (o site fica numa subpasta), cadastrar `SESSION_SECRET` em Environment Variables antes do deploy. Opcionais: `NEXT_PUBLIC_MQTT_PREFIXO`, `ADMIN_SENHA_ARMARIO`, `NEXT_PUBLIC_MQTT_WS_URL`, `MQTT_URL_SERVIDOR`.

---

## 6. Decisões e limitações (material para o relatório)

### Mudanças desde a proposta (incluir no relatório, com o motivo de cada uma)

A proposta diz coisas diferentes do que foi feito:

- Prometia **HiveMQ Cloud com usuário/senha e TLS**; usamos o **broker público**. Motivo: o plano gratuito do HiveMQ Cloud não aparecia e o Starter é teste de 15 dias; é um projeto acadêmico.
- Prometia hospedar em **GitHub Pages**, que não roda as rotas `/api`; o site foi para Next.js na **Vercel**.
- Citava o evento "porta aberta por muito tempo", que **não existe** no firmware.
- Dizia "sem servidor próprio"; o site tem um servidor Next.js (login, registros, "abrir" do admin).
- Prometia leitura de QR e não citava o tópico `online`; hoje o aluno digita o código e o tópico `online` existe.

### Limitações assumidas

- O HC-SR04 no teto não detecta objetos pequenos (moeda, chave) nem tecido macio.
- Sem fechamento automático da porta (decisão do grupo, complexidade desnecessária): se a porta ficar aberta, o armário fica destravado e só trava quando fecha. O painel do admin mostra a porta aberta.
- Broker público sem senha: comandos e token são visíveis para quem assinar o tópico.
- Logins e senha de admin fixos no código (demo).
- Registros do site não são permanentes na Vercel.
- Reed switch simulado por chave deslizante (só Wokwi).

### Pontos fortes para destacar

- Regras locais no ESP32, reconexão que não bloqueia, LWT e estado retido (R2 atendido).
- Segredos do site não vão para o navegador; comandos e papéis são validados no servidor.
- Os tópicos, campos JSON, comandos e tempos são os mesmos no firmware, no site e nos testes.

### Preparação para a defesa

A nota de defesa é individual e qualquer um pode ser perguntado sobre qualquer parte. Antes da apresentação, cada integrante deve conseguir explicar: a máquina de estados do firmware, a função `regras()`, o fluxo de pareamento (token no navegador × token no ESP), os tópicos MQTT e por que o broker é público.
