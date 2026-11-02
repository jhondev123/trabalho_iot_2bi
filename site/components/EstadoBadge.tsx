import type { Armario } from "@/lib/useArmarios";

// Mostra o estado do armário com a mesma cor do LED do ESP32.
export default function EstadoBadge({ armario }: { armario?: Armario }) {
  if (!armario || !armario.estado) return <span className="badge cinza">sem dados</span>;
  if (armario.online === false) return <span className="badge cinza">offline</span>;

  const rotulo = { livre: "Livre", pareado: "Pareado", ocupado: "Ocupado" }[armario.estado];
  const cor = { livre: "verde", pareado: "amarelo", ocupado: "vermelho" }[armario.estado];
  return <span className={`badge ${cor}`}>{rotulo}</span>;
}
