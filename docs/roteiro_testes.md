# Roteiro de testes – Armário Inteligente (Wokwi + MQTT)

## Preparação

1. Abra o projeto no Wokwi e clique em play. Espere o ESP32 conectar.
2. Abra o cliente web da HiveMQ: https://www.hivemq.com/demos/websocket-client/ e confira os campos de **Connection** antes de clicar em **Connect**:
   - **Host:** `broker.hivemq.com` (o padrão `mqtt-dashboard.com` é o mesmo broker público)
   - **Port:** `8884`
   - **SSL:** marcado
   - **Username:** deixe em branco
   - **Password:** deixe em branco

   O broker público não tem usuário nem senha, então nada é preenchido ali. A senha `admin123` é a do administrador e só vai dentro do JSON do teste 7.
3. Em **Subscriptions**, assine: `academia/armario/A07/#`
4. Para mandar comandos, use o campo **Publish**:
   - **Topic:** `academia/armario/A07/cmd`
   - **Message:** o JSON do teste (precisa ser JSON, texto solto como `parear` é ignorado)

**Estado inicial esperado:** `{"id":"A07","estado":"livre","porta":"fechada","travado":true,...}` e LED verde.

**No Wokwi:**
- **Porta:** a chave deslizante abre e fecha (o campo `porta` do estado mostra `aberta` ou `fechada`).
- **Objeto:** o controle de distância do HC-SR04 (clique no sensor). Menos de 26 cm = tem objeto. 30 cm = vazio.

**Dicas:**
- O armário publica o `estado` a cada 10 s e sempre que algo muda. Confira a última mensagem do tópico `estado`.
- Os tempos (30 s, 10 s, 1 min) contam no relógio da simulação. Se ela estiver lenta, demora mais em tempo real.

---

## Teste 1 – Guardar um objeto (LIVRE → PAREADO → OCUPADO)

1. Publique:
   ```json
   {"acao":"parear","token":"teste123"}
   ```
   **Esperado:** `estado:"pareado"`, `travado:false`, LED amarelo piscando.
2. Em até 30 s, clique na chave (abre a porta). **Esperado:** `porta:"aberta"`.
3. Ponha a distância em **10 cm**.
4. Clique na chave (fecha a porta). **Esperado:** `estado:"ocupado"`, `travado:true`, LED vermelho.

## Teste 2 – Abrir como dono e guardar de novo (continua OCUPADO)

1. Publique:
   ```json
   {"acao":"abrir","token":"teste123"}
   ```
   **Esperado:** `travado:false`.
2. Clique na chave (abre a porta). Deixe a distância em **10 cm**.
3. Clique na chave (fecha a porta). **Esperado:** continua `ocupado` e `travado:true`.

## Teste 3 – Abrir como dono e esvaziar (OCUPADO → LIVRE)

1. Publique:
   ```json
   {"acao":"abrir","token":"teste123"}
   ```
2. Abra a porta e ponha a distância em **30 cm** (vazio).
3. Feche a porta. **Esperado:** `estado:"livre"`, LED verde. O token foi apagado: o `teste123` não abre mais nada.

## Teste 4 – Pareou e não abriu a porta (volta para LIVRE)

1. Publique:
   ```json
   {"acao":"parear","token":"teste123"}
   ```
2. Não mexa em nada por 30 s. **Esperado:** volta sozinho para `livre` e `travado:true`.

## Teste 5 – Pareou, abriu e fechou vazio (volta para LIVRE)

1. Publique o `parear` do teste 4.
2. Abra a porta, deixe a distância em **30 cm** e feche. **Esperado:** `livre`.

## Teste 6 – Token errado e bloqueio

Deixe o armário `ocupado` (repita o teste 1). Publique **3 vezes**:

```json
{"acao":"abrir","token":"errado"}
```

**Esperado no tópico `evento`:**
- 1ª e 2ª vez: `{"evento":"token_invalido","tentativas":1}` e depois `2`.
- 3ª vez: `token_invalido` com `tentativas:3` e depois `{"evento":"bloqueado",...}`.

Logo depois, publique o token correto:

```json
{"acao":"abrir","token":"teste123"}
```

**Esperado:** nada acontece (`travado` continua `true`). Os comandos são ignorados por 1 min.

## Teste 7 – Admin abre

Com o armário `ocupado`, publique:

```json
{"acao":"abrir","admin":"admin123"}
```

**Esperado:** `travado:false`. Se a porta não abrir em 10 s, ele trava de novo.

## Teste 8 – Comandos que devem ser ignorados

- Com o armário `ocupado`, publique um novo `parear`:
  ```json
  {"acao":"parear","token":"outro"}
  ```
  **Esperado:** nada muda.
- Publique texto que não é JSON, como `parear`. **Esperado:** nada muda.

## Teste 9 – Armário offline

Pare a simulação no Wokwi e espere 15 a 30 s. **Esperado:** `{"online":false}` no tópico `online` (aviso automático de queda). Ao dar play de novo, volta `{"online":true}`.

---

## Resumo dos comandos

| Ação | Tópico | Mensagem |
|---|---|---|
| Parear | `academia/armario/A07/cmd` | `{"acao":"parear","token":"teste123"}` |
| Abrir (dono) | `academia/armario/A07/cmd` | `{"acao":"abrir","token":"teste123"}` |
| Abrir (admin) | `academia/armario/A07/cmd` | `{"acao":"abrir","admin":"admin123"}` |
| Token errado | `academia/armario/A07/cmd` | `{"acao":"abrir","token":"errado"}` |

## Resumo dos estados

| Estado | LED | Quando |
|---|---|---|
| `livre` | verde | Sem dono, aceita pareamento |
| `pareado` | amarelo piscando | Destravado, esperando guardar (30 s) |
| `ocupado` | vermelho | Trancado com objeto, só abre com token ou admin |
