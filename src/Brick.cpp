#include "Brick.h"

/**
 * @brief 砖块构造函数实现
 */
Brick::Brick(float x, float y, float w, float h, Color c) {
    rect = {x, y, w, h};
    active = true;
    color = c;
}

/**
 * @brief 绘制砖块的实现
 * 激活状态的砖块会绘制填充矩形和半透明白色边框，未激活则不绘制
 */
void Brick::Draw() {
    if (!active) return;
    DrawRectangleRec(rect, color);
    DrawRectangleLinesEx(rect, 2, Fade(WHITE, 0.5f));
}