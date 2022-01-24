#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**********************************GLOBAIS*************************************/
#define THRESHOLD 150
#define MAX_WHITE 255
#define MAX_BLACK 0

/*********************************ESTRUTURAS***********************************/
enum EType
{
  INPUT_FILE = 0,
  OUTPUT_FILE
};

enum ELength
{
  ROWS = 0,
  COLUMNS
};

enum EHeader
{
  IMG_MATRIX = 1,
  MAX_GRAY
};

typedef struct SLength
{
  int cols, rows;
  int whichIs;
} Length;

typedef struct SHeader
{
  char *fullHeader;
  int maxGray;
  Length length;
} Header;

typedef struct SImage
{
  Header header;
  int *imgArray;
} Image;

/********************************PROTOTIPOS************************************/
FILE *openFile(char *mode, char *type, int method);
Image loadImage();
void setImgHeader(FILE *pgmFile, Image *img);
void setImgArray(FILE *pgmFile, Image *img);
char *parseImgArrayToPgmFile(Image imgThold, int rows, int cols);
void writeInFile(char *str, FILE *pgm, int printMsg);
void createThresholdingImg(Image img);
void createNegativeImg(Image img);
void generateHistogram(Image img);

/**********************************MAIN****************************************/
int main(void)
{
  char op;
  Image img;

  printf("\n*----------------------------------------------------------------------------------*\n");
  printf("*----------------------------------------------------------------------------------*\n");
  printf("*---------------- BEM VINDO AO SISTEMA DE PROCESSAMENTO DE IMAGENS ----------------*\n");
  printf("*----------------------------------------------------------------------------------*\n");
  printf("*----------------------------------------------------------------------------------*\n\n");

  printf("Antes de tudo, entre, somente, com o NOME da imagem. Lembrando que DEVE ser (.pmg)\n\n");
  img = loadImage();
  system("cls");

  do
  {
    printf("*----------------------------------------------------------------------------------*\n");
    printf("*---------------- QUAL OPERACAO VOCE DESEJA REALIZAR COM A IMAGEM? ----------------*\n");
    printf("*----------------------------------------------------------------------------------*\n\n");

    printf("*-------------------MENU------------------*\n");
    printf("| Digite   |             Para             |\n");
    printf("*-----------------------------------------*\n");
    printf("|   L      | Criar uma imagem limiarizada |\n");
    printf("|   N      | Criar uma imagem negativa    |\n");
    printf("|   H      | Gerar histograma             |\n");
    printf("|   S      | Sair do sistema              |\n");
    printf("*-----------------------------------------*\n\n");

    printf("Digite aqui: ");

    fflush(stdin);
    scanf("%c", &op);
    system("cls");

    switch (op)
    {
    case 'L':
    {
      createThresholdingImg(img);
      break;
    }
    case 'N':
    {
      createNegativeImg(img);
      break;
    }
    case 'H':
    {
      generateHistogram(img);
      break;
    }
    case 'S':
    {
      free(img.imgArray);
      printf("Finalizando o sistema...");
      break;
    }
    default:
      printf("ERRO: Opcao invalida\n");
    }
  } while (op != 'S');
}

/*********************************FUNCOES**************************************/
Image loadImage()
{
  //Abrir arquivo PGM
  FILE *pgmFile;
  Image img;

  do
  {
    pgmFile = openFile("rb", ".pgm", INPUT_FILE);

    if (pgmFile == NULL)
      printf("\nNao foi possivel abrir o arquivo! Verifique se o nome esta correto.\n\n");

  } while (pgmFile == NULL);

  //Obter cabeçalho da imagem
  setImgHeader(pgmFile, &img);

  //Obter matriz da imagem
  setImgArray(pgmFile, &img);

  printf("A imagem PGM foi carregada com sucesso!\n\n");
  return img;
}

FILE *openFile(char *mode, char *type, int method)
{
  FILE *file;
  char *path = malloc(sizeof(char) * 5000);

  if (method == INPUT_FILE)
    printf("Digite aqui o nome do arquivo %s de entrada: ", type);
  else if (method == OUTPUT_FILE)
    printf("Digite aqui o nome do arquivo %s de saida: ", type);

  scanf("%s", path);

  path = strcat(path, type);
  file = fopen(path, mode);

  free(path);
  return file;
}

void setImgHeader(FILE *pgmFile, Image *img)
{
  img->header.length.whichIs = 0;

  //Contadores do Header
  int ctNl = 0;
  int ctHeader = 0;
  int ctCols = 0;
  int ctRows = 0;
  int ctMaxGray = 0;

  int data;
  char parsed;

  char *rows = malloc(sizeof(char));
  char *cols = malloc(sizeof(char));
  char *maxGray = malloc(sizeof(char));

  img->header.fullHeader = (char *)malloc(sizeof(char));

  //Define o cabecalho da imagem
  while (ctNl < 3)
  {
    data = fgetc(pgmFile);
    parsed = (char)data;

    if (ctNl == MAX_GRAY)
    {
      maxGray = realloc(maxGray, sizeof(char) * (ctMaxGray + 1));
      maxGray[ctMaxGray] = parsed;
      ctMaxGray++;
    }

    if (ctNl == IMG_MATRIX)
    {
      if (parsed == ' ')
      {
        img->header.length.whichIs = 1;
      }

      if (img->header.length.whichIs == ROWS)
      {
        rows = realloc(rows, sizeof(char) * (ctRows + 1));
        rows[ctRows] = parsed;
        ctRows++;
      }
      else if ((img->header.length.whichIs == COLUMNS))
      {
        cols = realloc(cols, sizeof(char) * (ctCols + 1));
        cols[ctCols] = parsed;
        ctCols++;
      }
    }

    if (data == '\n')
      ctNl++;

    img->header.fullHeader = (char *)realloc(img->header.fullHeader, sizeof(char) * (ctHeader + 1));
    img->header.fullHeader[ctHeader] = parsed;
    ctHeader++;
  }

  //Adicionando o '\0' que indica o final da string
  img->header.fullHeader[ctHeader] = '\0';

  img->header.length.rows = atoi(rows);
  img->header.length.cols = atoi(cols);
  img->header.maxGray = atoi(maxGray);

  free(rows);
  free(cols);
  free(maxGray);

  return;
}

void setImgArray(FILE *pgmFile, Image *img)
{
  img->imgArray = (int *)malloc(sizeof(int));

  int rows = img->header.length.rows;
  int cols = img->header.length.cols;

  int i, j;

  int data;
  char dataParsed;

  char *aux;
  int ctAux = 0;
  int parsedInt;

  //Alocando espaço para matriz da imagem
  img->imgArray = malloc(cols * rows * sizeof(int));
  aux = malloc(sizeof(char));

  //Obter informações da matriz da imagem
  for (i = 0; i < rows; i++)
  {
    for (j = 0; j < cols;)
    {
      data = fgetc(pgmFile);

      if (data == ' ')
      {
        aux[ctAux] = '\0';
        img->imgArray[(i * cols) + j] = atoi(aux);
        free(aux);
        aux = malloc(sizeof(char));

        ctAux = 0;
        j++;

        continue;
      }

      dataParsed = (char)data;
      aux = realloc(aux, sizeof(char) * (ctAux + 1));
      *(aux + ctAux) = dataParsed;
      ctAux++;

      continue;
    }
  }

  free(aux);
  return;
}

char *parseImgArrayToPgmFile(Image img, int rows, int cols)
{
  int i, j;

  char *parser = malloc(sizeof(char));
  int ctImg = 0;
  int ctStr = 0;

  char *imgParsed = malloc(sizeof(char));

  //Converter matriz img de INT para STRING
  for (i = 0; i < rows; i++)
    for (j = 0; j < cols; j)
    {
      //Dispara quando encontra um espaço
      if (ctStr == strlen(parser))
      {
        ctStr = 0;
        j++;
        imgParsed = realloc(imgParsed, sizeof(char) * (ctImg + 1));
        imgParsed[ctImg] = ' ';
        ctImg++;
        continue;
      }

      sprintf(parser, "%d", img.imgArray[(i * cols) + j]);

      imgParsed = realloc(imgParsed, sizeof(char) * (ctImg + 1));
      imgParsed[ctImg] = *(parser + ctStr);

      ctImg++;
      ctStr++;
    }

  imgParsed[ctImg] = '\0';
  free(parser);

  return imgParsed;
}

void writeInFile(char *str, FILE *pgm, int printMsg)
{
  if (fputs(str, pgm) == EOF)
    printf("ERRO: O processamento falhou!\n");

  if (printMsg == 1)
    printf("\nArquivo criado com sucesso! Verifique a pasta de destino.\n\n");
}

void createThresholdingImg(Image img)
{
  printf("*------------------------------------------------------------*\n");
  printf("*----------------- CRIAR IMAGEM LIMIARIZADA -----------------*\n");
  printf("*------------------------------------------------------------*\n\n");

  FILE *pgmThresholded;

  int i, j;
  char *parsed;

  Image imgThold;
  char *imgTholdParsed;

  imgThold.header.fullHeader = img.header.fullHeader;

  int rows = img.header.length.rows;
  int cols = img.header.length.cols;

  imgThold.imgArray = malloc(rows * cols * sizeof(int));

  //Limiarização da imagem
  for (i = 0; i < rows; i++)
    for (j = 0; j < cols; j++)
    {
      if (img.imgArray[(i * cols) + j] > THRESHOLD)
        imgThold.imgArray[(i * cols) + j] = MAX_WHITE;
      else
        imgThold.imgArray[(i * cols) + j] = MAX_BLACK;
    }

  imgTholdParsed = parseImgArrayToPgmFile(imgThold, rows, cols);
  pgmThresholded = openFile("wb", ".pgm", OUTPUT_FILE);

  writeInFile(imgThold.header.fullHeader, pgmThresholded, 0);
  writeInFile(imgTholdParsed, pgmThresholded, 1);

  free(imgTholdParsed);
  fclose(pgmThresholded);
  return;
}

void createNegativeImg(Image img)
{
  printf("*------------------------------------------------------------*\n");
  printf("*------------------ CRIAR IMAGEM NEGATIVA -------------------*\n");
  printf("*------------------------------------------------------------*\n\n");

  FILE *pgmNegative;
  int i, j;

  Image imgNegative;
  char *imgNegativeParsed;

  int rows = img.header.length.rows;
  int cols = img.header.length.cols;

  imgNegative.header = img.header;
  imgNegative.imgArray = malloc(rows * cols * sizeof(int));
  int v;
  int value;

  //Tornar a imagem negativa
  for (i = 0; i < rows; i++)
    for (j = 0; j < cols; j++)
    {
      value = img.imgArray[(i * cols) + j];
      v = (255 - value);
      imgNegative.imgArray[(i * cols) + j] = v;
    }

  imgNegativeParsed = parseImgArrayToPgmFile(imgNegative, rows, cols);
  pgmNegative = openFile("wb", ".pgm", OUTPUT_FILE);

  writeInFile(imgNegative.header.fullHeader, pgmNegative, 0);
  writeInFile(imgNegativeParsed, pgmNegative, 1);

  free(imgNegativeParsed);
  fclose(pgmNegative);
  return;
}

void generateHistogram(Image img)
{
  printf("*------------------------------------------------------------*\n");
  printf("*---------------- GERAR HISTOGRAMA DA IMAGEM ----------------*\n");
  printf("*------------------------------------------------------------*\n\n");

  FILE *histogram;

  int maxGray = img.header.maxGray;
  int grays[maxGray], ctHistogram = 0;

  int rows = img.header.length.rows;
  int cols = img.header.length.cols;

  int i, j, k, ctStr = 0;

  char *parser = (char *)malloc(sizeof(char));
  char *graysParsed = (char *)malloc(sizeof(char));

  for (k = 0; k <= maxGray;)
  {
    if (i == rows)
    {
      grays[k] = ctHistogram;
      ctHistogram = 0;
      k++;
    }

    for (i = 0; i < rows; i++)
    {
      if (i == rows)
        break;

      for (j = 0; j < cols; j++)
        if (img.imgArray[(i * cols) + j] == k)
          ctHistogram++;
    }
  }

  //Converter o array de cinzas de int para string
  for (i = 0; i <= maxGray; i++)
  {
    sprintf(parser, "%d", grays[i]);

    for (j = 0; j < strlen(parser); j++)
    {
      graysParsed = (char *)realloc(graysParsed, sizeof(char) * (ctStr + 2));
      graysParsed[ctStr] = *(parser + j);
      ctStr++;
    }

    graysParsed[ctStr] = '\n';
    ctStr++;
  }

  graysParsed[ctStr] = '\0';

  histogram = openFile("wb", ".txt", OUTPUT_FILE);
  writeInFile(graysParsed, histogram, 1);

  free(graysParsed);
  free(parser);
  fclose(histogram);

  return;
}