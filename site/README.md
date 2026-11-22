# Site do Armário Inteligente

Site em Next.js para usar e acompanhar os armários.

- `/`: tela do aluno. Digita o código do armário (ex.: A07), pareia e abre.
- `/admin`: painel da recepção. Mostra todos os armários e os eventos ao vivo e pode abrir qualquer um.

O navegador conversa direto com o broker MQTT por WebSocket. O servidor só cuida do login, dos registros (`data/registros.json`) e do comando de abrir do admin.

## Rodando

```bash
npm install
npm run dev
```

Abra http://localhost:3000. Com o Wokwi rodando, o armário A07 aparece sozinho.

Logins de teste: `aluno` / `aluno123` e `admin` / `admin123`.

## Variáveis de ambiente

Para rodar em produção (`npm run build` + `npm start` ou Vercel), copie `.env.example` para `.env.local` e preencha:

- `SESSION_SECRET`: texto aleatório e longo (obrigatório)
- `SENHA_ALUNO` e `SENHA_ADMIN`: senhas dos dois logins
- `NEXT_PUBLIC_MQTT_PREFIXO`: tem que ser igual ao `MQTT_PREFIXO` do `secrets.h` do firmware (opcional)
- `ADMIN_SENHA_ARMARIO`: a `ADMIN_SENHA` do `secrets.h` (opcional)

Na Vercel o arquivo de registros fica em `/tmp`, então ele se perde de vez em quando. Localmente fica salvo normal.

## Pastas

- `app/`: páginas e rotas da API
- `components/`: telas do aluno e do admin
- `lib/`: login, sessão, registros e conexão MQTT
- `data/`: registros.json
