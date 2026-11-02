import { NextResponse } from "next/server";
import { COOKIE, criarCookie } from "@/lib/sessao";
import { validarLogin } from "@/lib/usuarios";
import { salvarRegistro } from "@/lib/registros";

export const runtime = "nodejs";

export async function POST(req: Request) {
  const corpo = await req.json().catch(() => null);
  const usuario = String(corpo?.usuario ?? "").trim();
  const senha = String(corpo?.senha ?? "");

  const encontrado = validarLogin(usuario, senha);
  if (!encontrado) {
    return NextResponse.json({ erro: "Usuário ou senha inválidos." }, { status: 401 });
  }

  const { valor, maxAge } = criarCookie(encontrado.usuario, encontrado.papel);
  const resposta = NextResponse.json({ papel: encontrado.papel });
  resposta.cookies.set(COOKIE, valor, {
    httpOnly: true,
    sameSite: "lax",
    secure: process.env.NODE_ENV === "production",
    path: "/",
    maxAge,
  });

  await salvarRegistro({ usuario: encontrado.usuario, papel: encontrado.papel, armario: "-", acao: "login" });
  return resposta;
}
