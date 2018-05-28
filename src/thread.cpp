#include "thread.h"
#include "analisis.h"
#include <assert.h>

#ifdef WINDOWS
#include <windows.h>
#include <process.h>
#endif
#ifdef LINUX
#include <pthread.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#ifdef LINUX
#include <time.h>   //nanosleep
#endif

int NumThreads=1;

void inicialitzar_threads(){
	int i;
	paramroot.parar = true;
#ifdef WINDOWS
	Sleep(100); //esperar a que es parin les existents
#endif
#ifdef LINUX
	struct timespec ts;
	ts.tv_sec = 0; //milliseconds / 1000;
	ts.tv_nsec = 100000; //(milliseconds % 1000) * 1000000;
	nanosleep(&ts, NULL);
#endif

	for (i = 0; i < MaxThreads; i++)
		ssbase[i].numthread = i;
	
	paramroot.iniciar = false;
	paramroot.parar = false;
	paramroot.analitzant = false;

	for (i = 0; i < NumThreads; i++) {
		ssbase[i].acabatroot = true; // també perquè no comenci
		paramroot.numthread[i] = i;
		//http://stackoverflow.com/questions/3169009/passing-arguments-to-beginthread-whats-wrong
		if (i != 0) {
#ifdef WINDOWS
			_beginthread(proces_thread, 0, (void*)&paramroot.numthread[i]);
#endif
#ifdef LINUX
			pthread_create(&paramroot.vthread[i], NULL, &proces_thread, (void *)paramroot.numthread[i]);
#endif
		}
	}
}
