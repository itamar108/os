touch results.txt;
for i in {2,10,50,100}; do for j in {1..3}; do time FileWordCounter test_word_count $i >> results.txt; echo "thread_amount " $i >> results.txt ;done;done;
	
