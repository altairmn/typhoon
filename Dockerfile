FROM ubuntu:18.04

RUN apt-get update &&\
    apt-get install -y vim wget git gcc make g++ libssl-dev libtool gettext pkg-config autoconf &&\
    libboost-all-dev

WORKDIR /root/provider

CMD tail -f /dev/null
