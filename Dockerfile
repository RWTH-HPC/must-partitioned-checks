FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Berlin

RUN apt-get update \
    && apt-get -y -qq --no-install-recommends install \
    cmake \
    curl \
    binutils-dev \
    make \
    automake \
    autotools-dev \
    autoconf \
    libtool \
    zlib1g \
    zlib1g-dev \
    libmpich-dev \
    libatomic1 \
    libfabric-dev \
    libxml2-dev \
    python3 \
    python3-pip \
    python3-venv \
    gfortran \
    gcc \
    g++ \
    git \
    graphviz \
    libgtest-dev \
    clang-15 \
    libomp-15-dev \
    libdw-dev \
    libdwarf-dev \
    clang-format-15 \
    llvm-15 \
    lldb-15 \
    ninja-build \
    vim \
    openssh-client \
    gdb \
    wget \
    googletest \
    && apt-get -yq clean \
    && rm --recursive --force /var/lib/apt/lists/*

RUN ln -s /usr/bin/FileCheck-15 /usr/bin/FileCheck
RUN ln -s /usr/bin/clang-15 /usr/bin/clang
RUN ln -s /usr/bin/clang++-15 /usr/bin/clang++
RUN ln -s /usr/bin/clang-format-15 /usr/bin/clang-format

COPY requirements.txt .

# Install Python dependencies and ensure to activate virtualenv (by setting PATH variable)
ENV VIRTUAL_ENV=/opt/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"
RUN pip3 install --no-input --no-cache-dir --disable-pip-version-check -r /requirements.txt

ENV MPICH_CC=clang
ENV MPICH_CXX=clang++
ENV OMPI_CC=clang
ENV OMPI_CXX=clang++

# Run as non-privileged user
RUN     useradd -ms /bin/bash user

WORKDIR /home/user
USER user

# Install MUST
RUN mkdir -p ~/must && cd ~/must && \
      wget https://hpc.rwth-aachen.de/must/files/MUST-v1.9.1.tar.gz && \
      tar xf MUST-v1.9.1.tar.gz && \
      cd MUST-v1.9.1 && \
      mkdir BUILD-mpich && cd BUILD-mpich && \
      cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DUSE_BACKWARD=ON -DENABLE_FORTRAN=OFF -DENABLE_TYPEART=OFF \
      -DCMAKE_INSTALL_PREFIX=~/must \
      -DENABLE_TESTS=ON && \
      make -j$(nproc) install

# Copy test cases in container
COPY --chown=user:user tests /home/user/tests

ENV PATH="/home/user/must/bin:$PATH"

