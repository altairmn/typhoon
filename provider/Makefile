include_dirs=./include

bitmex_port=5557
binance_port=5558
align_port=5559
strategy_port=5560
targets=bitmex_provider binance_provider bitmex_agg binance_agg aligner
latency=1000

%: %.cpp
	g++ $@.cpp -lboost_system -lpthread -lssl -lcrypto -lzmq\
		-I$(include_dirs)\
		-o $@

.PHONY: all clean run stop
all: $(targets)

clean:
	rm -rf $(targets)

run: 
	./bitmex_provider www.bitmex.com 443 "/realtime?subscribe=trade:XBTUSD" & &&\
	./binance_provider stream.binance.com 9443 "/ws/btcusdt@trade" & &&\
	./bitmex_agg $(latency) $(bitmex_port) & &&\
	./binance_agg $(latency) $(binance_port) & &&\
	./aligner &
 

stop:
	killall $(targets)

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
	g++ -DBITMEX -DALIGN_PORT=$(align_port) -DDEBUG\
		aggregator.cpp -lboost_system -lpthread -lssl\
		-lcrypto -lzmq\
		-I$(include_dirs)\
		-o bitmex_agg

binance_agg: aggregator.cpp
	g++ -DBINANCE -DALIGN_PORT=$(align_port) -DDEBUG\
		aggregator.cpp -lboost_system -lpthread -lssl\
		-lcrypto -lzmq\
		-I$(include_dirs)\
		-o binance_agg

aligner: aligner.cpp
	g++ -DALIGN_PORT=$(align_port) -DSTRATEGY_PORT=$(strategy_port) -DDEBUG\
		aligner.cpp -lboost_system -lpthread -lssl\
		-lcrypto -lzmq\
		-I$(include_dirs)\
		-o aligner
