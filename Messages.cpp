#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include "Messages.h"

CustomerRequest::CustomerRequest() { customer_id = -1; order_number = -1; request_type = -1; }
void CustomerRequest::Set(int cid, int onum, int rtype) { customer_id = cid; order_number = onum; request_type = rtype; }
void CustomerRequest::SetCustomerId(int cid) { customer_id = cid; }
void CustomerRequest::SetOrderNumber(int onum) { order_number = onum; }
void CustomerRequest::SetRequestType(int type) { request_type = type; }
int CustomerRequest::GetCustomerId() const { return customer_id; }
int CustomerRequest::GetOrderNumber() const { return order_number; }
int CustomerRequest::GetRequestType() const { return request_type; }
int CustomerRequest::Size() const { return sizeof(int)*3; }
void CustomerRequest::Marshal(char *buffer) const {
  int a = htonl(customer_id), b = htonl(order_number), c = htonl(request_type);
  int o=0; std::memcpy(buffer+o,&a,4); o+=4; std::memcpy(buffer+o,&b,4); o+=4; std::memcpy(buffer+o,&c,4);
}
void CustomerRequest::Unmarshal(char *buffer) {
  int a,b,c; int o=0; std::memcpy(&a,buffer+o,4); o+=4; std::memcpy(&b,buffer+o,4); o+=4; std::memcpy(&c,buffer+o,4);
  customer_id = ntohl(a); order_number = ntohl(b); request_type = ntohl(c);
}
bool CustomerRequest::IsValid() const { return (request_type==1 || request_type==2); }
void CustomerRequest::Print() const {
  std::cout << "req[cid=" << customer_id << " onum=" << order_number << " type=" << request_type << "]\n";
}

RobotInfo::RobotInfo() { customer_id=-1; order_number=-1; request_type=-1; engineer_id=-1; admin_id=-1; }
void RobotInfo::SetInfo(int cid, int order_num, int req_type, int engid, int adminid) {
  customer_id=cid; order_number=order_num; request_type=req_type; engineer_id=engid; admin_id=adminid;
}
void RobotInfo::CopyRequest(const CustomerRequest &req) {
  customer_id=req.GetCustomerId(); order_number=req.GetOrderNumber(); request_type=req.GetRequestType();
}
void RobotInfo::SetEngineerId(int id) { engineer_id=id; }
void RobotInfo::SetAdminId(int id) { admin_id=id; }
int RobotInfo::GetCustomerId() const { return customer_id; }
int RobotInfo::GetOrderNumber() const { return order_number; }
int RobotInfo::GetRequestType() const { return request_type; }
int RobotInfo::GetEngineerId() const { return engineer_id; }
int RobotInfo::GetAdminId() const { return admin_id; }
int RobotInfo::Size() const { return sizeof(int)*5; }
void RobotInfo::Marshal(char *buffer) const {
  int a=htonl(customer_id), b=htonl(order_number), c=htonl(request_type), d=htonl(engineer_id), e=htonl(admin_id);
  int o=0; std::memcpy(buffer+o,&a,4); o+=4; std::memcpy(buffer+o,&b,4); o+=4; std::memcpy(buffer+o,&c,4); o+=4;
  std::memcpy(buffer+o,&d,4); o+=4; std::memcpy(buffer+o,&e,4);
}
void RobotInfo::Unmarshal(char *buffer) {
  int a,b,c,d,e; int o=0; std::memcpy(&a,buffer+o,4); o+=4; std::memcpy(&b,buffer+o,4); o+=4; std::memcpy(&c,buffer+o,4); o+=4;
  std::memcpy(&d,buffer+o,4); o+=4; std::memcpy(&e,buffer+o,4);
  customer_id=ntohl(a); order_number=ntohl(b); request_type=ntohl(c); engineer_id=ntohl(d); admin_id=ntohl(e);
}
bool RobotInfo::IsValid() const { return (customer_id != -1); }
void RobotInfo::Print() const {
  std::cout << "RobotInfo[cid=" << customer_id << " onum=" << order_number
            << " rtype=" << request_type << " engid=" << engineer_id
            << " adminid=" << admin_id << "]\n";
}

CustomerRecord::CustomerRecord() : customer_id(-1), last_order(-1) {}
CustomerRecord::CustomerRecord(int cid, int last) : customer_id(cid), last_order(last) {}
void CustomerRecord::Set(int cid, int last) { customer_id=cid; last_order=last; }
int CustomerRecord::GetCustomerId() const { return customer_id; }
int CustomerRecord::GetLastOrder() const { return last_order; }
int CustomerRecord::Size() const { return sizeof(int)*2; }
void CustomerRecord::Marshal(char *buffer) const {
  int a=htonl(customer_id), b=htonl(last_order);
  std::memcpy(buffer,&a,4); std::memcpy(buffer+4,&b,4);
}
void CustomerRecord::Unmarshal(char *buffer) {
  int a,b; std::memcpy(&a,buffer,4); std::memcpy(&b,buffer+4,4);
  customer_id=ntohl(a); last_order=ntohl(b);
}
bool CustomerRecord::IsValid() const { return (customer_id != -1); }

ReplicationRequest::ReplicationRequest() : pfa_id(-1), committed_index(-1), last_index(-1) { op={0,0,0}; }
ReplicationRequest::ReplicationRequest(int p, int cidx, int lidx, const MapOp& m)
  : pfa_id(p), committed_index(cidx), last_index(lidx), op(m) {}
int   ReplicationRequest::GetPfaId() const { return pfa_id; }
int   ReplicationRequest::GetCommittedIndex() const { return committed_index; }
int   ReplicationRequest::GetLastIndex() const { return last_index; }
MapOp ReplicationRequest::GetOp() const { return op; }
int ReplicationRequest::Size() const { return 6 * static_cast<int>(sizeof(int)); } // 24
void ReplicationRequest::Marshal(char* buf) const {
  int a=htonl(pfa_id), b=htonl(committed_index), c=htonl(last_index),
      d=htonl(op.opcode), e=htonl(op.arg1), f=htonl(op.arg2);
  int o=0;
  std::memcpy(buf+o,&a,4); o+=4; std::memcpy(buf+o,&b,4); o+=4; std::memcpy(buf+o,&c,4); o+=4;
  std::memcpy(buf+o,&d,4); o+=4; std::memcpy(buf+o,&e,4); o+=4; std::memcpy(buf+o,&f,4);
}
void ReplicationRequest::Unmarshal(char* buf) {
  int a,b,c,d,e,f; int o=0;
  std::memcpy(&a,buf+o,4); o+=4; std::memcpy(&b,buf+o,4); o+=4; std::memcpy(&c,buf+o,4); o+=4;
  std::memcpy(&d,buf+o,4); o+=4; std::memcpy(&e,buf+o,4); o+=4; std::memcpy(&f,buf+o,4);
  pfa_id=ntohl(a); committed_index=ntohl(b); last_index=ntohl(c);
  op.opcode=ntohl(d); op.arg1=ntohl(e); op.arg2=ntohl(f);
}

ReplicationReply::ReplicationReply() : backup_id(-1), ok(0), last_index(-1), committed_index(-1) {}
ReplicationReply::ReplicationReply(int bid, int okv, int lidx, int cidx)
  : backup_id(bid), ok(okv), last_index(lidx), committed_index(cidx) {}
int ReplicationReply::GetBackupId() const { return backup_id; }
int ReplicationReply::GetOk() const { return ok; }
int ReplicationReply::GetLastIndex() const { return last_index; }
int ReplicationReply::GetCommittedIndex() const { return committed_index; }
int ReplicationReply::Size() const { return 4 * static_cast<int>(sizeof(int)); } // 16
void ReplicationReply::Marshal(char* buf) const {
  int a=htonl(backup_id), b=htonl(ok), c=htonl(last_index), d=htonl(committed_index);
  int o=0; std::memcpy(buf+o,&a,4); o+=4; std::memcpy(buf+o,&b,4); o+=4; std::memcpy(buf+o,&c,4); o+=4; std::memcpy(buf+o,&d,4);
}
void ReplicationReply::Unmarshal(char* buf) {
  int a,b,c,d; int o=0; std::memcpy(&a,buf+o,4); o+=4; std::memcpy(&b,buf+o,4); o+=4; std::memcpy(&c,buf+o,4); o+=4; std::memcpy(&d,buf+o,4);
  backup_id=ntohl(a); ok=ntohl(b); last_index=ntohl(c); committed_index=ntohl(d);
}
