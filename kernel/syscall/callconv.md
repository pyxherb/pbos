# PbOS System Call Calling Convention

## Parameters

The first n-th parameters that are smaller or equal to the width of the
registers are stored in architecture-specific registers in specific order,
where n is architecture-specific.

If a parameter is too big to store in a single register, it can be divided
into multiple parts and stored in multiple continuous registers in order.
e.g. a 64-bit value can be divided into two 32-bit values and stored in two
registers that are logically specified as former and latter.

If there are more than n parameters or the registers are not enough to contain
the values, the first (n-1) registers are used for storing the first (n-1)
parameters (or fewer if the parameter is too big to be contained) and the rest
of the parameters are stored in a continuous area in the user memory, which is
passed by the last register.

### Example

x86 stores first 6 arguments in `eax`, `ebx`, `ecx`, `edx`, `esi`, `edi`
registers, when the parameter number is less than or equal to 6 and the total
parameter size is less than or equal to 4 * 6 bytes, we use the registers in
order of `eax`, `ebx`, `ecx`, `edx`, `esi`, `edi`.

When the parameter number is greater than 6, we use `eax`, `ebx`, `ecx`, `edx`,
`esi`, `edi` to store the first 20-byte parameter values and use `edi` to store
a pointer to an area which holds the rest of the arguments.

e.g. We have a function under x86:

```c
void sysent_f(int a, int b, int c, int d, int e, int f, int g, int h);
```

We store the parameters like following:

```c
eax -> a
ebx -> b
ecx -> c
edx -> d
esi -> e
edi -> struct { int f; int g; int h; }*
```

## Return Value

The return value is stored in architecture-specific registers (e.g. eax for
32-bit or eax and edx for 64-bit under x86).

The return value should not be greater than 64-bit or platform-specific size
width.
