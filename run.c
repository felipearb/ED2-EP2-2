#include "headers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
#define bool int;
#define true 1;
#define false 0;*/

int main(){
    char input[100];
    int run=1;
    //bool valid; // indica se o registro foi eliminado ou não
    int codigoLivro; // chave
    char titulo[31];
    char nome[31];
    int anoPublicacao;
    //printf("%s",input);
    FILE *indice= fopen("indice.dat", "rb+");
    if(indice){
        printf("Arquivo indice.dat não foi encontrado!\nPrograma abortado.\n");
        return 1;
    }
    FILE *data = fopen("data.dat", "rb+");
    if(data){
        printf("Arquivo data.dat não foi encontrado!\nPrograma abortado.\n");
        return 2;
    }

    fgets(input,100,stdin);
    while(run){
        printf("input:%s",input);
        switch(input[0]){
        case 'I':
            char * temp;
            printf ("string \"%s\" into tokens:\n",input);
            temp = strtok (input," ,.-");//case

            temp = strtok (NULL, ";");//codigo
            codigoLivro=atoi(temp);
            temp = strtok (NULL, ";");//titulo
            strncpy(temp,titulo,30);
            temp = strtok (NULL, ";");//nomeCompletoPrimeiroAutor
            strncpy(temp,nome,30);
            temp= strtok (NULL, ";");//anoPublicacao
            anoPublicacao = atoi(temp);

            //Inserir(codigoLivro,titulo,nomeCompletoPrimeiroAutor,anoPublicacao)
            break;
        case 'R':
            input[0]=' ';
            codigoLivro=atoi(input);
            printf("codigo %d\n",codigoLivro);
            //remove(codigoLivro);
            break;
        case 'B':
            input[0]=' ';
            codigoLivro=atoi(input);
            printf("codigo %d\n",codigoLivro);
            /*
            type resultado=busca(codigoLivro);
            if(resultado){
                printf("O livro com código %d não existe na biblioteca\n",codigoLivro);
                break;
            }
            printf("%d %s %S %d\n",resultado->codigo,resultado->titulo,resultado->autor,resultado->ano);
            */
            break;
        case 'P':
            switch(input[1]){
                case 1:
                    //Imprime todas as páginas do índice, sem incluir as páginas que foram eliminadas
                    break;
                case 2:
                    /*Imprime todas as páginas que estão no arquivo indice.dat na ordem em que estão no arquivo, inclusive as
                    páginas que foram eliminadas.*/
                    break;
                case 3:
                    /*Imprime todos os registros que estão no arquivo data.dat na ordem em que estão no arquivo, inclusive os
                    registros que foram eliminados.*/
                    break;
                default:
                    printf("Comando P identificado, porém versão não identificada!\n");
                    break;
            }
            break;
        case 'F':
            if(indice)fclose(indice);
            if(data)fclose(data);
            run=0;
            break;
        default:
            printf("Comando não identificado!\n");
            break;
        }
        if(run)fgets(input,100,stdin);
    }
}