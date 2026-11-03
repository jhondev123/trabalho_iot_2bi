# Revisão técnica – Armário Inteligente (02/11/2026)

## Veredito
O conjunto cobre R1–R5 e as 4 camadas, e o protocolo do site bate com o firmware (tópicos, campos JSON, comandos e senha padrão). O site compila: `tsc --noEmit` e `next build` passaram sem erro. O maior risco para 23/11 é o que ainda não foi feito, não o código. O v1 nunca foi compilado nem testado no Wokwi, o site nunca rodou junto com o ESP32, e ainda faltam relatório final, slides e vídeo. Os problemas abaixo se corrigem em poucas linhas cada.

## Problemas
**Média**
1. `work/sketch_corrigido.ino:159-161`: o B4 foi corrigido só em parte. Com o armário OCUPADO, um `{"acao":"abrir"}` sem token ainda conta como token_invalido, então o T8 do relatório vai falhar de novo no reteste. Correção: `else if (estado == OCUPADO && (strlen(tk) > 0 || strlen(adm) > 0))`.
2. `armario-site/lib/useArmarios.ts:37-47`: se alguém publicar `null` (JSON válido) em `.../online` ou `.../evento`, o handler lança TypeError e o mqtt.js para de entregar mensagens, mas a tela continua mostrando "Conectado". Testei em Node com o mesmo mqtt.js 5.16.1 e um broker local: depois do erro chegaram 0 mensagens, contra 3 no teste de controle. Que o navegador se comporta igual é suposição. Como o broker é público e a mensagem fica retida, o site trava para todo mundo. Correção: `if (!dados || typeof dados !== "object") return;`.
3. Credenciais no código:
   - `lib/sessao.ts:15` usa um segredo fixo quando falta `SESSION_SECRET`, e o README (`:33`) diz que essa variável é "Opcional". Testei: um cookie de admin forjado com esse segredo foi aceito em `/api/registros`.
   - `lib/usuarios.ts:7-8` e `sketch_corrigido.ino:19` têm as senhas escritas no código.
   - `admin123` é a senha do login do site e também a do ESP32, e essa segunda trafega aberta no broker.

   Correção: no site, ler as senhas do `.env` e exigir `SESSION_SECRET` em produção; no firmware, criar `secrets.h` + `secrets.example.h`; usar senhas diferentes. Isso também conta para o bônus.
4. B6 continua aberto (`sketch_corrigido.ino:239-240`). Se o ESP32 reinicia com algo dentro, ele volta LIVRE, o site apaga o token do aluno (`PainelAluno.tsx:39`) e qualquer pessoa consegue parear e abrir. Correção de uma linha no `setup()`: `if (distancia < LIMITE_CM) estado = OCUPADO;`. Sem token, só o admin abre.
5. A proposta diz coisas diferentes do que foi feito (`work/gen.py`):
   - `:55,73`: HiveMQ Cloud com senha e TLS.
   - `:65`: GitHub Pages, que não roda as rotas `/api`.
   - `:60`: evento "porta aberta por muito tempo", que não existe no firmware.
   - `:63`: "sem servidor próprio".
   - Promete leitura de QR e não cita o tópico `online`.

   Correção: incluir no relatório final uma seção "Mudanças desde a proposta", com o motivo de cada uma.

**Baixa**
6. `sketch_corrigido.ino:145`: o bloqueio também barra o admin. Basta alguém mandar 3 comandos errados por minuto para ninguém abrir, nem a recepção. Correção: calcular `ehAdmin` antes da checagem e usar `if (millis() < bloqueadoAte && !ehAdmin) return;`. Detalhe em `:131-134`: o evento `bloqueado` sai com `tentativas:0`.
7. `sketch_corrigido.ino:124`: parear um armário LIVRE com a porta já aberta (por exemplo, depois que o admin abriu) não marca `portaAbriuNaJanela`. A janela expira e, quando a porta fecha com objeto dentro, o armário fica LIVRE. Correção: `portaAbriuNaJanela = !portaFechada;`.
8. `PainelAluno.tsx:107`: o botão "Abrir armário" aparece mesmo com o armário offline e mostra "enviado", mas o comando se perde porque não é retido. Correção: somar `&& armario?.online !== false` à condição.
9. `app/login/page.tsx:17` e `PainelAdmin.tsx:29`: `fetch` sem `try/finally`. Se a rede falhar, o botão fica preso em "Entrando...".
10. `app/api/admin/abrir/route.ts:23,27-29`: o servidor conecta por TCP 1883, sem TLS, a partir da Vercel (que funcione é suposição, não testei), e o `catch` não fecha o cliente. Sugestão: `mqtts://broker.hivemq.com:8883`.
11. `.gitignore`: faltam `.env` e `*.tsbuildinfo`. O `tsc` gera esse arquivo na raiz; apaguei o que eu gerei. Também em `lib/registros.ts:41-44`, o registro é lido, alterado e gravado sem trava, então duas gravações simultâneas podem perder dados. A perda dos registros entre instâncias da Vercel já está documentada.
12. `work/testes.md:115`: com `setKeepAlive(30)`, o broker só marca o armário como offline depois de cerca de 45 s (1,5 × keepalive). O roteiro ainda diz 15–30 s.

## Escopo e entrega
**Falta, obrigatório para 23/11:**
- Colar no Wokwi o v1 com as correções 1 e 4, compilar e repetir T1–T11. Isso gera a evidência de "falha e correção" (o B1 é o melhor exemplo) e o link do Wokwi passa a apontar para o v1.
- Fazer um teste ponta a ponta site ↔ Wokwi.
- Relatório em PDF de 8–15 páginas com: as 4 camadas, R3 justificado por alcance, consumo, taxa e custo (a proposta só diz "leve"), um payload real capturado, os testes e o uso de IA.
- Repositório único: `firmware/` (sketch, `diagram.json`, `libraries.txt`), `site/` e um README na raiz. Hoje o README cita um `sketch.ino` que não está na pasta.
- Vídeo de 3–5 min. O que existe tem 92 s e só mostra testes.
- Slides.
- Um jeito de demonstrar R2, por exemplo uma cópia do projeto com o SSID errado, mostrando que trava, LEDs e janela continuam funcionando.

**Bônus barato:**
- Checklist OWASP IoT Top 10, que é só um documento.
- A correção 3 como "correção a partir do checklist" (item I1, senhas fixas).
- Autenticação e criptografia ficam parciais por causa da decisão de usar broker público; o navegador já usa wss com TLS. Vale registrar isso com honestidade no item I7.

**Cortar:** leitura de QR, persistência na Vercel, token na flash (a regra de boot da correção 4 basta), B7 e HiveMQ Cloud.

## O que está bom
- A arquitetura é enxuta e segue KISS: regras locais no ESP32, reconexão que não bloqueia, LWT e estado retidos.
- No site:
  - Nenhum segredo vai para o navegador; conferi o `.next/static`.
  - O cookie é assinado com HMAC, `httpOnly` e `SameSite=Lax`.
  - Código do armário e papéis são validados no servidor. Testei as respostas 401, 403, 400 e o redirecionamento 307.
- No firmware v1:
  - Resolve B1, B2, B3, B5 e B8.
  - A ordem em `regras()` está certa e `destravarSilencioso` evita a publicação dupla no parear.
  - Religar o trinco com `estado != PAREADO` cobre o caso do admin abrindo um armário LIVRE.
  - Na revisão estática não achei erro de compilação: o protótipo vem antes do uso e `setKeepAlive` é chamado antes do `connect`. Publicar dentro do callback já funcionava no v0.
- Tópicos, campos JSON, comandos, tempos (30 s / 10 s / 1 min) e logins são os mesmos no firmware, no site, no README e no `testes.md`.
