- C99 standard
- Sử dụng space thay cho tab 1tab = 4space
- Sử dụng một khoảng trắng trước và sau các toán tử so sánh và gán
```c
int32_t a;
a = 3 + 4;              /* OK */
for (a = 0; a < 5; ++a) /* OK */
```
- Luôn khai báo các biến cục bộ ở đầu khối, trước câu lệnh thực thi đầu tiên
- Khai báo các biến bộ đếm trong `for`
```c
/* OK */
for (size_t i = 0; i < 10; ++i)

/* OK, if you need counter variable later */
size_t i;
for (i = 0; i < 10; ++i) {
    if (...) {
        break;
    }
}
if (i == 10) {

}

/* Wrong */
size_t i;
for (i = 0; i < 10; ++i) ...
```
- Ngoại trừ `char`, `float` hoặc `double`, luôn sử dụng các kiểu được khai báo trong `stdint.h`
- Luôn sử dụng `size_t` cho các biến độ dài hoặc kích thước
- Luôn sử dụng dấu ngoặc với `sizeof` toán tử
- Sử dụng phân bổ bộ nhớ động thay thế với C `malloc` và các `free` chức năng tiêu chuẩn hoặc nếu thư viện / dự án cung cấp phân bổ bộ nhớ tùy chỉnh, hãy sử dụng triển khai của nó.
- Luôn bao gồm kiểm tra C++với `extern` từ khóa trong tệp tiêu đề
- Mọi hàm global phải bao gồm nhận xét hỗ trợ doxygen
- Luôn sử dụng `<`và `>`đối với Thư viện Tiêu chuẩn C bao gồm các tệp, ví dụ.`#include <stdlib.h>`
- Luôn sử dụng `""` cho các thư viện tùy chỉnh, ví dụ.`#include "my_library.h"`

## Function
- Các hàm được truy cập từ bên ngoài luôn được mở đầu bằng `NGUYÊN MẪU` và phân cách bằng `_`
```c
void LCD_Init(void);
void LCD_DrawLine(void);
```
- Các hàm `static` nếu nằm trong `module` thì được mở đầu bằng `nguyên mẫu` hoặc bắt đầu bằng cụm `s_`, sử dụng các kí tự thường và phân cách bằng dấu `_`
```c
void lcd_delay(uint32_t time);
void s_compare_data(uint8_t* pData1, uint8_t*pData2);
```

## Variable
- Không khai báo các biến `local` sau câu lệnh đầu tiên
- Các biến `global` của các `module` được khai báo trên cùng của tập file
- Các biến sử dụng tiêu chuẩn camleCase
- Các hằng số sử dựng kí tự viết hoa và `_` để ngăn cách

## Struct, Enum, Typedefs
- Tên cấu trúc phải là chữ cái thường phân cách bằng dấu `_`
- Tên thành viên trong cấu trúc phải là chữ cái thường
- Tên thành viên trong `enum` phải là kí tự viết hoa
- Khi cấu trúc được khai báo bằng `typedefs` thì phải được chứa hậu tố `_t`
```c
typedef  struct {
    char * a;
    char b; 
} struct_name_t ;

typedef enum { 
    MY_ENUM_TESTA, 
    MY_ENUM_TESTB, 
} my_enum_t ;

```

## Macro
- Luôn sử dụng macro thay thế cho các hằng số
- Tất cả các macro đều phải được viết hoa và phân cách bằng `_`
```c
/* OK */
#define MIN(x, y)           ((x) < (y) ? (x) : (y))
```

- Khi macro sử dụng nhiều dòng lệnh thực hiện bảo vệ bằng `do-while(0)`
```c
#define SET_POINT(p, x, y) do {\
    (p)->px = (x);             \
    (p)->py = (y);             \
}while(0)
```

## Header File
- Header file phải bao gồm `#ifdef`
- Phải bao gồm check `c++`
- Tệp chỉ bao gồm các kiểu dữ liệu, biến và hàm global của module
- Không đưa file `.c` vào một file `.c` khác
```c
#ifndef TEMPLATE_HDR_H
#define TEMPLATE_HDR_H

/* Include headers */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* File content here */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEMPLATE_HDR_H */
```

## Compound statement
- Luôn sử dụng `{}` khi dùng với `if` bất kể là có 1 câu lệnh hay nhiều câu lệnh
- Trường hợp sử dụng nhiều `if` và `else if`
```c
/* OK */
if(a)
{
    /* some thing */
}
else if (b)
{
    /* do some thing */
}
```
- Trường hợp sử dụng `do-while`
```c
/* Ok */
do
{
    /* do some thing */
}while(check());
```

- Trường hợp sử dụng `while`
```c
/* OK */
while(check()) {}
while(a)
{
    /* do some thing */
}
```

## Switch statement
- Luôn tồn tại `default`
- Luôn thụt vào đối với `case` và `break`
```C
switch(check())
{
    case 0:
        break;
    case 1:
        break;
    default:
        break;
}
```

