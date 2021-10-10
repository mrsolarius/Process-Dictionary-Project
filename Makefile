all : prj_0

prj_0: project_0.o
	gcc project_0.o -o prj_0

project_0.o: project_0.c
	gcc -c project_0.c

clean:
	@echo "suppression de fichier temporaires et des builds"
	rm *.o
	rm prj*