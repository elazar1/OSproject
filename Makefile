comp:
	gcc -Wall -o ruleaza main.c

test_one:
	make comp
	./ruleaza -o ../result ../dir1
		
test_all:
	make comp
	./ruleaza -o ../result ../dir1 ../dir2 ../dir3 ../dir4 ../dir5 ../dir6 ../dir7 ../dir8 ../dir9

remove:
	./script.sh r

create:
	./script.sh c

refresh:
	make comp
	make remove
	make create
