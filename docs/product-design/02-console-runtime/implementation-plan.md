# Implementation Plan - Console Runtime

## Steps

| Step | Task | Done when |
| --- | --- | --- |
| 1 | Tạo base command loop | Các CMD dùng chung input/result |
| 2 | Tạo actor/context builder | Mỗi command có context đầy đủ |
| 3 | Implement Customer CMD | Xem menu, submit order |
| 4 | Implement Staff CMD | Mở bàn, duyệt, thanh toán |
| 5 | Implement Kitchen CMD | Xem/update task |
| 6 | Implement Manager CMD | Config/menu/report/recommendation |
| 7 | Add notification refresh | Sau command hiển thị event mới |

## Acceptance criteria

- Chạy được 4 CMD cùng lúc.
- CMD không truy cập repository trực tiếp.
- Lỗi policy hiển thị rõ cho người dùng.
