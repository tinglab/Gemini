## How to use

#### 1. preparatory work

First you should confirm that your computer is support g++, python and OpenMp. If all of them is ok, then you could compile our code as follows

	g++ ./Gemini.cpp -fopenmp -o Gemini.exe

#### 2. training data

Now you can start training data. 

	./Gemini.exe ./trainfile.txt ./outdir/train.ge --train

	./Gemini.exe ./testfile.txt ./outdir/test.ge --test ./outdir/train.ge

-l: kmerlen
-n: kmernum
-h: hashnum
-m: minnum
-t: threadnum
-rh: read hashtable
-sh: save hashtable
-rk: read kmer
-sk: save kmer


#### 3. test
	
	./Gemini.sh ./outdir/train.ge ./outdir/test.ge

or you can set a Gemini cutoff like 0.001 (default is 0.01)
	
	./Gemini.sh ./outdir/train.ge ./outdir/test.ge 0.001