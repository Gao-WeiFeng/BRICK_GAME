#ifndef BALL_H
#define BALL_H
#include "raylib.h"

/**
 * @brief 游戏中的球对象类
 * 负责管理球的位置、速度、运动逻辑，以及与边界、挡板、砖块的碰撞处理
 */
class Ball {
private:
    Vector2 position;       ///< 球的当前位置
    Vector2 speed;          ///< 球的当前速度
    float radius;           ///< 球的半径
    float gravity;          ///< 重力加速度
    float maxSpeed;         ///< 球的最大速度限制
    float bounceForce;      ///< 碰撞时的额外反弹力
    bool launched;          ///< 球是否已经被发射（是否处于运动状态）
    float launchCooldown;   ///< 发射冷却时间
    
public:
    /**
     * @brief 构造函数，创建一个球对象
     * @param pos 初始位置
     * @param sp 初始速度
     * @param r 球的半径
     */
    Ball(Vector2 pos, Vector2 sp, float r);
    
    /**
     * @brief 更新球的位置
     * @param speedFactor 速度缩放因子，用于实现减速道具效果，默认1.0为正常速度
     */
    void Move(float speedFactor = 1.0f);
    
    /**
     * @brief 绘制球，包括渐变效果和运动轨迹
     */
    void Draw();
    
    /**
     * @brief 对球应用重力加速度
     */
    void ApplyGravity();
    
    /**
     * @brief 处理球与屏幕边界的碰撞反弹
     * @param screenWidth 屏幕宽度
     * @param screenHeight 屏幕高度
     */
    void BounceEdge(int screenWidth, int screenHeight);
    
    /**
     * @brief 处理球与挡板的碰撞反弹，根据撞击位置调整反弹角度
     * @param paddleRect 挡板的矩形区域
     */
    void BouncePaddle(Rectangle paddleRect);
    
    /**
     * @brief 检测球与砖块的碰撞，并处理碰撞后的速度反弹
     * 使用最近点法计算圆与矩形碰撞，比分离轴定理更高效，适用于轴对齐矩形
     * @param brickRect 砖块的矩形区域
     * @return true 发生了碰撞，false 未碰撞
     */
    bool CheckBrickCollision(Rectangle brickRect);
    
    /**
     * @brief 发射球，从挡板位置随机角度发射
     * @param paddleX 挡板的X坐标
     * @param paddleWidth 挡板的宽度
     */
    void Launch(float paddleX, float paddleWidth);
    
    /**
     * @brief 将球重置到挡板位置，等待发射
     * @param paddleX 挡板的X坐标
     * @param paddleY 挡板的Y坐标
     */
    void ResetToPaddle(float paddleX, float paddleY);
    
    /**
     * @brief 重置球的位置和速度
     * @param pos 新的位置
     * @param sp 新的速度
     */
    void Reset(Vector2 pos, Vector2 sp);
    
    /**
     * @brief 给球添加额外的反弹力
     * @param force 要添加的力的大小
     */
    void AddBounceForce(float force);
    
    /// @brief 获取球的当前位置
    Vector2 GetPosition() { return position; }
    /// @brief 获取球的半径
    float GetRadius() { return radius; }
    /// @brief 获取球的当前速度
    Vector2 GetSpeed() { return speed; }
    /// @brief 设置球的速度
    void SetSpeed(Vector2 sp) { speed = sp; }
    /// @brief 检查球是否已经被发射（是否处于运动状态）
    bool IsLaunched() { return launched; }
};
#endif