#include "bktournamentmodule.h"

#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QJsonDocument>
#include <QJsonObject>

#include <QDebug>




bkTournamentModule::bkTournamentModule(QJsonObject& defaults, bkAccount* account, QObject *parent) :
    bkModule("Tournament", defaults, account, parent)
{
    addPath("/joust/");

    addStateName(no_login, tr("nicht angemeldet"));
    addStateName(do_login, tr("going login"));
    addStateName(is_login, tr("logged in"));
    addStateName(prepare, tr("erste Runde startet"));
    addStateName(round, tr("wait for round"));
    addStateName(round+1, tr("wait for round"));
    addStateName(round+2, tr("wait for round"));
    addStateName(round+3, tr("wait for round"));
}

void bkTournamentModule::joustHtml(QWebPage* page, QVariant progressbarEndTime, QVariant progressbarDuration)
{
    QWebElement button = page->mainFrame()->findFirstElement("a#btnApply");
    if(!button.isNull()) {
        //Anzahl der Ritter auslesen?
        if(button.hasClass("disabledBtn")) {
            setState("state", is_login);
            //qDebug() << "Zum Turnier angemeldet. Warten auf Start.";
        } else {
            setState("state", no_login);
            setState("timerProgress", 0);
            //qDebug() << "nicht angemeldet!";
        }
        return;
    }

    if(!progressbarEndTime.isValid() || !progressbarDuration.isValid()) return;

    setState("timerProgress", progressbarEndTime.toInt());
    setState("timerProgressDuration", progressbarDuration.toInt());

    // Runde suchen:
    QWebElement main = page->mainFrame()->findFirstElement("div#mainContent");
    QWebElementCollection rounds = main.findAll("div.boardHead");

    if(rounds.count() == 0) return;

    foreach(QWebElement head, rounds) {
        QWebElement h4 = head.firstChild();
        QString begin = h4.firstChild().toPlainText().trimmed();
        if(!begin.contains(" - ")) {
            int runde = head.toPlainText().trimmed().split(" ", QString::SkipEmptyParts).at(1).toInt();
            setState("state", round + runde);
            qDebug() << "bkTournamentModule::parse(rounds)" << state("state").toInt() - round << begin;
        }
    }
}

void bkTournamentModule::parse(QWebPage* page, QVariant, QVariant progressbarEndTime, QVariant progressbarDuration)
{
    QString dpath = page->mainFrame()->url().path().toLower();
    if(!hasPath(dpath)) return;

    if(dpath == "/joust/") {
        joustHtml(page, progressbarEndTime, progressbarDuration);
        updated();
    }

    if(active()) emit jobDone();
}

void bkTournamentModule::logic(QWebPage*)
{
    if(lastUpdate().daysTo(QDateTime::currentDateTime()) > 0) {
        int str = strength();
        ++str;
        setState("strength", str);
    }
}

void bkTournamentModule::action(QWebPage*)
{
}

void bkTournamentModule::view(QWebPage* page)
{
    bkModule::view(page);
    if(!state("enable").toBool()) return;
    QWebElement moduleElement = getModule(page->mainFrame()->documentElement());

    int modus = state("state").toInt();
    if(modus < round) {

        QWebElement text = moduleElement.findFirst("#km" + name() + "Modus");
        if(!text.isNull()) return;
        QString content = stateName(modus);
        text = appendLine(moduleElement);

        //if(!state("enable").toBool()) return;

        text.appendInside(content);
        text.setAttribute("id", "km" + name() + "Modus");
        text.setAttribute("align", "center");

    } else {
        QVariant result = state("timerProgress").toVariant();
        if(result.isValid() && result.toInt() > 0) {
            QString text = QString("%1. Runde").arg(modus - round);
            addProgressBar(moduleElement, "Progress", text);
        }
    }
}
