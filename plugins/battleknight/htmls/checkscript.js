
var my = new Object();
var modus = new Object();


function checkPlayer() {
    my.id = document.id('shieldNeutral').href.split('/')[5];
    my.name = document.id('life').retrieve('tip:title').split(', Stufe')[0];
    my.level = document.id('userLevel').get('text').trim();
    my.life = document.id('lifeCount').get('text').trim();
    my.experience = document.id('levelCount').get('text').trim();
    my.silver = document.id('silverCount').get('text').trim();
    my.rubies = document.id('rubyCount').get('text').trim();

    modus.title = document.id('contentTitle').get('text').trim();

    if(typeof(progressbarEndTime) !== 'undefined') {
        modus.waitTime = progressbarEndTime;
    } else if(typeof(l_titleTimerEndTime) !== 'undefined') {
        modus.waitTime = l_titleTimerEndTime;
    }

    g_notify.alert('Title: ' + JSON.encode(modus));

    if(typeof(modus.waitTime) !== 'undefined') {
        var audio = document.id('devAudioPling');
        if($chk(audio) && !($chk(Browser.Engine.trident))) {
            //audio.play();
        }
    }

    document.id('knightName').set('text', my.name);

    window.setInterval(checkProgress, 250);
}

function checkProgress() {
    if(account.isActive()) {

    } else {

    }
}

window.setTimeout(checkPlayer, 250);
