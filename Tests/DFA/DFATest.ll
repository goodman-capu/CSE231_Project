; ModuleID = '/tests/DFA/DFATest.cpp'
source_filename = "/tests/DFA/DFATest.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define i32 @_Z3fooj(i32) #0 {
  %2 = alloca i32, align 4
  %3 = alloca [20 x i32], align 16
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  store i32 0, i32* %4, align 4
  br label %7

; <label>:7:                                      ; preds = %25, %1
  %8 = load i32, i32* %4, align 4
  %9 = icmp slt i32 %8, 20
  br i1 %9, label %10, label %28

; <label>:10:                                     ; preds = %7
  %11 = load i32, i32* %2, align 4
  %12 = load i32, i32* %4, align 4
  %13 = shl i32 1, %12
  %14 = and i32 %11, %13
  %15 = icmp ugt i32 %14, 0
  br i1 %15, label %16, label %20

; <label>:16:                                     ; preds = %10
  %17 = load i32, i32* %4, align 4
  %18 = sext i32 %17 to i64
  %19 = getelementptr inbounds [20 x i32], [20 x i32]* %3, i64 0, i64 %18
  store i32 1, i32* %19, align 4
  br label %24

; <label>:20:                                     ; preds = %10
  %21 = load i32, i32* %4, align 4
  %22 = sext i32 %21 to i64
  %23 = getelementptr inbounds [20 x i32], [20 x i32]* %3, i64 0, i64 %22
  store i32 0, i32* %23, align 4
  br label %24

; <label>:24:                                     ; preds = %20, %16
  br label %25

; <label>:25:                                     ; preds = %24
  %26 = load i32, i32* %4, align 4
  %27 = add nsw i32 %26, 1
  store i32 %27, i32* %4, align 4
  br label %7

; <label>:28:                                     ; preds = %7
  store i32 0, i32* %5, align 4
  store i32 0, i32* %6, align 4
  br label %29

; <label>:29:                                     ; preds = %49, %28
  %30 = load i32, i32* %6, align 4
  %31 = icmp slt i32 %30, 20
  br i1 %31, label %32, label %52

; <label>:32:                                     ; preds = %29
  %33 = load i32, i32* %6, align 4
  %34 = sext i32 %33 to i64
  %35 = getelementptr inbounds [20 x i32], [20 x i32]* %3, i64 0, i64 %34
  %36 = load i32, i32* %35, align 4
  %37 = icmp sgt i32 %36, 0
  br i1 %37, label %38, label %48

; <label>:38:                                     ; preds = %32
  %39 = load i32, i32* %6, align 4
  %40 = sext i32 %39 to i64
  %41 = getelementptr inbounds [20 x i32], [20 x i32]* %3, i64 0, i64 %40
  %42 = load i32, i32* %41, align 4
  %43 = icmp sgt i32 %42, 0
  %44 = zext i1 %43 to i64
  %45 = select i1 %43, i32 1, i32 0
  %46 = load i32, i32* %5, align 4
  %47 = add nsw i32 %46, %45
  store i32 %47, i32* %5, align 4
  br label %48

; <label>:48:                                     ; preds = %38, %32
  br label %49

; <label>:49:                                     ; preds = %48
  %50 = load i32, i32* %6, align 4
  %51 = add nsw i32 %50, 1
  store i32 %51, i32* %6, align 4
  br label %29

; <label>:52:                                     ; preds = %29
  %53 = load i32, i32* %5, align 4
  switch i32 %53, label %57 [
    i32 0, label %54
    i32 1, label %55
    i32 2, label %56
  ]

; <label>:54:                                     ; preds = %52
  store i32 2, i32* %5, align 4
  br label %58

; <label>:55:                                     ; preds = %52
  store i32 4, i32* %5, align 4
  br label %58

; <label>:56:                                     ; preds = %52
  store i32 5, i32* %5, align 4
  br label %58

; <label>:57:                                     ; preds = %52
  br label %58

; <label>:58:                                     ; preds = %57, %56, %55, %54
  %59 = load i32, i32* %5, align 4
  ret i32 %59
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.1 (tags/RELEASE_501/final 322011)"}
