#include "RawKeyboardFilter_win.h"
#include "InputController.h"
#include <QtCore/qglobal.h>
#include <QApplication>
#include <QWidget>
#include <QDebug>
#include <QTimer>


RawKeyboardFilterWin::RawKeyboardFilterWin(QWidget* target, QGBA::InputController* parent)
    : m_parent(parent), m_targetWidget(target) {
	QTimer::singleShot(0, this, &RawKeyboardFilterWin::registerDevice);
}

bool RawKeyboardFilterWin::nativeEventFilter(const QByteArray& type, void* message, long*) {
	static bool firstHit = true;
	if (firstHit) {
		qDebug() << "WM_INPUT filter active – raw background keyboard enabled";
		firstHit = false;
	}


	if (type != "windows_generic_MSG")
		return false;


	MSG* msg = static_cast<MSG*>(message);
	if (msg->message != WM_INPUT)
		return false;

	UINT size = 0;
	GetRawInputData(reinterpret_cast<HRAWINPUT>(msg->lParam), RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));

	QByteArray buf(size, 0);
	RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(buf.data());
	if (GetRawInputData(reinterpret_cast<HRAWINPUT>(msg->lParam), RID_INPUT, raw, &size, sizeof(RAWINPUTHEADER)) !=
	    size)
		return false;

	if (raw->header.dwType == RIM_TYPEKEYBOARD) {
		const RAWKEYBOARD& k = raw->data.keyboard;
		bool pressed = !(k.Flags & RI_KEY_BREAK);
		int qtKey = qtKeyFromVirtualKey(k.VKey);
		if (qtKey != Qt::Key_unknown)
			m_parent->enqueueKey(qtKey, pressed);
	}
	return false;
}

void RawKeyboardFilterWin::registerDevice() {
	if (m_registered)
		return; 
	if (!m_targetWidget || !m_targetWidget->winId())
		return;

	RAWINPUTDEVICE rid {};
	rid.usUsagePage = 0x01; // generic desktop controls
	rid.usUsage = 0x06; // keyboard
	rid.dwFlags = RIDEV_INPUTSINK;
	rid.hwndTarget = reinterpret_cast<HWND>(m_targetWidget->winId());

	if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
		qWarning() << "RegisterRawInputDevices failed" << GetLastError();
	else
		m_registered = true;
}


int RawKeyboardFilterWin::qtKeyFromVirtualKey(quint16 vk) const {
	switch (vk) {
	case VK_UP:
		return Qt::Key_Up;
	case VK_DOWN:
		return Qt::Key_Down;
	case VK_LEFT:
		return Qt::Key_Left;
	case VK_RIGHT:
		return Qt::Key_Right;
	case 'Z':
		return Qt::Key_Z;
	case 'X':
		return Qt::Key_X;
	case 'A':
		return Qt::Key_A;
	case 'S':
		return Qt::Key_S;
	case VK_RETURN:
		return Qt::Key_Return;
	case VK_BACK:
		return Qt::Key_Backspace;
	default:
		return Qt::Key_unknown;
	}
}
