#ifndef INCLOS_FINALS_H
#define INCLOS_FINALS_H

void kpk_Build();
bool kbpk_taules_auxiliar(tss *ss);
int kbpk_taules(tss *ss);
int eval_KPK(tss *ss, struct InfoEval *ie, int color);
int mirar_si_final_especialitzat(tss *ss, struct InfoEval *ie, bool *sortir);
int eval_KRKP(tss *ss, struct InfoEval *ie, int jo, bool *sortir);
int eval_KRKP_llarg(tss *ss);
int eval_KRPKR(tss *ss, struct InfoEval *ie, int jo);

#endif