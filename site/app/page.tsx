import { exigirSessao } from "@/lib/sessao";
import Cabecalho from "@/components/Cabecalho";
import PainelAluno from "@/components/PainelAluno";

export const dynamic = "force-dynamic";

// Tela do aluno. O admin é mandado para /admin.
export default async function Home() {
  const sessao = await exigirSessao("aluno");
  return (
    <>
      <Cabecalho usuario={sessao.usuario} papel={sessao.papel} />
      <PainelAluno />
    </>
  );
}
