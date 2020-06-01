# TypeGettingLanguage
《编译原理》小练习

语法：

![](CodeCogsEqn.png)

示例程序：
```cpp
{
	int x;
	char y;
	{
		y;
		bool y;
		x;
		y;
	}
	y;
}
```
输出结果：
```cpp
{
	{
		y: char;
		x: int;
		y: bool;
	}
	y: char;
}
```
