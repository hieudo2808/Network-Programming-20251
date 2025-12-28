# BÁO CÁO BÀI TẬP LẬP TRÌNH MẠNG

## Ứng dụng TCP Server với Non-blocking I/O và select()

**Tuần:** 14 - Network Programming  
**Ngày:** 23/12/2025

---

## 1. Giới thiệu

### 1.1. Mục tiêu
Xây dựng hệ thống client-server TCP hỗ trợ:
- Đăng ký, đăng nhập, đăng xuất người dùng
- Xử lý đồng thời nhiều client bằng **Non-blocking I/O với select()**
- Khóa tài khoản sau 3 lần nhập sai mật khẩu
- Ghi log các hoạt động xác thực

### 1.2. Công nghệ sử dụng
- **Ngôn ngữ:** C
- **I/O Model:** Non-blocking với select()
- **Giao thức:** TCP/IP, text-based protocol
- **Lưu trữ:** File-based (account.txt, auth.log)

---

## 2. Kiến trúc hệ thống

### 2.1. Tổng quan kiến trúc

```mermaid
graph TB
    subgraph Clients
        C1[Client 1]
        C2[Client 2]
        C3[Client N]
    end
    
    subgraph Server[Server Process - Single Thread]
        subgraph EventLoop[Event Loop]
            FDSET[fd_set master]
            SELECT[select - multiplexing]
            FDSET --> SELECT
        end
        
        subgraph NonBlock[Non-blocking IO]
            SRV_FD[server_fd - O_NONBLOCK]
            CLI_FD[client_fd - O_NONBLOCK]
        end
        
        subgraph Handler[Command Handler]
            PROC[process_cmd]
        end
        
        subgraph Modules[Business Logic]
            ACC[Account]
            SESS[Session]
            LOG[Auth Log]
        end
    end
    
    subgraph Storage
        F1[(account.txt)]
        F2[(auth.log)]
    end
    
    C1 -->|TCP socket| SRV_FD
    C2 -->|TCP socket| SRV_FD
    C3 -->|TCP socket| SRV_FD
    
    SRV_FD -->|accept| CLI_FD
    CLI_FD -->|add to| FDSET
    
    SELECT -->|ready fd| NonBlock
    NonBlock -->|recv data| PROC
    
    PROC --> ACC
    PROC --> SESS
    PROC --> LOG
    
    ACC --> F1
    LOG --> F2
```

### 2.2. Mô hình Non-blocking IO với select

```mermaid
graph LR
    subgraph Traditional[Blocking IO]
        B1[recv] -->|block| B2[wait...]
        B2 --> B3[data ready]
    end
    
    subgraph NonBlocking[Non-blocking IO + select]
        NB1[select] -->|monitor all fds| NB2{any fd ready?}
        NB2 -->|No| NB1
        NB2 -->|Yes| NB3[recv - immediate return]
        NB3 --> NB4[process]
        NB4 --> NB1
    end
```

**Giải thích:**
- **fd_set master**: Tập hợp tất cả file descriptors cần theo dõi
- **select**: System call chờ đến khi có ít nhất 1 fd sẵn sàng đọc/ghi
- **O_NONBLOCK**: Flag đặt socket ở chế độ non-blocking, recv/send không block
- **Single thread**: Một vòng lặp xử lý tất cả clients tuần tự

### 2.2. Cấu trúc module

| Module | File | Chức năng |
|--------|------|-----------|
| Server | server.c | Main loop, xử lý lệnh, quản lý client |
| Account | account.c/h | CRUD tài khoản, load/save file |
| Session | session.c/h | Quản lý phiên đăng nhập |
| Auth Log | auth_log.c/h | Ghi log xác thực |
| Client | client.c | Giao diện người dùng |

---

## 3. Thiết kế chi tiết

### 3.1. Cấu trúc dữ liệu

```mermaid
classDiagram
    class Account {
        +char username[50]
        +char password[50]
        +int status
        +int failed_attempts
        +Account* next
    }
    
    class Session {
        +char username[50]
        +int client_fd
        +Session* next
    }
    
    class Client {
        +int fd
        +char ip[16]
        +int port
        +char buf[1024]
        +int buf_len
    }
    
    Account --> Account : next
    Session --> Session : next
```

**Giải thích:**
- **Account:** Linked list lưu thông tin tài khoản, `status=0` là bị khóa
- **Session:** Linked list lưu phiên đăng nhập, map `client_fd` với `username`
- **Client:** Mảng tĩnh trong server, lưu buffer để xử lý partial reads

### 3.2. Giao thức ứng dụng

| Lệnh | Cú pháp | Yêu cầu đăng nhập | Mô tả |
|------|---------|-------------------|-------|
| REGISTER | `REGISTER <user> <pass>` | Không | Tạo tài khoản mới |
| LOGIN | `LOGIN <user> <pass>` | Không | Đăng nhập |
| LOGOUT | `LOGOUT` | Có | Đăng xuất |
| WHO | `WHO` | Có | Danh sách user online |
| HELP | `HELP` | Không | Hướng dẫn sử dụng |
| QUIT | `QUIT` | Không | Ngắt kết nối |

---

## 4. Luồng xử lý

### 4.1. Server Main Loop

```mermaid
flowchart TD
    START([Start]) --> INIT[Khởi tạo socket non-blocking]
    INIT --> LOAD[Load accounts từ file]
    LOAD --> FDSET[FD_SET server_fd vào master]
    FDSET --> LOOP{select loop}
    
    LOOP --> SELECT[select - chờ IO ready]
    SELECT --> CHECK_SRV{server_fd ready?}
    
    CHECK_SRV -->|Yes| ACCEPT[accept client mới]
    ACCEPT --> NONBLOCK[Set non-blocking]
    NONBLOCK --> ADD[Thêm vào clients và fd_set]
    ADD --> LOOP
    
    CHECK_SRV -->|No| CHECK_CLI{Duyệt clients}
    CHECK_CLI --> RECV{recv data}
    
    RECV -->|n <= 0| DISCONNECT[Ngắt kết nối và cleanup]
    DISCONNECT --> LOOP
    
    RECV -->|n > 0| BUFFER[Thêm vào buffer]
    BUFFER --> PARSE{Có dòng hoàn chỉnh?}
    PARSE -->|Yes| PROCESS[Xử lý lệnh]
    PROCESS --> PARSE
    PARSE -->|No| LOOP
```

### 4.2. Xử lý Login

```mermaid
flowchart TD
    LOGIN[LOGIN user pass] --> CHK1{Đã đăng nhập?}
    CHK1 -->|Yes| ERR1[Already logged in]
    CHK1 -->|No| CHK2{Account tồn tại?}
    
    CHK2 -->|No| ERR2[Account does not exist]
    CHK2 -->|Yes| CHK3{Account bị khóa?}
    
    CHK3 -->|Yes| ERR3[Account is locked]
    CHK3 -->|No| CHK4{Password đúng?}
    
    CHK4 -->|No| FAIL[Wrong password - tăng failed_attempts]
    FAIL --> CHK5{attempts >= 3?}
    CHK5 -->|Yes| LOCK[Khóa tài khoản]
    CHK5 -->|No| END1([Kết thúc])
    LOCK --> END1
    
    CHK4 -->|Yes| SUCCESS[Reset failed_attempts - Tạo session]
    SUCCESS --> OK[Login successful]
    OK --> END2([Kết thúc])
    
    ERR1 --> END3([Kết thúc])
    ERR2 --> END3
    ERR3 --> END3
```

### 4.3. Client Flow

```mermaid
flowchart TD
    START([Start]) --> CONNECT[Kết nối server]
    CONNECT --> LOOP{Main Loop}
    
    LOOP --> RECV[Nhận response]
    RECV --> DISPLAY[Hiển thị]
    DISPLAY --> INPUT[Nhập lệnh]
    INPUT --> SEND[Gửi đến server]
    SEND --> CHECK{Lệnh QUIT?}
    
    CHECK -->|No| LOOP
    CHECK -->|Yes| CLOSE[Đóng kết nối]
    CLOSE --> END([Exit])
```

---

## 5. Non-blocking I/O với select()

### 5.1. Nguyên lý hoạt động

```mermaid
sequenceDiagram
    participant S as Server
    participant SEL as select
    participant C1 as Client 1
    participant C2 as Client 2
    
    S->>SEL: Đăng ký fd_set
    SEL-->>S: Block cho đến khi có IO ready
    
    C1->>SEL: Gửi data
    SEL-->>S: c1 ready for read
    S->>C1: recv và xử lý
    
    C2->>SEL: Kết nối mới
    SEL-->>S: server_fd ready
    S->>C2: accept
    
    Note over S,C2: Single thread xử lý tất cả
```

### 5.2. Ưu điểm so với Multi-threading

| Tiêu chí | pthread | select |
|----------|---------|--------|
| **Memory** | Stack riêng mỗi thread | Shared memory |
| **Synchronization** | Cần mutex/semaphore | Không cần |
| **Context Switch** | Tốn CPU | Không có |
| **Complexity** | Race conditions | Linear, dễ debug |
| **Scalability** | ~1000 threads | ~1000+ connections |

### 5.3. Xử lý Partial Reads

Do non-blocking mode, recv có thể trả về data không đầy đủ. Server sử dụng buffer để ghép các phần:

```
Lần 1: recv() → "LOG"
Lần 2: recv() → "IN user"
Lần 3: recv() → " pass\n"
Buffer: "LOGIN user pass\n" → Xử lý
```

---

## 6. Quản lý tài khoản

### 6.1. File Format (account.txt)
```
username1 password1 1
username2 password2 0
```
- Cột 3: `1` = active, `0` = locked

### 6.2. Cơ chế khóa tài khoản
- Mỗi lần sai password: `failed_attempts++`
- Khi `failed_attempts >= 3`: `status = 0` (khóa)
- Đăng nhập thành công: `failed_attempts = 0`

---

## 7. Logging

### 7.1. Format log (auth.log)
```
[2025-12-23 14:30:00] LOGIN alice from 127.0.0.1:45678 SUCCESS
[2025-12-23 14:30:05] LOGIN bob from 127.0.0.1:45679 FAIL
[2025-12-23 14:30:10] ACCOUNT_LOCKED bob
[2025-12-23 14:31:00] LOGOUT alice from 127.0.0.1:45678
```

### 7.2. Các sự kiện được log
- LOGIN (success/fail)
- LOGOUT
- REGISTER
- ACCOUNT_LOCKED

---

## 8. Kết luận

### 8.1. Kết quả đạt được
- ✅ Server xử lý đồng thời nhiều client với single thread
- ✅ Hệ thống xác thực đầy đủ (register/login/logout)
- ✅ Bảo mật với cơ chế khóa tài khoản
- ✅ Logging đầy đủ các hoạt động
- ✅ Code tối ưu, modular (~540 dòng)

### 8.2. Hạn chế
- Chưa mã hóa password
- Chưa hỗ trợ IPv6
- Giới hạn 100 client đồng thời (có thể tăng)

### 8.3. Hướng phát triển
- Sử dụng epoll thay select cho Linux
- Thêm mã hóa TLS/SSL
- Hash password với bcrypt/argon2
