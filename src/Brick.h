#ifndef BRICK_H
#define BRICK_H
#include "raylib.h"

/**
 * @brief 游戏中的砖块对象类
 * 负责管理砖块的位置、激活状态和绘制
 */
class Brick {
private:
    Rectangle rect; ///< 砖块的矩形区域
    bool active;    ///< 砖块是否激活（是否存在，未被击碎）
    Color color;    ///< 砖块的颜色
public:
    /**
     * @brief 构造函数，创建一个砖块
     * @param x 初始X坐标
     * @param y 初始Y坐标
     * @param w 宽度
     * @param h 高度
     * @param c 颜色
     */
    Brick(float x, float y, float w, float h, Color c);
    
    /**
     * @brief 绘制砖块，如果砖块未激活则不绘制
     */
    void Draw();
    
    Rectangle GetRect() { return rect; } ///< 获取砖块的矩形区域
    bool IsActive() { return active; }    ///< 检查砖块是否激活
    void SetActive(bool a) { active = a; } ///< 设置砖块的激活状态
    Color GetColor() { return color; }    ///< 获取砖块的颜色
    void SetColor(Color c) { color = c; } ///< 设置砖块的颜色
};
#endif