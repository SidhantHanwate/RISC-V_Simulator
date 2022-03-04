li x5, 5
li x29, 3
li x28, 1

LABEL1: 
add x5 , x5 , x28
sub x5, x5 , x28
bne x5, x29 , LABEL #branch instruction
end

LABEL: 
sub x5, x5 , x28
jal x5, LABEL1 