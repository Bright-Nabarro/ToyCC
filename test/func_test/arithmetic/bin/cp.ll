; ModuleID = 'toycc.expr'
source_filename = "toycc.expr"

define i32 @int_add(i32 %0, i32 %1) {
entry:
  %2 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %3 = alloca i32, align 4
  store i32 %1, ptr %3, align 4
  %4 = load i32, ptr %2, align 4
  %5 = load i32, ptr %3, align 4
  %6 = add i32 %4, %5
  ret i32 %6
}

define i32 @int_sub(i32 %0, i32 %1) {
entry:
  %2 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %3 = alloca i32, align 4
  store i32 %1, ptr %3, align 4
  %4 = load i32, ptr %2, align 4
  %5 = load i32, ptr %3, align 4
  %6 = sub i32 %4, %5
  ret i32 %6
}

define i32 @int_mul(i32 %0, i32 %1) {
entry:
  %2 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %3 = alloca i32, align 4
  store i32 %1, ptr %3, align 4
  %4 = load i32, ptr %2, align 4
  %5 = load i32, ptr %3, align 4
  %6 = mul i32 %4, %5
  ret i32 %6
}

define i32 @int_div(i32 %0, i32 %1) {
entry:
  %2 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %3 = alloca i32, align 4
  store i32 %1, ptr %3, align 4
  %4 = load i32, ptr %2, align 4
  %5 = load i32, ptr %3, align 4
  %6 = sdiv i32 %4, %5
  ret i32 %6
}
