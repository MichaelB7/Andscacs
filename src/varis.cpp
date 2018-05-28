#include <string>
#include "debug.h"
#include "varis.h"

#define verprog "0.921"
#ifdef ES32BITS
std::string VersioPrograma = verprog"32n";  //+n per no popcnt
#else
#ifdef POPCOUNT
#ifdef PEXT
std::string VersioPrograma = verprog"b";
#else
std::string VersioPrograma = verprog;
#endif
#else
std::string VersioPrograma = verprog"n";  //+n per no popcnt
#endif
#endif

std::string presentaprograma() {
	return NomPrograma " " + VersioPrograma + " by " + CreadorPrograma;
}
