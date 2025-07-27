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
Beskrivning: Kort och tydligt, indikerar att kommandot kör applikationen med en given konfiguration.  

Exempel: myapp run config.md eller myapp run config.json  

Fördel: Enkelt och generiskt, passar bra för att köra en konfiguration oavsett format.  

Nackdel: Kan vara för generiskt om du planerar att lägga till fler typer av körningar.

exec  
Beskrivning: Förkortning av "execute", vilket antyder att en fördefinierad konfiguration körs.  

Exempel: myapp exec config.md  

Fördel: Känns tekniskt och passar bra i en CLI-kontext.  

Nackdel: Kan förväxlas med andra betydelser av "exec" (t.ex. systemkommandon).

apply  
Beskrivning: Indikerar att en konfigurationsfil "appliceras" för att köra applikationen.  

Exempel: myapp apply config.json  

Fördel: Beskriver tydligt att inställningarna från filen används.  

Nackdel: Kan kännas mindre dynamiskt än "run" eller "exec".

config  
Beskrivning: Betonar att kommandot använder en konfigurationsfil för att styra körningen.  

Exempel: myapp config config.md  

Fördel: Tydlig koppling till konfigurationsfilen.  

Nackdel: Kan uppfattas som att det bara hanterar konfigurationsfiler, inte kör själva applikationen.

preset  
Beskrivning: Antyder att kommandot använder en fördefinierad ("preset") konfiguration.  

Exempel: myapp preset myconfig.md  

Fördel: Ger en känsla av att använda sparade inställningar, vilket är intuitivt för användare.  

Nackdel: Kan vara mindre vanligt i CLI-sammanhang.

load  
Beskrivning: Fokuserar på att ladda en konfigurationsfil för att köra applikationen.  

Exempel: myapp load config.json  

Fördel: Tydligt att en fil laddas och används.  

Nackdel: Kan implicera att konfigurationen bara laddas utan att köras.



*/