#include "bklogic.h"
#include "bkaccount.h"
#include "bkparser.h"
#include "bkworld.h"

#include <QWebPage>
#include <QWebFrame>




bkLogic::bkLogic(QObject *parent) :
    QObject(parent),
    m_workerState(w_nothing)
{
    p_account = qobject_cast<bkAccount*>(parent);
    Q_ASSERT(p_account);
    connect(&t_workerTimer, SIGNAL(timeout()), this, SLOT(workerTimer()));
    t_workerTimer.setSingleShot(true);

    m_state.insert("strengthMission", (int)32);
    m_state.insert("strengthGM", (int)32);
    m_state.insert("strengthDuel", (int)32);
    m_state.insert("strengthWork", (int)32);
    m_state.insert("strengthOS", (int)32);
    m_state.insert("strengthTurnier", (int)32);
    m_state.insert("strengthTreasury", (int)32);
    m_state.insert("strengthTravel", (int)32);

    //m_state.insert("maxStrength", (int)16384);

    m_state.insert("workerState", (int)w_nothing);
}

void bkLogic::startWorker(int min, int max)
{
    int wait = qrand() % (max) + min;
    m_state.insert("workerState", (int)m_workerState);

    QVariant m = m_workerState;
    p_account->setStatus("modusLogic", m);
    m = wait / 1000;
    p_account->setStatus("timerLogic", m);
    p_account->setStatus("timerLogicDuration", m);
    t_workerTimer.start(wait);
    //qDebug() << "bkLogic::startWorker" << m_workerState;
}

QVariant bkLogic::status(const QString key)
{
    QVariant ret;
    if(!m_state.keys().contains(key)) return ret;
    ret = m_state.value(key).toVariant();
    return ret;
}

void bkLogic::workerTimer()
{
    if(m_baseUrl.isEmpty()) return;
    switch(m_workerState) {
        case w_call_working:
            {
                QUrl url(m_baseUrl);
                url.setPath("/market/work/");
                p_account->loadUrl(url);
            }
            break;
        case w_go_working:
            goWorking();
            break;
        case w_get_paying:
            {
                QWebPage* page = p_account->currentPage();
                // <a id="encashLink" class="button"><span>Sold abholen</span></a>
                /*
                 * <form id="formEncash" method="POST" action="https://s12-de.battleknight.gameforge.com:443/market/work">
                 *     <input type="hidden" name="paycheck" value="encash">
                 * </form>
                 */
                page->mainFrame()->evaluateJavaScript("document.id('formEncash').submit();");
                //next step. duell? entscheiden, wenn die seite geladen ist... ;-)
            }
            break;
        case w_call_duel:
            {
                QUrl url(m_baseUrl);
                url.setPath("/duel/");
                p_account->loadUrl(url);
            }
            break;
        case w_next_proposal:
            {
                QWebPage* page = p_account->currentPage();
                page->mainFrame()->evaluateJavaScript("requestProposals('weaker');");
            }
            break;
        case w_go_compare:
            {
                QWebPage* page = p_account->currentPage();
                //<a id="devBtnNext" class="button dev" href="https://s12-de.battleknight.gameforge.com/duel/compare/?enemyID=2387"><span>Weiter</span></a>        </div>
                page->mainFrame()->evaluateJavaScript("document.id('devBtnNext').click();");
            }
            break;
        case w_call_gm:
            {
                QUrl url(m_baseUrl);
                url.setPath("/groupmission/group/");
                p_account->loadUrl(url);
            }
            break;
        case w_reload_gm:
            {
                QUrl url(m_baseUrl);
                url.setPath("/groupmission/group/");
                p_account->loadUrl(url);
            }
            break;
        default:
            break;
    }
    qDebug() << "bkLogic::workerTimer" << m_workerState;
}

void bkLogic::calculateStrength()
{
}

void bkLogic::loadFinished(QWebPage* page)
{
    QString debugOut;
    if(t_workerTimer.isActive()) {
        t_workerTimer.stop();
        QVariant val(0);
        p_account->setStatus("timerLogic", val);
        //m_workMode = nothing;
        qDebug() << "bkLogic::loadFinished(activeWork):" << m_workerState;
    }

    if(!p_account->enabled()) return;

    m_baseUrl = page->mainFrame()->baseUrl();
    QString urlPath = page->mainFrame()->url().path();
    if(urlPath.endsWith("/")) urlPath.chop(1); // clean path
    QVariant titleTimer = page->mainFrame()->evaluateJavaScript("if(typeof l_titleTimerEndTime !== 'undefined') l_titleTimerEndTime;");
    //p_account->status("modusCooldown").toInt()

    if(urlPath == "/market/work") {
        if(!titleTimer.isValid()) {
            checkWorking();
        }
    }
/*
    if(urlPath == "/groupmission" || urlPath == "/groupmission/group") {
        if(!titleTimer.isValid()) {
            checkGM();
        } else {
            if(p_account->status("modusGM").toInt() == gm_in_group) {
                qDebug() << "GM-Cooldown?" << p_account->status("timerCooldownDuration").toInt();
                QVariant val(gm_no_points);
                p_account->setStatus("modusGM", val);
            }
        }
    }
    if(urlPath == "/duel") {
        if(p_account->status("modusGM").toInt() != gm_in_group && !titleTimer.isValid()) { // gegner suchen
            m_workerState = w_check_proposal;
            checkProposal();
        }
    }
*/
    // hier entscheiden, ob wir zuhauen.
    if(urlPath == "/duel/compare") {
    }
    if(urlPath == "/duel/duel") {
        if(titleTimer.isValid()) { // cooldown nach duell
            int wait = qrand() % ((titleTimer.toInt()*1000)/6) + 123;
            //QTimer::singleShot(wait, p_account->parser(), SLOT(callReports()));
            int work = (titleTimer.toInt()*1000) - wait;
            if(p_account->status("groupMissionPoints").toInt() > 110) {
                m_workerState = w_call_gm;
            } else {
                if(p_account->status("timerTreasury").toInt() < 5400) {
                    m_workerState = w_call_duel; // oder "geh zur billigsten bank!"
                } else {
                    m_workerState = w_call_working; // oder w_call_duel
                }
            }
            startWorker(wait, work);
            debugOut.append(QString("tt:%1, wait:%2, work:%3, treasury:%4, modus:%5")
                            .arg(titleTimer.toInt())
                            .arg(wait)
                            .arg(work)
                            .arg(p_account->status("timerTreasury").toInt())
                            .arg(m_workerState));
        }
    }
    if(urlPath == "/tavern/fightresult") {
        int wait = qrand() % ((titleTimer.toInt()*1000)/6) + 829;
        //QTimer::singleShot(wait, p_account->parser(), SLOT(callReports()));
    }


    if(!debugOut.isEmpty()) debugOut.prepend(" ");
    qDebug() << "bkLogic::loadFinished" << debugOut.prepend(urlPath.prepend("(").append(")"));
}

void bkLogic::checkWorking()
{
    QWebPage* page = p_account->currentPage();
    if(page == Q_NULLPTR) return; // tja, was tun?
    QVariant avail = page->mainFrame()->evaluateJavaScript("if(typeof availableSides !== 'undefined') availableSides;");
    qDebug() << "bkLogic::checkWorking" << avail;
    int wait = (5*60*1000); // max 5min
    switch(m_workerState) {
        case w_call_working:
            if(avail.isValid()) {
                m_workerState = w_go_working;
            } else {
            }
            startWorker(2123, wait);
            break;
        case w_go_working:
            //qDebug() << "Geld abholen?";
            m_workerState = w_get_paying;
            startWorker(1783, wait);
            break;
        case w_get_paying:
            {
                qDebug() << "neue Idee?";//w_call_duel
                QWebElement form = page->mainFrame()->findFirstElement("#formEncash");
                if(form.isNull()) m_workerState = w_call_gm;
                startWorker(726, wait);
            }
            break;
        default:
            break;
    }
}

void bkLogic::goWorking()
{
    QWebPage* page = p_account->currentPage();
    if(page == Q_NULLPTR) return; // tja, was tun?
    QVariant avail = page->mainFrame()->evaluateJavaScript("if(typeof availableSides !== 'undefined') availableSides;");
    if(!avail.isValid()) {
    }
    QString c = "neutral";
    QVariant course = p_account->player()->value("knight_course");
    if(course.isValid()) {
        if(course.toBool()) {
            c = "good";
        } else {
            c = "evil";
        }
    } else {
        // get_highscore?
    }
    /*
     * <form id="workSubmit" action="https://s12-de.battleknight.gameforge.com:443/market/work" method="POST">
     *     <input id="workFormHours" name="hours" type="hidden" value="1">
     *     <input id="workFormSide" name="side" type="hidden" value="">
     * </form>
     * side = "good", "evil", "neutral"
     */
    if(avail.toList().contains(c)) {
        QWebElement side = page->mainFrame()->findFirstElement("#workFormSide");
        side.setAttribute("value", c);
        page->mainFrame()->evaluateJavaScript("document.id('workSubmit').submit();");
        qDebug() << "go_working knight_course:" << course << c;
    } else {
        // change_location?
    }
}

void bkLogic::checkGM()
{
/*
    int max = (7*60*1000); // max 7min
    int gmModus = p_account->status("modusGM").toInt();
    switch(gmModus) {
        case gm_group_found: // best group
        case gm_in_group:
            max = (25*60*1000); // max 25min
            m_workerState = w_reload_gm;
            break;
        default:
            m_workerState = w_call_duel;
            break;
    }
    //qDebug() << "bkLogic::checkGM" << wait << m_workerState;
    startWorker(123, max);
*/
}

void bkLogic::checkProposal()
{
    if(m_workerState != w_check_proposal && m_workerState != w_next_proposal) return;
    int proposal = p_account->status("currentProposal").toInt();
    if(proposal == 0) return;
    bkPlayer* knight = p_account->player(proposal);
    bkPlayer* acc = p_account->player();
    if(acc->isGreaterThan(knight)) {
        m_workerState = w_go_compare;
        startWorker(384, (45*1000));
        QWebPage* page = p_account->currentPage();
        if(page != Q_NULLPTR) {
            QString script;
            script = "kmUpdater.updateProgressText('Logic', 'go compare: ');\n";
            page->mainFrame()->evaluateJavaScript(script);
        }
    } else {
        m_workerState = w_next_proposal;
        startWorker(245, (90*1000));
        QWebPage* page = p_account->currentPage();
        if(page != Q_NULLPTR) {
            QString script;
            script = "kmUpdater.updateProgressText('Logic', 'next proposal: ');\n";
            page->mainFrame()->evaluateJavaScript(script);
        }
    }
    qDebug() << "bkLogic::checkProposal" << m_workerState << "\n" << knight << acc->isGreaterThan(knight);
}
