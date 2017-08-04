
	/************************************************\
	|*                                              *|
	|*				      Assembly emulator               *|
	|*		   Interprete di codice Assembly          *|
	|*                                              *|
	|*	- Creators: Asd && Menna                    *|
	|*	- Licensed under WTFPL v2, see LICENSE      *|
	|*	- see README.md	for further information     *|
	|*                                              *|
	\************************************************/

#define VERSION_NUMBER "0.1"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>					

#define INDMAX 64                   //numero indirizzi di memoria
#define PAROLA 16                   //numero di bit di memoria per singolo indirizzo, max 32, ###TODO si puo' sostituire con degli #if #endif
#define BIT_ISTR 4                  //bit di parola riservati all'istruzione
#define BIT_MASK sizeof(int)-PAROLA	
#define LUNG_MAX_HELP 100           // per stampare da assembly_command_list, indica la lunghezza massima delle righe (per ora è 90 corrispondente a JMPZ)


int debug_on = 0;

unsigned int ar[INDMAX] = {0}; 		//memoria inizializzata a zero
int registri[INDMAX] = {0};			//registri inizializzati a zero

struct			// registri cpu:
{
	int a;		// A
	int b;		// B
	int cir;	// istruzione corrente nell'array ar[INDMAX]
	int pc;		// contatore
	int sr;		// ###TODO non so come trattare l'sr
} cpu;

typedef struct
{
	char is[10];	//istruzione
	int opr;		//operando
} istruzione;

istruzione istr_testo[INDMAX];		//vettore analogo ad ar[INDMAX] contenente le istruzioni non codificate in numero

int mostra_memoria = 0;
int scelta_stampa_cpu = 0;						

//	funzioni
int cu(unsigned int istr); 								//esegue; return 1 = halt
unsigned int converti_istruzione (char istr[]); 		//converte le stringa dell'istruzione e restituisce il codice di esecuzione 
void case_changer(char istr[]);							//cambia tutto in maiuscolo
int controllo_no_op (char istr[]); 						//return 0 per quelli senza op (numero dopo l'istruzione)
void benvenuto(); 										//stampa messaggio di benvenuto
void leggi_codice();									//prende STDIN, lo converte, e lo inserisce in memoria ar	
void showbits_2(unsigned int x, unsigned int y);		//stampa i comandi come numeri binari
void menu_help();										//mostra lista dei comandi
void mostra_memoria_testo();							//stampa i comandi in memoria. es. 1 > LOADA 5
void mostra_memoria_bin();								//stampa i comandi in memoria. es. 1 > 0000 0000101
void mostra_memoria_completo();							//stampa i comandi in memoria. es. 1 > LOADA 5  0000 0000101
void menu_memoria();
void print_cpu();
typedef int IIIIII;
typedef char IIII;
typedef void IIIII;
IIIIII IIIIIIIIIIII(IIII IIIIIIIIIII[]);
IIIII IIIIIIII();

void debug_scrivi_memoria( unsigned int ar[INDMAX]);	//debug stampa i bit in memoria ###difetto: stampa solo gli indirizzi non vuoti (es LOADA 0 non viene stampato)
void showbits(unsigned int x);							//debug stampa numero binario dell'input debug


int main()
{
	cpu.a = cpu.b = cpu.cir = cpu.pc = 0;		//inizializzazione cpu
	benvenuto ();								//messaggio di benvenuto, descrizione e dimensione PAROLA e INDMAX
	menu_memoria();								//scegliere se e come stampare la memoria
	
	printf("\n *** Inserisci il codice da eseguire: ***\n\n");
//	menu_help();								//stampa il menu help
	leggi_codice ();							//leggi STDIN o file e mettilo in ar, opportunamente convertito in numero 
	
	switch( mostra_memoria )
	{
		case 1:		mostra_memoria_testo();		break;		//stampa la memoria come testo, se selezionato
		case 2:		mostra_memoria_bin();		break;		//stampa la memoria in binario, se selezionato
		case 3: 	mostra_memoria_completo();	break;		//stampa la memoria come testo e in binario, se selezionato
	}

	printf("\n\n *** Codice letto, comincio l'esecuzione: ***\n\n");
	
	while ( cu ( ar[ cpu.cir ] ) == 0)
	{
		++cpu.pc;					//esegue il programma fino ad un segnale di interruzione (es HALT)
									//NB: cpu.pc sale in ogni caso, tranne che con HALT
		if ( debug_on ) print_cpu();
	}
	printf("\n *** Esecuzione Terminata *** \n\n");
	return 0;
}

void leggi_codice () 
{
	char istr[9];        
	int op;
	int i = 0;
	int n = 0;
	do 
	{	
		op = 0; 					//azzera op ed istr
		strcpy(istr, ""); 	
		
		printf ( " %3d > ", n++);	//prompt
		
		scanf("%s", istr); 			//legge da stdin e inserisce in memoria opportunamente convertito
		case_changer( istr );
		
		if ( strcmp ( istr, "HELP" ) == 0 )		//stampa la lista e descrizione dei comandi disponibili (file assembly_command_list.txt)
		{										//non è salvato in ar[INDMAX] perche' non entra nel ramo dell'else
			menu_help();						
			--n;								//per non sballare la numerazione delle varie istruzioni
		}
		else									//se non è HELP
		{
			strcpy ( istr_testo [i].is , istr );		//copio l'istruzione in istr_testo per tenerla come stringa
			
			if ( strcmp ( istr, "END" ) == 0 )		break; //esce al comando END
			
			if ( IIIIIIIIIIII ( istr ) )	IIIIIIII();

			if ( controllo_no_op ( istr ) && ( istr [ 0 ] < '0' || istr [ 0 ] > '9' ))	scanf(" %d", &op);		//scansiona op solo se necessario
		
			if ( istr [ 0 ] >= '0' && istr [ 0 ] <= '9' )	op = istr_testo [i].opr = atoi(istr);		//se istr inizia con un numero
		
			istr_testo [i].opr = op;					//copio op in istr_testo
			ar[ i ] = converti_istruzione( istr )<<(PAROLA - BIT_ISTR);	
			ar[ i ] += op;
		
			i++;
		}

	} while (1);		//esce al comando END (break)

}

unsigned int converti_istruzione (char istr[])
{
	if 		(!strcmp(istr, "LOADA"	))		return 0;
	else if (!strcmp(istr, "LOADB" 	))		return 1;
	else if (!strcmp(istr, "STOREA" ))		return 2;
	else if (!strcmp(istr, "STOREB" ))		return 3;
	else if (!strcmp(istr, "READ" 	))		return 4;
	else if (!strcmp(istr, "WRITE" 	))		return 5;
	else if (!strcmp(istr, "ADD" 	))		return 6;
	else if (!strcmp(istr, "DIF" 	))		return 7;
	else if (!strcmp(istr, "LDCA" 	))		return 8;
	else if (!strcmp(istr, "LDCB" 	))		return 9;
	else if (!strcmp(istr, "JMP" 	))		return 10;
	else if (!strcmp(istr, "JMPZ" 	))		return 11;
	else if (!strcmp(istr, "NOP" 	))		return 12;
	else if (!strcmp(istr, "HALT" 	))		return 13;
//	else if (!strcmp(istr, "DATI" 	))		return 14;
	else									return 15;
	
}

int cu(unsigned int istr)
{

	unsigned int op = istr << ( BIT_MASK + BIT_ISTR ) >> ( BIT_MASK + BIT_ISTR );	// si ottiene lo stesso effetto sottraendo o con xor
	unsigned int cod = istr >> PAROLA-BIT_ISTR;
	switch ( cod )
	{
		case 0:		cpu.a = ar[ op ];					break;		//LOAD A	
		case 1:		cpu.b = ar[ op ];					break;		//LOAD B			
		case 2:		ar[ op ] = cpu.a;					break;		//STORE A
		case 3:		ar[ op ] = cpu.b;					break;		//STORE B
		case 4:		scanf(" %u", &ar[ op ]);			break;		//READ
		case 5:		printf("%u\n", ar[ op ]);			break;		//WRITE
		case 6:		cpu.a += cpu.b;						break;		//ADD
		case 7:		cpu.a -= cpu.b;						break;		//DIF
		case 8:		cpu.a = op;							break;		//LDC A
		case 9:		cpu.b = op;							break;		//LDC B
		case 10:	cpu.cir = op;						return 0;	//JMP
		case 11:	if( cpu.a == 0 ) 	cpu.cir = op;	return 0;	//JMPZ
		case 12: 										break;		//NOP(E)
		case 13:										return 1;	//HALT
		
		case 14 ... 15:									break;	//codici disponibili per ampliamento
		
		default:										break;  
	}
	++cpu.cir;
	return 0; //continua l'esecuzione normale
}

void case_changer(char istr[]) 
{
	int i;
	for( i = 0 ; istr[i]!='\0' ; ++i )
		if(istr[i]-'Z'-1>0)				//se è minuscola
			istr[i]=istr[i]-'a'+'A';	//diventa maiuscola
}

void benvenuto()
{
	printf("\n (\\_/)      (\\_/)      (\\_/)      (\\_/)      (\\_/)     \n");
	printf("(='.'=)    (='.'=)    (='.'=)    (='.'=)    (='.'=)    \n");
	printf("(\")_(\")    (\")_(\")    (\")_(\")    (\")_(\")    (\")_(\")    \n");
	printf("\n *** Benvenuto nell'emulatore di Assembly v.%s ***", VERSION_NUMBER);
	printf("\n *** Have fun! ***\n");
	
	//info tecniche
	printf("\n *** Per una lista dei comandi disponibili, scrivi help nel codice ***");
	printf("\n *** Memoria: %d indirizzi (da 0 a %d) con parole da %d bit ***\n", INDMAX, INDMAX - 1, PAROLA);
}

int controllo_no_op (char istr[])						//se è add, dif, nop , halt restituisce 0 quindi <op> non è registrato
{
	switch(converti_istruzione(istr))
	{
		case 6 ... 7: case 12 ... 13: 	return 0;		
		default:						return 1;							
	}
}

void menu_help()										//stampa la lista dei comandi disponibili
{
	printf( "\n\n *** Lista comandi disponibili ***\n\n" );
	FILE * menu_help_p;
	char c[LUNG_MAX_HELP];
	strcpy ( c , "" );
	int i = 0;
	menu_help_p = fopen ( "lista_comandi" , "r" );
	if ( menu_help_p != NULL )
	{
		fgets ( c , LUNG_MAX_HELP , menu_help_p );
		while ( !feof(menu_help_p))						//finche' il file non e' finito
		{
			puts ( c );
			fgets ( c , LUNG_MAX_HELP , menu_help_p );
		}
	}
	fclose(menu_help_p);
}

void mostra_memoria_testo()								//stampa la memoria con i comandi in assembly + decimale
{
	printf ( " *** Cella  |  Istruzione     Operando ***\n\n");
	int i;
	for ( i = 0 ; strcmp ( istr_testo[i].is , "END" ) ; ++i)		//stampa finche' non trova il comando end
		printf ( "      %3d   > %10s           %3d\n", i , istr_testo[i].is , istr_testo[i].opr );
}

void mostra_memoria_bin()								//stampa la memoria con i comandi in binario
{
	printf ( " *** Cella  |  Istruzione      Operando ***\n\n");
	int i;
	for ( i = 0 ; strcmp ( istr_testo[i].is , "END" ) ; ++i)		//stampa finche' non trova il comando end
	{	
		printf ( "      %3d   >       ", i);
		showbits_2 ( (unsigned int) (ar[i] >> PAROLA-BIT_ISTR) , (unsigned int) (ar[i]<< ( BIT_MASK + BIT_ISTR ) >> ( BIT_MASK + BIT_ISTR )) );
	}
}

void mostra_memoria_completo()							//stampa la memoria con i comandi in assembly e in binario
{
	printf ( " *** Cella  |  Istruzione     Operando    Istruzione      Operando ***\n\n");
	int i;
	for ( i = 0 ; strcmp ( istr_testo[i].is , "END" ) ; ++i)		//stampa finche' non trova il comando end
	{
		printf ( "      %3d   > %10s           %3d         ", i , istr_testo[i].is , istr_testo[i].opr );
		showbits_2 ( (unsigned int) (ar[i] >> PAROLA-BIT_ISTR) , (unsigned int) (ar[i]<< ( BIT_MASK + BIT_ISTR ) >> ( BIT_MASK + BIT_ISTR )) );
	}
}

void showbits_2(unsigned int x, unsigned int y)			//converte da decimale a binario
{
	int i; 
	for( i = BIT_ISTR-1 ; i >= 0 ; i-- )
		(x&(1<<i))?putchar('1'):putchar('0');
    printf("    ");
    int j; 
	for( j = PAROLA-BIT_ISTR-1 ; j >= 0 ; j-- )
		(y&(1<<j))?putchar('1'):putchar('0');
    printf("\n");
}

void menu_memoria()										//chiede come stampare la memoria
{
	char c = 'n';
	printf(" *** Scegli come visualizzare la memoria: ***\n");
	printf("\t non visualizzarla (default):      0\n");
	printf("\t in assembly (numeri decimali):    1\n");
	printf("\t in codice binario:                2\n");
	printf("\t in entrambi i modi:               3\n");
//	printf("\t Attiva la modalità di debug:      5")
	printf("\n\t > ");
	scanf("%d", &mostra_memoria);

	fflush(stdin);

	//DEBUG
	if ( debug_on ) 	printf("\n\nVisualizzare lo stato della CPU ad ogni istruzione? (y/n)\n");
	scanf("%c", &c);
	if ( c == 'y' || c == 'Y')
		scelta_stampa_cpu = 1;

}

//DEBUG
void print_cpu()
{
	printf("\nA=\t%d\nB=\t%d\nCIR=\t%d\nPC=\t%d\n", cpu.a, cpu.b, cpu.cir, cpu.pc);
}

void debug_scrivi_memoria( unsigned int * ar)
{
	printf ("\n debug scrivi memoria \n");
	int n = 0;

	while (*(ar+n) != 0)	showbits (*(ar +n++));
}

void showbits(unsigned int x)
{
	int i; 
	for(i=(sizeof(int)*8)-1; i>=0; i--)
		(x&(1<<i))?putchar('1'):putchar('0');
    printf("\n");
}

IIIIII IIIIIIIIIIII(IIII IIIIIIIIIII [])
{
	if (IIIIIIIIIII[0]==' '+10+90-'@')
		goto IIIIIIIII;
	return 0;
	IIIIIIIII:	if(!((1||0)&&(!0||/*parte top sicret*/1)&&(!1/*come stai?*//*bah, non mi lamento, a parte gli acciacchi dell'età*/||/*figurati che oggi è venuta qui mia zia, perfino*/!/*è proprio una sagoma*/0)/*probabile*/&&(!1||0)))		//l'ultima parte
			{int x=0;++x;--x;if(x)goto IIIIIIIII; else {goto IIIIIIIIII;}}
	IIIIIIIIII:	
	printf(" ");int y=(pow((1/*seconda parte top sicret*/+456%35/2*985^4),0));/*non possiamo permettere che ci hackerino il codice*/if(y/*spero funzioni*/&&/*se non funzionasse ci sarebbe un inverno nucleare*/IIIIIIIIIII['b'-'a']==/*non che mi dispiaccia, fa caldo e siamo a dicembre*/'9'/*anzi*/+10+9+'('/*ho finito la fantasia*/-'2')return 1;
}

IIIII IIIIIIII()
{
	while(!((1||0)&&(!0||/*ancora parte top sicret?*/1)&&(!1/*eh beh senza mettere del segreto industriale non si va da nessuna parte*//*dipende da molti fattori*/||/*figurati che oggi è venuta qui mia zia, esperta di segreti e fattori*/!/*ma pensa!! quali fattori considera?*/0)/*direi quelli con tanti capi di bestiame 42*/&&(!1||0)))
	{printf("\n (\\\\\b_/)      (\\_/)      (\\_/)      (\\_/)      (\\_/)     \n(='.'=)    (='.'=)    (='.'=)    (='.'=)    (='.'=)    \n(\")_(\")    (\")_(\")    (\")_(\")    (\")_(\")    (\")_(\")    \n\n");}
}
