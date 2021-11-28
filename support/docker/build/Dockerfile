FROM 7ca6812beae7

MAINTAINER Moan <jianzhi.gjz@alibaba-inc.com>

WORKDIR /root

COPY vepc.tar.bz2 /root/

RUN cd /root && tar jxvf vepc.tar.bz2

RUN cd /root/epc && \
    autoreconf -iv && \
    ./configure --prefix=/epc && \
    make install && \
    cp -rf support/docker/host_root / && \
    rm -rf /root/epc && \
    rm -rf /root/vepc.tar.bz2

ENTRYPOINT ["/sbin/init"]
