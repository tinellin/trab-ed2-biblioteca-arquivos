#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>

/*********************************ESTRUTURAS***********************************/
typedef struct SLivro
{
	char isbn[14];
	char titulo[50];
	char autor[50];
	char ano[5];
} Livro;

/********************************PROTOTIPOS************************************/
FILE *abrirArquivo(char *ch);
void fecharArquivo(FILE *arq);
char *obterRegistro(char *nomeArq, char flag, int tam);
void inserir();
void remover();

/**********************************MAIN****************************************/
int main()
{
	setlocale(LC_ALL, "");
	int op;

	// lista existe dentro do arquivo!!
	// lembrar do sprintf para salvar as variaveis dos arquivos na memória
	// no remover pegar o endereco do registro removido e ja atualizar no header

	// Menu
	printf("*-------- BIBLIOTECA --------*\n");

	do
	{
		printf("*____________________________________*\n");
		printf("|  Opcao      | Funcao                |\n");
		printf("|    1.       | Inserir               |\n");
		printf("|    2.       | Remover               |\n");
		printf("|    3.       | Compactacao           |\n");
		printf("|    4.       | Carregar arquivo      |\n");
		printf("|    0.       | Sair do programa      |\n");
		printf("*------------------------------------*\n");
		printf("Digite a opcao: ");
		scanf("%d", &op);

		switch (op)
		{
		case 1:
		{
			inserir();
			break;
		}
		case 2:
		{
			remover();
			break;
		}
		case 3:
		{
			printf("c");
			break;
		}
		case 4:
		{
			printf("d");
			break;
		}
		case 0:
		{
			printf("e");
			exit(0);
			break;
		}
		default:
			printf("Digite uma das opcoes\n");
		}

	} while (op != 0);
}

/*********************************FUNCOES**************************************/

FILE *abrirArquivo(char *ch)
{
	FILE *arq;

	if (((arq = fopen(ch, "r+b")) == NULL))
	{
		printf("ERRO: Falha ao abrir o arquivo\n");
		return arq;
	}

	return arq;
}

void fecharArquivo(FILE *arq)
{
	fclose(arq);
}

/* função inserir
abrir arquivo inserir.bin
ler dado e salvar na struct
atualizar o contador no arquivo inserir.bin
fechar arquivo inserir.bin
abrir arquivo biblioteca.bin
salvar dados da struct no arquivo biblioteca
fechar arquivo biblioteca
*/

char *obterRegistro(char *nomeArq, char flag, int tam)
{
	Livro livro;
	char isbn[14];
	FILE *arq;

	char *buffer = malloc(sizeof(char));
	char *ch = malloc(sizeof(char));
	int ct;

	arq = abrirArquivo(nomeArq);

	if (fgetc(arq) == '@')
	{
		*ch = fgetc(arq);
		ct = atoi(ch);

		// Pular para posição desejada e ler o registro
		fseek(arq, tam * ct, SEEK_SET);
		flag == 'i' ? fread(&livro, tam, 1, arq) : fread(isbn, tam, 1, arq);

		// Salvar o próximo registro a ser lido
		rewind(arq);
		ct++;
		*ch = ct + '0';

		fseek(arq, 1, SEEK_SET);
		fwrite(ch, 1, sizeof(char), arq);
	}
	else
	{
		rewind(arq);
		flag == 'i' ? fread(&livro, tam, 1, arq) : fread(isbn, tam, 1, arq);
		rewind(arq);
		fwrite("@1", 1, 2 * sizeof(char), arq);
	}

	if (flag == 'i')
		sprintf(buffer, "%s#%s#%s#%s#", livro.isbn, livro.titulo, livro.autor, livro.ano);
	else
		strcpy(buffer, isbn);

	fecharArquivo(arq);

	return buffer;
}

void inserir()
{
	FILE *arqFinal;
	char *buffer;
	int header, tam;

	buffer = obterRegistro("insere.bin", 'i', sizeof(Livro));

	printf("%s\n", buffer);

	tam = strlen(buffer);

	arqFinal = abrirArquivo("biblioteca.bin");

	// Se os primeiros 4 bytes não retornar um inteiro, coloca -1
	fread(&header, sizeof(int), 1, arqFinal);

	if (header != -1)
	{
		rewind(arqFinal);
		header = -1;
		fwrite(&header, sizeof(int), 1, arqFinal);
		// fprintf(arqFinal, "*");
	}

	/* Inserir em algum espaço disponível */

	// /* Inserindo no final do arquivo */
	fseek(arqFinal, 0, SEEK_END);

	// Salva o tamanho do registro
	fwrite(&tam, sizeof(int), 1, arqFinal);

	// Salva o buffer do registro
	fwrite(buffer, tam, 1, arqFinal);

	fecharArquivo(arqFinal);
}

void remover()
{
	FILE *arqFinal;
	const int TAM_ISBN = 13;
	char *isbn = (char *)malloc(sizeof(char) * TAM_ISBN);
	char *isbnBiblioteca = (char *)malloc(sizeof(char) * TAM_ISBN);
	int tamRegistro = 0, header, novoHeader;
	char *pos_Prox = malloc(sizeof(char));

	char ch;

	isbn = obterRegistro("remove.bin", 'r', TAM_ISBN);

	arqFinal = abrirArquivo("biblioteca.bin");

	// Procurar no biblioteca.bin, o registro que será removido a partir do ISBN
	// fseek(arqFinal, 4, SEEK_SET);
	fread(&header, sizeof(int), 1, arqFinal);
	// printf("%d\n", ftell(arqFinal));

	do
	{
		// Ler o tamanho do registro
		fread(&tamRegistro, sizeof(int), 1, arqFinal);

		// Ler ISBN
		fread(isbnBiblioteca, TAM_ISBN, 1, arqFinal);

		// printf("ISBN: %d\n", strlen(isbn));
		// printf("Biblioteca: %d\n", strlen(isbnBiblioteca));

		// Ir pro próximo registro a partir da posição atual
		fseek(arqFinal, tamRegistro - TAM_ISBN, SEEK_CUR);
	} while (strcmp(isbnBiblioteca, isbn) != 0);

	// voltando para o inicio do registro

	// header = posição no arquivo do ultimo elemento removido
	novoHeader = ftell(arqFinal) - TAM_ISBN - sizeof(int);

	fseek(arqFinal, (ftell(arqFinal) - tamRegistro), SEEK_SET);

	printf("POSICAO: %d\n", ftell(arqFinal));

	// fseek(arqFinal, -1, SEEK_END);
	fwrite("*", 1, sizeof(char), arqFinal);

	// inserindo tamanho * offset no inicio do registro

	// fwrite(&header, sizeof(int), 1, arqFinal);

	// while (ftell(arqFinal) != ftell(arqFinal) + tamRegistro - 9)
	// {
	// 	fwrite(" ", tamRegistro, 1, arqFinal);
	// }

	// atualizar o header
	//<tamanho em bytes do registro removido>*<offset para o próximo elemento da lista>
	// ultimo elemento aponta para o que o header apontava
	// header aponta para o ultimo elemento que foi removido
	// printf("%s\n", buffer);

	// tam*header33333

	fecharArquivo(arqFinal);
}