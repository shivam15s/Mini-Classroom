#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include "server.hpp"

using namespace std;

void recv_data(int cli_sock, string &header, string &data){
    int data_recv;
    char temp_header[BUF_SIZE];

    if((data_recv = recv(cli_sock, temp_header, BUF_SIZE, 0)) < 0){
        perror("recv() failed");
    }

    header = temp_header;
    data = "";

    char temp_buffer[BUF_SIZE];
    
    vector<string> strings_list = split_string(header);
    int data_to_recv = atoi(strings_list[2].c_str());
    while((data_to_recv > 0) && ((data_recv = recv(cli_sock, temp_buffer, BUF_SIZE, 0)) > 0)){
        string buffer = temp_buffer;
        data += buffer;
        data_to_recv -= data_recv;
    }
}

void send_data(int cli_sock, bool ok, string msg){
    string code;
    if(ok){
        code = "OK";
    }
    else{
        code = "NOK";
    }
    string len = to_string(msg.length());
    string resp = code + SPLITTER + len;
    if(send(cli_sock, resp.c_str(), BUF_SIZE, 0) < 0){
        perror("send() failed");
    }
    int data_sent;
    if((data_sent = send((cli_sock), msg.c_str(), stoi(len), 0)) != stoi(len)){
        perror("send() failed");
        exit(1); // Replace this with fail message
        // fail_msg();
    }
}

void send_file(int cli_sock, string filepath){
    // fopen(filepath.c_str());
}
