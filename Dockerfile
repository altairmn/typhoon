FROM ubuntu:18.04

RUN apt-get update &&\
    apt-get install -y vim wget git gcc make g++ libssl-dev libtool gettext pkg-config cmake autoconf libboost-all-dev &&\
    ln -s /usr/lib/x86_64-linux-gnu/librtmp.so.1 /usr/lib/x86_64-linux-gnu/librtmp.so.0

WORKDIR /root

# zeromq and zeromq C++ bindings install
RUN git clone git://github.com/jedisct1/libsodium.git &&\
    cd ./libsodium && ./autogen.sh && ./configure &&\
    make check && make install &&\
    ldconfig &&\
    cd ../ &&\
    git clone git://github.com/zeromq/libzmq.git && cd libzmq  &&\
    ./autogen.sh && ./configure --with-libsodium && make &&\
    make install && ldconfig &&\
    cd ../ &&\
    git clone https://github.com/zeromq/cppzmq.git && cd cppzmq &&\
    mkdir build && cd build && cmake .. && make -j install &&\
    cd ../../

# install jsoncpp
RUN git clone https://github.com/open-source-parsers/jsoncpp.git &&\
    cd jsoncpp && mkdir build && cd build && cmake .. && make -j &&\
    make install && ldconfig

CMD tail -f /dev/null
