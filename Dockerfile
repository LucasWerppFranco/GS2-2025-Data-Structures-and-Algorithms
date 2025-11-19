FROM debian:stable

RUN apt-get update && apt-get install -y \
    gcc \
    libc6-dev \
    libcurl4-openssl-dev \
    libcjson-dev \
    locales \
    locales-all && \
    locale-gen pt_BR.UTF-8 && \
    update-locale LANG=pt_BR.UTF-8

ENV LANG=pt_BR.UTF-8
ENV LC_ALL=pt_BR.UTF-8

WORKDIR /app

COPY main.c /app/
COPY mentores.txt /app/
COPY mentorados.txt /app/
COPY prompt.txt /app/

RUN gcc main.c -o sistema -Wall -Wextra -D_GNU_SOURCE -lcurl -lcjson

CMD ["./sistema"]
