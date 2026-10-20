
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    g++ \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY backend/ /app/backend/
COPY frontend/ /app/frontend/
COPY data/ /app/data/

WORKDIR /app/backend

RUN g++ -std=c++17 -pthread main.cpp -o airline_server

EXPOSE 8080

CMD ["./airline_server"]
