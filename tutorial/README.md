This is an example of how to use Gemini to test your own data

In Gemini, we provide a data that has been trained(../data/train.ge). So you can use your data for testing directly.

In this tutorial, our test data is from [https://www.ebi.ac.uk/metagenomics/projects/ERP005558]. First we need creat a filelist like _testfile.txt_. Then, run the following instructions

	Gemini ./testfile.txt ./test.ge --test ../data/train.ge

Of course, you can add some other paramters, which is introduced in help command

	Gemini --help

finally, use two .ge files to get the p value

	Gemini.sh ../data/train.ge ./test.ge

the result is in _similar.txt_ file