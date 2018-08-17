include_dirs=./include

bitmex_port=5557
binance_port=5558

%: %.cpp
	g++ $@.cpp -lboost_system -lpthread -lssl -lcrypto -lzmq\
		-I$(include_dirs)\
		-o $@

bitmex_provider: provider.cpp
	g++ -DPORT=$(bitmex_port)\
		provider.cpp -lboost_system -lpthread -lssl\
		-lcrypto -lzmq\
		-I$(include_dirs)\
		-o bitmex_provider

binance_provider: provider.cpp
	g++ -DPORT=$(binance_port)\
		provider.cpp -lboost_system -lpthread -lssl\
		-lcrypto -lzmq\
		-I$(include_dirs)\
		-o binance_provider

bitmex_agg: aggregator.cpp
	g++ -DBITMEX aggregator.cpp -lboost_system -lpthread -lssl\
		-lcrypto -lzmq\
		-I$(include_dirs)\
		-o bitmex_agg

binance_agg: aggregator.cpp
	g++ -DBINANCE aggregator.cpp -lboost_system -lpthread -lssl\
		-lcrypto -lzmq\
		-I$(include_dirs)\
		-o binance_agg
