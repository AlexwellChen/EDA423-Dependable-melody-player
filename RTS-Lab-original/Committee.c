#include "Committee.h"

extern App app;
extern Sound generator;
extern Controller controller;
extern Serial sci0;
extern SysIO sio0;
extern Can can0;

Committee committee = {initObject(), 1, 0, -1, INIT, 1};

void committee_recv(Committee *self, int addr)
{
    CANMsg msg = *(CANMsg *)addr;
    char strbuff[100];
    snprintf(strbuff, 100, "Committe MSGID: %d\n", msg.msgId);
    SCI_WRITE(&sci0, strbuff);
    switch (self->mode)
    {
    case INIT:
        switch (msg.msgId)
        {
        case 122:
        {
            // For initBoardNum function
            SCI_WRITE(&sci0, "----------------Recv msgId 122---------------------\n");
            self->boardNum++;
            break;
        }
        case 123:
        {
            // For initMode function
            self->mode = SLAVE;
            ASYNC(&app, setMode, SLAVE);
            self->leaderRank = msg.nodeId;
        }
        case 126:
        {
            // For initBoardNum function
            SCI_WRITE(&sci0, "----------------Recv msgId 126---------------------\n");
            int boardsNum_recv = atoi(msg.buff);
            if (boardsNum_recv > self->boardNum)
            {
                self->boardNum = boardsNum_recv;
            }
            // No need to print this line because 126 will be recieved three times
            // SCI_WRITE(&sci0,"Ready for getting leadership!\n");
            break;
        }
            // case 127:{
            //     //Send response to those who wants leadership
            //     ASYNC(self, send_ResponseLeadership_msg,msg.nodeId);
            //     break;
            // }
        }
        break;
    case MASTER:
        switch (msg.msgId)
        {
        case 127:
            self->mode = INIT;
            SCI_WRITE(&sci0, "Leadership Void\n");
            ASYNC(&app, setMode, INIT);
            break;
        }
        break;
    case SLAVE:
        break;
    case WAITING:
        switch (msg.msgId)
        {
        case 127:
        {
            if (msg.nodeId > self->myRank)
            {
                // fail if has higher rank appears to compete
                self->isLeader = 0;
            }
            break;
        }
        }
        break;
        // case COMPETE:
        //     break;
    }
}

void send_BoardNum_msg(Committee *self, int arg)
{
    CANMsg msg;
    SCI_WRITE(&sci0, "--------------------send_BoardNum_msg-------------------------\n");
    char strbuff[100];
    snprintf(strbuff, 100, "BoardNum: %d \nMyRank: %d \n", self->boardNum, self->myRank);
    SCI_WRITE(&sci0, strbuff);
    msg.nodeId = self->myRank;
    msg.msgId = 126;
    char str_num[1];
    sprintf(str_num, "%d", abs(self->boardNum));
    msg.length = 1;
    msg.buff[0] = str_num[0];
    CAN_SEND(&can0, &msg);
}

void send_Detecting_msg(Committee *self, int num)
{
    CANMsg msg;
    SCI_WRITE(&sci0, "--------------------send_Detecting_msg-------------------------\n");
    char strbuff[100];
    snprintf(strbuff, 100, "BoardNum: %d\nLeaderRank: %d\nMyRank: %d\n", self->boardNum, self->leaderRank, self->myRank);
    SCI_WRITE(&sci0, strbuff);
    msg.nodeId = self->myRank;
    msg.msgId = 122;
    // CAN_SEND(&can0, &msg);
    // SCI_WRITE(&sci0,"CAN message send!\n");
}

// NOT USED ANYMORE
//  void send_Detecting_ack_msg(Committee* self,int num){
//  	CANMsg msg;
//  	SCI_WRITE(&sci0,"--------------------send_Detecting_ack-------------------------\n");
//  	char strbuff[100];
//  	snprintf(strbuff,100,"BoardNum: %d\nLeaderRank: %d\nMyRank: %d\n",self->boardNum,self->leaderRank,self->myRank);
//  	SCI_WRITE(&sci0,strbuff);
//  	msg.nodeId = self->leaderRank;
//  	msg.msgId = 121;
//  	CAN_SEND(&can0, &msg);
//  	SCI_WRITE(&sci0,"CAN message send!\n");
//  }

void send_Reset_msg(Committee *self, int arg)
{
    CANMsg msg;
    SCI_WRITE(&sci0, "--------------------send_Reset_msg-------------------------\n");
    // char strbuff[100];
    // snprintf(strbuff,100,"BoardNum: %d \nLeaderRank: %d \nMyRank: %d \n",self->boardNum,self->leaderRank,self->myRank);
    // SCI_WRITE(&sci0,strbuff);
    msg.nodeId = self->leaderRank;
    msg.msgId = 125;
    CAN_SEND(&can0, &msg);
}
void send_DeclareLeader_msg(Committee *self, int arg)
{
    CANMsg msg;
    SCI_WRITE(&sci0, "--------------------send_DeclareLeader_msg-------------------------\n");
    msg.nodeId = self->myRank;
    msg.msgId = 123;
    CAN_SEND(&can0, &msg);
}
void send_GetLeadership_msg(Committee *self, int arg)
{
    CANMsg msg;
    SCI_WRITE(&sci0, "--------------------Claim for Leadership-------------------------\n");
    msg.nodeId = self->myRank;
    msg.msgId = 127;
    CAN_SEND(&can0, &msg);
    self->mode = WAITING;
}

// NOT USED ANYMORE
//  void send_ResponseLeadership_msg (Committee *self ,int nodeId){
//      CANMsg msg;
//  	SCI_WRITE(&sci0,"--------------------Response for leadership compete-------------------------\n");
//  	msg.nodeId = self->myRank;
//  	msg.msgId = 124;
//      char str_num[1];
//     	sprintf(str_num,"%d", nodeId);
//     	msg.length = 1;
//      msg.buff[0] = str_num[0];
//      CAN_SEND(&can0, &msg);
//  }

void IorS_to_W(Committee *self, int arg)
{
    // Trying to get leadership from init mode or slave mode
    self->mode = WAITING;
}
void change_StateAfterCompete(Committee *self, int arg)
{
    if (self->isLeader)
    {
        self->mode = MASTER;
        ASYNC(self, send_DeclareLeader_msg, 0);
        SCI_WRITE(&sci0, "Claimed Leadership!\n");
    }
    else
    {
        self->mode = INIT;
    }
    // Reset before exit
    self->isLeader = 1;
}
void initBoardNum(Committee *self, int unused)
{
    ASYNC(&committee, send_Detecting_msg, 0); // msgId 122
    SCI_WRITE(&sci0, "send_Detecting_msg send!\n");
    // after two second, collect the board number and send to slaves
    AFTER(SEC(2), &committee, send_BoardNum_msg, 0); // msgId 126
}

void initMode(Committee *self, int unused)
{
    char strbuff[100];
    snprintf(strbuff, 100, "New boardNum: %d\n", self->boardNum);
    SCI_WRITE(&sci0, strbuff);
}
