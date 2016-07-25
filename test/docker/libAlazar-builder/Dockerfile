# start from Anaconda to get their python
FROM continuumio/anaconda3

MAINTAINER Colm Ryan <cryan@bbn.com>

# add some build tools
RUN apt-get update && apt-get install -y \
 unzip \
 git \
 build-essential \
 libboost-all-dev

# Anaconda starts from Debian 8.5 which only has cmake 3.0.2
RUN wget https://cmake.org/files/v3.5/cmake-3.5.2-Linux-x86_64.sh \
 && echo y | sh cmake-3.5.2-Linux-x86_64.sh
ENV PATH /cmake-3.5.2-Linux-x86_64/bin:$PATH

# add BBN certficiate
RUN wget --no-check-certificate https://bua.bbn.com/trac/bua-sysadmin/raw-attachment/wiki/HowToInstallCACerts/ca.zip \
 && unzip ca.zip \
 && mv CA/bbnca.pem /usr/local/share/ca-certificates/bbnca.crt \
 && update-ca-certificates
