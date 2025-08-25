#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

struct PathPoint {
    double x, y, angle, time;
    std::string direction;
};

// 解析数据块
std::vector<std::vector<PathPoint>> parse_block(const std::string& block) {
    std::vector<std::vector<PathPoint>> result;
    std::istringstream block_stream(block);
    std::string line;
    
    std::vector<PathPoint> current_block;
    while (std::getline(block_stream, line)) {
        // 查找数据行（包含坐标点的行）
        if (line.find('[') != std::string::npos && line.find(']') != std::string::npos) {
            // 简化的解析逻辑，实际项目中可能需要更复杂的解析
            // 移除方括号
            size_t start = line.find('[');
            size_t end = line.find(']');
            if (start != std::string::npos && end != std::string::npos && end > start) {
                std::string data_part = line.substr(start + 1, end - start - 1);
                
                // 解析数据
                std::istringstream data_stream(data_part);
                std::string item;
                std::vector<std::string> items;
                
                // 按逗号分割
                while (std::getline(data_stream, item, ',')) {
                    // 去除空格
                    item.erase(0, item.find_first_not_of(" \t"));
                    item.erase(item.find_last_not_of(" \t") + 1);
                    // 去除引号
                    if (item.front() == '"' && item.back() == '"') {
                        item = item.substr(1, item.length() - 2);
                    } else if (item.front() == '\'' && item.back() == '\'') {
                        item = item.substr(1, item.length() - 2);
                    }
                    items.push_back(item);
                }
                
                // 确保有足够的数据
                if (items.size() >= 5) {
                    PathPoint point;
                    point.x = std::stod(items[0]);
                    point.y = std::stod(items[1]);
                    point.angle = std::stod(items[2]);
                    point.time = std::stod(items[3]);
                    point.direction = items[4];
                    current_block.push_back(point);
                }
            }
        } else if (line.empty() || line.find("[") == std::string::npos) {
            // 如果遇到空行或新块的开始，保存当前块
            if (!current_block.empty()) {
                result.push_back(current_block);
                current_block.clear();
            }
        }
    }
    
    // 添加最后一个块
    if (!current_block.empty()) {
        result.push_back(current_block);
    }
    
    return result;
}

int path_data_change_main(std::string id) {
    std::string filepath = id + "\\timed_path_result.txt";
    std::string filepath_output = id + "\\data.txt";
    
    std::ifstream input_file(filepath);
    if (!input_file.is_open()) {
        std::cerr << "无法打开输入文件: " << filepath << std::endl;
        return 1;
    }
    
    std::ostringstream buffer;
    buffer << input_file.rdbuf();
    std::string input_data = buffer.str();
    input_file.close();
    
    // 按双换行符分割数据块
    std::vector<std::string> blocks;
    std::istringstream data_stream(input_data);
    std::string block;
    std::string current_block;
    
    while (std::getline(data_stream, block)) {
        if (block.empty()) {
            if (!current_block.empty()) {
                blocks.push_back(current_block);
                current_block.clear();
            }
        } else {
            if (!current_block.empty()) {
                current_block += "\n" + block;
            } else {
                current_block = block;
            }
        }
    }
    
    // 添加最后一个块
    if (!current_block.empty()) {
        blocks.push_back(current_block);
    }
    
    // 添加：打印数据块数量
    std::cout << "数据块数量: " << blocks.size() << std::endl;
    
    std::ofstream output_file(filepath_output);
    if (!output_file.is_open()) {
        std::cerr << "无法打开输出文件: " << filepath_output << std::endl;
        return 1;
    }
    
    output_file << blocks.size() << "\n";
    
    // 处理每个数据块
    for (const auto& b : blocks) {
        auto data_blocks = parse_block(b);
        
        // 对于这种格式，每个blocks元素应该只包含一个数据块
        for (const auto& block_data : data_blocks) {
            // 添加：打印每个块的行数
            std::cout << "当前块的行数: " << block_data.size() << std::endl;
            output_file << block_data.size() << "\n";
            
            for (const auto& point : block_data) {
                output_file << point.x << " " << point.y << " " << point.angle << " " << point.time  << "\n";
                // output_file << point.x << " " << point.y << " " << point.angle << " " << point.time << " " << point.direction << "\n";
            }
        }
    }
    
    output_file.close();
    std::cout << "处理完成，结果已写入: " << filepath_output << std::endl;
    
    return 0;
}