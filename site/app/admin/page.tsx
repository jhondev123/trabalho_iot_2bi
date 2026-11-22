import { exigirSessao } from "@/lib/sessao";
import Cabecalho from "@/components/Cabecalho";
import PainelAdmin from "@/components/PainelAdmin";

export const dynamic = "force-dynamic";

export default async function Admin() {
  const sessao = await exigirSessao("admin");
  return (
    <>
      <Cabecalho usuario={sessao.usuario} papel={sessao.papel} />
      <PainelAdmin />
    </>
  );
}
