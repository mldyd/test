#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <cctype>

// 添加Windows头文件以支持控制台编码设置
#ifdef _WIN32
#include <windows.h>
#endif

// 速度计算相关常量定义
const float POSITION_ERROR_SAFETY = 1.0f;
const float RADIUS_MIN = 9.0f;
const float RADIUS_SAFETY = 15.0f;
const float END_DISTANCE_SAFETY = 2.0f;
const float V_MAX = 2.7f;
const float END_DISTANCE_DETECT = 0.15f;
const float positive_acceleration = 1.5f;
const float negative_acceleration = -1.5f;

// 定义带行驶方向的路径点结构体
struct PathPointWithDrive {
    double x;
    double y;
    double theta;
    std::string drive;  // "正向" 或 "倒车"
};

// 定义带时间戳的完整路径点结构体
struct TimedPathPointWithDrive {
    double x;
    double y;
    double theta;
    double t;
    std::string drive;
};

// 安全的字符串转数字函数
double safe_stod(const std::string& str, double default_value = 0.0) {
    try {
        // 去除首尾空格
        size_t start = str.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return default_value;
        
        size_t end = str.find_last_not_of(" \t\n\r");
        std::string trimmed = str.substr(start, end - start + 1);
        
        // 检查是否为空或只包含空格
        if (trimmed.empty()) return default_value;
        
        // 检查是否包含非数字字符（除了数字、小数点、负号和科学计数法）
        bool has_digit = false;
        for (char c : trimmed) {
            if (std::isdigit(c)) {
                has_digit = true;
                break;
            }
        }
        if (!has_digit) return default_value;
        
        return std::stod(trimmed);
    } catch (const std::invalid_argument& e) {
        std::cerr << "警告: 无法解析数字 '" << str << "', 使用默认值 " << default_value << std::endl;
        return default_value;
    } catch (const std::out_of_range& e) {
        std::cerr << "警告: 数字超出范围 '" << str << "', 使用默认值 " << default_value << std::endl;
        return default_value;
    }
}

// 计算两点之间的距离
double calculateDistance(const PathPointWithDrive& p1, const PathPointWithDrive& p2) {
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    return std::sqrt(dx * dx + dy * dy);
}

// 计算曲率半径
double calculateRadius(const PathPointWithDrive& p0, const PathPointWithDrive& p1, const PathPointWithDrive& p2) {
    // 计算三角形的边长
    double a = calculateDistance(p1, p2);
    double b = calculateDistance(p0, p2);
    double c = calculateDistance(p0, p1);

    // 检查共线情况
    if (a < 1e-6 || b < 1e-6 || c < 1e-6) {
        return 1e6;
    }

    // 计算三角形的面积
    double s = (a + b + c) / 2.0;
    double area_squared = s * (s - a) * (s - b) * (s - c);
    
    if (area_squared < 1e-12) {
        return 1e6;
    }

    double area = std::sqrt(area_squared);

    // 计算曲率半径: R = (a*b*c)/(4*area)
    double radius = (a * b * c) / (4 * area);
    return radius;
}

// 从文件中读取路径数据（新的格式）
std::vector<std::vector<PathPointWithDrive>> readPathFromFileNew(const std::string& filename) {
    std::vector<std::vector<PathPointWithDrive>> allPaths;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return allPaths;
    }

    std::string line;
    std::vector<PathPointWithDrive> currentPath;
    bool inSmoothedPath = false;
    int pathCount = 0;

    while (std::getline(file, line)) {
        // 查找Smoothed Path开始标记
        if (line.find("Smoothed Path (x, y, heading_deg, drive):") != std::string::npos) {
            inSmoothedPath = true;
            currentPath.clear();
            continue;
        }

        // 查找路径完成标记
        if (line.find("路径规划完成") != std::string::npos && inSmoothedPath) {
            if (!currentPath.empty()) {
                allPaths.push_back(currentPath);
                pathCount++;
            }
            inSmoothedPath = false;
            continue;
        }

        // 解析路径点数据
        if (inSmoothedPath && !line.empty()) {
            // 跳过空行和注释
            if (line.find_first_not_of(" \t") == std::string::npos) continue;
            
            // 查找数字开始的行
            size_t num_start = line.find_first_of("0123456789.-");
            if (num_start == std::string::npos) continue;
            
            std::string data_line = line.substr(num_start);
            
            // 解析逗号分隔的数据
            std::istringstream iss(data_line);
            std::string token;
            std::vector<std::string> tokens;
            
            while (std::getline(iss, token, ',')) {
                // 去除token首尾空格
                token.erase(0, token.find_first_not_of(" \t"));
                token.erase(token.find_last_not_of(" \t") + 1);
                if (!token.empty()) {
                    tokens.push_back(token);
                }
            }

            if (tokens.size() >= 4) {
                PathPointWithDrive point;
                point.x = safe_stod(tokens[0], 0.0);
                point.y = safe_stod(tokens[1], 0.0);
                point.theta = safe_stod(tokens[2], 0.0);
                point.drive = tokens[3];
                
                // 验证数据有效性
                if (point.drive == "正向" || point.drive == "倒车") {
                    currentPath.push_back(point);
                } else {
                    // 尝试清理drive字段
                    std::string clean_drive = tokens[3];
                    clean_drive.erase(0, clean_drive.find_first_not_of(" \t\""));
                    clean_drive.erase(clean_drive.find_last_not_of(" \t\"") + 1);
                    if (clean_drive == "正向" || clean_drive == "倒车") {
                        point.drive = clean_drive;
                        currentPath.push_back(point);
                    }
                }
            }
        }
    }

    file.close();
    
    if (allPaths.empty() && !currentPath.empty()) {
        allPaths.push_back(currentPath);
    }
    
    return allPaths;
}

// 处理路径段（考虑加速度的物理速度计算）
std::vector<TimedPathPointWithDrive> processPathSegment(
    const std::vector<PathPointWithDrive>& segment, 
    double startTime,
    double startTheta) {
    
    std::vector<TimedPathPointWithDrive> timedSegment;
    if (segment.empty()) return timedSegment;

    // 如果只有一个点，直接返回
    if (segment.size() == 1) {
        TimedPathPointWithDrive point;
        point.x = segment[0].x;
        point.y = segment[0].y;
        point.theta = startTheta;
        point.t = startTime;
        point.drive = segment[0].drive;
        timedSegment.push_back(point);
        return timedSegment;
    }

    // 计算每个点的目标速度（基于曲率和安全距离）
    std::vector<double> targetSpeeds(segment.size());
    for (size_t i = 0; i < segment.size(); ++i) {
        double radius = 1e6;
        
        // 计算曲率半径
        if (i >= 2 && i < segment.size() - 1) {
            radius = calculateRadius(segment[i-2], segment[i-1], segment[i]);
        } else if (i == 1 && segment.size() >= 3) {
            radius = calculateRadius(segment[0], segment[1], segment[2]);
        } else if (i == segment.size() - 1 && segment.size() >= 3) {
            radius = calculateRadius(segment[i-2], segment[i-1], segment[i]);
        }

        // 计算到终点的距离
        double endDistance = (calculateDistance(segment[i-1], segment.back())+calculateDistance(segment[i-1], segment.back()))/2;
        
        // 基于曲率和安全距离计算目标速度
        float radius_factor = 0.0f;
        if (radius > RADIUS_MIN && RADIUS_SAFETY > RADIUS_MIN) {
            radius_factor = (radius - RADIUS_MIN) / (RADIUS_SAFETY - RADIUS_MIN);
            radius_factor = std::max(0.0f, std::min(1.0f, radius_factor));
        }

        float end_distance_factor = 0.0f;
        if (END_DISTANCE_SAFETY > 0) {
            end_distance_factor = (endDistance - END_DISTANCE_DETECT) / END_DISTANCE_SAFETY;
            end_distance_factor = std::max(0.0f, std::min(1.0f, end_distance_factor));
        }

        float targetSpeed = V_MAX * (POSITION_ERROR_SAFETY + radius_factor) * 0.5f * end_distance_factor;
        targetSpeed = std::max(targetSpeed, 0.1f); // 最小速度
        targetSpeed = std::min(targetSpeed, V_MAX); // 最大速度限制
        
        targetSpeeds[i] = targetSpeed;
    }

    // 前向传递：考虑加速度限制
    std::vector<double> forwardSpeeds(segment.size());
    forwardSpeeds[0] = 0.0; // 起点速度为0
    
    for (size_t i = 1; i < segment.size(); ++i) {
        double distance = calculateDistance(segment[i-1], segment[i]);
        double maxSpeedIncrease = std::sqrt(2 * positive_acceleration * distance);
        forwardSpeeds[i] = std::min(targetSpeeds[i], forwardSpeeds[i-1] + maxSpeedIncrease);
    }

    // 后向传递：考虑减速度限制
    std::vector<double> actualSpeeds(segment.size());
    actualSpeeds[segment.size() - 1] = 0.0; // 终点速度为0
    
    for (int i = segment.size() - 2; i >= 0; --i) {
        double distance = calculateDistance(segment[i], segment[i+1]);
        double maxSpeedDecrease = std::sqrt(2 * std::abs(negative_acceleration) * distance);
        actualSpeeds[i] = std::min(forwardSpeeds[i], actualSpeeds[i+1] + maxSpeedDecrease);
    }

    // 计算时间
    // 计算时间
    timedSegment.reserve(segment.size());
    double currentTime = startTime;
    
    // 添加第一个点
    TimedPathPointWithDrive firstPoint;
    firstPoint.x = segment[0].x;
    firstPoint.y = segment[0].y;
    firstPoint.theta = startTheta;  // 使用传入的startTheta
    firstPoint.t = currentTime;     // 第一个点的时间就是startTime
    firstPoint.drive = segment[0].drive;
    timedSegment.push_back(firstPoint);
    
    // 计算后续点的时间和位置（从第二个点开始）
    for (size_t i = 1; i < segment.size(); ++i) {
        double distance = calculateDistance(segment[i-1], segment[i]);
        double avgSpeed = (actualSpeeds[i-1] + actualSpeeds[i]) / 2.0;
        
        if (avgSpeed < 1e-6) {
            avgSpeed = 1e-6; // 防止除零错误
        }
        
        double timeIncrement = distance / avgSpeed;
        currentTime += timeIncrement;
    
        TimedPathPointWithDrive timedPoint;
        timedPoint.x = segment[i].x;
        timedPoint.y = segment[i].y;
        timedPoint.theta = segment[i].theta;
        timedPoint.t = currentTime;
        timedPoint.drive = segment[i].drive;
        timedSegment.push_back(timedPoint);
    }

    return timedSegment;
}

// 主处理函数 - 修改版，实现每组数据第一行使用第二行朝向
std::vector<TimedPathPointWithDrive> processPathWithDirectionChanges(
    const std::vector<PathPointWithDrive>& path) {
    
    std::vector<TimedPathPointWithDrive> result;
    if (path.empty()) return result;

    // 创建修改后的路径副本
    std::vector<PathPointWithDrive> modifiedPath = path;
    
    // 如果路径至少有2个点，将第一行的朝向改为第二行的朝向
    if (modifiedPath.size() >= 2) {
        modifiedPath[0].drive = modifiedPath[1].drive;
    }

    // 按行驶方向分段
    std::vector<std::vector<PathPointWithDrive>> segments;
    std::vector<PathPointWithDrive> currentSegment;
    
    currentSegment.push_back(modifiedPath[0]);
    
    for (size_t i = 1; i < modifiedPath.size(); ++i) {
        if (modifiedPath[i].drive != modifiedPath[i-1].drive) {
            // 方向改变，结束当前段
            if (!currentSegment.empty()) {
                segments.push_back(currentSegment);
                currentSegment.clear();
            }
        }
        currentSegment.push_back(modifiedPath[i]);
    }
    
    if (!currentSegment.empty()) {
        segments.push_back(currentSegment);
    }

    // 处理每个段
    double currentTime = 0.0;
    
    for (size_t segIdx = 0; segIdx < segments.size(); ++segIdx) {
        const auto& segment = segments[segIdx];
        
        // 获取下一个段的起始theta（如果存在）
        double nextTheta = (segIdx < segments.size() - 1) ? 
                          segments[segIdx + 1][0].theta : segment.back().theta;
        
        // 处理当前段
        auto timedSegment = processPathSegment(segment, currentTime, nextTheta);
        
        // 合并结果
        result.insert(result.end(), timedSegment.begin(), timedSegment.end());
        
        // 更新时间
        if (!timedSegment.empty()) {
            currentTime = timedSegment.back().t;
        }
    }

    return result;
}

int path_timing_main(std::string myString) {
    // 设置Windows控制台编码为UTF-8
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    #endif

    std::cout << "路径时间计算程序启动..." << std::endl;
    // 从终端获取输入
    //std::string myString = "point copy 2";
    // 从文件中读取路径数据
    std::string filename = myString + "/point.txt";
    //std::string filename = "point.txt";
    auto allPaths = readPathFromFileNew(filename);

    if (allPaths.empty()) {
        std::cerr << "错误：未读取到任何路径数据！" << std::endl;
        std::cerr << "请检查文件格式是否正确，确保包含 'Smoothed Path' 和 '路径规划完成' 标记" << std::endl;
        return 1;
    }

    std::cout << "成功读取到 " << allPaths.size() << " 组路径数据" << std::endl;

    // 创建输出文件
    std::ofstream outfile(myString+"/timed_path_result.txt");
    if (!outfile.is_open()) {
        std::cerr << "错误：无法创建输出文件: timed_path_result.txt" << std::endl;
        return 1;
    }

    // 处理每组路径
    for (size_t i = 0; i < allPaths.size(); ++i) {
        double currentStartTime = 0.0;
        std::cout << "\n处理第 " << (i + 1) << " 组路径:" << std::endl;
        std::cout << "请输入第 " << (i + 1) << " 组路径的初始时间 (秒): ";
        
        while (!(std::cin >> currentStartTime)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "请输入有效的数字: ";
        }
        std::cin.ignore();

        // 处理路径
        auto timedPath = processPathWithDirectionChanges(allPaths[i]);
        
        // 调整时间偏移
        for (auto& point : timedPath) {
            point.t += currentStartTime;
        }

        // 输出结果
        std::cout << "处理完成，共 " << timedPath.size() << " 个点" << std::endl;

        // 写入到文件中（五元组格式）
        outfile << "[" << std::endl;
        for (size_t j = 0; j < timedPath.size(); ++j) {
            outfile << "  [" << timedPath[j].x << ", " << timedPath[j].y << ", " 
                   << timedPath[j].theta << ", " << timedPath[j].t << ", \"" 
                   << timedPath[j].drive << "\"]";
            if (j < timedPath.size() - 1) {
                outfile << ",";
            }
            outfile << std::endl;
        }
        outfile << "]" << std::endl;
        if (i < allPaths.size() - 1) {
            outfile << std::endl;
        }
    }

    outfile.close();
    std::cout << "\n结果已成功写入到 timed_path_result.txt 文件中" << std::endl;

    return 0;
}
