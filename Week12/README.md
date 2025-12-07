# TCP Socket Application with Login/Logout and Registration

Ứng dụng sử dụng TCP socket với fork() hỗ trợ đăng ký, đăng nhập/đăng xuất cho người dùng.

## Tính năng

- **REGISTER**: Đăng ký tài khoản mới
- **LOGIN**: Đăng nhập vào hệ thống
- **LOGOUT**: Đăng xuất khỏi phiên làm việc
- **WHO**: Xem danh sách người dùng đang đăng nhập
- **HELP**: Hiển thị danh sách lệnh có sẵn
- **QUIT**: Đóng kết nối

## Biên dịch

```bash
make
```

## Chạy Server

```bash
./server <PortNumber>
```

Ví dụ:
```bash
./server 5500
```

## Chạy Client

```bash
./client <IPAddress> <PortNumber>
```

Ví dụ:
```bash
./client 127.0.0.1 5500
```

## Sử dụng

### Đăng ký tài khoản mới

```
REGISTER <username> <password>
```

Ví dụ:
```
REGISTER user1 pass123
```

Phản hồi:
- Thành công: `OK Registration successful. You can now login with username: user1`
- Lỗi: 
  - `ERR Username already exists` - Tên người dùng đã tồn tại
  - `ERR Invalid username` - Tên người dùng không hợp lệ
  - `ERR Invalid password` - Mật khẩu không hợp lệ

### Đăng nhập

```
LOGIN <username> <password>
```

Ví dụ:
```
LOGIN user1 pass123
```

Phản hồi:
- Thành công: `OK Login successful. Welcome user1!`
- Lỗi:
  - `ERR Already logged in` - Đã đăng nhập trên kết nối này
  - `ERR Account does not exist` - Tài khoản không tồn tại
  - `ERR Account is locked` - Tài khoản bị khóa
  - `ERR Wrong password` - Sai mật khẩu (3 lần sai → khóa tài khoản)

### Đăng xuất

```
LOGOUT
```

### Xem danh sách người dùng đang online

```
WHO
```

Phản hồi: `LIST user1 user2 user3`

### Trợ giúp

```
HELP
```

### Thoát

```
QUIT
```

## Quy tắc

1. Mỗi cửa sổ client chỉ đăng nhập được 1 tài khoản
2. Mỗi tài khoản có thể đăng nhập được trên nhiều cửa sổ
3. Nếu đăng nhập sai quá 3 lần, tài khoản bị khóa
4. Tài khoản được lưu trong file `account.txt` với định dạng: `UserID Password Status`
   - Status = 0: tài khoản bị khóa
   - Status = 1: tài khoản hoạt động
5. Lịch sử đăng nhập/đăng xuất/đăng ký được ghi vào file `auth.log`

## File log

File `auth.log` ghi lại các sự kiện:
- `[timestamp] REGISTER user1 from 127.0.0.1:54321 SUCCESS`
- `[timestamp] REGISTER user2 from 127.0.0.1:54322 FAIL (username exists)`
- `[timestamp] LOGIN user1 from 127.0.0.1:54321 SUCCESS`
- `[timestamp] LOGIN user1 from 127.0.0.1:54322 FAIL (wrong password)`
- `[timestamp] ACCOUNT_LOCKED user1`
- `[timestamp] LOGOUT user1 from 127.0.0.1:54321`

## Cấu trúc Project

- `server.c` - Server chính sử dụng fork() để xử lý nhiều client
- `client.c` - Client với giao diện command-line
- `account.c/h` - Quản lý tài khoản người dùng
- `session.c/h` - Quản lý phiên đăng nhập
- `auth_log.c/h` - Ghi log xác thực
- `account.txt` - File lưu trữ tài khoản
- `auth.log` - File log lịch sử xác thực
- `Makefile` - Script biên dịch

## Giao thức ứng dụng (Application Protocol)

Giao thức text-based với format:
- Request: `COMMAND [arg1] [arg2]`
- Response: 
  - Thành công: `OK <message>`
  - Lỗi: `ERR <error_message>`
  - Danh sách: `LIST <items>`

## Dọn dẹp

```bash
make clean
```
