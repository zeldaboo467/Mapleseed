#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <QObject>
#include <QtConcurrent>
#include <QtGamepad>
#include <QGamepadManager>

class QGamepad;

class Gamepad : public QObject
{
    Q_OBJECT
public:
    explicit Gamepad(QObject *parent = nullptr);
    ~Gamepad();

    static Gamepad *initialize();

    void init();
    void closeGame();

    static void button(int deviceId, QGamepadManager::GamepadButton button, double value);
    static void release(int deviceId, QGamepadManager::GamepadButton button);
    static void enable();
    static void disable();
    static void terminate();

    static Gamepad *instance;
    static bool isEnabled;

signals:
    void gameStart(bool pressed);
    void gameClose(bool pressed);
    void gameUp(bool pressed);
    void gameDown(bool pressed);
    void nextTab(bool pressed);
    void prevTab(bool pressed);

public slots:

private:
    QMap<int, QGamepad*> m_gamepads;
    bool l1 = false;
    bool l2 = false;
    bool r1 = false;
    bool r2 = false;
    bool select = false;
};

#endif // GAMEPAD_H
