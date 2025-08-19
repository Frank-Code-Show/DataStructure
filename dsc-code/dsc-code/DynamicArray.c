#include <stdio.h>
#include <stdlib.h>
#include "DynamicArray.h"

#define INITIAL_CAPACITY 10

// 内部辅助函数：当容量不足时候，进行扩容
static int resize_array(DynamicArray* arr, size_t new_capacity) {

	// realloc arr->data 老地皮的地址
	Data* new_data = realloc(arr->data, new_capacity * sizeof(Data));
	// 1. 最佳情况，原地扩容：地址没变，原先的老地址旁边正好有连续不断的空白地址。
	// 2. 普遍情况，搬家扩容：地址变化，原先的数据全部搬迁到新的地址，并且，清空原先旧地址的数据

	if (!new_data) {
		// realloc失败，内存不足!
		return -1;
		// 我这里一返回，意味着，原先的那个arr->data 还在！
	}

	// realloc成功之后，才会用新的地址去更新结构体
	arr->data = new_data;
	arr->capacity = new_capacity;

	return 0;
}

// 创建并初始化一个动态数组
DynamicArray* create_array(size_t initial_capcity) {

	if (initial_capcity == 0)
	{
		initial_capcity == INITIAL_CAPACITY;
	}
	DynamicArray* arr = (DynamicArray*)malloc(sizeof(DynamicArray));

	/*
	typedef struct {
	Data* data;		// 指向存储数据的连续内存块; 指向一个数组的首地址
	// int* data;	// 说白了，我们可以放很多int类型的数据，因为指向的是一个连续不断的地址空间

	size_t size;

	size_t capacity;
	} DynamicArray;

	*/
	if (!arr) return NULL;

	arr->data = (Data*)malloc(initial_capcity * sizeof(Data));
	// arr作为结构体变量，它有三个成员，这三个成员中，最重要的就是Data* data;
	// data指向一个新的连续不断的内存空间
	// 而整个空间，现在被我们指向的是一个capcity = 10的数组

	if (!arr->data) {
		free(arr);
		return NULL;
	}

	arr->size = 0;
	arr->capacity = initial_capcity;

	return arr;
}

// 销毁数组，释放内存
void destroy_array(DynamicArray* arr) {
	if (arr) {
		free(arr->data);
		free(arr);
	}
}

// 在数组末尾追加元素 Amortized O(1)
void array_append(DynamicArray* arr, Data value) {
	// 检查是否要扩容
	if (arr->size >= arr->capacity) {
		size_t new_capacity = arr->capacity * 2;
		resize_array(arr, new_capacity);
	}

	arr->data[arr->size] = value;
	arr->size++;
}

// 读取指定的索引元素
// 返回一个指针，以便能够检查是否成功，如果索引无效，返回NULL
Data* array_read(DynamicArray* arr, size_t index) {
	if (index >= arr->size) {
		return NULL;
	}

	// address = base_address + index * sizeof(element);
	return &(arr->data[index]);
}

// 更新指定索引的元素
// 返回0表示成功，返回-1表示失败
int array_update(DynamicArray* arr, size_t index, Data value) {
	if (index >= arr->size) {
		return -1;
	}

	arr->data[index] = value;
	return 0;
}


int array_insert(DynamicArray* arr, size_t index, Data value) {
	if (index > arr->size) {
		return -1;
		// 索引越界，允许在末尾插入, index == size
	}

	// 检查是否要扩容
	if (arr->size >= arr->capacity) {

		// 函数合约 The Function Contract
		if (resize_array(arr, arr->capacity * 2) != 0) {
			return -1;
		}

	}

	for (size_t i = arr->size; i > index; --i) {
		arr->data[i] = arr->data[i - 1];
	}

	/*

	arr->data = [10, 20, 30, 40]
	arr->data = [10, 20, 30, 40, □]


	arr->data = [10, 99, 20, 30, 40, □]
	arr->size = 4;
	index = 1 目标位置
	capacity 8

	1.
		i arr->size i = 4;
		i > index 4>1 t
		arr->data[4] = arr->data[3];
		把索引3的值(40) 复制到 索引4上（空白）
			arr->data = [10, 20, 30, □, 40]

	
	*/


	arr->data[index] = value;

	arr->size++;
	return 0;

}

// 删除指定索引的元素
int array_delete(DynamicArray* arr, size_t index) {
	if (index >= arr->size) {
		return -1;
	}


	for (size_t i = index; i < arr->size - 1; ++i) {
		arr->data[i] = arr->data[i + 1];
	}

	/*

	arr->data = [10, 20, 30, 40, 50]

	arr->data = [10, , 30, 40, □]

	
	*/

	arr->size--;

	// size capacity 1/4,

	if (arr->size > 0 & arr->size <= arr->capacity / 4 && arr->capacity > INITIAL_CAPACITY) {
		size_t new_capacity = arr->capacity / 2;

		// 保证缩容后的容量仍然能够装得下所有元素，并且不会小于初始容量
		if (new_capacity < arr->size) {
			new_capacity = arr->size;
		}

		if (new_capacity < INITIAL_CAPACITY) {
			new_capacity = INITIAL_CAPACITY;
		}

		printf("\n---> [缩容警告!] Size (%zu) <= Capacity/4 (%zu). 准备缩容至 %zu. \n",
			arr->size, arr->capacity / 4, new_capacity);

		resize_array(arr, new_capacity);
	}

	return 0;
}

// print_array第二个参数，要求传递一个指向函数的指针
// 这个传递的函数必须满足条件是参数必须是const void*
void print_array(const DynamicArray* arr, void (*print_func)(const void* data)){
	if (!print_func) {
		printf("错误：未提供有效的打印函数!\n");
		return;
	}
	
	printf("Array (Size: %zu, Capacity: %zu): [\n", arr->size, arr->capacity);

	for (size_t i = 0; i < arr->size; i++)
	{
		printf("	");
		
		// 关键是，调用外部传入的函数指针，来打印每一个元素
		// 我们需要传递每个元素的地址 &arr->data[i]
		// 因为print_func接收一个void*指针

		print_func(&arr->data[i]);
		printf("\n");
	}

	printf("]\n");
}