/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Copyright (C) 2015-2017 Gerasim Troeglazov,
** Contact: 3dEyes@gmail.com
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qhaikuplatforminputcontext.h"

#include "qhaikuwindow.h"

#include <QtCore/QLocale>
#include <QtCore/QRectF>
#include <QtGui/QInputMethodQueryEvent>
#include <QtWidgets/QApplication>

#include <Looper.h>
#include <Messenger.h>

#include <InputServerTypes.h>
#include <input_globals.h>


QHaikuPlatformInputContext::QHaikuPlatformInputContext()
{
    m_looper = new BLooper();
}


QHaikuPlatformInputContext::~QHaikuPlatformInputContext()
{
    if (m_looper) {
        if (m_looper->Lock()) {
            m_looper->Quit();
        }
    }
}


bool QHaikuPlatformInputContext::isValid() const
{
    return true;
}


bool QHaikuPlatformInputContext::hasCapability(Capability capability) const
{
    switch (capability) {
        case HiddenTextCapability:
            return false;
        default:
            break;
    }
    return true;
}


void QHaikuPlatformInputContext::commit()
{
    // commit preedit when focus changed to other widget
    // and send stop message to the input method
    QWindow *window = QGuiApplication::focusWindow();
    if (window && window->handle()) {
        QHaikuWindow *platformWindow =
                dynamic_cast<QHaikuWindow *>(window->handle());
        if (platformWindow && platformWindow->m_window) {
            platformWindow->m_window->PostMessage(kInputMethodCommit);
        }
    }
}


void QHaikuPlatformInputContext::setFocusObject(QObject *object)
{
    if (object) {
        QInputMethodQueryEvent query(Qt::ImEnabled);
        QGuiApplication::sendEvent(object, &query);
        if (query.value(Qt::ImEnabled).toBool()) {
            BMessenger messenger(m_looper);
            BMessage message((object != NULL) ? IS_FOCUS_IM_AWARE_VIEW : IS_UNFOCUS_IM_AWARE_VIEW);
            message.AddMessenger("view", messenger);
            BMessage reply;
            _control_input_server_(&message, &reply);
        }
    }
}


bool QHaikuPlatformInputContext::filterEvent(const QEvent *)
{
    // QKeyEvent is processed in the input method passed by the input server.
    // Therefore, we do not need anything here.
    return false;
}
