# Zadanie 2. Operacje na strukturze katalogów (45%)
Napisz program wyszukujący w drzewie katalogu (ścieżka do katalogu jest pierwszym argumentem programu), 
w zależności od wartości drugiego argumentu ('<', '>','=') , pliki zwykłe z datą modyfikacji wcześniejszą, późniejszą lub 
równą dacie podanej jako trzeci argument programu. Program ma wypisać na standardowe wyjście następujące informacje znalezionych plików:

* ścieżka bezwzględna pliku,
* rodzaj pliku (zwykły plik - file, katalog - dir, urządzenie znakowe - char dev, urządzenie blokowe - block dev, potok nazwany - fifo, link symboliczny - slink, soket - socket_fd) 
* rozmiar w bajtach,
* datę ostatniego dostępu,
* datę ostatniej modyfikacji.

Ścieżka podana jako argument wywołania może być względna lub bezwzględna.  Program nie powinien podążać za dowiązaniami symbolicznymi do katalogów.

Program należy zaimplementować w dwóch wariantach:
* Korzystając z funkcji opendir, readdir oraz funkcji z rodziny stat (25%)
* Korzystając z funkcji nftw (20%)
* W ramach testowania funkcji utwórz w badanej strukturze katalogów jakieś dowiązania symboliczne, zwykłe pliki i katalogi.

## FILE_INFO ERRORS NUMBERS
* 1	 - Operation not permitted
* 2	 - No such file or directory
* 3	 - No such process
* 4	 - Interrupted system call
* 5	 - I/O error
* 6	 - No such device or address
* 7	 - Argument list too long
* 8	 - Exec format error
* 9	 - Bad file number
* 10 -  No child processes
* 11 -  Try again
* 12 -  Out of memory
* 13 -  Permission denied
* 14 -  Bad address
* 15 -  Block device required
* 16 -  Device or resource busy
* 17 -  File exists
* 18 -  Cross-device link
* 19 -  No such device
* 20 -  Not a directory
* 21 -  Is a directory
* 22 -  Invalid argument
* 23 -  File table overflow
* 24 -  Too many open files
* 25 -  Not a typewriter
* 26 -  Text file busy
* 27 -  File too large
* 28 -  No space left on device
* 29 -  Illegal seek
* 30 -  Read-only file system
* 31 -  Too many links
* 32 -  Broken pipe
* 33 -  Math argument out of domain of func
* 34 -  Math result not representable