#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <vector>

using namespace std; 
using namespace cv;

#define BUF_SIZE 655350
// size enough for 640*480

int main(int argc, char** argv)
{

    // setup connection
    int port_num = 5000;

    int listenfd, connfd;
    int reuseaddr = 1;
    struct sockaddr_in6 addr;
    int pid;

    listenfd = socket(AF_INET6, SOCK_STREAM, 0);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));

    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port_num);
    addr.sin6_addr = in6addr_any;

    bind(listenfd, (struct sockaddr *)&addr, sizeof(addr));
    listen(listenfd, 5);


    /* ---main task process--- */
    Mat frame;
    VideoCapture cap;
    vector<unsigned char> inImage(BUF_SIZE);  
    /* open camera */
    cap.open(0);
    if (!cap.isOpened()) 
    {
        cout << "ERROR! Unable to open camera\n";
        return -1;
    }

    printf("open camera success\n");
    printf("======waiting for client's request======\n");

    if( (connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1){
        printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
        exit(0);
    }
    
    char cok[1]={0x55};
    char cokstart[1]={0};
    long long j=0;
    int sizelen=0;
    int sizejpg=0;
    unsigned char msgImage[BUF_SIZE];
    for (;;)
    {
        /* read one frame */
        cap.read(frame);
        /* check if we succeeded */
        if (frame.empty()) {
            cout << "ERROR! blank frame grabbed\n" ;
            break;
        }
        /* stop display after get key press */
        if(waitKey(5)>=0)
            break;

        /* get trigger from client's command */
        if (cok[0]==0x55){
            cok[0]=0;
            //printf("read one frame!\n");

            /* encode frame to JPG data */
            imencode(".jpg",frame,inImage);  
            /* get length of jpg */
            int datalen=inImage.size();

            /* prepare char to save jpg data */   
            unsigned char msgLen[4];
            msgLen[0]=datalen >> 24;
            msgLen[1]=datalen >> 16;
            msgLen[2]=datalen >> 8;
            msgLen[3]=datalen;
            
            /* send lenght to client first */
            sizelen=send(connfd,msgLen,4,0);
            

            /* put vector data to char */
            for(int i=0;i<datalen;i++)  
            {  
                    msgImage[i]=inImage[i];  
            } 

            /* get lenght response ack from client */
            recv(connfd,cokstart,1,0);
            if(cokstart[0] == 0x33)
            {
                vector<char>vec;
                Mat img_decode;
                string filename="";

                /* decode than save display to save to file. This is optional function */
                cokstart[0]=0x0;

                /* put data to vector */
                for(int i=0;i<datalen;i++)
                {
                    vec.push_back(msgImage[i]);
                }
                
                /* send data to client */
                j++;
                cout << "sending frame # " << j <<endl;
                sizejpg=send(connfd,msgImage,datalen,0);
                
                usleep(1000);

                /* get response ack from client then can send the next frame*/
                recv(connfd,cok,1,0);
            }
        }

    }

    /* if get key press than break display and stop the socket connection */
    close(listenfd);
    return 0;
}

