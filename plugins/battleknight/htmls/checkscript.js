
window.onerror = null;

var kmProfile = new Class({Implements: [Options, Events, Chain], options: {
    },data: {
        manor: {}
    },modus: {
    },initialize: function(options) {
        this.setOptions(options);
    },parseData: function() {
        this.data.id = parseInt(document.id('shieldNeutral').href.split('/')[5]);
        this.data.name = document.id('life').retrieve('tip:title').split(', Stufe')[0];
        this.data.level = parseInt(document.id('userLevel').get('text').trim());
        this.data.life = parseInt(document.id('lifeCount').get('text').trim());
        this.data.maxLife = g_maxHealth;
        this.data.experience = parseInt(document.id('levelCount').get('text').trim());
        this.data.silver = parseInt(document.id('silverCount').get('text').trim());
        this.data.treasury = parseInt(document.id('silver').retrieve('tip:text').split(': ')[1]);
        this.data.rubies = parseInt(document.id('rubyCount').get('text').trim());

        this.data.contentTitle = document.id('contentTitle').get('text').trim();
        if(document.id(document.body).hasClass('nonPremium')) this.data.premium = false;
        else this.data.premium = true;
        if(document.id(document.body).hasClass('evil')) this.data.course = 'evil';
        else this.data.course = 'good';

        if(typeof(progressbarEndTime) !== 'undefined') {
            this.data.waitTime = progressbarEndTime;
            if(typeof(progressbarDuration) !== 'undefined') this.data.waitDuration = progressbarDuration;
        } else if(typeof(l_titleTimerEndTime) !== 'undefined') {
            this.data.waitTime = l_titleTimerEndTime;
        }

        var path = window.location.pathname;
        var mainContent = document.id('mainContent');
        var classes = mainContent.classList;

        if(mainContent.hasClass('cooldownDuel')) this.data.waitReason = 'duel';
        else if(mainContent.hasClass('cooldownWork')) this.data.waitReason = 'work';
        else if(mainContent.hasClass('cooldownTravel')) this.data.waitReason = 'travel';
        else if(mainContent.hasClass('cooldownFight')) this.data.waitReason = 'fight';

        if((path === '/world/location' || path === '/world') && classes.contains('location')) {
            this.data.location = classes["1"];
            this.data.missionPoints = parseInt(document.id('zoneChangeCosts').get('text').trim());
        } else if(path === '/groupmission/group/' && classes.contains('tavern')) {
            var div = document.querySelector('div.innerContent');
            div = document.id(div);
            div = div.getFirst().getFirst();
            this.data.gmPoints = parseInt(div.get('text').trim());
        } else if(path === '/manor/') {
        }

        account.setProfile(this.data);
        this.checkProfile();

        g_notify.alert('BattleKnight Plugin', 'classes = ' + classes + ', ' + path,{duration: 6000});
        //g_notify.alert('BattleKnight Plugin', 'innerContent = ' + account.profile('gmPoints'),{duration: 6000});

    },checkProfile: function() {
        document.id('knightName').set('text', this.data.name);
        document.id('location').set('text', km_locations[account.profile('location')].title);
        document.id('missionPoints').set('text', account.profile('missionPoints'));
        document.id('gmPoints').set('text', account.profile('gmPoints'));
    }});

function km_itemData(item_id) {
    if(document.id(item_id)) account.setItem(document.id(item_id).retrieve('item:allAttributes'));
}

function km_checkProfile() {

    km_game.contentTitle = document.id('contentTitle').get('text').trim();

    if(typeof(progressbarEndTime) !== 'undefined') {
        km_game.waitTime = progressbarEndTime;
    } else if(typeof(l_titleTimerEndTime) !== 'undefined') {
        km_game.waitTime = l_titleTimerEndTime;
    }

    // alert: function(title, message, options)
    g_notify.alert('BattleKnight Plugin', 'km_game = ' + JSON.encode(km_game),{duration: 6000});

    if(typeof(km_game.waitTime) !== 'undefined') {
        var audio = document.id('devAudioPling');
        if($chk(audio) && !($chk(Browser.Engine.trident))) {
            //audio.play();
        }
    }

}

function km_checkProgress() {
    if(account.isActive()) {

    } else {

    }
}

var km_profile = new kmProfile();
var km_game = new Object();

window.setInterval(km_checkProgress, 250);
