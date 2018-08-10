
# Docker

`docker-compose -f docker-compose.yml up --build --detach`

# Attach

`docker exec -ti provider /bin/bash`

# Compile

Navigate to the `provider` directory (by default the entry point).
Compile and run.

`make provider.exe`

*note*: `.exe` extension for `.gitignore` blacklisting


# Run


## Bitmex

`./provider www.bitmex.com 443 "/realtime?subscribe=orderBook:XBTUSD"`


## Binance

`./provider stream.binance.com 9443 "/ws/bnbbtc@trade"`



