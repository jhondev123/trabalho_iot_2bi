# Armário Inteligente – site (Next.js)

Site simples para controlar os armários pelo MQTT:

- **Aluno** (`/`): escolhe o código do armário (ex.: A07), usa o armário (parear) e abre.
- **Admin** (`/admin`): vê todos os armários ao vivo, os eventos (token inválido, bloqueio) e abre qualquer armário.
- **Registros**: login, parear, abrir e abrir do admin ficam salvos em `data/registros.json`.

O navegador fala direto com o broker (MQTT por WebSocket), como no ESP32/Wokwi. O servidor Next só cuida do login, dos registros e do "abrir" do admin.

## Logins fixos

Definidos em código, em `lib/usuarios.ts`:

| Usuário | Senha | Vai para |
|---|---|---|
| `aluno` | `aluno123` | `/` |
| `admin` | `admin123` | `/admin` |

## Rodar no seu computador

```bash
npm install
npm run dev
```

Abra http://localhost:3000. (No `npm run dev` o `SESSION_SECRET` não é obrigatório; no `npm run build` + `npm start` é.) Com o Wokwi rodando (ESP32 conectado no `broker.hivemq.com`), o armário **A07** aparece sozinho.

## Subir na Vercel

1. Coloque esta pasta num repositório do GitHub (o `.gitignore` já ignora `node_modules`, `.next` e `.env`).
2. Na Vercel: **Add New > Project** e escolha o repositório.
3. Antes do Deploy, em **Environment Variables**, cadastre **`SESSION_SECRET`** com um texto longo e aleatório. Ela é obrigatória: sem ela o login dá erro, de propósito, porque com um segredo padrão (que está no código público) daria para forjar o cookie de admin.
4. Clique em **Deploy**.
5. Opcionais (modelo em `.env.example`):
   - `NEXT_PUBLIC_MQTT_PREFIXO`: prefixo dos tópicos, igual ao do `firmware/sketch.ino`. Depois de mudar, faça um novo deploy.
   - `ADMIN_SENHA_ARMARIO`: a `ADMIN_SENHA` do `firmware/sketch.ino` (padrão `admin123`).

Ou pela linha de comando: `npx vercel`.

## Importante: o JSON na Vercel

A Vercel não deixa gravar no disco do projeto. Lá, o site grava em `/tmp`, que funciona para a demo, mas **some quando a função dorme ou quando a Vercel usa outra instância**. Na sua máquina (`npm run dev`) o `data/registros.json` guarda tudo de verdade.

Se quiser registros permanentes na Vercel, o próximo passo é trocar só o arquivo `lib/registros.ts` por um serviço de armazenamento (Vercel Blob, Upstash Redis ou um Postgres). As telas não mudam.

## Como o MQTT é usado

Prefixo padrão: `academia/armario/<id>/...` (o mesmo do `firmware/sketch.ino`).

| Tópico | Quem publica | O site faz |
|---|---|---|
| `estado` (retido) | ESP32 | assina e mostra |
| `evento` | ESP32 | assina e mostra no admin e para o aluno |
| `online` (retido) | ESP32 (LWT) | assina; mostra "offline" |
| `cmd` | site | aluno: `parear` e `abrir` com token. Admin: `abrir` com a senha de admin, publicado pelo servidor |

O token do aluno é gerado no navegador (`crypto.randomUUID()`) e guardado no `localStorage` (`armario:A07`). Se o armário aparece Livre, o token antigo é apagado.

## Limitações conhecidas (versão 1)

- Broker público: qualquer pessoa que assine `.../cmd` enxerga o token e a senha de admin. Use um prefixo único do grupo.
- Os logins e a senha de admin ficam no código (só para demo).
- Sem leitura de QR ainda: o aluno digita o código do armário.
- Os eventos do admin (token inválido, bloqueio) aparecem ao vivo, mas não são salvos no JSON.

## Estrutura

```
app/            páginas (/, /login, /admin) e rotas da API (/api/...)
components/     telas do aluno e do admin
lib/            usuários, sessão, registros (JSON), configuração e hook do MQTT
data/           registros.json
```
