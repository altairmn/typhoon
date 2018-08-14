include_dirs=./include

%: %.cpp
	g++ $@.cpp -lboost_system -lpthread -lssl -lcrypto -lzmq\
		-I$(include_dirs)\
		-o $@
