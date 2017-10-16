## How to use

#### 1. preparatory work

First you should confirm that your computer is support g++, python and OpenMp. If all of them is ok, then you could compile our code as follows

	g++ ./Gemini.cpp -fopenmp -o Gemini

#### 2. training data

Now you can start training data. 

	./Gemini ./trainfile.txt ./outdir/train.ge --train

	./Gemini ./testfile.txt ./outdir/test.ge --test ./outdir/train.ge

--help

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

### 4. tutorial

This is an example of how to use Gemini to test your own data

In Gemini, we provide a data that has been trained(../data/train.ge) and a hash table(../data/hashtable.*, you can unzip them and there will be a file named "hashtable"). So you can use your data for testing directly.

In this tutorial, our test data is from [https://www.ebi.ac.uk/metagenomics/projects/ERP005558]. First we need creat a filelist like _testfile.txt_. Then, run the following instructions

	Gemini ./testfile.txt ./test.ge --test ../data/train.ge

Of course, you can add some other paramters, which is introduced in help command

	Gemini --help

finally, use two .ge files to get the p value

	Gemini.sh ../data/train.ge ./test.ge

the result is in _similar.txt_ file