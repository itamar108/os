for i in {$1,$2,$3,$4,$5}; do time ./FileWordCounter test_word_count $i >>/dev/null; echo "thread_amount " $i ;done;
	
