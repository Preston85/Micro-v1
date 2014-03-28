#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#define MAX 16777213
#define DICT 256
#define DEN MAX
#define SYMBOL_SIZE 1
#define SIZE_OF_INT 4
#define BITS_IN_INT 32
#define BITS_IN_BUFF 524288
#define BUFF_SIZE 16384
#define OUTPUT_SIZE BUFF_SIZE



unsigned int gram[MAX];
unsigned int map[MAX];
unsigned int rank[MAX];
unsigned int lead[MAX];
unsigned int stat[MAX];
unsigned int tree[BUFF_SIZE];
unsigned int input[BUFF_SIZE];
unsigned short left[BUFF_SIZE];
unsigned short right[BUFF_SIZE];
unsigned char output[OUTPUT_SIZE];

unsigned long long end;
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
		if((long long)(i - l + 1) * DEN % (r - l + 1))		// Je¿eli by³a czêœæ u³amkowa w wyniku dzielenia
        	tree[there]++;									// to zwiêksz o 1 (czyli generalnie oblicz sufit z liczby).
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
   	map[0] = DICT;	    // DICT ma byæ na starcie pierwszy w rankingu,
   	map[DICT] = 0;      // zamien go zatem miejscami z zerem.
   	rank[0] = DICT;		//  
   	rank[DICT] = 0;     // 
   	
   	lead[0] = 1;	// Pierwszy w rankingu DICT dostaje na starcie
   	stat[0] = 1;	// darmowe zwiekszenie statystyki z zera do 1.
}


unsigned int Search_for_Demicro(unsigned int l, unsigned int r, unsigned int t, FILE *f1)
{	
	unsigned int mid;
	while(l != r)
	{	
		mid = (unsigned long long)(r - l) * tree[t] / DEN + l;
		if((input[bits_number % BITS_IN_BUFF / BITS_IN_INT]) & (1 << bits_number % BITS_IN_INT))
		{	// Jezeli pos znajduje sie na prawo od srodka.
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
			fread(input, SIZE_OF_INT, BUFF_SIZE, f1);
	}

	return l;
}



void Update_Tables(unsigned int pos)
{
	unsigned int size = stat[pos];  		// Okreœl licznoœæ.
	unsigned int edge = lead[size];		// Okreœl pozycjê lidera.
    stat[edge]++;		     	// Aktualizacja statystyki.
    lead[size]++;         	    // Aktualizacja lidera.

	size = rank[pos];           // Zamiana z liderem.
    rank[pos] = rank[edge];		// Size tymczasowo stanowi bufor.
    rank[edge] = size;			//

	map[rank[pos]] = pos;    	// Aktualizacja mapy.
    map[rank[edge]] = edge;  	// 
}

	
	

unsigned int Decode(FILE *f1)
{	
	unsigned int i, pos, x = 1;
	Ini(MAX);
	i = DICT;


	while((i < MAX) && (bits_number < end))
	{	
		if(i - DICT == x)	// Po wczytaniu x symboli uruchamia siê funkcja Podziel,
		{					// potem nastêpuje podwojenie wartoœci x.
			Bisect(0, lead[0] - 1, i - DICT + stat[map[DICT]]);
			x *= 2;
		}
		pos = Search_for_Demicro(0, lead[0] - 1, 1, f1);
		if(rank[pos] != DICT)
		{
			gram[i] = rank[pos];
			Update_Tables(pos);
		}
		else
		{
			Update_Tables(pos);
	  		pos = Search_for_Demicro(lead[0], i - 1, 0, f1);
	  		gram[i] = rank[pos];
			Update_Tables(pos);
		}
		i++;
	}
	
	return i;
}



void Demicro(char *input_file_name, char *output_file_name)
{
	unsigned int i, j = 0, g;
	
	FILE *f1 = fopen(input_file_name, "rb");
	FILE *f2 = fopen(output_file_name, "wb");
	
	
	fseek(f1, -8, 2);
	fread(&end, sizeof(unsigned long long), 1, f1);
	fseek(f1, 0, 0);

	fread(input, SIZE_OF_INT, BUFF_SIZE, f1);
	while(bits_number < end)
	{	
		g = Decode(f1);
		
		for(i = DICT; i < MAX; i++)
    		map[i] = gram[i];
    	i = DICT;
		
		while(i < g)
		{	
			while(map[i] > DICT)
			{	
				map[i-1] = gram[map[i]-1];
				map[i] = gram[map[i]];
        		i--;
    		}
          	
			output[j] = map[i];
			i++;
			j++;
			
			if(j == OUTPUT_SIZE)
			{	
				fwrite(output, OUTPUT_SIZE, 1, f2);
				j = 0;
			}
		}
		
	}
	fwrite(output, j, 1, f2);
	fclose(f1);
	fclose(f2);
	
}



int main(int argc, char *argv[])
{
	clock_t start, finish;
	double duration = 0;
	start = clock();

	if(argc != 3)
		printf("Blad! Liczba podanych argumentow: %d. Powinny byc 3.", argc);
	else
		Demicro(argv[1], argv[2]);
	
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;   
	printf("\n\n Czas wykonywania %2.5f sek.\n", duration);
	// system("pause");

	return 0;
}
