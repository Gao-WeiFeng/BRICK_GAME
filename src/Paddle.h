#ifndef PADDLE_H
#define PADDLE_H
#include "raylib.h"

/**
 * @brief 游戏中的挡板对象类
 * 负责管理挡板的位置、移动，以及道具效果（如加长）的处理
 */
class Paddle {
private:
    Rectangle rect;           ///< 挡板的矩形区域
    float screenWidth;         ///< 屏幕宽度，用于边界检测
    float originalWidth;       ///< 挡板的原始宽度，用于道具效果结束后恢复
    float extendEffectTime;    ///< 加长道具效果的剩余时间
public:
    /**
     * @brief 构造函数，创建一个挡板
     * @param x 初始X坐标
     * @param y 初始Y坐标
     * @param width 初始宽度
     * @param height 高度
     */
    Paddle(float x, float y, float width, float height);
    
    /**
     * @brief 向左移动挡板
     * @param speed 移动速度
     */
    void MoveLeft(float speed);
    
    /**
     * @brief 向右移动挡板
     * @param speed 移动速度
     */
    void MoveRight(float speed);
    
    /**
     * @brief 绘制挡板，有道具效果时会改变颜色提示玩家
     */
    void Draw();
    
    /**
     * @brief 获取挡板的矩形区域
     * @return 挡板的矩形引用
     */
    Rectangle& GetRect() { return rect; }
    
    /**
     * @brief 获取挡板的矩形区域（const版本）
     * @return 挡板的矩形const引用
     */
    const Rectangle& GetRect() const { return rect; }
    
    /**
     * @brief 加长挡板的宽度，应用加长道具效果
     * @param extraWidth 额外增加的宽度
     * @param duration 效果持续时间
     */
    void Extend(float extraWidth, float duration);
    
    /**
     * @brief 更新道具效果的剩余时间，到期自动恢复挡板宽度
     * @param dt 帧间隔时间
     */
    void Update(float dt);
    
    float GetOriginalWidth() { return originalWidth; } ///< 获取挡板的原始宽度
};
#endif