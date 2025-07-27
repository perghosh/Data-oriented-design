/**
* @file CLIRun.cpp
*/

// @TAG #ui.cli

#include "CLIRun.h"


NAMESPACE_CLI_BEGIN

std::pair<bool, std::string> Run_g(const std::string& stringCommand, CApplication* papplication )
{

   return { true, "" };
}


NAMESPACE_CLI_END


/*

run  
Beskrivning: Kort och tydligt, indikerar att kommandot k�r applikationen med en given konfiguration.  

Exempel: myapp run config.md eller myapp run config.json  

F�rdel: Enkelt och generiskt, passar bra f�r att k�ra en konfiguration oavsett format.  

Nackdel: Kan vara f�r generiskt om du planerar att l�gga till fler typer av k�rningar.

exec  
Beskrivning: F�rkortning av "execute", vilket antyder att en f�rdefinierad konfiguration k�rs.  

Exempel: myapp exec config.md  

F�rdel: K�nns tekniskt och passar bra i en CLI-kontext.  

Nackdel: Kan f�rv�xlas med andra betydelser av "exec" (t.ex. systemkommandon).

apply  
Beskrivning: Indikerar att en konfigurationsfil "appliceras" f�r att k�ra applikationen.  

Exempel: myapp apply config.json  

F�rdel: Beskriver tydligt att inst�llningarna fr�n filen anv�nds.  

Nackdel: Kan k�nnas mindre dynamiskt �n "run" eller "exec".

config  
Beskrivning: Betonar att kommandot anv�nder en konfigurationsfil f�r att styra k�rningen.  

Exempel: myapp config config.md  

F�rdel: Tydlig koppling till konfigurationsfilen.  

Nackdel: Kan uppfattas som att det bara hanterar konfigurationsfiler, inte k�r sj�lva applikationen.

preset  
Beskrivning: Antyder att kommandot anv�nder en f�rdefinierad ("preset") konfiguration.  

Exempel: myapp preset myconfig.md  

F�rdel: Ger en k�nsla av att anv�nda sparade inst�llningar, vilket �r intuitivt f�r anv�ndare.  

Nackdel: Kan vara mindre vanligt i CLI-sammanhang.

load  
Beskrivning: Fokuserar p� att ladda en konfigurationsfil f�r att k�ra applikationen.  

Exempel: myapp load config.json  

F�rdel: Tydligt att en fil laddas och anv�nds.  

Nackdel: Kan implicera att konfigurationen bara laddas utan att k�ras.



*/