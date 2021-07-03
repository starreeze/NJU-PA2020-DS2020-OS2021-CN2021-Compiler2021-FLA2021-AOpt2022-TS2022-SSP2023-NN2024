debug : parse
	g++ -O0 -g main.cpp -o a

release : parse
	g++ -O2 main.cpp -o a

parse: make_parse
	bin/parser main.cpp

make_parse: 
	cd bin && make
	cd ..
