#include "Committee.h"

extern App app;
extern Sound generator;
extern Controller controller;
extern Serial sci0;
extern SysIO sio0;
extern Can can0;

void committee_recv(Committee * self,  int addr){
    CANMsg msg = *(CANMsg *) addr;
   char strbuff[100];
	snprintf(strbuff,100,"Committe MSGID: %d\n",msg.msgId);
	SCI_WRITE(&sci0,strbuff);
    switch(self->mode){
        case INIT:
            switch(msg.msgId){

            }
            break;
        case MASTER:
            break;
        case SLAVE:
            break;
        case WAITING:
             break;  
        case COMPETE:
            break;
    }
}

void send_BoardNum_msg(Committee* self,int arg){
	CANMsg msg;
	SCI_WRITE(&sci0,"--------------------send_BoardNum_msg-------------------------\n");
	char strbuff[100];
	snprintf(strbuff,100,"BoardNum: %d \nLeaderRank: %d \nMyRank: %d \n",self->boardNum,self->leaderRank,self->myRank);
	SCI_WRITE(&sci0,strbuff);
	msg.nodeId = self->leaderRank;
	msg.msgId = 126;
	char str_num[1]; 
   	sprintf(str_num,"%d", abs(self->boardNum));
   	msg.length = 1;
    msg.buff[0] = str_num[0];
	CAN_SEND(&can0, &msg);
}

void send_Detecting_msg(Committee* self,int num){
	CANMsg msg;
	SCI_WRITE(&sci0,"--------------------send_Detecting_msg-------------------------\n");
	char strbuff[100];
	snprintf(strbuff,100,"BoardNum: %d\nLeaderRank: %d\nMyRank: %d\n",self->boardNum,self->leaderRank,self->myRank);
	SCI_WRITE(&sci0,strbuff);
	msg.nodeId = self->leaderRank;
	msg.msgId = 12;

	CAN_SEND(&can0, &msg);
	SCI_WRITE(&sci0,"CAN message send!\n");
}

void send_Detecting_ack_msg(Committee* self,int num){
	CANMsg msg;
	SCI_WRITE(&sci0,"--------------------send_Detecting_ack-------------------------\n");
	char strbuff[100];
	snprintf(strbuff,100,"BoardNum: %d\nLeaderRank: %d\nMyRank: %d\n",self->boardNum,self->leaderRank,self->myRank);
	SCI_WRITE(&sci0,strbuff);
	msg.nodeId = self->leaderRank;
	msg.msgId = 13;
	CAN_SEND(&can0, &msg);
	SCI_WRITE(&sci0,"CAN message send!\n");
}

void send_Reset_msg(Committee* self, int arg){
    CANMsg msg;
	SCI_WRITE(&sci0,"--------------------send_Reset_msg-------------------------\n");
	//char strbuff[100];
	//snprintf(strbuff,100,"BoardNum: %d \nLeaderRank: %d \nMyRank: %d \n",self->boardNum,self->leaderRank,self->myRank);
	//SCI_WRITE(&sci0,strbuff);
	msg.nodeId = self->leaderRank;
	msg.msgId = 125;
	CAN_SEND(&can0, &msg);
}

void send_Getleadership_msg(Committee* self ,int arg){
	CANMsg msg;
	SCI_WRITE(&sci0,"--------------------Claim for leadership from slave-------------------------\n");
	msg.nodeId = self->myRank;
	msg.msgId = 127;
}
