FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    bash \
    sudo \
    curl \
    git \
    && rm -rf /var/lib/apt/lists/*

COPY requirements.sh /tmp/requirements.sh
RUN chmod +x /tmp/requirements.sh && /tmp/requirements.sh

WORKDIR /workspace

CMD ["/bin/bash"]
