KOMPILACJA ORAZ PRZYKŁADOWE UŻYCIE

1) Skompiluj kompresor 'micro.c' oraz dekompresor 'decmiro.c' poleceniami:
gcc -Wall -Ofast micro.c -o micro
gcc -Wall -Ofast demicro.c -o demicro

2) Skompresuj przykładowy plik 'bible.txt':
./micro bible.txt

3) Zdekompresuj powstałe archiwum 'bible.txt.mic' do pliku 'wynik.txt':
./demicro bible.txt.mic wynik.txt

----

Jeżeli plik źródłowy 'bible.txt' oraz plik 'wynik.txt' są identyczne, to znaczy, że kompresja i dekompresja przebiegła prawidłowo.
