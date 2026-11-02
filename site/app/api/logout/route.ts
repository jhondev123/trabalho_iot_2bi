import { NextResponse } from "next/server";
import { COOKIE } from "@/lib/sessao";

export const runtime = "nodejs";

export async function POST() {
  const resposta = NextResponse.json({ ok: true });
  resposta.cookies.set(COOKIE, "", { httpOnly: true, path: "/", maxAge: 0 });
  return resposta;
}
