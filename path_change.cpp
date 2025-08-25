#include <iostream>
#include <string>
#include "path_timing.h"
#include "path_data_change.h"
#include "path_time_change.h"
// 声明外部函数

int main() {
    std::string myString;
    std::cout << "请输入文件夹名称: ";
    std::getline(std::cin, myString);
    std::cout << "开始执行 path_timing_main..." << std::endl;
    path_timing_main(myString);
    
    std::cout << "\n开始执行 path_data_change_main..." << std::endl;
    path_data_change_main(myString);
    
    std::cout << "\n开始执行 path_time_change_main..." << std::endl;
    path_time_change_main(myString);
    
    std::cout << "\n所有程序执行完成。" << std::endl;
    return 0;
}