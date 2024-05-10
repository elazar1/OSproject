comp:
		gcc -Wall -o run main.c

test_one:
		./run ../target ../dir1
		
test_all:
	make comp
	./run -o ../target ../dir1 ../dir2 ../dir3 ../dir4 ../dir5 ../dir6 ../dir7 ../dir8 ../dir9

remove:
	./script.sh r

create:
	./script.sh c

refresh:
	make comp
	make remove
	make create

m?=wip
git_up:
	git add .
	git commit -m "$(m)"
	git push
	