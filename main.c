#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include "estruturas.h"

//esse eh o arquivo que iremos escrever
//o deixamos como variavel global

//também deixaremos as tabelas aleatórias como variáveis globais
uint64_t tabelasAleatorias[8][0x100];

struct pos_tabela {
  uint64_t chave;
  //'0' se posição for vazia, '1' se for liberada, '2' se for ocupada
  vEB *V;
  unsigned int flag : 2;
};

struct hashtable {
  //consiste em um vetor de inteiros
  posTabela *tabela;

  //o número de elementos suportado pela tabela
  uint64_t tamanho;

  //precisaremos desse valor para implementarmos a operacao de "limpar"
  uint64_t numeroDeCodigosDeRemocao;
};

//hashtable_dinamica implementa table doubling/halving
struct hashtable_dinamica {
  uint64_t tamanho;
  uint64_t numeroElementos;

  HashTable *hashtable;
};

struct vEB {
  uint64_t w;

  uint64_t *min;
  uint64_t *max;
  vEB *resumo;
  HashTableDinamica *cluster;
};




//Funções da tabela de dispersão normal

HashTable *criaHashTable (uint64_t tamanho) {

  HashTable *H = malloc(sizeof(HashTable));

  H->tabela = malloc(tamanho*sizeof(posTabela));
  for (uint64_t i = 0; i < tamanho; i++) {
    (H->tabela)[i].chave = 0;
    (H->tabela)[i].flag = 0;
  }

  H->tamanho = tamanho;

  return H;

}

//a nossa função de dispersão
uint64_t funcHashTable (uint64_t chave) {

  uint64_t valor = 0;

  for (uint64_t i = 0; i < 8; i++) {
    //a cada iteração fazemos um 'XOR' entre o valor anterior e o valor a que a tabela mapeia um determinado grupo de dígitos de 'chave'

    valor ^= (tabelasAleatorias[i][chave%(0x100)]);

    chave >>= 8;

  }

  return valor;
}

//retorna nulo se a tabela nao for encontrada
vEB *buscaHashTable (HashTable *H, uint64_t chave) {

  uint64_t funcChave = funcHashTable(chave);
  uint64_t posChave = funcChave%(H->tamanho);
  uint64_t posChaveInicial = posChave;

  while (posChave < H->tamanho) {

    //'0' indica que espaço é vazio
    if (H->tabela[posChave].flag == 0) {
      return NULL;
    }

    if ( (H->tabela[posChave].flag == 2) && (H->tabela[posChave].chave == chave) ) {
      return H->tabela[posChave].V;
    }

    posChave += 1;
  }

  //se chegamos no fim da tabela e ainda nao encontramos a resposta voltamos para o inicio
  posChave = 0;
  
  while (posChave < posChaveInicial) {

    //'0' indica que espaço é vazio
    if (H->tabela[posChave].flag == 0) {
      return NULL;
    }

    if ( (H->tabela[posChave].flag == 2) && (H->tabela[posChave].chave == chave) ) {
      return H->tabela[posChave].V;
    }

    posChave += 1;
  }

  return NULL;
}

//retorna '1' em caso de sucesso e '0' caso contrário
//'imprimir' indica se devemos escrever algo no arquivo de texto
int insereHashTable (HashTable *H, uint64_t chave, vEB *V) {

  uint64_t funcChave = funcHashTable(chave);
  uint64_t posChave = funcChave%(H->tamanho);
  uint64_t posChaveInicial = posChave;

  while (posChave < H->tamanho) {

    //flag diferente de '2' significa que a posicao é vazia ou liberada
    if ( H->tabela[posChave].flag != 2 ) {
      H->tabela[posChave].chave = chave;
      H->tabela[posChave].V = V;
      H->tabela[posChave].flag = 2;

      return 1;
    }

    posChave +=1;
  }

  //chegamos ao final do vetor 'tabela' e ainda nao encontramos posicao vazia, então passamos a buscar do começo do vetor
  posChave = 0;

  while (posChave < posChaveInicial) {

    //flag diferente de '2' significa que a posicao é vazia ou liberada
    if ( H->tabela[posChave].flag != 2 ) {
      H->tabela[posChave].chave = chave;
      H->tabela[posChave].V = V;
      H->tabela[posChave].flag = 2;

      return 1;
    }
    posChave += 1;
  }

  return 0;
}

//limpamos os codigos de remocao simplesmente reconstruindo a tabela
void limpaCodigosRemocao (HashTable *H) {

  uint64_t* tabelaAuxiliar = malloc(sizeof(uint64_t)*H->tamanho); 

  uint64_t cont = 0;
  for (uint64_t i = 0; i < H->tamanho; i++) {

    if (H->tabela[i].flag == 2) {
      //Adicionando todas as chaves a tabela auxiliar
      tabelaAuxiliar[cont] = H->tabela[i].chave;
      cont++;
    }
    H->tabela[i].chave = 0;
    H->tabela[i].V = NULL;
    H->tabela[i].flag = 0;

  }

  for (uint64_t i = 0; i < cont; i++) {
    //Reinserimos todas as chaves
    insereHashTable(H, tabelaAuxiliar[i], 0);
  }

  H->numeroDeCodigosDeRemocao = 0;

  free(tabelaAuxiliar);

}

//retorna '1' em caso de sucesso e '0' caso contrário
uint64_t removeHashTable (HashTable *H, uint64_t chave) {
  
  uint64_t funcChave = funcHashTable(chave);
  uint64_t posChave = funcChave%(H->tamanho);
  uint64_t posChaveInicial = posChave;

  while (posChave < H->tamanho) {

    //'0' indica que espaço é vazio
    if (H->tabela[posChave].flag == 0) {
      return 0;
    }
    
    if ( (H->tabela[posChave].flag == 2) && (H->tabela[posChave].chave == chave)) {
      //liberamos o espaco
      H->tabela[posChave].flag = 1;
      H->tabela[posChave].chave = 0;
      H->tabela[posChave].V = NULL;

      H->numeroDeCodigosDeRemocao += 1;
      if (H->numeroDeCodigosDeRemocao >= H->tamanho/4) {
        limpaCodigosRemocao(H);
      }

      return 1;
    }
    posChave += 1;
  }

  //chegamos ao final do vetor 'tabela' e ainda nem encontramos a chave nem alcançamos uma posição vazia, entao passamos a buscar do começo do vetor
  posChave = 0;

  while (posChave < posChaveInicial) {

    //'0' indica que espaço é vazio
    if (H->tabela[posChave].flag == 0) {
      return 0;
    }

    if ( (H->tabela[posChave].flag == 2) && (H->tabela[posChave].chave == chave)) {
      //liberamos o espaco
      H->tabela[posChave].flag = 1;
      H->tabela[posChave].chave = 0;
      H->tabela[posChave].V = NULL;

      H->numeroDeCodigosDeRemocao += 1;
      if (H->numeroDeCodigosDeRemocao >= H->tamanho/4) {
        limpaCodigosRemocao(H);
      }

      return 1;
    }

    posChave += 1;

  }

  return 0;
}

void deletaHashTable (HashTable *H) {
  
  free(H->tabela);
  free(H);
  
}



//Funções da HashTable hashtableDinamica

uint64_t geraNumeroAleatorio () {

  uint64_t randomInt = 0;
  int logRandMax = log2(RAND_MAX)+1;
  int cont = 64;
  while (cont >= logRandMax) {
    cont -= logRandMax;
    randomInt = (randomInt<<logRandMax) + rand();
  }
  randomInt = (randomInt<<cont) + rand()%((int) pow(2,cont));

  return randomInt;
}

void preencheTabelaAleatoria (uint64_t tabelaAleatoria[0x100]) {

  //a tabelaAleatoria tem 2^8 posicoes
  for (uint64_t i = 0; i < 0x100; i++) {
    tabelaAleatoria[i] = geraNumeroAleatorio();
  }

}

HashTableDinamica *criaHashTableDinamica () {
  HashTableDinamica *T = (HashTableDinamica *)malloc(sizeof(HashTableDinamica));

  T->tamanho = 4;
  T->numeroElementos = 0;
  T->hashtable = criaHashTable(4);

  return T;
}

//deleta uma HashTableDinamica vazia
void deletaHashTableDinamica (HashTableDinamica *T) {
  deletaHashTable(T->hashtable);
  free(T);
}

//retorna NULL caso nada seja encontrado
vEB *buscaHashTableDinamica (HashTableDinamica *T, uint64_t chave) {

  return buscaHashTable (T->hashtable, chave);

}

//retorna '1' em caso de sucesso e '0' caso contrário
int insereHashTableDinamica (HashTableDinamica *T, uint64_t chave, vEB *V) {

  int res = insereHashTable(T->hashtable, chave, V);
  T->numeroElementos +=1;

  //O 'tamanho' sempre será pelo menos o dobro do número de elementos (epsilon = 1, o que significa que o tempo por operação deve ser O(1))
  if (T->tamanho == 2*T->numeroElementos) {

    HashTable *H = criaHashTable(2*(T->tamanho));
    for (uint64_t i = 0; i < (T->tamanho); i++) {
      if ( T->hashtable->tabela[i].flag == 2 ) {
        insereHashTable(H, T->hashtable->tabela[i].chave, T->hashtable->tabela[i].V);
      }
    }

    deletaHashTable(T->hashtable);
    T->hashtable = H;
    T->tamanho = 2*(T->tamanho);

  }

  return res;
}

//retorna '1' em caso de sucesso e '0' caso contrário
int removeHashTableDinamica (HashTableDinamica *T, uint64_t chave) {

  int res = removeHashTable(T->hashtable, chave);

  if (res) {
    T->numeroElementos -= 1;

    if (T->tamanho == 8*T->numeroElementos) {

      HashTable *H = criaHashTable((T->tamanho)/2);
      for (uint64_t i = 0; i < (T->tamanho); i++) {
        if ( T->hashtable->tabela[i].flag == 2 ) {
          insereHashTable(H, T->hashtable->tabela[i].chave, T->hashtable->tabela[i].V);
        }
      }

      deletaHashTable(T->hashtable);
      T->hashtable = H;
      T->tamanho = (T->tamanho)/2;

    }

  }

  return res;
}



//Funções da Arvore de Van Emde Boas

vEB *criaVEB (uint64_t w) {

  vEB *V = (vEB *)malloc(sizeof(vEB));

  V->w = w;
  //V->min = malloc(sizeof(uint64_t));
  V->min = NULL;
  //V->max = malloc(sizeof(uint64_t));
  V->max = NULL;
  V->resumo = NULL;
  V->cluster = malloc(sizeof(HashTableDinamica));
  V->cluster = criaHashTableDinamica();

  return V;

}

//deleta uma vEB vazia
void deletaVEB (vEB *V) {
  free(V->min);
  free(V->max);
  free(V->resumo);
  deletaHashTableDinamica(V->cluster);
  free(V);
}

void printVEB (vEB *V) {
  printf("\n");

//check if min max are null!!!!!!!1
  for (int i = 0; i <= V->w; i++) {
    printf("---");
  }
  printf("\nARVORE COM W = %"PRIu64" ", V->w);
  if (V->min != NULL) {
    printf("MIN = %"PRIu64" MAX = %"PRIu64"\n", *(V->min), *(V->max));;
  }
  printf("\nRESUMO\n");
  if (V->resumo) {
    printVEB(V->resumo);
  } else {
    printf("\nNULL\n");
  }
  printf("\nCLUSTER\n");
  if (V->w == 1) {
    printf("\nNO CLUSTER\n");
  } else {
    for (int i = 0; i < pow(2,V->w/2); i++) {
      printf("\nCLUSTER %d\n", i);
      if (buscaHashTableDinamica(V->cluster, i)) {
        printVEB(buscaHashTableDinamica(V->cluster, i));
      } else {
        printf("\nCLUSTER VAZIO\n");
      }
    }
  }
  printf("FIM CLUSTERS\n\n");
  for (int i = 0; i <= V->w; i++) {
    printf("---");
  }
  printf("\n");
  
}


//c, seguindo a nomenclatura vista em sala, corresponde ao numero formado pelos primeiros w/2 bits da chave
uint64_t get_c (uint64_t chave, uint64_t w) {

  return chave >> (w/2);
}

//i, seguindo a nomenclatura vista em sala, corresponde ao numero formado pelos ultimos w/2 bits da chave
uint64_t get_i (uint64_t chave, uint64_t w) {

  uint64_t c = get_c(chave, w);

  return chave - ( c << (w/2) );

}

//obtem-se a chave a partir do 'c' e do 'i'
uint64_t get_chave (uint64_t c, uint64_t i, uint64_t w) {

  return i + (c << w/2);

}

int buscaVEB (vEB *V, uint64_t chave) {

  if (V->min) {
    if ( (chave == *(V->min)) || (chave == *(V->max)) ) {
      printf("Elemento encontrado");
      return 1;
    }
  }

  if (V->w == 1) {
    return 0;
  }

  uint64_t c = get_c(chave, V->w);
  uint64_t i = get_i(chave, V->w);

  if (!buscaHashTableDinamica(V->cluster, c)) {
    return 0;
  }

  return buscaVEB(buscaHashTableDinamica(V->cluster, c), i);

}


int insereElementoNaoArmazenadoVEB (vEB *V, uint64_t chave, vEB *X) {

  if (V->min == NULL) {

    V->min = malloc(sizeof(uint64_t));
    V->max = malloc(sizeof(uint64_t));


    *(V->min) = chave;
    *(V->max) = chave;

  } else { 

    if ( chave < *(V->min) ) {
      //trocamos chave e V->min
      uint64_t novaChave = *(V->min);
      *(V->min) = chave;
      chave = novaChave;
    } 
    
    if ( chave > *(V->max) ) {
      *(V->max) = chave;
    }

    if (V->w > 1) {

      uint64_t c = get_c(chave, V->w);
      uint64_t i = get_i(chave, V->w);

      if (!buscaHashTableDinamica(V->cluster, c)) {

        if ( !(V->resumo) ) {
          V->resumo = criaVEB((V->w)/2);
          /*
          V->resumo = malloc(sizeof(vEB));
          //V->resumo = criaVEB( (V->w)/2 );
          V->resumo->w = (V->w)/2;
          V->resumo->cluster = criaHashTableDinamica();
          V->resumo->min = NULL;
          V->resumo->max = NULL;
          V->resumo->resumo = NULL;
          */
        }

        insereElementoNaoArmazenadoVEB(V->resumo, c, V);
        /*
        vEB *X = malloc(sizeof(vEB));
        X->min = NULL;
        X->max = NULL;
        X->resumo = NULL;
        X->w = (V->w)/2;
        X->cluster = criaHashTableDinamica();
        */
        insereHashTableDinamica(V->cluster, c, criaVEB((V->w)/2) );
      }

      insereElementoNaoArmazenadoVEB(buscaHashTableDinamica(V->cluster, c), i, V);

    }

  }

  return 0;

}

void insereVEB (vEB *V, uint64_t chave) {

  if (!buscaVEB(V, chave)) {
    insereElementoNaoArmazenadoVEB(V, chave, V);
  }
}

/*
void insereVEB (vEB *V, uint64_t chave) {

  if (V->min == NULL) {


    V->min = malloc(sizeof(uint64_t));
    V->max = malloc(sizeof(uint64_t));


    *(V->min) = chave;
    *(V->max) = chave;

  } else {

    if ( chave < *(V->min) ) {
      //trocamos chave e V->min
      uint64_t novaChave = *(V->min);
      *(V->min) = chave;
      chave = novaChave;

      printf("2");
    } 
    
    if ( chave > *(V->max) ) {
      *(V->max) = chave;
    }

    uint64_t c = get_c(chave, V->w);
    uint64_t i = get_i(chave, V->w);

    if ( !buscaHashTableDinamica(V->cluster, c) ) {
      if ( !(V->resumo) ) {
        V->resumo = criaVEB( (V->w)/2 );
      }
      insereVEB(V->resumo, c);
    }

    if ( !buscaHashTableDinamica(V->cluster, c) ) {
      insereHashTableDinamica(V->cluster, c, criaVEB( (V->w)/2 ));
    }
    insereVEB(buscaHashTableDinamica(V->cluster, c), i);

  }

}
*/

int removeVEB (vEB *V, uint64_t chave) {

  uint64_t c = get_c(chave, V->w);
  uint64_t i = get_i(chave, V->w);

  if (V->min == NULL) {
    return 0;
  }

  if (chave == *(V->min)) {
    
    if (V->resumo) {

      c = *(V->resumo->min);
      i = *(buscaHashTableDinamica(V->cluster, c)->min);
      *(V->min) = get_chave(c, i, V->w);
      chave = *(V->min);

    } else {

      V->min = NULL;
      return 0;

    }

  }

  if (buscaHashTableDinamica(V->cluster, c)) {
    removeVEB(buscaHashTableDinamica(V->cluster, c), i);
    if (buscaHashTableDinamica(V->cluster, c)->min == NULL) {
      vEB *arvoreVazia = buscaHashTableDinamica(V->cluster, c);
      removeHashTableDinamica(V->cluster, c);
      deletaVEB(arvoreVazia);
      removeVEB(V->resumo, c);
    }
    if (V->resumo->min == NULL) {
      V->max = V->min;
      deletaVEB(V->resumo);
    } else {
      uint64_t novo_c = *(V->resumo->max);
      *(V->max) = get_chave(novo_c, *(buscaHashTableDinamica(V->cluster, novo_c)->max), V->w);
    }
  }

  return 0;

}

//retorna NULL caso nao haja sucessor
uint64_t *sucessorVEB (vEB *V, uint64_t chave) {

  //uint64_t sucessorValor;
  uint64_t *sucessor;
  //sucessorValor;
  sucessor = malloc(sizeof(uint64_t));

  if (V->w == 1) {
    if ( V->max ) {
      if ( (chave == 0) && (*(V->max) == 1) ) {
        *sucessor = 1;
        return sucessor;
      }
    }
    return NULL;
  }

  if ( (V->min) && (chave < *(V->min))) {
    *sucessor = *(V->min);
    return sucessor;
  }

  uint64_t maxLow;
  uint64_t offset;
  uint64_t c = get_c(chave, V->w);
  uint64_t i = get_i(chave, V->w);

  if (buscaHashTableDinamica(V->cluster, c)) {
    maxLow = *(buscaHashTableDinamica(V->cluster, c)->max);
    if (i < maxLow) {
      offset = *sucessorVEB(buscaHashTableDinamica(V->cluster, c), i);
      *sucessor = get_chave(c, offset, V->w);
      return sucessor;
    }
  }

  if (V->resumo) {
    uint64_t *succCluster = sucessorVEB(V->resumo, c);
    if (succCluster) {
      offset = *(buscaHashTableDinamica(V->cluster, *succCluster)->min);
      *sucessor = get_chave(*succCluster, offset, V->w);
      return sucessor;
    }
  }

  return NULL;


/*
  if ( V->min == NULL ) {
    return NULL;
  }

  if ( chave < *(V->min) ) {
    return V->min;
  } 

  if ( buscaHashTableDinamica(V->cluster, c) && ( i < *(buscaHashTableDinamica(V->cluster, c)->max) ) ) {
    *sucessor = get_chave(c, *sucessorVEB(buscaHashTableDinamica(V->cluster, c) , i), V->w);
    return sucessor;
  }

  if ( (V->resumo != NULL) && (sucessorVEB(V->resumo, c)) ) {
    uint64_t new_c = *sucessorVEB(V->resumo, c);
    *sucessor = get_chave( new_c, *(buscaHashTableDinamica( V->cluster, new_c)->min), V->w );
    return sucessor;
  } else {
    return NULL;
  }
  */

}

//retorna NULL caso nao haja predecessor
uint64_t *predecessorVEB (vEB *V, uint64_t chave) {

  //uint64_t sucessorValor;
  uint64_t *predecessor;
  //sucessorValor;
  predecessor = malloc(sizeof(uint64_t));

  if (V->w == 1) {
    if ( V->min ) {
      if ( (chave == 1) && (*(V->min) == 0) ) {
        *predecessor = 0;
        return predecessor;
      }
    }
    return NULL;
  }

  if ( (V->max) && (chave > *(V->max))) {
    *predecessor = *(V->max);
    return predecessor;
  }

  uint64_t minLow;
  uint64_t offset;
  uint64_t c = get_c(chave, V->w);
  uint64_t i = get_i(chave, V->w);

  if (buscaHashTableDinamica(V->cluster, c)) {
    minLow = *(buscaHashTableDinamica(V->cluster, c)->min);
    if (i > minLow) {
      offset = *predecessorVEB(buscaHashTableDinamica(V->cluster, c), i);
      *predecessor = get_chave(c, offset, V->w);
      return predecessor;
    }
  }

  if (V->resumo) {
    uint64_t *predCluster = predecessorVEB(V->resumo, c);
    if (predCluster) {
      offset = *(buscaHashTableDinamica(V->cluster, *predCluster)->max);
      *predecessor = get_chave(*predCluster, offset, V->w);
      return predecessor;
    }
  }

  if ( (V->min) && (chave > *(V->min)) ) {
    *predecessor = *(V->min);
    return predecessor;
  }

  return NULL;

  /*

  uint64_t *predecessor;

  uint64_t c = get_c(chave, V->w);
  uint64_t i = get_i(chave, V->w);

  if ( V->min == NULL ) {
    return NULL;
  }

  if ( chave > *(V->max) ) {
    return V->max;
  } 

  if ( buscaHashTableDinamica(V->cluster, c) && ( i > *(buscaHashTableDinamica(V->cluster, c)->min) ) ) {
    *predecessor = get_chave( c, *predecessorVEB(buscaHashTableDinamica(V->cluster, c) , i), V->w);
  }

  if ( (V->resumo != NULL) && (predecessorVEB(V->resumo, c)) ) {
    uint64_t new_c = *predecessorVEB(V->resumo, c);
    *predecessor = get_chave( new_c, *(buscaHashTableDinamica( V->cluster, new_c)->max), V->w );
    return predecessor;
  } else {
    return NULL;
  }
  */

}





int main(int argc, char **argv) {

  for (uint64_t i = 0; i < 8; i++) {
    preencheTabelaAleatoria (tabelasAleatorias[i]);
  }

  
  vEB *V = criaVEB(4);

/*
  for (int i=0; i<6; i=i+2) {
    printf("\n%d", i);
    insereVEB(V, i);
  }
  */

  insereVEB(V,5);
  insereVEB(V,7);
  insereVEB(V,15);
  insereVEB(V,3);
  insereVEB(V,2);
  insereVEB(V,14);
  insereVEB(V,4);
  insereVEB(V,15);
  insereVEB(V,3);
  insereVEB(V,2);
  insereVEB(V,14);
  insereVEB(V,3);
  insereVEB(V,2);


/*
  for (int i = 0; i < 16; i=i+1) {
    insereVEB(V,i);
  }

  printf("HERE");

  for (int i = 0; i < 16; i=i+1) {
    insereVEB(V,i);
  }
  */


  /*
  for (int i=0; i<16; i=i+2) {
    printf("\n%d", i);
    insereVEB(V, i);
  }
  for (int i=1; i<16; i=i+2) {
    printf("\n%d", i);
    insereVEB(V, i);
  }
  */

  //printVEB(V);

  /*

  if ( (!argv[1]) || (!argv[2]) ) {
    printf("Por favor passe o caminho do arquivo a ser lido e o do arquivo a ser criado como parâmetros para o programa.\n");
    exit(1);
  }

  FILE *file_in = fopen(argv[1], "r");
  if ( !file_in ) {
    printf("Erro abrindo o arquivo. Cheque se o caminho do arquivo a ser lido passado para o programa está correto.");
    exit(1);
  }
  FILE *file_out = fopen(argv[2], "w");
  if ( !file_out ) {
    printf("Erro criando o arquivo. Cheque se o caminho do arquivo a ser criado passado para o programa está correto.");
    exit(1);
  }

  HashTableDinamica *T = criaHashTableDinamica();

  char line[256];
  uint64_t chave;
  while (fgets(line, sizeof(line), file_in)) {

    fprintf(file_out, "%s", line);
    if (line[strlen(line)-1] != '\n') {
      fprintf(file_out, "%s", "\n");
    } 
      
    char c;
    sscanf(line+4, "%"PRIu64"%c", &chave, &c);

    switch (line[0]) {
      case 'I': 
        insereVEB(V, chave);
        break;
      case 'S': 
        fprintf(file_out, "%"PRIu64"", *sucessorVEB(V, chave));
        break;
      case 'P': 
        fprintf(file_out, "%"PRIu64"", *predecessorVEB(V, chave));
        break;
      case 'R': 
        removeVEB(V, chave);
        break;
    }

    fprintf(file_out, "\n");
  }

  fclose(file_in);
  fclose(file_out);

  */

  return 0;
}
