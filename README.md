# TypeGettingLanguage
《编译原理》小练习

语法：

$$ code \rightarrow block $$
$$ block \rightarrow \{ stmts \} $$
$$ stmts \rightarrow stmt \ stmts \ | \ \epsilon $$
$$ stmt \rightarrow \textbf{id;} \ | \ \textbf{type id;} \ | \ block \ | \ \epsilon$$

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
