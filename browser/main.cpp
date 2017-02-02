/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "browserapplication.h"
#include <QCommandLineParser>
#include <QMessageBox>


enum CommandLineParseResult
{
    CommandLineOk,
    CommandLineError,
    CommandLineVersionRequested,
    CommandLineHelpRequested
};

CommandLineParseResult parseCommandLine(QCommandLineParser &parser, BrowserApplication *app, QString *errorMessage)
{
    const QCommandLineOption profileOption(QStringList() << "p" << "profile",
            app->translate("main", "use the specified profile &lt;profile&gt;."),
            app->translate("main", "profile"));
    parser.addOption(profileOption);
    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

    if (!parser.parse(app->arguments())) {
        *errorMessage = parser.errorText();
        return CommandLineError;
    }

    if (parser.isSet(versionOption))
        return CommandLineVersionRequested;

    if (parser.isSet(helpOption))
        return CommandLineHelpRequested;

    if (parser.isSet(profileOption)) {
        const QString profile = parser.value(profileOption);
        app->setProfile(profile);
    }

    return CommandLineOk;
}




int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(data);
    BrowserApplication application(argc, argv);
    if (!application.isTheOnlyBrowser())
        return 0;
    QCommandLineParser parser;
    //parser.setApplicationDescription("DaimonNetworks gameBrowser");
    QString errorMessage;

    switch (parseCommandLine(parser, &application, &errorMessage)) {
        case CommandLineOk:
            break;
        case CommandLineError:
            QMessageBox::warning(0, application.applicationDisplayName(),
                                 "<html><head/><body><h2>" + errorMessage + "</h2><pre>"
                                 + parser.helpText() + "</pre></body></html>");
            return 1;
        case CommandLineVersionRequested:
            QMessageBox::information(0, application.applicationDisplayName(),
                                     application.applicationDisplayName() + ' '
                                     + application.applicationVersion());
            return 0;
        case CommandLineHelpRequested:
            QMessageBox::warning(0, application.applicationDisplayName(),
                                 "<html><head/><body><pre>"
                                 + parser.helpText() + "</pre></body></html>");
            return 0;
    }

    application.newMainWindow();
    return application.exec();
}

