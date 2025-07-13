#pragma once

#include <QAbstractNativeEventFilter>
#include <QObject>
#include <windows.h>

class QWidget;
namespace QGBA {
class InputController;
}

class RawKeyboardFilterWin final : public QObject, public QAbstractNativeEventFilter {
public:
	explicit RawKeyboardFilterWin(QWidget* target, QGBA::InputController* parent);
	bool nativeEventFilter(const QByteArray& eventType, void* message, long* result) override;

private:
	QGBA::InputController* m_parent;
	void registerDevice();
	int qtKeyFromVirtualKey(quint16 vkey) const;

    QWidget* m_targetWidget;
	bool m_registered = false;
};
