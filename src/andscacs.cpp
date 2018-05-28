#include "debug.h"
#ifdef WINDOWS
#include <Windows.h>
#include <Dbghelp.h>
#endif
#include <stdio.h>
#include "varis.h"
#include "es.h"
#include "hash.h"
#include "utils.h"
//necessari per linux
#undef min
#undef max
#include "temps.h"
#include "analisis.h"
#include "finals.h"
#include "avaluacio.h"
#include "thread.h"
#include <string.h>

#ifdef WINDOWS
//http://stackoverflow.com/questions/5028781/how-to-write-a-sample-code-that-will-crash-and-produce-dump-file
void make_minidump(EXCEPTION_POINTERS* e)
{
	auto hDbgHelp = LoadLibraryA("dbghelp");
	if (hDbgHelp == nullptr)
		return;
	auto pMiniDumpWriteDump = (decltype(&MiniDumpWriteDump))GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
	if (pMiniDumpWriteDump == nullptr)
		return;

	char name[MAX_PATH];
	{
		auto nameEnd = name + GetModuleFileNameA(GetModuleHandleA(0), name, MAX_PATH);
		SYSTEMTIME t;
		GetSystemTime(&t);
		wsprintfA(nameEnd - strlen(".exe"),
			"_%4d%02d%02d_%02d%02d%02d.dmp",
			t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
	}

	auto hFile = CreateFileA(name, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return;

	MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
	exceptionInfo.ThreadId = GetCurrentThreadId();
	exceptionInfo.ExceptionPointers = e;
	exceptionInfo.ClientPointers = FALSE;

	auto dumped = pMiniDumpWriteDump(
		GetCurrentProcess(),
		GetCurrentProcessId(),
		hFile,
		MINIDUMP_TYPE(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory),
		e ? &exceptionInfo : nullptr,
		nullptr,
		nullptr);

	CloseHandle(hFile);

	return;
}

LONG CALLBACK unhandled_handler(EXCEPTION_POINTERS* e)
{
	make_minidump(e);
	return EXCEPTION_CONTINUE_SEARCH;
}
#endif

void inicialitzacions(){
	char fenini[1000];
	inicialitza_claus_hash(numaleatori);
	inicialitza_hash();
	strcpy(HashFile, "hash.hsh");
	valorpeca[CasellaBuida] = 0;
	valorpeca[ReiBlanc] = 0;
	valorpeca[ReiNegre] = 0;

	calcular_imbalanc();
	carregar_fitxer_avaluacions();
	inicialitzar_lmr();
	precalcul_flags_enroc();
	precalcul_bitboards();
	precalcul_estructura_peons();
	kpk_Build();
	strcpy(fenini, fenposini.c_str());
	inicialitzar_taulell_fen(ssbase, fenini);
	neteja_varis(ssbase);
	buidar_refutacio();
	buidar_refutacio3();

	temps_TempsMoviment = 5000;
	temps_tipus = TipusTempsMoviment;
	tasca = tasca_cap;
	movimentsperfer = 24;
	infostring[0] = 0;
	printfpensar = true;
	inicialitzar_threads();
	// get ticks per second
#ifdef WINDOWS
	QueryPerformanceFrequency(&frequency);
#endif
}

int main(int argc, char* argv[])
{

#ifdef WINDOWS
#ifdef GENERARDMP
	SetUnhandledExceptionFilter(unhandled_handler);
#endif
#endif
	fprintf(stdout, "%s\n", presentaprograma().c_str());
	inicialitzacions();
	prepara_entrada();
	comandes_uci((argc == 2 && std::string(argv[1]) == "test"));
}

