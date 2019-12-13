#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include<vector>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <unistd.h>
#include <iostream>

using namespace std;
using namespace cv;

#define BUF_SIZE 655350
// size enough for 640*480

int main(int argc, char** argv)
{

    // setup connection
    int port_num = 5000;
    const char *server_ip_str="::1"; //A IPV6 ADDRESS REACHABLE = ipv6addr%your_net_device = xxxx::xxxx:xxxx:xxx:xxxx%ethx

    int sockfd;
    struct sockaddr_in6 addr;

    sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port_num);
    inet_pton(AF_INET6, server_ip_str, &addr.sin6_addr);
    connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));

    
    char buffer[ BUF_SIZE ];
    vector<char> vec(BUF_SIZE);
    Mat img_decode; 
    string filename="";   
    int size = 0;
    int mylen = 0;
    long long j= 0;
    char cokstart[ 1 ]={0x33};
    memset( buffer, 0, sizeof(buffer) );

    while(1)
    {
        size = recv(sockfd,buffer,4,0);
    
        mylen = ((buffer[0]<<24)&(0xFF000000))|((buffer[1]<<16)&(0xFF0000))|((buffer[2]<<8)&(0xFF00))|((buffer[3])&(0xFF));
        if (mylen>0){
            /*received length then send ack signal cokstart.*/  
            send(sockfd, cokstart, 1, 0);      
        }

        /* receive one frame of jpg data */
        j++;
        cout << " receive one frame of jpg data # " << j << " image size=" << mylen << "bytes" <<endl;
        vec.clear();

        while(mylen) 
        {
            size=recv(sockfd,buffer,mylen,0);
            
            /* put char values to vector */
            for(int i = 0 ; i < size ; i ++)  
            {  
                vec.push_back(buffer[i]);
            }
            mylen = mylen-size;  
        }
        
        /* decode jpg data */
        img_decode = imdecode(vec, CV_LOAD_IMAGE_COLOR); 

        /* display the jpg in windows */
        namedWindow("pic",WINDOW_AUTOSIZE);
        if(!img_decode.empty()){
            imshow("pic",img_decode);     
        }
        else
        {
            cout << " receive empty frame of jpg data # " << j <<endl;
        }
        
        /* reflash display window in every 33ms */
        cvWaitKey(10);
        
        /* send ack signal cok to tell that I'm ready to handler next frame */
        char cok[ 1 ]={0x55};
        send(sockfd, cok, 1, 0);
    }
}

