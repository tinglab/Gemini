#include <iostream>
#include <fstream>
#include <cstring>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <omp.h>
#include <string>

using namespace std;

time_t startt, endt;
int hashkey = 28;
const int hashlen = 300000000;
int hashcnt[hashlen];
int* indextable;
char** refkmer;
unsigned long long* refencode;
int** hitindex;


struct Parameter
{
	bool readhash;
	char hashfile[100];
	bool readkmer;
	char kmerfile[100];
	bool savehash;
	char shfile[100];
	bool savekmer;
	char skfile[100];
	int kmerlen;
	int hashnum;
	int kmernum;
	int minnum;
	bool train;
	bool test;
	char ref[100];
	char outdir[100];
	char infile[100];
	int threadnum;
}paramt;

void setDefault()
{
	paramt.readhash = false;
	paramt.readkmer = false;
	paramt.kmerlen = 18;
	paramt.hashnum = 60000000;
	paramt.kmernum = 1000000000;
	paramt.minnum = 10000;
	paramt.threadnum = 1;
	paramt.train = false;
	paramt.test = false;
}

bool checkpStrOpt(int argc, char* argv[], const char* arg, char* argStr)
{
    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], arg) == 0) {
            strcpy(argStr, argv[i+1]);
            if (strcmp(argStr, ".") == 0)
            	return(false);
            return(true);
        }
    }
    return(false);
}

bool checkUnIntOpt(int argc, char* argv[], const char* arg, int& argValue)
{
    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], arg) == 0) {
            if ((int) atof(argv[i+1]) != 0)
            	argValue = (int) atof(argv[i+1]);
            return(true);
        }
    }
    return(false);
}

bool checkBoolOpt(int argc, char* argv[], const char* arg, bool& argValue)
{
    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], arg) == 0) {
            argValue = true;
            return(true);
        }
    }
    return(false);
}

void catString(string str, int& start, int& end)
{
	int len = str.size();
	start = 0;
	end = len;
	for (int j = len-1; j >= 0; j--)
	{
		if (str[j] == '.')
			end = j;
		if (str[j] == '/')
		{
			start = j+1;
			break;
		}
	}
}

bool getParameter(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("the Parameter is too less, must contain input file and output file\n");
		return false;
	}
	
	strcpy(paramt.infile, argv[1]);
	strcpy(paramt.outdir, argv[2]);
	
	if (checkpStrOpt(argc, argv, "-rh", paramt.hashfile))
		paramt.readhash = true;
	if (checkpStrOpt(argc, argv, "-rk", paramt.kmerfile))
		paramt.readkmer = true;
	if (checkpStrOpt(argc, argv, "-sh", paramt.shfile))
		paramt.savehash = true;
	if (checkpStrOpt(argc, argv, "-sk", paramt.skfile))
		paramt.savekmer = true;
	if (checkpStrOpt(argc, argv, "--test", paramt.ref))
		paramt.test = true;

	checkBoolOpt(argc, argv, "--train", paramt.train);
	
	checkUnIntOpt(argc, argv, "-l", paramt.kmerlen);
	checkUnIntOpt(argc, argv, "-n", paramt.kmernum);
	checkUnIntOpt(argc, argv, "-m", paramt.minnum);
	checkUnIntOpt(argc, argv, "-h", paramt.hashnum);
	checkUnIntOpt(argc, argv, "-t", paramt.threadnum);

	if (paramt.train && (!paramt.savehash))
	{
		paramt.savehash = true;
		int start,end;
		catString(paramt.infile, start, end);
		start = 0;
		strncpy(paramt.shfile, paramt.infile+start, (end-start));
		paramt.shfile[end-start] = '\0';
	}

	if (paramt.test)
	{
		FILE* fin;
   		fin = fopen(paramt.ref, "r");
    	char getfilen[6][100];
    	int start[6];
    	int end[6];
    	for (int i = 0; i < 6; i++)
        {
        	fgets(getfilen[i], 100, fin);
    		for (int j = 0; j < strlen(getfilen[i]); j++)
    		{
    			if (getfilen[i][j] == '=')
    			{
    				start[i] = j+1;
    				break;
    			}
    		}
    		end[i] = strlen(getfilen[i])-1;
		}
		fclose(fin);
		strncpy(paramt.hashfile, getfilen[0]+start[0], (end[0]-start[0]));
		paramt.readhash = true;
		paramt.kmerlen = (int)atof(getfilen[1]+start[1]);
		paramt.kmernum = (int)atof(getfilen[2]+start[2]);
		paramt.hashnum = (int)atof(getfilen[3]+start[3]);
		paramt.minnum = (int)atof(getfilen[4]+start[4]);
		paramt.threadnum = (int)atof(getfilen[5]+start[5]);
	}
	return true;
}


char randomhash(int kmerlen, int hashnum)
{
	srand((unsigned)time(0));
	for (int i = 0; i < hashnum; i++)
	{
		for (int j = 0; j < kmerlen; j++)
		{
			refencode[i] <<= 2;
			int num = rand() % 4;
			switch(num)
			{
				case 0 : 
					refkmer[i][j] = 'A';
					break;
				case 1 : 
					refkmer[i][j] = 'C';
					refencode[i] += 1;
					break;
				case 2 :
					refkmer[i][j] = 'G';
					refencode[i] += 2;
					break;
				case 3 :
					refkmer[i][j] = 'T';
					refencode[i] += 3;
					break;
				default :
					break;
			}
		}
		//cout << refkmer[i] << ' ' << refencode[i] << endl;
	}
}

void buildhash(int kmerlen, int hashnum)
{
	if (hashkey > (kmerlen*2))
		hashkey = kmerlen*2;
	
	int offset = kmerlen*2 - hashkey;
	//count num

	for (int i = 0; i < hashnum; i++)
	{
		hashcnt[refencode[i] >> offset] += 1;
	}
	// count the multiple bucket
	/*
	int cnt = 0;
	for (int i = 0; i < hashlen; i++)
		if (hashcnt[i] > 5)
			cnt += 1;
	printf("the multiple bucket number is %d\n", cnt);
	*/
	for (int i = 1; i < hashlen; i++)
		hashcnt[i] += hashcnt[i-1];

	//hashkmer2bucket
	for (int i = 0; i < hashnum; i++)
	{
		if (hashcnt[refencode[i] >> offset]< 1)
		{
			printf("%s\n", "hash table error!");
			return;
		}
		hashcnt[refencode[i] >> offset]--;
		indextable[hashcnt[refencode[i] >> offset]] = i;
	}
}


int encode(char base)
{
	switch(base)
	{
		case 'A':
		case 'a':
			return 0;
		case 'C':
		case 'c':
			return 1;
		case 'G':
		case 'g':
			return 2;
		case 'T':
		case 't':
			return 3;
		default:
			return -1;
	}
}

void doHash(char* read, int kmerlen, bool* hit)
{
	int offset1 = kmerlen*2 - hashkey;
	int offset2 = 64 - kmerlen*2;
	unsigned long long tmp = 0;
	int len = strlen(read)-1;
	for (int i = 0; i < kmerlen-1; i++)
	{
		tmp <<= 2;
		tmp += encode(read[i]);
	}
	for (int i = kmerlen-1; i < len; i++)
	{
		tmp <<= 2;
		tmp += encode(read[i]);
		unsigned long long subtmp = (tmp << offset2) >> offset2;
		int seed = subtmp >> offset1;
		int start = hashcnt[seed]; 
		int end = hashcnt[seed+1]; 
		for (int i = end-1; i >= start; i--)
		{
			if ((subtmp ^ refencode[indextable[i]]) == 0)
			{
				hit[indextable[i]] = 1;
				break;
			}
		}
	}
}

void getMat(int kmerlen, int hashnum, int kmernum, int minnum, int threadnum = 2)
{
	refkmer = new char* [hashnum];
	refencode = new unsigned long long [hashnum];
	for (int i = 0; i < hashnum; i++)
	{
		refencode[i] = 0;
		refkmer[i] = new char [kmerlen];
	}

	if (paramt.readhash)
	{
		FILE* rh;
		rh = fopen(paramt.hashfile, "r");
		if (rh == NULL)
		{
			printf("the hash file is not exist!\n");
			return;
		}
		for (int i = 0; i < hashnum; i++)
			fscanf(rh, "%llu", &refencode[i]);
		fclose(rh);
	}
	else
		randomhash(kmerlen, hashnum);	
	
	indextable = new int [hashnum];
	buildhash(kmerlen, hashnum);
	
	printf("build the hash table successfully......\n");


	if (paramt.savehash)
	{
		FILE* sh;
		sh = fopen(paramt.shfile, "w");
		for (int i = 0; i < hashnum; i++)
			fprintf(sh, "%llu\n", refencode[i]);
		printf("save the hash file successfully......\n");
		fclose(sh);
	}

	FILE* filein;
	filein = fopen(paramt.infile, "r");
	char filebuf[1000];
	vector<string> filename;
	while(fgets(filebuf, 1000, filein))
		filename.push_back(filebuf);
	int filenum = filename.size();
	fclose(filein);
	printf("file number is %d\n", filenum);
	

	hitindex = new int* [filenum];
	for (int i = 0; i < filenum; i++)
		hitindex[i] = new int [minnum];

	int numberOfCPUs = omp_get_num_procs();
    printf("the computer has %d CPUs and now will use %d CPUs\n", numberOfCPUs, threadnum);
	#pragma omp parallel for num_threads(threadnum)
	for (int j = 0; j < filenum; j++)
	{
		char readbuf[1000];
		unsigned long long wholelen = 0;
		unsigned long long readnum = 0;
		int namelen = filename.at(j).size()-1;
		FILE* fin;
		fin = fopen(filename.at(j).substr(0, namelen).c_str(), "r");
		//printf("%s\n", filename.at(j).substr(0, namelen).c_str());
		
		while(fgets(readbuf, 1000, fin))
		{
			if (readbuf[0] == '@')
			{
				fgets(readbuf, 1000, fin);
				readnum += 1;
				wholelen += strlen(readbuf)-1;
				fgets(readbuf, 1000, fin);
				fgets(readbuf, 1000, fin);
			}
		}

		int avelen = wholelen / readnum;
		int randnum = kmernum / (avelen - kmerlen + 1);
		fclose(fin);

		printf("the whole read num is %lld, the whole lenth is %lld, the avelen is %d\n", readnum, wholelen, randnum);
		if (readnum < randnum)
		{
			printf("the read in file is too less\n");
		}
		else
		{
			bool* hit = new bool [hashnum];
			int* pos = new int [randnum];
			srand((unsigned)time(0));
			for (int i = 0; i < randnum; i++)
			{
				pos[i] = 0;
				pos[i] = rand() % readnum;
			}
			sort(pos, pos+randnum);

			//printf("start hashing\n");
			fin = fopen(filename.at(j).substr(0, namelen).c_str(), "r");
			
			readnum = 0;
			int k = 0;
			int nowread = pos[k];
			while(fgets(readbuf, 1000, fin))
			{
				if (readbuf[0] == '@')
				{
					fgets(readbuf, 1000, fin);
					if (readnum == nowread)
					{
						doHash(readbuf, kmerlen, hit);
						k += 1;
						while(nowread == pos[k])
							k += 1;
						nowread = pos[k];
					}
					fgets(readbuf, 1000, fin);
					fgets(readbuf, 1000, fin);
					readnum += 1;
				}
			}
			fclose(fin);

			//printf("start writing\n");
			int cnt = 0;
			for (int i = 0; i < hashnum; i++)
			{
				if (hit[i] == 1)
				{
					hitindex[j][cnt] = i;
					cnt += 1;
				}
				if (cnt == minnum)
					break;
			}
			delete [] pos;
			delete [] hit;
		}
	}

	FILE* fo;
	fo = fopen(paramt.outdir, "w");
	fprintf(fo, "#hashtable=%s\n", paramt.shfile);
	fprintf(fo, "#kmerlen=%d\n", paramt.kmerlen);
	fprintf(fo, "#kmernum=%d\n", paramt.kmernum);
	fprintf(fo, "#hashnum=%d\n", paramt.hashnum);
	fprintf(fo, "#minnum=%d\n", paramt.minnum);
	fprintf(fo, "#threadnum=%d\n", paramt.threadnum);
	fprintf(fo, "#filenum=%d\n", filenum);

	for (int i = 0; i < filenum; i++)
	{
		int start, end;
		catString(filename[i], start, end);
		fprintf(fo, "%s\n", filename[i].substr(start, (end-start)).c_str());
		for (int j = 0; j < minnum; j++)
			fprintf(fo, "%d ", hitindex[i][j]);
		fprintf(fo, "\n");
	}
	fclose(fo);
			
	printf("hashing is done......\n");
}


int main(int argc, char* argv[])
{
	time(&startt);
	setDefault();
	if(getParameter(argc, argv))
	{
		
		printf("%s, %s, %d, %d, %d, %s, %d, %s, %d, %s, %d, %s, %d, %d, %d, %d %d\n", 
		paramt.infile, paramt.outdir, paramt.train, paramt.test,
		paramt.readhash, paramt.hashfile, paramt.readkmer, paramt.kmerfile, 
		paramt.savehash, paramt.shfile, paramt.savekmer, paramt.skfile,
		paramt.kmerlen, paramt.kmernum, paramt.hashnum, paramt.minnum, paramt.threadnum);
	
		getMat(paramt.kmerlen, paramt.hashnum, paramt.kmernum, paramt.minnum, paramt.threadnum);
	}
	//getMat(18, 60000000, 1000000000, 10000, 2);
	
	time(&endt);
	cout << "using CPU hours:" << (unsigned int)(endt - startt) << endl;
	return 0;
}