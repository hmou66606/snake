#include "widget.h"

#include <QKeyEvent>
#include <QPainter>
#include <QRandomGenerator>
#include <QSet>

Widget::Widget(QWidget *parent)
    : QWidget(parent),
      m_direction(Right),
      m_nextDirection(Right),
      m_score(0),
      m_bestScore(0),
      m_started(false),
      m_paused(false),
      m_gameOver(false)
{
    // 初始化窗口外观和键盘焦点，使窗口打开后能直接接收控制按键。
    setWindowTitle(QStringLiteral("贪吃蛇"));
    setFixedSize(Columns * CellSize + BoardX * 2, BoardY + Rows * CellSize + 30);
    setFocusPolicy(Qt::StrongFocus);
    setStyleSheet(QStringLiteral("background-color: #101923;"));

    // 定时器每次超时都会推动游戏逻辑前进一步。
    connect(&m_timer, &QTimer::timeout, this, &Widget::advanceGame);
    m_timer.setInterval(140);
    resetGame();
}

void Widget::resetGame()
{
    // 蛇在棋盘中央向右排列，初始长度为三格。
    m_snake.clear();
    m_snake.append(QPoint(12, 10));
    m_snake.append(QPoint(11, 10));
    m_snake.append(QPoint(10, 10));
    m_direction = Right;
    m_nextDirection = Right;
    m_score = 0;
    m_started = false;
    m_paused = false;
    m_gameOver = false;
    spawnFood();
    m_timer.stop();
    update();
}

void Widget::startGame()
{
    // 游戏结束后按开始键会直接开启新的一局。
    if (m_gameOver)
        resetGame();
    m_started = true;
    m_paused = false;
    m_timer.start();
    setFocus();
    update();
}

void Widget::spawnFood()
{
    // 随机选择格子，若与蛇身重合则重新选择。
    do {
        m_food = QPoint(QRandomGenerator::global()->bounded(Columns),
                        QRandomGenerator::global()->bounded(Rows));
    } while (m_snake.contains(m_food));
}

void Widget::advanceGame()
{
    // 把已接受的转向应用到本次移动，并计算新的蛇头格子。
    m_direction = m_nextDirection;
    QPoint next = m_snake.first();
    switch (m_direction) {
    case Up:    next.ry()--; break;
    case Down:  next.ry()++; break;
    case Left:  next.rx()--; break;
    case Right: next.rx()++; break;
    }

    // 先判断是否吃到食物以及是否越过棋盘边界。
    const bool eating = next == m_food;
    const bool outside = next.x() < 0 || next.x() >= Columns ||
                         next.y() < 0 || next.y() >= Rows;
    // 若本步不吃食物，蛇尾会同时移走，因此尾格允许蛇头进入。
    const int bodyLength = m_snake.size() - (eating ? 0 : 1);
    bool hitBody = false;
    for (int i = 0; i < bodyLength; ++i) {
        if (m_snake.at(i) == next) {
            hitBody = true;
            break;
        }
    }

    // 撞墙或撞到仍会保留的身体时结束游戏。
    if (outside || hitBody) {
        m_gameOver = true;
        m_timer.stop();
        update();
        return;
    }

    // 将新蛇头放到队首；吃到食物时保留原蛇尾以增长一格。
    m_snake.prepend(next);
    if (eating) {
        m_score += 10;
        m_bestScore = qMax(m_bestScore, m_score);
        spawnFood();
        // 分数提升后逐渐加快移动速度，但设置最小间隔。
        if (m_timer.interval() > 65)
            m_timer.setInterval(m_timer.interval() - 3);
    } else {
        m_snake.removeLast();
    }
    update();
}

void Widget::keyPressEvent(QKeyEvent *event)
{
    // 同时支持方向键和 WASD；忽略直接反向的转向，避免撞入自身。
    switch (event->key()) {
    case Qt::Key_Up: case Qt::Key_W:
        if (m_direction != Down) m_nextDirection = Up;
        break;
    case Qt::Key_Down: case Qt::Key_S:
        if (m_direction != Up) m_nextDirection = Down;
        break;
    case Qt::Key_Left: case Qt::Key_A:
        if (m_direction != Right) m_nextDirection = Left;
        break;
    case Qt::Key_Right: case Qt::Key_D:
        if (m_direction != Left) m_nextDirection = Right;
        break;
    case Qt::Key_Space:
        // 空格开始、暂停或继续游戏。
        if (m_gameOver || !m_started) {
            startGame();
        } else if (m_paused) {
            m_paused = false;
            m_timer.start();
            update();
        } else {
            m_paused = true;
            m_timer.stop();
            update();
        }
        break;
    case Qt::Key_R:
        // R 键随时重置本局。
        resetGame();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void Widget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    // 每次状态变化后重绘整个窗口。
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    drawGame(painter);
}

void Widget::drawGame(QPainter &painter)
{
    // 绘制背景、标题、分数和操作说明。
    painter.fillRect(rect(), QColor("#101923"));
    painter.setPen(QColor("#e8f0f5"));
    QFont titleFont = painter.font();
    titleFont.setPixelSize(27);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(30, 43, QStringLiteral("贪吃蛇"));

    QFont infoFont = painter.font();
    infoFont.setPixelSize(15);
    infoFont.setBold(false);
    painter.setFont(infoFont);
    painter.setPen(QColor("#9fb0bd"));
    painter.drawText(30, 78, QStringLiteral("得分  %1").arg(m_score));
    painter.drawText(150, 78, QStringLiteral("最高  %1").arg(m_bestScore));
    painter.drawText(365, 78, QStringLiteral("方向键 / WASD  移动    空格  暂停    R  重开"));

    // 建立棋盘区域，并把后续网格内容限制在棋盘边框内。
    const QRect board(BoardX, BoardY, Columns * CellSize, Rows * CellSize);
    painter.setPen(QPen(QColor("#2a3a48"), 1));
    painter.setBrush(QColor("#182630"));
    painter.drawRoundedRect(board, 10, 10);
    painter.save();
    painter.setClipRect(board.adjusted(1, 1, -1, -1));

    painter.setPen(QPen(QColor(255, 255, 255, 12), 1));
    for (int x = 1; x < Columns; ++x)
        painter.drawLine(BoardX + x * CellSize, BoardY, BoardX + x * CellSize, BoardY + Rows * CellSize);
    for (int y = 1; y < Rows; ++y)
        painter.drawLine(BoardX, BoardY + y * CellSize, BoardX + Columns * CellSize, BoardY + y * CellSize);

    // 食物在格子中心绘制成带高光的小圆点。
    const QRect foodRect(BoardX + m_food.x() * CellSize + 5,
                         BoardY + m_food.y() * CellSize + 5,
                         CellSize - 10, CellSize - 10);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#ff6b6b"));
    painter.drawEllipse(foodRect);
    painter.setBrush(QColor("#ffd0c9"));
    painter.drawEllipse(foodRect.adjusted(4, 3, -7, -8));

    // 从蛇尾向蛇头绘制，保证蛇头显示在最上层。
    for (int i = m_snake.size() - 1; i >= 0; --i) {
        const QPoint cell = m_snake.at(i);
        const QRect body(BoardX + cell.x() * CellSize + 2,
                         BoardY + cell.y() * CellSize + 2,
                         CellSize - 4, CellSize - 4);
        painter.setBrush(i == 0 ? QColor("#63e6a5") : QColor("#35c982"));
        painter.drawRoundedRect(body, 7, 7);
        // 蛇头颜色更亮，并根据移动方向调整眼睛位置。
        if (i == 0) {
            painter.setBrush(QColor("#123b32"));
            if (m_direction == Left || m_direction == Right) {
                const int eyeX = m_direction == Right ? body.right() - 7 : body.left() + 4;
                painter.drawEllipse(QPoint(eyeX, body.top() + 6), 2, 2);
                painter.drawEllipse(QPoint(eyeX, body.bottom() - 6), 2, 2);
            } else {
                const int eyeY = m_direction == Down ? body.bottom() - 7 : body.top() + 4;
                painter.drawEllipse(QPoint(body.left() + 6, eyeY), 2, 2);
                painter.drawEllipse(QPoint(body.right() - 6, eyeY), 2, 2);
            }
        }
    }
    painter.restore();

    // 等待开始、暂停或游戏结束时，在棋盘上显示对应提示。
    if (!m_started || m_paused || m_gameOver) {
        painter.fillRect(board, QColor(5, 10, 15, 175));
        painter.setPen(Qt::white);
        QFont overlayFont = painter.font();
        overlayFont.setPixelSize(26);
        overlayFont.setBold(true);
        painter.setFont(overlayFont);
        QString headline;
        QString hint;
        if (m_gameOver) {
            headline = QStringLiteral("游戏结束");
            hint = QStringLiteral("得分：%1    按空格再来一次，或按 R 重开").arg(m_score);
        } else if (m_paused) {
            headline = QStringLiteral("已暂停");
            hint = QStringLiteral("按空格继续");
        } else {
            headline = QStringLiteral("准备好了吗？");
            hint = QStringLiteral("按空格开始，用方向键或 WASD 控制");
        }
        painter.drawText(board, Qt::AlignCenter, headline + QLatin1Char('\n') + hint);
    }
}
