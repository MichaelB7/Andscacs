#ifndef INCLOS_VARIS_H
#define INCLOS_VARIS_H

#include <string>

#ifdef RANDOM
#define NomPrograma "RAndscacs"
#else
#define NomPrograma "Andscacs"
#endif
#define CreadorPrograma "Daniel Jose"
extern std::string VersioPrograma;

std::string presentaprograma();

#endif