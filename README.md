# Casual Dining Ordering MVP

Ứng dụng MVP cho đồ án: hệ thống đặt món tại bàn cho nhà hàng **casual dining**, chạy bằng Web UI hoặc nhiều cửa sổ CMD và cùng đọc/ghi dữ liệu trong `data/db/`.

## Công nghệ

- C++17
- CMake
- Lưu trữ MVP bằng nhiều file `.txt` theo bảng trong `data/db/`
- Không dùng thư viện ngoài để dễ nộp và chạy trên máy học phần

## Build

Trên máy hiện tại có `g++` và `mingw32-make`, dùng lệnh:

```powershell
cmake -S . -B build-mingw -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER="D:/App/MSYS3/ucrt64/bin/g++.exe"
cmake --build build-mingw
```

File chạy nằm tại:

```powershell
.\build-mingw\restaurant_mvp.exe
```

Nếu dùng Visual Studio generator, file có thể nằm tại:

```powershell
.\build\Debug\restaurant_mvp.exe
```

## Chạy nhiều cửa sổ CMD

Mỗi vai trò mở một cửa sổ riêng nhưng dùng chung các table file trong `data/db/`.

### Cách dễ nhất bằng script

```bat
scripts\build\build.bat
scripts\maintenance\reset_data.bat
scripts\demo\start_console_demo.bat
```

Chạy từng vai trò:

```bat
scripts\run\run_cashier.bat
scripts\run\run_customer_T01.bat
scripts\run\run_kitchen.bat
scripts\run\run_bar.bat
scripts\run\run_manager.bat
```

Đóng toàn bộ cửa sổ app và reset dữ liệu về seed ban đầu:

```bat
scripts\maintenance\stop_and_reset.bat
```

## Chạy Web UI + C++ Server

Khởi động server và mở toàn bộ màn hình browser:

```bat
scripts\demo\start_web_demo.bat
```

Hoặc chạy server thủ công:

```bat
scripts\run\run_server.bat
```

Sau đó mở:

```text
http://localhost:8080/
http://localhost:8080/cashier.html
http://localhost:8080/customer.html?table=T01
http://localhost:8080/kitchen.html?station=kitchen
http://localhost:8080/kitchen.html?station=bar
http://localhost:8080/manager.html
```

Luồng web demo:

1. Cashier mở bàn `T01`.
2. Customer `T01` thêm món và submit order.
3. Cashier nhận toast `NEW_ORDER`, accept order.
4. Kitchen/Bar nhận toast `TASK_CREATED`, start rồi ready.
5. Customer request bill.
6. Cashier confirm payment.

### Cách chạy trực tiếp executable

```powershell
.\build-mingw\restaurant_mvp.exe reset
.\build-mingw\restaurant_mvp.exe test
.\build-mingw\restaurant_mvp.exe cashier
.\build-mingw\restaurant_mvp.exe customer T01
.\build-mingw\restaurant_mvp.exe kitchen kitchen
.\build-mingw\restaurant_mvp.exe kitchen bar
.\build-mingw\restaurant_mvp.exe manager
.\build-mingw\restaurant_mvp.exe server 8080
```

Nếu dùng MinGW:

```powershell
.\build\restaurant_mvp.exe cashier
```

Xem chi tiết toàn bộ script tại `scripts/README.md`. Script được chia theo nhóm trong `scripts/build/`, `scripts/run/`, `scripts/demo/`, `scripts/maintenance/` và `scripts/legacy/`.

## Luồng demo nhanh

1. `cashier` mở bàn `T01`.
2. `customer T01` xem menu, thêm món vào giỏ, submit order.
3. `cashier` duyệt order.
4. `kitchen kitchen` hoặc `kitchen bar` nhận task, start, mark ready.
5. `customer T01` yêu cầu bill.
6. `cashier` xác nhận thanh toán.
7. `cashier` chuyển bàn từ `CLEANING` về `AVAILABLE`.

## Kiểm thử nghiệp vụ P0

Chạy bộ scenario test nội bộ không cần thư viện ngoài:

```powershell
.\build-mingw\restaurant_mvp.exe test
```

Hoặc dùng script:

```bat
scripts\build\test.bat
```

Bộ test hiện kiểm tra: idempotency khi submit, sold-out yêu cầu khách xác nhận, replacement rồi accept lại, kitchen issue chặn bill, resolve issue, bill stale, payment thiếu tiền và permission tối thiểu.

## Nghiệp vụ đã có trong MVP

- Mở phiên ăn tại bàn.
- Ghép bàn và chuyển bàn.
- Menu item có trạng thái `AVAILABLE` / `SOLD_OUT`.
- Khách đặt món, lễ tân duyệt trước khi bếp nhận.
- Nếu món vừa hết khi cashier duyệt, order chuyển sang `NEEDS_CUSTOMER_CONFIRMATION` để khách hủy/bỏ món/thay món.
- Hủy món đặt nhầm qua `CANCEL_REQUESTED`; chỉ hủy nếu bếp chưa bắt đầu.
- Tách task cho `kitchen` và `bar`; bếp có thể report `ISSUE`, staff resolve bằng hủy/remake/override.
- Tính bill cuối bữa bằng bill snapshot/line; món `CANCELLED` không bị tính tiền, bill stale bị chặn khi thanh toán.
- Payment yêu cầu paid amount hợp lệ và role cashier/manager.
- Permission policy tối thiểu cho customer/cashier/kitchen/waiter/manager.
- Recommendation đơn giản theo latent-factor mô phỏng trên `DiningSession x MenuItem`.
- Audit log cho các hành động quan trọng.

## Kiến trúc code

```text
src/
  domain/                 # Entity/record dùng chung
  shared/                 # Utility chung
  infrastructure/         # Table-per-file storage, FileDatabase facade
  policies/               # Business rule/policy dùng để quyết định nghiệp vụ
  modules/
    table_session/        # Mở bàn, ghép bàn, chuyển bàn
    menu_inventory/       # Menu item, sold-out/available
    order_management/     # Submit, duyệt, từ chối, hủy món đặt nhầm
    kitchen_fulfillment/  # Task bếp/bar
    payment_billing/      # Bill cuối bữa, thanh toán
    recommendation_ai_ml/ # Latent-factor recommendation MVP
    reporting_audit/      # Doanh thu và audit log
  console/                # Adapter CMD cho manager/cashier/customer/kitchen
```

`main.cpp` chỉ còn nhiệm vụ chọn mode chạy. Rule nghiệp vụ nằm trong `src/policies/`, còn xử lý theo module nằm trong `src/modules/`.

Xem thêm mapping giữa tài liệu thiết kế và code tại `docs/product-design/13-implementation-roadmap/code-architecture-map.md`.

## Ghi chú đồng bộ CMD

Mỗi vòng thao tác sẽ đọc lại các table file trong `data/db/`, sau đó ghi lại file bảng bị thay đổi. Vì vậy nhiều cửa sổ CMD có thể nhìn thấy trạng thái mới nhất sau khi chọn lại menu/chức năng.

Đây là cơ chế phù hợp cho MVP đồ án. Nếu nâng cấp thành sản phẩm thật, nên thay bằng SQLite/PostgreSQL và transaction/locking chuẩn.
