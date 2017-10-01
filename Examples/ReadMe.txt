\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\


TestInputRelay

nastavit nìjaký serial monitor na COM port na kterém je pøipojené arduino, nastavit 38400 bd
pøíklad spíná jednotlivé relé po aktivaci pøíslušných vstupù (opto) a pošle na serial monitor zprávu
které relé seplo\rozeplo a èas z RTC


\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\



TestRtcModule

slouží pro nastavení èasu na modulu RTC



\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\


TestGSM

v souboru TestGsm.ino nastavit požadované tel. èíslo do promìnné "myNumber"

funkce po ativaci jednotlivých vstupù A0-A15 (pøímé- bez optoèlenù)

A0   pošle SMS 
A1   vytoèí èíslo
A15  zavìsí
A2   zjistí úroveò signálu
A3   zjistí operátora

vše posílá na serial monitor