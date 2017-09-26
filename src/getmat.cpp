#include <iostream>
#include <fstream>
#include <cstring>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <string>
#include <unistd.h>  
#include <dirent.h> 
#include <vector>

using namespace std;
time_t startt, endt;
vector<string> filename;
int minnum;

int Jacard(int* A, int* B)
{
    int* uion = new int [minnum];
    // x = h(A U B)
    int i = 0;
    int j = 0;
    int k = 0;
    while(k < minnum)
    {
        if (A[i] < B[j])
        {
            uion[k] = A[i];
            i++;
        }
        else if (B[j] < A[i])
        {
            uion[k] = B[j];
            j++;
        }
        else
        {
            uion[k] = A[i];
            i++;
            j++;
        }
        k++;
    }

    // x & A & B
    i = 0;
    j = 0;
    k = 0;
    int cnt = 0;
    while((i < minnum) && (j < minnum) && (k < minnum))
    {
        if ((A[i] == B[j]) && (A[i] == uion[k]))
        {
            cnt += 1;
            i++;
            j++;
            k++;
        }
        else
        {
            if ((uion[k] <= A[i]) && (uion[k] <= B[j]))
            {
                k++;
            }
            else
            {
                if (A[i] <= B[j])
                    i++;
                else
                    j++;
            }
        }
    }

    delete [] uion;
    return cnt;
}

void readFile(char* fname, int** index, int start, int filenum)
{
	ifstream fin(fname);
	char tmp[100];
	for (int i = 0; i < 7; i++)
		fin >> tmp;
	for (int i = 0; i < filenum; i++)
	{
		fin >> tmp;
		filename.push_back(tmp);
		for (int j = 0; j < minnum; j++)
			fin >> index[start+i][j];
	}
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("the paramter is too less, must contain two input file\n");
        exit(1);
    }

    float thresh = 0.01;
    if (argc >= 4)
    	thresh = atof(argv[3]);
    time(&startt);
	FILE* fin;
    fin = fopen(argv[1], "r");
    char getfilen[100];
    for (int i = 0; i < 5; i++)
        fgets(getfilen, 100, fin);
    char tmp[100];
    strcpy(tmp, getfilen+8);
    minnum = (int)atof(tmp);

    fgets(getfilen, 100, fin);
    fgets(getfilen, 100, fin);
    strcpy(tmp, getfilen+9);
    int filenum1 = (int)atof(tmp);

    fclose(fin);

    fin = fopen(argv[2], "r");
    for (int i = 0; i < 7; i++)
        fgets(getfilen, 100, fin);
    strcpy(tmp, getfilen+9);
    int filenum2 = (int)atof(tmp);
    fclose(fin);
    
    int filenum = filenum1 + filenum2;
    if (strcmp(argv[1], argv[2]) == 0)
    	filenum = filenum1;    
    
    int** index;
    double** mat;
    index = new int* [filenum];
    mat = new double* [filenum];

    for (int i = 0; i < filenum; i++)
    {
        mat[i] = new double [filenum];
        mat[i][i] = 1.0;
        index[i] = new int [minnum];
    }
    readFile(argv[1], index, 0, filenum1);
    if (strcmp(argv[1], argv[2]) != 0)
    	readFile(argv[2], index, filenum1, filenum);

    for (int i = 0; i < filenum; i++)
        for (int j = i+1; j < filenum; j++)
        {
            int num = Jacard(index[i], index[j]);
            mat[i][j] = double(num) / minnum;
            mat[j][i] = double(num) / minnum;
        }

    string outfile = "result.mat";
    string outfilen = "result.name";
    ofstream fout(outfile.c_str());
    ofstream foutn(outfilen.c_str());
    foutn << filenum1 << ' ' << (filenum - filenum1) << ' ' << thresh << endl;
    for (int i = 0; i < filenum; i++)
    {
        foutn << filename[i] << endl;
        for (int j = 0; j < filenum; j++)
            fout << mat[i][j] << ' ';
        fout << endl;
    }

    time(&endt);
    printf("similarity matrix is computed, using %d CPU senconds\n", ((unsigned int)(endt - startt)));
    
    return 0;
}
