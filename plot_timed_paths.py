import matplotlib.pyplot as plt
import re
import numpy as np
import matplotlib
import os
matplotlib.rcParams['font.sans-serif'] = ['SimHei']  # 设置中文字体
matplotlib.rcParams['axes.unicode_minus'] = False  # 解决负号显示问题


def read_timed_paths_from_file(file_path):
    """
    读取带时间信息的路径数据（支持新的五元组格式）
    参数:
        file_path: 文件路径
    返回:
        paths: 路径列表，每个元素是一个包含(x, y, theta, t, drive)的元组列表
    """
    paths = []
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 匹配完整的路径数组
        # 新格式: [55.7447, 63.7705, 306, 0, "倒车"]
        point_pattern = r'\[\s*([-+]?\d*\.?\d+)\s*,\s*([-+]?\d*\.?\d+)\s*,\s*([-+]?\d*\.?\d+)\s*,\s*([-+]?\d*\.?\d+)\s*,\s*"([^"]+)"\s*\]'
        
        # 分割不同的路径
        path_blocks = re.split(r'\]\s*\n\s*\[', content)
        
        for block in path_blocks:
            block = '[' + block.strip()  # 补全方括号
            point_matches = re.findall(point_pattern, block)
            
            if point_matches:
                current_path = []
                for match in point_matches:
                    x, y, theta, t, drive = match
                    current_path.append((
                        float(x), 
                        float(y), 
                        float(theta), 
                        float(t), 
                        str(drive)
                    ))
                paths.append(current_path)
    
    except Exception as e:
        print(f"读取文件时出错: {e}")
        
    return paths


def plot_timed_paths(paths,save_path, colors=None):
    """
    绘制带时间和行驶模式标注的路径（无箭头版本）
    参数:
        paths: 路径列表，每个路径是[(x, y, theta, t, drive), ...]
        colors: 颜色列表
    """
    if not paths:
        print("没有路径数据可以绘制")
        return
    
    if colors is None:
        colors = ['#FF6B6B', '#4ECDC4', '#45B7D1', '#96CEB4', 
                 '#FECA57', '#FF9FF3', '#54A0FF', '#5F27CD']
    
    plt.figure(figsize=(14, 10))
    
    for i, path in enumerate(paths):
        color = colors[i % len(colors)]
        
        # 提取数据
        x_coords = [p[0] for p in path]
        y_coords = [p[1] for p in path]
        times = [p[3] for p in path]
        drive_modes = [p[4] for p in path]
        
        # 绘制路径线
        plt.plot(x_coords, y_coords, color=color, linewidth=2.5, 
                marker='o', markersize=4, 
                label=f'路径{i+1} ({drive_modes[0]})')
        
        # 添加时间标注（简洁版本）
        label_step = max(1, len(path) // 6)
        for j in range(0, len(path), label_step):
            plt.annotate(f'{times[j]:.1f}s', 
                        (x_coords[j], y_coords[j]), 
                        xytext=(5, 5), 
                        textcoords='offset points',
                        fontsize=9,
                        color=color,
                        weight='bold')
    
    plt.title('车辆路径轨迹', fontsize=16)
    plt.xlabel('X坐标', fontsize=12)
    plt.ylabel('Y坐标', fontsize=12)
    plt.legend(fontsize=10)
    plt.grid(True, alpha=0.3)
    plt.axis('equal')
    
    # 自动调整坐标轴范围
    if paths:
        all_x = [p[0] for path in paths for p in path]
        all_y = [p[1] for path in paths for p in path]
        margin = max(max(all_x) - min(all_x), max(all_y) - min(all_y)) * 0.1
        plt.xlim(min(all_x) - margin, max(all_x) + margin)
        plt.ylim(min(all_y) - margin, max(all_y) + margin)
    
    plt.tight_layout()
    plt.savefig(save_path, dpi=300, bbox_inches='tight')
    print(f"图片已保存至 {save_path}")
    plt.show()
    
    # plt.show()  



def main():
    """主函数"""
    # 从终端获取输入
    # folder_name = input("请输入文件夹名称: ")
    folder_name = 'point copy 5'
    script_dir = os.path.dirname(os.path.abspath(__file__))
    file_path = rf'{script_dir}\{folder_name}\timed_path_result.txt'
    # 获取保存图片的路径，将 .txt 替换为 .png
    save_path = os.path.splitext(file_path)[0] + '.png'
    # 读取数据
    paths = read_timed_paths_from_file(file_path)
    
    if not paths:
        print("没有找到有效的路径数据")
        return
    
    print(f"成功读取 {len(paths)} 条路径")
    for i, path in enumerate(paths):
        drive_modes = list(set([p[4] for p in path]))
        print(f"  路径{i+1}: {len(path)}个点，模式: {drive_modes}")
    
    # 绘制图形
    plot_timed_paths(paths,save_path)        
    

    


if __name__ == '__main__':
    main()