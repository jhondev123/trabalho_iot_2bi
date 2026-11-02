import { NextResponse } from "next/server";
import { getSessao } from "@/lib/sessao";
import { lerRegistros, salvarRegistro } from "@/lib/registros";
import { CODIGO_VALIDO } from "@/lib/mqtt-config";

export const runtime = "nodejs";
export const dynamic = "force-dynamic";

const ACOES_DO_ALUNO = ["parear", "abrir"];

// Lista os registros (só admin), do mais novo para o mais antigo.
export async function GET() {
  const sessao = await getSessao();
  if (!sessao) return NextResponse.json({ erro: "Faça login." }, { status: 401 });
  if (sessao.papel !== "admin") return NextResponse.json({ erro: "Só o admin vê os registros." }, { status: 403 });

  const lista = await lerRegistros();
  return NextResponse.json(lista.slice(-100).reverse());
}

// Salva uma ação feita pelo aluno no navegador (parear ou abrir).
export async function POST(req: Request) {
  const sessao = await getSessao();
  if (!sessao) return NextResponse.json({ erro: "Faça login." }, { status: 401 });

  const corpo = await req.json().catch(() => null);
  const acao = String(corpo?.acao ?? "");
  const armario = String(corpo?.armario ?? "").toUpperCase();
  if (!ACOES_DO_ALUNO.includes(acao) || !CODIGO_VALIDO.test(armario)) {
    return NextResponse.json({ erro: "Dados inválidos." }, { status: 400 });
  }

  const registro = await salvarRegistro({ usuario: sessao.usuario, papel: sessao.papel, armario, acao });
  return NextResponse.json(registro, { status: 201 });
}
