#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include <string>
#include <cstdint>

enum class RoleType : int32_t {
  CUSTOMER = 1,
  PFA      = 2
};


struct MapOp {
  int opcode;
  int arg1;
  int arg2;
};

class CustomerRequest {
private:
    int customer_id;
    int order_number;
    int request_type;
public:
    CustomerRequest();
    void operator=(const CustomerRequest &r) {
        customer_id = r.customer_id;
        order_number = r.order_number;
        request_type = r.request_type;
    }
    void Set(int cid, int onum, int rtype);
    void SetCustomerId(int cid);
    void SetOrderNumber(int onum);
    void SetRequestType(int type);

    int GetCustomerId() const;
    int GetOrderNumber() const;
    int GetRequestType() const;

    int Size() const;
    void Marshal(char *buffer) const;
    void Unmarshal(char *buffer);

    bool IsValid() const;
    void Print() const;
};

class RobotInfo {
private:
    int customer_id;
    int order_number;
    int request_type;
    int engineer_id;
    int admin_id;

public:
    RobotInfo();
    void operator=(const RobotInfo &info) {
        customer_id = info.customer_id;
        order_number = info.order_number;
        request_type = info.request_type;
        engineer_id = info.engineer_id;
        admin_id   = info.admin_id;
    }
    void SetInfo(int cid, int order_num, int req_type, int engid, int adminid);
    void CopyRequest(const CustomerRequest &req);
    void SetEngineerId(int id);
    void SetAdminId(int id);

    int GetCustomerId() const;
    int GetOrderNumber() const;
    int GetRequestType() const;
    int GetEngineerId() const;
    int GetAdminId() const;

    int Size() const;
    void Marshal(char *buffer) const;
    void Unmarshal(char *buffer);

    bool IsValid() const;
    void Print() const;
};

class CustomerRecord {
private:
    int customer_id;  
    int last_order;   
public:
    CustomerRecord();
    CustomerRecord(int cid, int last);

    void Set(int cid, int last);
    int GetCustomerId() const;
    int GetLastOrder() const;

    int Size() const;
    void Marshal(char *buffer) const;
    void Unmarshal(char *buffer);

    bool IsValid() const;
};


class ReplicationRequest {
private:
  int pfa_id;           
  int committed_index;   
  int last_index;        
  MapOp op;             
public:
  ReplicationRequest();
  ReplicationRequest(int p, int cidx, int lidx, const MapOp& m);

  int   GetPfaId() const;
  int   GetCommittedIndex() const;
  int   GetLastIndex() const;
  MapOp GetOp() const;

  int Size() const;           
  void Marshal(char* buf) const;
  void Unmarshal(char* buf);
};

class ReplicationReply {
private:
  int backup_id;
  int ok;               
  int last_index;
  int committed_index;
public:
  ReplicationReply();
  ReplicationReply(int bid, int okv, int lidx, int cidx);

  int GetBackupId() const;
  int GetOk() const;
  int GetLastIndex() const;
  int GetCommittedIndex() const;

  int Size() const;     
  void Marshal(char* buf) const;
  void Unmarshal(char* buf);
};

#endif
