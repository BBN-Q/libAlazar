FROM gitlab/gitlab-runner:latest
RUN apt-get update
RUN apt-get install -y cmake libboost-all-dev 
COPY ca-certificates.crt /etc/ssl/certs/
RUN wget https://3230d63b5fc54e62148e-c95ac804525aac4b6dba79b00b39d1d3.ssl.cf1.rackcdn.com/Anaconda2-2.5.0-Linux-x86_64.sh
RUN bash Anaconda2-2.5.0-Linux-x86_64.sh -b -f -p /opt/anaconda2
ENV LD_LIBRARY_PATH .:./bin
ENV PATH /opt/anaconda2/bin:$PATH
