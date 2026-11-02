import { createHmac, timingSafeEqual } from "crypto";
import { cookies } from "next/headers";
import { redirect } from "next/navigation";
import type { Papel } from "./usuarios";

// Sessão simples: um cookie assinado com HMAC, sem banco de dados.
// Formato do cookie: <dados em base64url>.<assinatura>

export const COOKIE = "armario_sessao";
const DURACAO_SEGUNDOS = 8 * 60 * 60; // 8 horas

export type Sessao = { usuario: string; papel: Papel; exp: number };

// Em produção o segredo é obrigatório: o código é público, então um segredo padrão
// permitiria forjar o cookie de admin.
function segredo() {
  const s = process.env.SESSION_SECRET;
  if (s) return s;
  if (process.env.NODE_ENV === "production") {
    throw new Error("Defina a variável de ambiente SESSION_SECRET (veja o README).");
  }
  return "segredo-so-para-npm-run-dev";
}

function assinar(dados: string) {
  return createHmac("sha256", segredo()).update(dados).digest("base64url");
}

export function criarCookie(usuario: string, papel: Papel) {
  const sessao: Sessao = { usuario, papel, exp: Math.floor(Date.now() / 1000) + DURACAO_SEGUNDOS };
  const dados = Buffer.from(JSON.stringify(sessao)).toString("base64url");
  return { valor: `${dados}.${assinar(dados)}`, maxAge: DURACAO_SEGUNDOS };
}

function ler(valor: string | undefined): Sessao | null {
  if (!valor) return null;
  const [dados, assinatura] = valor.split(".");
  if (!dados || !assinatura) return null;

  const esperada = Buffer.from(assinar(dados));
  const recebida = Buffer.from(assinatura);
  if (esperada.length !== recebida.length || !timingSafeEqual(esperada, recebida)) return null;

  try {
    const sessao = JSON.parse(Buffer.from(dados, "base64url").toString()) as Sessao;
    if (sessao.exp < Math.floor(Date.now() / 1000)) return null;
    return sessao;
  } catch {
    return null;
  }
}

// Para usar em páginas e rotas (servidor).
export async function getSessao(): Promise<Sessao | null> {
  const jar = await cookies();
  return ler(jar.get(COOKIE)?.value);
}

// Para páginas: manda para o login se não estiver logado ou não tiver o papel certo.
export async function exigirSessao(papel?: Papel): Promise<Sessao> {
  const sessao = await getSessao();
  if (!sessao) redirect("/login");
  if (papel && sessao.papel !== papel) redirect(sessao.papel === "admin" ? "/admin" : "/");
  return sessao;
}
