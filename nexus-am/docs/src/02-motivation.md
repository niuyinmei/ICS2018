# 简易C模型：什么能做什么不能做

## C可以做任计算(Turing Completeness)

## 但也有很多事不能做

看起来很蠢，但刚才我们那个计算机其实不能输出任何东西。它能算出有意思的结果来，但只在内存里。连

    printf("Hello World");

都不行。

printf怎么实现？最后总逃不了要putchar，putchar的发生实际改变了物理世界的状态(LCD屏幕上的电特性)。

计算机是在物理世界里的。

画图、声音，C模型里都是没有的。你会发现，如果你要做这些事情，都要调用别人写好的代码(库) -> OS -> 不知道发生了什么。

不知道发生了什么是什么？总的来说，还是执行的*指令*。

### 中断

比如，CPU可以自发地改变处理器的流程，比如

    while (1) ;

但这个C语言里没有啊。

### I/O

比如画图，每个设备都有自己的“内存”，你往里面读/写可以得到东西。比如显存、声音缓存。

### 多处理器

(???)

### 怎么让C语言能做这些事？

这些事机器提供了指令来做。

所以就写机器代码，然后和C互相调用就行了。

比如在bare metal上

    asm volatile ("... outb $0x3f8");

就能往串口输出。

这时候 (PC, M) => (PC', M') 就不仅是改变状态了，还改变计算机世界之外的东西，这都是硬件帮我们实现的。基本就想象有电，灯泡就能亮/不亮。