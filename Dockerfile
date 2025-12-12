FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    python3 \
    python3-pip \
    python3-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

# Install pybind11
RUN pip3 install pybind11

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Build project
RUN mkdir build && cd build && \
    cmake .. -DBUILD_PYTHON_BINDINGS=ON && \
    cmake --build . --config Release

# Set PATH
ENV PATH="/app/build:${PATH}"
ENV PYTHONPATH="/app/build/python:${PYTHONPATH}"

# Default command
CMD ["/bin/bash"]

