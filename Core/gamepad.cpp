#include "gamepad.h"
#include "settings.h"

Gamepad *Gamepad::instance = new Gamepad;
bool Gamepad::isEnabled = false;

Gamepad::Gamepad() = default;

Gamepad *Gamepad::initialize()
{
    if (!instance)
    {
        instance = new Gamepad;
    }

    instance->isEnabled = Settings::value("Gamepad/enabled").toBool();
    instance->init();

    qDebug() << "Gamepad handler initialized";
    return instance;
}

void Gamepad::init()
{
    connect(QGamepadManager::instance(), &QGamepadManager::gamepadConnected, this, [=](int deviceId)
    {
        qInfo("Gamepad connected: (%i)", deviceId);
        m_gamepads.insert(deviceId, new QGamepad(deviceId, this));
    });
    connect(QGamepadManager::instance(), &QGamepadManager::gamepadDisconnected, this, [=](int deviceId)
    {
        qInfo("Gamepad disconnected: (%i)", deviceId);
        m_gamepads.remove(deviceId);
    });
    connect(QGamepadManager::instance(), &QGamepadManager::gamepadButtonPressEvent, this, Gamepad::button);
    connect(QGamepadManager::instance(), &QGamepadManager::gamepadButtonReleaseEvent, this, &Gamepad::release);
}

void Gamepad::closeGame()
{
    if (!instance) return;
    if (l1 && l2 && r1 && r2 && select)
    {
        qDebug() << "Terminating cemu process";
        emit instance->gameClose(true);
    }
}

void Gamepad::button(int deviceId, QGamepadManager::GamepadButton button, double value)
{
    if (!instance) return;
    auto manager = QGamepadManager::instance();
    qDebug() << "Gamepad(" << deviceId << ") button:" << button << "=" << value;

    if (button == manager->ButtonUp)
    {
        emit instance->gameUp(static_cast<bool>(value) && isEnabled);
    }
    if (button == manager->ButtonDown)
    {
        emit instance->gameDown(static_cast<bool>(value) && isEnabled);
    }
    if (button == manager->ButtonA)
    {
        emit instance->gameStart(static_cast<bool>(value) && isEnabled);
    }
    if (button == manager->ButtonL1)
    {
        if ((instance->l1 = static_cast<bool>(value)))
        {
            emit instance->prevTab(static_cast<bool>(value) && isEnabled);
        }
    }
    if (button == manager->ButtonL2)
    {
        instance->l2 = static_cast<bool>(value);
    }
    if (button == manager->ButtonR1)
    {
        if ((instance->r1 = static_cast<bool>(value)))
        {
            emit instance->nextTab(static_cast<bool>(value) && isEnabled);
        }
    }
    if (button == manager->ButtonR2)
    {
        instance->r2 = static_cast<bool>(value);
    }
    if (button == manager->ButtonSelect)
    {
        instance->select = static_cast<bool>(value);
        instance->closeGame();
    }
}

void Gamepad::release(int deviceId, QGamepadManager::GamepadButton button)
{
    return Gamepad::button(deviceId, button, 0);
}

void Gamepad::enable()
{
    qInfo() << "Gamepad mode" << (isEnabled = true);
}

void Gamepad::disable()
{
    qInfo() << "Gamepad mode" << (isEnabled = false);
}

void Gamepad::terminate()
{
    isEnabled = false;
    if (instance)
    {
        delete instance;
    }
}
