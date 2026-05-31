#include "Ball.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>

/**
 * @brief 球对象构造函数实现
 */
Ball::Ball(Vector2 pos, Vector2 sp, float r) {
    position = pos;
    speed = sp;
    radius = r;
    gravity = 0.08f;
    maxSpeed = 15.0f;
    bounceForce = 0.5f;
    launched = false;
    launchCooldown = 0.0f;
    
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(nullptr));
        seeded = true;
    }
}

/**
 * @brief 更新球的位置，支持速度缩放
 * 实现带减速因子的移动，默认是1.0，和原来完全一样
 */
void Ball::Move(float speedFactor) {
    if (!launched) return;
    position.x += speed.x * speedFactor;
    position.y += speed.y * speedFactor;
}

/**
 * @brief 绘制球对象
 * 包含渐变效果和运动轨迹提示，未发射时显示按空格提示
 */
void Ball::Draw() {
    DrawCircleGradient((int)position.x, (int)position.y, radius + 3, Fade(ORANGE, 0.3f), RED);
    DrawCircleV(position, radius, RED);
    
    if (launched) {
        // 绘制运动轨迹线，指示球的运动方向
        Vector2 endPos = { position.x + speed.x * 2, position.y + speed.y * 2 };
        DrawLineEx(position, endPos, 2, YELLOW);
    } else {
        // 未发射时，闪烁提示玩家按空格发射
        if ((int)(GetTime() * 2) % 2 == 0) {
            DrawText("PRESS SPACE", (int)position.x - 55, (int)position.y - 30, 16, YELLOW);
        }
    }
}

/**
 * @brief 发射球的实现
 * 从挡板位置随机角度发射，保证球向上运动
 */
void Ball::Launch(float paddleX, float paddleWidth) {
    if (launched) return;
    
    // 随机发射角度，范围60-120度，确保球向上发射
    float angle = (rand() % 61 + 60) * 3.14159f / 180.0f;
    if (rand() % 2 == 0) angle = 3.14159f - angle;
    
    float baseSpeed = 8.0f;
    float speedVariation = (rand() % 5 - 2) * 0.5f;
    float launchSpeed = baseSpeed + speedVariation;
    
    speed.x = launchSpeed * std::cos(angle);
    speed.y = -launchSpeed * std::abs(std::sin(angle));
    
    launched = true;
    position.x = paddleX;
    position.y = 550 - radius - 5;
}

/**
 * @brief 将球重置到挡板位置，等待发射
 */
void Ball::ResetToPaddle(float paddleX, float paddleY) {
    position.x = paddleX;
    position.y = paddleY - radius - 5;
    speed = {0, 0};
    launched = false;
}

/**
 * @brief 重置球的位置和速度
 */
void Ball::Reset(Vector2 pos, Vector2 sp) {
    position = pos;
    speed = sp;
}

/**
 * @brief 给球添加额外的反弹力
 */
void Ball::AddBounceForce(float force) {
    speed.y -= force;
}

/**
 * @brief 应用重力加速度到球上，并限制最大速度
 */
void Ball::ApplyGravity() {
    if (!launched) return;
    speed.y += gravity;
    
    // 限制球的最大速度，防止速度过快导致穿模
    float currentSpeed = std::sqrt(speed.x * speed.x + speed.y * speed.y);
    if (currentSpeed > maxSpeed) {
        speed.x = (speed.x / currentSpeed) * maxSpeed;
        speed.y = (speed.y / currentSpeed) * maxSpeed;
    }
}

/**
 * @brief 处理球与屏幕边界的碰撞反弹
 */
void Ball::BounceEdge(int screenWidth, int screenHeight) {
    if (!launched) return;
    
    // 左边界碰撞
    if (position.x - radius <= 5) {
        position.x = radius + 5;
        speed.x = std::abs(speed.x);
    }
    // 右边界碰撞
    if (position.x + radius >= screenWidth - 5) {
        position.x = screenWidth - radius - 5;
        speed.x = -std::abs(speed.x);
    }
    // 上边界碰撞
    if (position.y - radius <= 5) {
        position.y = radius + 5;
        speed.y = std::abs(speed.y);
        speed.y += bounceForce;
    }
}

/**
 * @brief 处理球与挡板的碰撞反弹
 * 根据撞击挡板的位置计算反弹角度，实现不同的反弹效果
 */
void Ball::BouncePaddle(Rectangle paddleRect) {
    if (!launched) return;
    if (speed.y <= 0) return; // 只处理向下运动的球，避免重复碰撞
    
    if (position.y + radius >= paddleRect.y &&
        position.y + radius <= paddleRect.y + paddleRect.height + std::abs(speed.y) &&
        position.x >= paddleRect.x - radius &&
        position.x <= paddleRect.x + paddleRect.width + radius) {
        
        // 计算撞击点在挡板上的相对位置，范围-1到1
        float hitPoint = (position.x - (paddleRect.x + paddleRect.width / 2.0f)) / (paddleRect.width / 2.0f);
        hitPoint = std::clamp(hitPoint, -1.0f, 1.0f);
        
        float speedMagnitude = std::sqrt(speed.x * speed.x + speed.y * speed.y);
        speedMagnitude = std::max(speedMagnitude + bounceForce * 2, 7.0f);
        
        // 根据撞击位置计算反弹角度，越靠边角度越大
        float angle = 90.0f - hitPoint * 50.0f;
        float angleRad = angle * 3.14159f / 180.0f;
        
        speed.x = speedMagnitude * std::cos(angleRad);
        speed.y = -speedMagnitude * std::abs(std::sin(angleRad));
        position.y = paddleRect.y - radius;
    }
}

/**
 * @brief 检测并处理球与砖块的碰撞
 * 使用最近点法计算圆与矩形碰撞，高效处理轴对齐矩形的碰撞
 */
bool Ball::CheckBrickCollision(Rectangle brickRect) {
    if (!launched) return false;
    
    // 找到矩形上离球最近的点
    float closestX = std::max(brickRect.x, std::min(position.x, brickRect.x + brickRect.width));
    float closestY = std::max(brickRect.y, std::min(position.y, brickRect.y + brickRect.height));
    
    float distX = position.x - closestX;
    float distY = position.y - closestY;
    float distance = std::sqrt(distX * distX + distY * distY);
    
    if (distance < radius) {
        // 计算碰撞方向，确定是水平还是垂直碰撞
        float distLeft = position.x - brickRect.x;
        float distRight = brickRect.x + brickRect.width - position.x;
        float distTop = position.y - brickRect.y;
        float distBottom = brickRect.y + brickRect.height - position.y;
        
        float minDistX = std::min(distLeft, distRight);
        float minDistY = std::min(distTop, distBottom);
        
        if (minDistX < minDistY) {
            // 水平方向碰撞，反转X方向速度
            speed.x *= -1;
            position.x = (distLeft < distRight) ? brickRect.x - radius : brickRect.x + brickRect.width + radius;
        } else {
            // 垂直方向碰撞，反转Y方向速度
            speed.y *= -1;
            if (distTop > distBottom) speed.y -= bounceForce;
            position.y = (distTop < distBottom) ? brickRect.y - radius : brickRect.y + brickRect.height + radius;
        }
        return true;
    }
    return false;
}