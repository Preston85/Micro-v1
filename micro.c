#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include <time.h>
#define MAX 16777213 //262144 //65536 //7192529 //131072 //
#define M 16777213 //25165813 //132166933  //37748717 //393209 //1966607 //100663291 //67108859  //
#define DICT 256
#define DEN MAX
#define SYMBOL_SIZE 1
#define SIZE_OF_INT 4
#define BITS_IN_INT 32
#define BITS_IN_BUFF 524288
#define BUFF_SIZE 16384
#define INPUT_SIZE BUFF_SIZE



unsigned int hash[M + 1];
unsigned int gram[MAX];
unsigned int map[MAX];
unsigned int rank[MAX];
unsigned int lead[MAX];
unsigned int stat[MAX];

unsigned int back[BUFF_SIZE];
unsigned int tree[BUFF_SIZE];
unsigned short left[BUFF_SIZE];
unsigned short right[BUFF_SIZE];
unsigned int output[BUFF_SIZE];
unsigned char input[INPUT_SIZE];

unsigned long long bits_number = 0;



void Bisect(unsigned int l,unsigned  int r,unsigned  int total)
{
	static unsigned int number = 0;
	unsigned int i, sum, half, there = ++number;

	if(r - l < 3 || there == BUFF_SIZE - 1)
		tree[there] = DEN / 2;
	else
	{
		sum = 0;
		half = total / 2;
		i = l;
		while(sum < half)
			sum += stat[i++];
		i--;
		if(sum - stat[i] / 2 > half)
			sum -= stat[i--];
		tree[there] = (unsigned long long)(i - l + 1) * DEN / (r - l + 1);
		if((unsigned long long)(i - l + 1) * DEN % (r - l + 1))		// Jeøeli by≥a czÍúÊ u≥amkowa w wyniku dzielenia
        	tree[there]++;									// to zwiÍksz o 1 (czyli generalnie oblicz sufit z liczby).
		if(tree[there] < DEN / 2)
		{
			left[there] = number + 1;
         	Bisect(l, i, sum);
        	if(tree[left[there]] == DEN / 2)
       		{
				left[there] = 0;
				number--;
        	}
		 	right[there] = number + 1;
        	Bisect(i + 1, r, total - sum);
        	if(tree[right[there]] == DEN / 2)
       	  	{
				right[there] = 0;
				number--;
         	}	
		}
	}
	if(there == 1)
		number = 0;
}



void Ini(unsigned int g)
{
	unsigned int i;
	
	tree[0] = DEN / 2;
   	left[0] = 0;
   	right[0] = 0;


	for(i = 1; i < g; i++)
   	{
		map[i] = i;
      	rank[i] = i;
      	lead[i] = 0;
   	   	stat[i] = 0;
   	}
   	map[0] = DICT;	    // DICT ma byÊ na starcie pierwszy w rankingu,
   	map[DICT] = 0;      // zamien go zatem miejscami z zerem.
   	rank[0] = DICT;		//
   	rank[DICT] = 0;     //

   	lead[0] = 1;	// Pierwszy w rankingu DICT dostaje na starcie
   	stat[0] = 1;	// darmowe zwiekszenie statystyki z zera do 1.

}



void Search_for_Micro(unsigned int l,unsigned  int r,unsigned  int t, FILE *f2,unsigned  int pos)
{	
	unsigned int mid;
	while(l != r)
	{	
		mid = (unsigned long long)(r - l) * tree[t] / DEN + l;
		if(pos > mid)	// Jezeli pos znajduje sie na prawo od srodka.
		{	
			output[bits_number % BITS_IN_BUFF / BITS_IN_INT] |= 1 << bits_number % BITS_IN_INT;
			l = mid + 1;
			t = right[t];
		}
		else	// Jezeli pos znajduje sie na srodku lub na lewo od srodka.
		{	
			r = mid;
			t = left[t];
		}
		
		bits_number++;
		if(!(bits_number % BITS_IN_BUFF))
		{
			fwrite(output, SIZE_OF_INT, BUFF_SIZE, f2);
			for(mid = 0; mid < BUFF_SIZE; mid++)
				output[mid] = 0;
		}
	}
}



void Update_Tables(unsigned int pos)
{
	unsigned int size = stat[pos];  		// Okreúl licznoúÊ.
	unsigned int edge = lead[size];		// Okreúl pozycjÍ lidera.
    stat[edge]++;		     	// Aktualizacja statystyki.
    lead[size]++;         	    // Aktualizacja lidera.

	size = rank[pos];           // Zamiana z liderem.
    rank[pos] = rank[edge];		// Size tymczasowo stanowi bufor.
    rank[edge] = size;			//

	map[rank[pos]] = pos;    	// Aktualizacja mapy.
    map[rank[edge]] = edge;  	// 
}



void Encode(unsigned int g, FILE *f2) 
{	
	unsigned int i, pos, x = 1;
	Ini(g);
   	for(i = DICT; i < g; i++)
   	{
		if(i - DICT == x)	// Po wczytaniu x symboli uruchamia siÍ funkcja Bisect,
		{					// potem nastÍpuje podwojenie wartoúci x.
			Bisect(0, lead[0] - 1, i - DICT + stat[map[DICT]]);
			x *= 2;
		}
		if(!stat[map[gram[i]]])		// Jeøeli gram[i] to jest pierwsze wystapienie symbolu.
			pos = map[DICT];
		else
	    	pos = map[gram[i]];
		Search_for_Micro(0, lead[0] - 1, 1, f2, pos);
		Update_Tables(pos);
	  	
		if(!stat[map[gram[i]]]) 
   		{  
	  		pos = map[gram[i]];  	// Okreúl pozycjÍ w rankingu.
	  		Search_for_Micro(lead[0], i - 1, 0, f2, pos);
	    	Update_Tables(pos);
		}
   	}
}



void Micro(char *file_name)
{
	unsigned int k, g = DICT - 1, h1, h2 = 0, i = 1, input_read_size = 1;
	gram[g] = MAX;

	
	FILE *f1 = fopen(file_name, "rb");
	FILE *f2 = fopen(strcat(file_name, ".mic"), "wb"); 
   
	while(input_read_size)	// P TLA MODYFIKUJE GRAMATYK  BEZKONTEKSTOW• DOP”KI S• SYMBOLE DO WCZYTANIA.        
	{
		if(g < MAX)
		{
			if((gram[g - 1] == 32) && (gram[g] != 32))
				h1 = M;
		    else
			{	// Wylicz klucz k dla digramu spod g.
				k = ((unsigned long long)(gram[g] - gram[g - 1]) - gram[g] * gram[g - 1]);   
        		h1 = k % M;               // h1 oraz h2 to pomocnicze funkcje haszujπce. 
        		h2 = (k % (M - 1)) + 1;   // hash[hk] == 0  - pozycja hk w tablicy haszujπcej jest wolna.
            }							  // hash[hk] >  0  - pozycja hk w tablicy haszujπcej jest zajÍta
										  // przez digram o indeksie hash[hk].

        	do
			{                      // SPRAWDè, CZY DIGRAM TAKI JAK POD G JEST JUØ W GRAMATYCE
				k = hash[h1];          // Zajrzyj do tablicy haszujπcej.    
				if(!k)
				{   ////// JEØELI TABLICA HASZUJ•CA M”WI, ØE NIE MA TAKIEGO DIGRAMU ///////
			  		
					if((gram[g - 1] == 32) && (gram[g] != 32))
						back[g % BUFF_SIZE] = M;
					else
					{
						hash[h1] = g;
              			back[g % BUFF_SIZE] = h1;
					}
					
              		g++;
              		if(g < MAX)                            // Jezeli jest jeszcze miejsce w gramatyce.
                 	{
						if(i == input_read_size) 
                    	{	input_read_size = fread(input, SYMBOL_SIZE, INPUT_SIZE, f1);
                    		i = 0;
                 		}
                 		gram[g] = (unsigned int)input[i];   // UmieúÊ symbol w gramatyce.
                 		i++;
              		}
           		}
           		else if((gram[k] == gram[g]) && (gram[k - 1] == gram[g - 1]) && (k != g - 1))
			  	{   //////// JEØELI TABLICA HASZUJ•CA M”WI, ØE TAKI DIGRAM JEST //////////
					g--;
              		hash[back[g % BUFF_SIZE]] = 0;
              		gram[g] = k;        // Wstaw do gramatyki symbol, ktÛry zastπpi symbol usuniÍty.
              		k = 0;              // Wyzeruj k, aby wyjúÊ z pÍtli do-while(hk)
           		}
				else    ////// JEØELI NIE UDA£O SI  ZNALEè∆ ZA PIERWSZYM STRZA£EM //////
              		h1 = (h1 + h2) % M;  
        	} while(k);
     	}
     	else 
		{	
			Encode(g, f2);   // Zakoduj gramatykÍ
			for(g = 0; g < M; g++)
        		hash[g] = 0;
        	for(g = 0; g < BUFF_SIZE; g++)
           		back[g] = 0;
        	g = DICT - 1;
	 	}     
	}
	// printf("g = %d\n", g);
	Encode(g, f2);   // Zakoduj koÒcÛwkÍ gramatyki.
	fwrite(output, SIZE_OF_INT, (bits_number / BITS_IN_INT + 1) % BUFF_SIZE, f2);   // Wypisz resztkÍ z bufora.
	fwrite(&bits_number, sizeof(unsigned long long), 1, f2);   // Zapisz liczbÍ bitÛw powsta≥ych danych.
   
	fclose(f1);
	fclose(f2);

}
   


int main(int argc, char *argv[])
{
	clock_t start, finish;
	double duration = 0;
	start = clock();
   
	if(argc != 2)
		printf("Blad! Liczba podanych argumentow: %d. Powinny byc 2.", argc);
	else
		Micro(argv[1]);

	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;   
	printf( "\n\nCzas wykonywania %2.5f sek.\n", duration);
	// system("pause");
	return 0;
}
