g++ client_test.cpp -o client_test `pkg-config --libs --cflags opencv` -lpthread
g++ server_test.cpp -o server_test `pkg-config --libs --cflags opencv` -lpthread