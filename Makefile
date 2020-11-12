all:
	g++ Model.cpp SC.cpp RC.cpp WO.cpp PC.cpp simulator.cpp  -o sim_program2

clear:
	rm sim_program2
