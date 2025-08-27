      

# -*- coding: utf-8 -*-
"""
车辆运行轨迹模拟与可视化程序 - 修复版本
读取output.txt文件中的三组车辆运行数据，将车辆建模为长方形进行可视化
修复了车辆B和C不运动的问题
"""

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.animation import FuncAnimation
import matplotlib.font_manager as fm
import os

# 设置中文字体
def setup_chinese_font():
    """设置matplotlib中文字体"""
    try:
        # 尝试使用系统自带的中文字体
        plt.rcParams['font.sans-serif'] = ['SimHei', 'Microsoft YaHei', 'SimSun', 'KaiTi']
        plt.rcParams['axes.unicode_minus'] = False  # 解决负号显示问题
    except:
        # 如果系统字体不可用，使用默认字体
        print("警告：中文字体设置可能存在问题，将使用默认字体")

# 在程序开始时调用字体设置
# 设置中文字体
setup_chinese_font()

# 添加生成颜色的函数
def generate_colors(n):
    """为n个车辆生成不同的颜色"""
    colors = ['#FF6B6B', '#4ECDC4', '#45B7D1', '#FFD166', '#06D6A0', '#118AB2', '#073B4C', '#8338EC', '#FF006E', '#FB5607']
    # 如果需要更多颜色，循环使用这些颜色或使用matplotlib的颜色循环
    if n <= len(colors):
        return colors[:n]
    else:
        # 重复使用颜色列表
        return (colors * (n // len(colors) + 1))[:n]

def read_vehicle_data(filepath):
    """
    从文件中读取车辆运行数据
    
    参数:
        filepath: 数据文件路径
        
    返回:
        vehicles_data: 包含三组车辆数据的列表，每组数据是一个字典包含x, y, time
    """
    with open(filepath, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    # 跳过第一行
    data_lines = lines[1:]
    
    # 分割三组数据（通过空行）
    vehicles_data = []
    current_vehicle = []
    
    for line in data_lines:
        line = line.strip()
        if line:
            # 解析数据行
            parts = line.split()
            if len(parts) >= 3:
                x = float(parts[0])
                y = float(parts[1])
                time = float(parts[2])
                current_vehicle.append((x, y, time))
        elif current_vehicle:
            # 遇到空行且当前有数据，保存一组车辆数据
            if current_vehicle:
                vehicle_dict = {
                    'x': [p[0] for p in current_vehicle],
                    'y': [p[1] for p in current_vehicle],
                    'time': [p[2] for p in current_vehicle]
                }
                vehicles_data.append(vehicle_dict)
                current_vehicle = []
    
    # 处理最后一组数据
    if current_vehicle:
        vehicle_dict = {
            'x': [p[0] for p in current_vehicle],
            'y': [p[1] for p in current_vehicle],
            'time': [p[2] for p in current_vehicle]
        }
        vehicles_data.append(vehicle_dict)
    
    return vehicles_data

class Vehicle:
    """车辆类，用于模拟长方形车辆的运动"""
    
    def __init__(self, x, y, angle=0, length=8.5, width=3.5, color='blue', label=''):
        """
        初始化车辆
        
        参数:
            x, y: 车辆中心坐标
            angle: 车辆朝向角度（弧度）
            length: 车辆长度
            width: 车辆宽度
            color: 车辆颜色
            label: 车辆标签
        """
        self.x = x
        self.y = y
        self.angle = angle
        self.length = length
        self.width = width
        self.color = color
        self.label = label
        
    def update_position(self, x, y, angle):
        """更新车辆位置和朝向"""
        self.x = x
        self.y = y
        self.angle = angle
        
    def get_patch(self):
        """获取车辆的matplotlib矩形补丁"""
        rect = patches.Rectangle(
            (self.x - self.length/2, self.y - self.width/2),
            self.length, self.width,
            angle=np.degrees(self.angle),
            rotation_point='center',
            facecolor=self.color,
            alpha=0.7,
            edgecolor='black'
        )
        return rect

def calculate_heading(x_coords, y_coords):
    """计算车辆朝向角度"""
    angles = []
    for i in range(len(x_coords)):
        if i < len(x_coords) - 1:
            dx = x_coords[i+1] - x_coords[i]
            dy = y_coords[i+1] - y_coords[i]
            angle = np.arctan2(dy, dx)
        else:
            angle = angles[-1] if angles else 0
        angles.append(angle)
    return angles

def create_animation(vehicles_data):
    """创建车辆运行动画 - 修复版本"""
    
    # 创建图形
    fig, ax = plt.subplots(figsize=(12, 8))
    
    # 设置坐标轴范围
    all_x = [x for vehicle in vehicles_data for x in vehicle['x']]
    all_y = [y for vehicle in vehicles_data for y in vehicle['y']]
    
    margin = 10
    ax.set_xlim(min(all_x) - margin, max(all_x) + margin)
    ax.set_ylim(min(all_y) - margin, max(all_y) + margin)
    
    # 设置标题和标签（使用中文字体）
    ax.set_title('车辆运行轨迹模拟', fontsize=16, fontweight='bold')
    ax.set_xlabel('X坐标', fontsize=12)
    ax.set_ylabel('Y坐标', fontsize=12)
    ax.grid(True, alpha=0.3)
    
    # 修改：根据车辆数量动态生成颜色和名称
    num_vehicles = len(vehicles_data)
    colors = generate_colors(num_vehicles)
    vehicle_names = [f'车辆{i+1}' for i in range(num_vehicles)]
    
    trajectories = []
    vehicles = []
    vehicle_patches = []  # 存储车辆矩形对象
    
    for i, (vehicle_data, color, name) in enumerate(zip(vehicles_data, colors, vehicle_names)):
        # 绘制轨迹线
        line, = ax.plot(vehicle_data['x'], vehicle_data['y'], 
                       color=color, linewidth=2, alpha=0.7, label=name)
        trajectories.append(line)
        
        # 创建车辆对象
        vehicle = Vehicle(vehicle_data['x'][0], vehicle_data['y'][0], 
                         color=color, length=8.5, width=3.5, label=name)
        vehicles.append(vehicle)
        
        # 添加车辆矩形并保存引用
        rect = vehicle.get_patch()
        vehicle_patches.append(ax.add_patch(rect))
    
    # 添加图例
    ax.legend(loc='upper right', fontsize=10)
    
    # 获取最大时间点
    max_time = max([max(v['time']) for v in vehicles_data])
    
    # 创建时间文本
    time_text = ax.text(0.02, 0.95, '', transform=ax.transAxes, 
                       fontsize=12, fontweight='bold',
                       bbox=dict(boxstyle='round', facecolor='white', alpha=0.8))
    
    def animate(frame):
        """动画更新函数 - 修复版本"""
        current_time = frame * 0.5  # 每帧0.5秒
        
        for i, (vehicle_data, vehicle, rect) in enumerate(zip(vehicles_data, vehicles, vehicle_patches)):
            # 找到当前时间点对应的索引
            times = vehicle_data['time']
            
            # 线性插值计算当前位置
            if current_time <= times[0]:
                # 在起始时间之前，保持在起始位置
                x = vehicle_data['x'][0]
                y = vehicle_data['y'][0]
                angle = calculate_heading(vehicle_data['x'], vehicle_data['y'])[0]
            elif current_time >= times[-1]:
                # 在结束时间之后，保持在结束位置
                x = vehicle_data['x'][-1]
                y = vehicle_data['y'][-1]
                angle = calculate_heading(vehicle_data['x'], vehicle_data['y'])[-1]
            else:
                # 在轨迹时间内，进行插值
                idx = 0
                while idx < len(times) - 1 and times[idx+1] < current_time:
                    idx += 1
                
                if idx < len(times) - 1:
                    t1, t2 = times[idx], times[idx+1]
                    alpha = (current_time - t1) / (t2 - t1) if t2 != t1 else 0
                    
                    x = vehicle_data['x'][idx] + alpha * (vehicle_data['x'][idx+1] - vehicle_data['x'][idx])
                    y = vehicle_data['y'][idx] + alpha * (vehicle_data['y'][idx+1] - vehicle_data['y'][idx])
                    
                    # 计算朝向
                    dx = vehicle_data['x'][idx+1] - vehicle_data['x'][idx]
                    dy = vehicle_data['y'][idx+1] - vehicle_data['y'][idx]
                    angle = np.arctan2(dy, dx)
                else:
                    x = vehicle_data['x'][-1]
                    y = vehicle_data['y'][-1]
                    angle = calculate_heading(vehicle_data['x'], vehicle_data['y'])[-1]
            
            # 更新车辆位置
            vehicle.update_position(x, y, angle)
            
            # 更新矩形补丁 - 使用引用直接修改
            rect.set_xy((vehicle.x - vehicle.length/2, vehicle.y - vehicle.width/2))
            rect.angle = np.degrees(vehicle.angle)
        
        # 更新时间显示
        time_text.set_text(f'时间: {current_time:.1f}秒')
        
        return vehicle_patches + [time_text] + trajectories
    
    # 创建动画
    frames = int(max_time * 2) + 1  # 每0.5秒一帧
    anim = FuncAnimation(fig, animate, frames=frames, 
                        interval=100, blit=True, repeat=True)
    
    return fig, anim

# 修改create_static_plot函数以支持任意数量的车辆数据
def create_static_plot(vehicles_data):
    """创建静态轨迹图"""
    
    fig, ax = plt.subplots(figsize=(14, 10))
    
    # 设置坐标轴范围
    all_x = [x for vehicle in vehicles_data for x in vehicle['x']]
    all_y = [y for vehicle in vehicles_data for y in vehicle['y']]
    
    margin = 15
    ax.set_xlim(min(all_x) - margin, max(all_x) + margin)
    ax.set_ylim(min(all_y) - margin, max(all_y) + margin)
    
    # 设置标题和标签（使用中文字体）
    ax.set_title('车辆运行轨迹图', fontsize=16, fontweight='bold', pad=20)
    ax.set_xlabel('X坐标 (米)', fontsize=12)
    ax.set_ylabel('Y坐标 (米)', fontsize=12)
    ax.grid(True, alpha=0.3, linestyle='--')
    
    # 修改：根据车辆数量动态生成颜色和名称
    num_vehicles = len(vehicles_data)
    colors = generate_colors(num_vehicles)
    vehicle_names = [f'车辆{i+1}' for i in range(num_vehicles)]
    
    for i, (vehicle_data, color, name) in enumerate(zip(vehicles_data, colors, vehicle_names)):
        x = vehicle_data['x']
        y = vehicle_data['y']
        time = vehicle_data['time']
        
        # 绘制轨迹线
        ax.plot(x, y, color=color, linewidth=3, alpha=0.8, label=name, marker='o', markersize=4)
        
        # 标记起点和终点
        ax.scatter(x[0], y[0], color=color, s=200, marker='s', 
                  edgecolors='white', linewidth=2, label=f'{name}起点')
        ax.scatter(x[-1], y[-1], color=color, s=200, marker='^', 
                  edgecolors='white', linewidth=2, label=f'{name}终点')
        
        # 添加时间标签
        for j in range(0, len(x), 3):
            ax.annotate(f'{time[j]:.0f}s',
                       (x[j], y[j]),
                       xytext=(5, 5), textcoords='offset points',
                       fontsize=9, color='darkslategray',
                       bbox=dict(boxstyle='round,pad=0.3', 
                                facecolor='white', alpha=0.7))
    
    # 添加图例
    ax.legend(loc='upper right', fontsize=10)
    
    # 添加比例尺
    ax.plot([min(all_x)-10, min(all_x)-10+10], 
            [min(all_y)-10, min(all_y)-10], 
            'k-', linewidth=3)
    ax.text(min(all_x)-10+5, min(all_y)-10-3, '10米', 
            ha='center', va='top', fontsize=10)
    
    plt.tight_layout()
    return fig

# 修改main函数以支持任意数量的车辆数据
def main():
    """主函数"""
    # 文件路径
    folder_name = input("请输入文件夹名称: ")
    # folder_name = 'point copy 5'
    script_dir = os.path.dirname(os.path.abspath(__file__))
    filepath = rf'{script_dir}\{folder_name}\output.txt'
    output_dir = rf'{script_dir}\{folder_name}'
    # 检查文件是否存在
    if not os.path.exists(filepath):
        print(f"文件不存在: {filepath}")
        return
    
    # 读取数据
    vehicles_data = read_vehicle_data(filepath)
    
    # 修改：添加对空数据的检查，保留对3组数据的警告但允许处理任意数量的数据
    if len(vehicles_data) < 1:
        print("错误: 未读取到任何车辆数据")
        return
    elif len(vehicles_data) != 3:
        print(f"警告: 期望3组数据，实际读取到{len(vehicles_data)}组")
    
    # 打印数据信息
    print("=== 车辆数据信息 ===")
    for i, vehicle in enumerate(vehicles_data):
        print(f"车辆{i+1}: {len(vehicle['x'])} 个轨迹点")
        print(f"  时间范围: {min(vehicle['time']):.1f}s - {max(vehicle['time']):.1f}s")
        print(f"  坐标范围: X({min(vehicle['x']):.1f}, {max(vehicle['x']):.1f}) Y({min(vehicle['y']):.1f}, {max(vehicle['y']):.1f})")
    
    # 创建静态可视化
    print("\n正在创建静态轨迹图...")
    
    static_fig = create_static_plot(vehicles_data)
    static_fig.savefig(os.path.join(output_dir, f'vehicle_trajectories_fixed.png'), 
                      dpi=300, bbox_inches='tight')
    print("静态轨迹图已保存: vehicle_trajectories_fixed.png")
    
    # 创建动画
    print("\n正在创建车辆运行动画...")
    try:
        animation_fig, anim = create_animation(vehicles_data)
        animation_fig.savefig(os.path.join(output_dir, f'vehicle_animation_start_fixed.png'), 
                             dpi=300, bbox_inches='tight')
        
        # 保存动画为GIF
        anim.save(os.path.join(output_dir, f'vehicle_animation_all_working.gif'), 
                 writer='pillow', fps=10)
        print("车辆运行动画已保存: vehicle_animation_all_working.gif")
        
        # 显示动画
        plt.show()
        
    except Exception as e:
        print(f"动画创建失败: {e}")
        print("请确保已安装pillow库: pip install pillow")
    
    print("\n=== 可视化完成 ===")

if __name__ == "__main__":
    main()

    