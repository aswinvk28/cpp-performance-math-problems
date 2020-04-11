FROM ubuntu:disco

ARG REPOSITORY_LINK

RUN apt-get update && apt-get install build-essential cmake -y

RUN apt-get install libomp5 libomp-dev libblas3 libblas-dev liblapack3 liblapack-dev -y

RUN apt-get install ca-certificates curl dos2unix fonts-noto g++ \
    gcc gpg make software-properties-common wget zip -y

RUN cd /tmp
RUN wget https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB
RUN apt-key add GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB
RUN sh -c 'echo deb https://apt.repos.intel.com/mkl all main > /etc/apt/sources.list.d/intel-mkl.list'
RUN apt-get update
RUN apt-get install intel-mkl-64bit-2018.2-046 -y

RUN apt-get install git -y

WORKDIR /opt/
RUN git clone https://github.com/autodiff/autodiff
WORKDIR /opt/
RUN git clone $REPOSITORY_LINK

## update alternatives
RUN update-alternatives --install /usr/lib/x86_64-linux-gnu/libblas.so     \
                    libblas.so-x86_64-linux-gnu      /opt/intel/mkl/lib/intel64/libmkl_rt.so 50
RUN update-alternatives --install /usr/lib/x86_64-linux-gnu/libblas.so.3   \
                    libblas.so.3-x86_64-linux-gnu    /opt/intel/mkl/lib/intel64/libmkl_rt.so 50
RUN update-alternatives --install /usr/lib/x86_64-linux-gnu/liblapack.so   \
                    liblapack.so-x86_64-linux-gnu    /opt/intel/mkl/lib/intel64/libmkl_rt.so 50
RUN update-alternatives --install /usr/lib/x86_64-linux-gnu/liblapack.so.3 \
                    liblapack.so.3-x86_64-linux-gnu  /opt/intel/mkl/lib/intel64/libmkl_rt.so 50
