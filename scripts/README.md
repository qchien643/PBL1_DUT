# Scripts Guide

Thư mục `scripts/` chứa toàn bộ lệnh `.bat` để build, chạy demo, test nghiệp vụ và reset dữ liệu cho **Casual Dining Ordering MVP**.

Các lệnh trong tài liệu này ưu tiên viết theo góc nhìn **đang đứng ở project root**:

```text
D:\Code\Test\PBL1_DUT
```

Nếu bạn đã `cd scripts`, hãy bỏ tiền tố `scripts\` trong lệnh. Ví dụ:

```bat
scripts\build\build.bat
```

tương đương với:

```bat
build\build.bat
```

## 1. Cấu Trúc Thư Mục

```text
scripts/
  build/          Build và test nghiệp vụ
  demo/           Mở demo nhiều cửa sổ
  legacy/         Alias tên cũ, chỉ giữ để tham khảo/tương thích tương đối
  maintenance/    Reset dữ liệu, dừng app
  run/            Chạy từng vai trò hoặc mode app
  README.md       Tài liệu hướng dẫn script
```

> Khuyến nghị: chạy trực tiếp các file `.bat` trong từng folder theo tác vụ. Root `scripts/` chỉ giữ README để tránh lẫn lộn.

## 2. Quick Start - Web Demo

Khuyến nghị dùng khi demo với giáo viên vì UI dễ quan sát hơn CMD.

```bat
scripts\build\build.bat
scripts\maintenance\reset_data.bat
scripts\demo\start_web_demo.bat
```

Nếu đang đứng trong thư mục `scripts/`:

```bat
build\build.bat
maintenance\reset_data.bat
demo\start_web_demo.bat
```

Script `scripts\demo\start_web_demo.bat` sẽ:

- Chạy C++ server tại `http://localhost:8080`.
- Mở trang home.
- Mở màn hình cashier.
- Mở customer bàn `T01`.
- Mở kitchen/bar.
- Mở manager dashboard.

## 3. Quick Start - Console Demo

Mở nhiều cửa sổ CMD cho từng vai trò:

```bat
scripts\demo\start_console_demo.bat
```

Nếu đang đứng trong thư mục `scripts/`:

```bat
demo\start_console_demo.bat
```

| Window | Vai trò |
| --- | --- |
| `Restaurant Cashier` | Mở bàn, duyệt order, bill/payment |
| `Restaurant Customer T01` | Khách đặt món tại bàn T01 |
| `Restaurant Kitchen` | Bếp nhận món food/dessert |
| `Restaurant Bar` | Bar nhận món drink |
| `Restaurant Manager` | Quản lý menu, report, audit |

## 4. Chạy Từng Vai Trò

| Vai trò | Từ project root | Nếu đang ở `scripts/` |
| --- | --- | --- |
| Cashier | `scripts\run\run_cashier.bat` | `run\run_cashier.bat` |
| Customer T01 | `scripts\run\run_customer_T01.bat` | `run\run_customer_T01.bat` |
| Customer bàn khác | `scripts\run\run_customer_T01.bat T02` | `run\run_customer_T01.bat T02` |
| Kitchen | `scripts\run\run_kitchen.bat` | `run\run_kitchen.bat` |
| Bar | `scripts\run\run_bar.bat` | `run\run_bar.bat` |
| Manager | `scripts\run\run_manager.bat` | `run\run_manager.bat` |
| Server | `scripts\run\run_server.bat` | `run\run_server.bat` |
| Server port khác | `scripts\run\run_server.bat 18080` | `run\run_server.bat 18080` |

## 5. Build, Test, Reset

| Tác vụ | Từ project root | Nếu đang ở `scripts/` |
| --- | --- | --- |
| Build C++ | `scripts\build\build.bat` | `build\build.bat` |
| Chạy business scenario test | `scripts\build\test.bat` | `build\test.bat` |
| Reset dữ liệu seed | `scripts\maintenance\reset_data.bat` | `maintenance\reset_data.bat` |
| Tắt app và reset dữ liệu | `scripts\maintenance\stop_and_reset.bat` | `maintenance\stop_and_reset.bat` |

## 6. Entry Point Nội Bộ

`scripts\run\run_app.bat` là entry point chung, truyền trực tiếp mode vào `restaurant_mvp.exe`.

Ví dụ:

```bat
scripts\run\run_app.bat server 8080
scripts\run\run_app.bat customer T01
scripts\run\run_app.bat test
```

Nếu đang đứng trong thư mục `scripts/`:

```bat
run\run_app.bat server 8080
run\run_app.bat customer T01
run\run_app.bat test
```

## 7. Alias Cũ

Các tên script cũ được chuyển vào `scripts\legacy\` để thư mục chính gọn hơn.

| Alias cũ | Từ project root | Nếu đang ở `scripts/` | Trỏ tới |
| --- | --- | --- | --- |
| `run.bat` | `scripts\legacy\run.bat` | `legacy\run.bat` | `run\run_app.bat` |
| `cashier.bat` | `scripts\legacy\cashier.bat` | `legacy\cashier.bat` | `run\run_cashier.bat` |
| `customer_T01.bat` | `scripts\legacy\customer_T01.bat` | `legacy\customer_T01.bat` | `run\run_customer_T01.bat` |
| `kitchen.bat` | `scripts\legacy\kitchen.bat` | `legacy\kitchen.bat` | `run\run_kitchen.bat` |
| `bar.bat` | `scripts\legacy\bar.bat` | `legacy\bar.bat` | `run\run_bar.bat` |
| `manager.bat` | `scripts\legacy\manager.bat` | `legacy\manager.bat` | `run\run_manager.bat` |
| `server.bat` | `scripts\legacy\server.bat` | `legacy\server.bat` | `run\run_server.bat` |
| `reset.bat` | `scripts\legacy\reset.bat` | `legacy\reset.bat` | `maintenance\reset_data.bat` |
| `start_all.bat` | `scripts\legacy\start_all.bat` | `legacy\start_all.bat` | `demo\start_console_demo.bat` |

## 8. Dữ Liệu

Dữ liệu MVP hiện lưu bằng nhiều file `.txt` theo bảng trong:

```text
data/db/
```

Không còn dùng `data/restaurant_db.txt` làm database tổng.

## 9. Gợi Ý Luồng Demo

1. Từ project root, chạy `scripts\maintenance\reset_data.bat`.
2. Từ project root, chạy `scripts\demo\start_web_demo.bat`.
3. Cashier mở bàn `T01`.
4. Customer `T01` submit order.
5. Cashier accept order.
6. Kitchen/Bar start rồi ready.
7. Customer request bill.
8. Cashier confirm payment.
9. Manager xem audit/report.
