comp:
	./script.sh o
	./script.sh a
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
	make remove
	make create

git_stat:
	git fetch
	git pull
	git status

m?=work_in_progress
git_up :
	git add .
	git commit -m "$(m)"
	git push