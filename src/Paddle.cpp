#include "Paddle.h"

/**
 * @brief 挡板构造函数实现
 */
Paddle::Paddle(float x, float y, float width, float height) {
    rect = { x, y, width, height };
    screenWidth = 800;
    originalWidth = width; // 初始化原始宽度
    extendEffectTime = 0.0f; // 初始没有效果
}

/**
 * @brief 向左移动挡板的实现
 * @param speed 移动速度
 */
void Paddle::MoveLeft(float speed) {
    rect.x -= speed;
    if (rect.x < 5) rect.x = 5; // 边界检测，防止移出屏幕左边界
}

/**
 * @brief 向右移动挡板的实现
 * @param speed 移动速度
 */
void Paddle::MoveRight(float speed) {
    rect.x += speed;
    if (rect.x + rect.width > screenWidth - 5) rect.x = screenWidth - rect.width - 5; // 边界检测，防止移出屏幕右边界
}

/**
 * @brief 更新道具效果的剩余时间
 * 当效果到期时，自动恢复挡板的原始宽度
 */
void Paddle::Update(float dt) {
    if (extendEffectTime > 0) {
        extendEffectTime -= dt;
        // 效果到期，恢复原始宽度
        if (extendEffectTime <= 0) {
            rect.width = originalWidth;
            extendEffectTime = 0.0f;
        }
    }
}

/**
 * @brief 应用加长挡板的道具效果
 * 重置效果时间，避免重复拾取道具时无限变长
 * @param extraWidth 额外增加的宽度
 * @param duration 效果持续时间
 */
void Paddle::Extend(float extraWidth, float duration) {
    rect.width = originalWidth + extraWidth;
    extendEffectTime = duration;
}

/**
 * @brief 绘制挡板的实现
 * 有加长效果时，挡板会变成黄色提示玩家当前有道具效果
 */
void Paddle::Draw() {
    Color color = BLUE;
    // 有加长效果的时候，板变成黄色，提示玩家
    if (extendEffectTime > 0) {
        color = YELLOW;
    }
    DrawRectangleRec(rect, color);
    DrawRectangleLinesEx(rect, 2, SKYBLUE);
}