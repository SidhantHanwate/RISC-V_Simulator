li x7, 100
li x4, 10
li x1, 5
li x8, 2

add x3, x2, x1
add x3, x3, x3
bne x3, x4, loop

loop:
addi x5, x4, 1
add x5, x5, x5
end
