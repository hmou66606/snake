#ifndef WIDGET_H
#define WIDGET_H

#include <QPoint>
#include <QTimer>
#include <QVector>
#include <QWidget>

class QKeyEvent;
class QPaintEvent;

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    // 定时器超时后前进一步，并处理吃食物与碰撞。
    void advanceGame();

private:
    // 蛇头每次沿一个网格方向移动。
    enum Direction { Up, Down, Left, Right };

    // 初始化或重置一局游戏。
    void resetGame();
    // 在未被蛇占据的格子上随机生成食物。
    void spawnFood();
    // 开始游戏并启动移动定时器。
    void startGame();
    // 绘制游戏区域、蛇、食物和状态提示。
    void drawGame(QPainter &painter);

    // 网格尺寸及棋盘在窗口中的位置，坐标均以格子为单位计算。
    static const int CellSize = 24;
    static const int Columns = 25;
    static const int Rows = 20;
    static const int BoardX = 30;
    static const int BoardY = 112;

    // 蛇身按“蛇头到蛇尾”顺序保存，坐标单位为格子。
    QVector<QPoint> m_snake;
    // 食物的网格坐标。
    QPoint m_food;
    // 当前方向与下一次移动采用的方向。
    Direction m_direction;
    Direction m_nextDirection;
    // 驱动蛇按固定时间间隔移动。
    QTimer m_timer;
    // 当前分数和本局窗口内的最高分。
    int m_score;
    int m_bestScore;
    // 游戏状态标记。
    bool m_started;
    bool m_paused;
    bool m_gameOver;
};

#endif // WIDGET_H
