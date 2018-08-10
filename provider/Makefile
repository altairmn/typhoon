include_dirs=./include

provider.exe: provider.cpp
	g++ provider.cpp -lboost_system -lpthread -lssl -lcrypto\
		-I$(include_dirs)\
		-o provider.exe
