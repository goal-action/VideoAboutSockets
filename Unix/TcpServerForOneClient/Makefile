CC=g++
CFLAGS=-Wall
DEGUG_FLAGS=-g #needs for debugging
SERVER_DIR=./Server/
CLIENT_DIR=./Client/
SERVER_PROG=./bin/Server_ex
CLIENT_PROG=./bin/Client_ex

buildAll: buildServer buildClient

buildServer:
	$(CC) $(CFLAGS) $(SERVER_DIR)*.cpp -o $(SERVER_PROG)

buildClient:
	$(CC) $(CFLAGS) $(CLIENT_DIR)*.cpp -o $(CLIENT_PROG)

debugServer:
	$(CC) $(CFLAGS) $(DEGUG_FLAGS) $(SERVER_DIR)*.cpp -o $(SERVER_PROG)

debugClient:
	$(CC) $(CFLAGS) $(DEGUG_FLAGS) $(CLIENT_DIR)*.cpp -o $(CLIENT_PROG)
