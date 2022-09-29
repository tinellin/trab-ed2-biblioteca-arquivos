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
char *obterRegistro(FILE *arq, char flag, int tam);
void inserir();
void remover();
void compactar(FILE *arqFinal);

/**********************************MAIN****************************************/
int main()
{
	setlocale(LC_ALL, "");
	int op;
	FILE *arqInserir, *arqRemover, *arqFinal;

	// lista existe dentro do arquivo!!
	// lembrar do sprintf para salvar as variaveis dos arquivos na mem�ria
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
			arqInserir = abrirArquivo("insere.bin");
			arqFinal = abrirArquivo("biblioteca.bin");
			inserir(arqInserir, arqFinal);
			fecharArquivo(arqInserir);
			fecharArquivo(arqFinal);
			break;
		}
		case 2:
		{
			arqRemover = abrirArquivo("remove.bin");
			arqFinal = abrirArquivo("biblioteca.bin");
			remover(arqRemover, arqFinal);
			fecharArquivo(arqRemover);
			fecharArquivo(arqFinal);
			break;
		}
		case 3:
		{
			arqFinal = abrirArquivo("biblioteca.bin");
			compactar(arqFinal);
			fecharArquivo(arqFinal);
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
		printf("ERRO: Falha ao abrir o arquivo\n%s", ch);
		return arq;
	}

	return arq;
}

void fecharArquivo(FILE *arq)
{
	fclose(arq);
}

/* fun��o inserir
abrir arquivo inserir.bin
ler dado e salvar na struct
atualizar o contador no arquivo inserir.bin
fechar arquivo inserir.bin
abrir arquivo biblioteca.bin
salvar dados da struct no arquivo biblioteca
fechar arquivo biblioteca
*/

char *obterRegistro(FILE *arq, char flag, int tam)
{
	Livro livro;
	char isbn[14];

	char *buffer = malloc(sizeof(char));
	char *ch = malloc(sizeof(char));
	int ct;

	if (fgetc(arq) == '@')
	{
		*ch = fgetc(arq);
		ct = atoi(ch);

		printf("%d\n", ct);

		// Pular para posicao desejada e ler o registro
		flag == 'i' ? fseek(arq, tam * ct, SEEK_SET) : fseek(arq, (tam * ct) + ct, SEEK_SET);
		flag == 'i' ? fread(&livro, tam, 1, arq) : fread(&isbn, tam + 1, 1, arq);

		printf("%s - %d\n", isbn, ftell(arq));

		// Salvar o proximo registro a ser lido
		rewind(arq);
		ct++;
		*ch = ct + '0';

		fseek(arq, 1, SEEK_SET);
		fwrite(ch, 1, sizeof(char), arq);
	}
	else
	{
		rewind(arq);
		flag == 'i' ? fread(&livro, tam, 1, arq) : fread(&isbn, tam, 1, arq);
		rewind(arq);
		fwrite("@1", 1, 2 * sizeof(char), arq);
	}

	if (flag == 'i')
		sprintf(buffer, "%s#%s#%s#%s#", livro.isbn, livro.titulo, livro.autor, livro.ano);
	else
		strcpy(buffer, isbn);

	return buffer;
}

void inserir(FILE *arqInserir, FILE *arqFinal)
{
	char *buffer;
	int tam, tamanhoRemovido;
	int offset = 0, header = 0, inserido = 0;

	buffer = obterRegistro(arqInserir, 'i', sizeof(Livro));
	tam = strlen(buffer);

	// Se os primeiros 4 bytes nao retornar um inteiro, coloca -1
	fread(&header, 1, sizeof(int), arqFinal);

	if (header == 0 || header == -1)
	{
		header = -1;
		fwrite(&header, sizeof(int), 1, arqFinal);

		/* Inserindo no final do arquivo */
		fseek(arqFinal, 0, SEEK_END);

		// Salva o tamanho do registro
		fwrite(&tam, sizeof(int), 1, arqFinal);

		// Salva o buffer do registro
		fwrite(buffer, tam, 1, arqFinal);
	}
	else // imprime em espacos vazios
	{
		int offset = header, aux = header;
		int tamRemovido;

		fseek(arqFinal, offset, SEEK_SET);

		fread(&tamRemovido, sizeof(int), 1, arqFinal);
		// 20
		//  Caso A
		if (tamRemovido >= tam)
		{
			// atualizarTamanho(arqFinal, tam);

			fseek(arqFinal, 1, SEEK_CUR);
			fread(&aux, sizeof(int), 1, arqFinal);

			fseek(arqFinal, -5, SEEK_CUR);
			fwrite(buffer, tam, 1, arqFinal);

			rewind(arqFinal);
			fwrite(&aux, sizeof(int), 1, arqFinal);

			inserido = 1;
		}
		else
		{
			// Caso B
			while (tamRemovido < tam && aux != -1)
			{
				// Próximo offset
				fseek(arqFinal, 1, SEEK_CUR);

				// aux 5 -> 78
				fread(&aux, sizeof(int), 1, arqFinal);

				fseek(arqFinal, aux, SEEK_SET);
				fread(&tamRemovido, sizeof(int), 1, arqFinal);

				if (tamRemovido >= tam)
				{
					// Pular asterico
					fseek(arqFinal, 1, SEEK_CUR);

					// Le o offset
					fread(&aux, sizeof(int), 1, arqFinal);

					// Inserir registro
					//  -> Retornar 5 posicoes, onde o tamanho do registro e usado por completo
					fseek(arqFinal, -5, SEEK_CUR);
					fwrite(buffer, tam, 1, arqFinal);

					fseek(arqFinal, offset + sizeof(int) + sizeof(char), SEEK_SET);
					fwrite(&aux, sizeof(int), 1, arqFinal);

					inserido = 1;
				}
				// Movimentar offset
				else
				{
					offset = aux;
					fseek(arqFinal, offset, SEEK_SET); // prox registro removido
					fseek(arqFinal, 4, SEEK_CUR);
				}
			}
		}

		// Se for -1, insere no final do arquivo
		if (aux == -1 && inserido == 0)
		{
			// /* Inserindo no final do arquivo */
			fseek(arqFinal, 0, SEEK_END);

			// Salva o tamanho do registro
			fwrite(&tam, sizeof(int), 1, arqFinal);

			// Salva o buffer do registro
			fwrite(buffer, tam, 1, arqFinal);
		}
	}
}

void remover(FILE *arqRemover, FILE *arqFinal)
{
	const int TAM_ISBN = 13;
	char *isbn = (char *)malloc(sizeof(char) * TAM_ISBN);
	char *isbnBiblioteca = (char *)malloc(sizeof(char) * TAM_ISBN);
	int tamRegistro = 0, header, novoHeader;
	char *pos_Prox = malloc(sizeof(char));
	int posRemovido, finalRegistroRemovido;

	char ch;

	isbn = obterRegistro(arqRemover, 'r', TAM_ISBN);
	// printf("ISBN Remover: %s\n", isbn);

	// Procurar no biblioteca.bin, o registro que sera removido a partir do ISBN
	fread(&header, sizeof(int), 1, arqFinal);

	do
	{
		// Ler o tamanho do registro
		fread(&tamRegistro, sizeof(int), 1, arqFinal);

		// Ler ISBN
		fread(isbnBiblioteca, TAM_ISBN, 1, arqFinal);

		// Ir pro proximo registro a partir da posicao atual
		fseek(arqFinal, tamRegistro - TAM_ISBN, SEEK_CUR);

		// printf("ISBN Biblioteca: %s\n", isbnBiblioteca);
	} while (strcmp(isbnBiblioteca, isbn) != 0);

	fseek(arqFinal, (ftell(arqFinal) - tamRegistro), SEEK_SET);

	fwrite("*", 1, sizeof(char), arqFinal);
	fwrite(&header, sizeof(int), 1, arqFinal);

	// printf("POSICAO: %d\n", ftell(arqFinal));

	// Obter a posicao do inicio do ISBN
	posRemovido = ftell(arqFinal);

	finalRegistroRemovido = posRemovido + (tamRegistro - sizeof(int) - sizeof(char));

	// Limpar registro colocando espa�os em brancos
	// posRemovido - tamRegistro - sizeof(int) - sizeof(char) -> este conta obtem o final do registro
	while (ftell(arqFinal) != finalRegistroRemovido)
		fwrite("\0", 1, sizeof(char), arqFinal);

	rewind(arqFinal);
	posRemovido -= 9;

	fwrite(&posRemovido, sizeof(int), 1, arqFinal);
}

void compactar(FILE *arqFinal)
{
	int tam, tamRegistro, header, pos, offset = 0;
	int tamTotal = 0;
	char ch, aux, *buffer = malloc(sizeof(char));
	char *bloco;
	Livro livro;

	fseek(arqFinal, 0, SEEK_END);
	int tamArquivo = ftell(arqFinal);

	rewind(arqFinal);

	while (ch != EOF)
	{
		ch = fgetc(arqFinal);

		// printf("fgetc: %c ftell: %d\n", ch, ftell(arqFinal));

		if (ch == '*')
		{
			printf("ftell: %d\n", ftell(arqFinal));
			pos = ftell(arqFinal) - 5;						 // Pega a posição do * - 5
			fseek(arqFinal, -5, SEEK_CUR);				 // voltando antes do asterisco
			fread(&tam, sizeof(int), 1, arqFinal); // pegando o tamanho do espaço vazio

			fseek(arqFinal, tam, SEEK_CUR); // percorrendo o espaço vazio para pegar o tamanho e começar a somar

			// While do pulo de registro
			while (ftell(arqFinal) != tamArquivo)
			{
				fread(&tamRegistro, sizeof(int), 1, arqFinal); // pegando o tamanho do registro após o vazio

				printf("tamanho reg: %d\n", tamRegistro);

				tamTotal += tamRegistro + sizeof(int);

				fseek(arqFinal, tamRegistro, SEEK_CUR);
			}

			printf("tam total: %d\n", tamTotal);

			bloco = (char *)malloc(sizeof(char) * tamTotal);

			fseek(arqFinal, pos + tam + sizeof(int), SEEK_SET); // voltando para o início do arquivo e voltando na posição do primeiro registro depois do *

			fread(bloco, tamTotal, 1, arqFinal); // armazenando todos os caracteres dos registros após o registro vazio

			printf("%s\n", bloco);

			printf("tam: %d\n", tamRegistro);

			fseek(arqFinal, -1 * (tam + sizeof(int)), SEEK_END);
			while (ftell(arqFinal) != tamArquivo)
			{
				fwrite("j", 1, sizeof(char), arqFinal);
			}

			fseek(arqFinal, pos, SEEK_SET);				// voltando para a posição do registro a ser compactado
			fwrite(bloco, tamTotal, 1, arqFinal); // inserindo o bloco na posicao

			fseek(arqFinal, pos, SEEK_SET);

			tamTotal = 0;

			printf("-----------------------------\n");
		}
	}
}