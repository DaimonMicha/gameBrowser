
window.onerror = null;

var targetAreas = {
    "Attack": {
        "1": ["r-S", "rechte Schulter"],
        "2": ["-K-", "Kopf"],
        "3": ["l-S", "linke Schulter"],
        "4": ["r-A", "rechter Arm"],
        "5": ["-B-", "Brust"],
        "6": ["l-A", "linker Arm"],
        "7": ["r-B", "rechtes Bein"],
        "8": ["-Z-", "Zentrum"],
        "9": ["l-B", "linkes Bein"]
    },
    "Defend": {
        "1": ["r-O", "rechter Oberkörper"],
        "2": ["l-O", "linker Oberkörper"],
        "3": ["-Z-", "Zentrum"],
        "4": ["r-S", "rechte Seite"],
        "5": ["l-S", "linke Seite"]
    }
}

var kmProfile = new Class({Implements: [Options, Events, Chain
    ],options: {
        lastDuels: 7
    },data: {
        manor: {}
    },modus: {
    },myTable: {
    },initialize: function(options) {
        this.setOptions(options);
        this.parseData();
    },parseData: function() {
        //this.notify(window.location.href);
        var mainContent = document.id('mainContent');
        var paths = [];
        Array.each(window.location.pathname.split('/'),function(item){
            if(item !== '') paths.push(item);
        });

        if(document.id(document.body).hasClass('nonPremium')) this.data.premium = false;
        else this.data.premium = true;
        this.data.contentTitle = document.id('contentTitle').get('text').trim();

        this.parsePlayer();

        if(mainContent.hasClass('cooldownDuel')) this.data.waitReason = 'duel';
        else if(mainContent.hasClass('cooldownWork')) this.data.waitReason = 'work';
        else if(mainContent.hasClass('cooldownTravel')) this.data.waitReason = 'travel';
        else if(mainContent.hasClass('cooldownFight')) this.data.waitReason = 'fight';

        if(typeof(progressbarEndTime) !== 'undefined') {
            this.data.waitTime = progressbarEndTime;
            if(typeof(progressbarDuration) !== 'undefined') {
                this.data.waitDuration = progressbarDuration;
                if(typeof(l_callUrl) !== 'undefined') {
                    this.data.waitCaller = l_callUrl;
                    //this.notify(this.data.waitCaller);
                }
            }
        } else if(typeof(l_titleTimerEndTime) !== 'undefined') {
            this.data.waitTime = l_titleTimerEndTime;
        }

        if(paths[0] === 'common') {
            if(paths.length > 1 && paths[1] === 'profile') {
                this.parseProfile(paths[2]);
            }
        } else if(paths[0] === 'world') {
            if(paths.length === 1) {
                if(mainContent.hasClass('location')) {
                    this.data.location = mainContent.classList["1"];
                    this.data.missionPoints = parseInt(document.id('zoneChangeCosts').get('text').trim());
                }
            } else {
                if(paths[1] === 'location') {
                    if(mainContent.hasClass('location')) {
                        this.data.location = mainContent.classList["1"];
                        this.data.missionPoints = parseInt(document.id('zoneChangeCosts').get('text').trim());
                    }
                }
            }
        } else if(paths[0] === 'groupmission') {
            if(paths.length === 1 || paths[1] === 'group') {
                if(mainContent.hasClass('tavern')) {
                    if(document.id('selectfoundPlandata')) {
                        this.data.gmPoints = 120;
                    } else {
                        var div = document.querySelector('div.innerContent');
                        div = document.id(div);
                        if(div) {
                            div = div.getFirst().getFirst();
                            this.data.gmPoints = parseInt(div.get('text').trim());
                        }
                    }
                }
            }
        } else if(paths[0] === 'duel') {
            if(paths.length === 1) {
                this.parseProposal();
            } else {
                if(paths[1] === 'compare') {
                    // ?enemyID=2343
                    //this.notify(JSON.encode(window.location.search));
                    if(window.location.search !== '') {
                        //this.notify(window.location.search.split('enemyID=').getLast());
                    }
                } else if(paths[1] === 'duel') {
                    // ?enemyID=2343
                    if(window.location.search !== '') {
                        //this.notify(window.location.search.split('enemyID=').getLast());
                    }
                }
            }
        } else if(paths[0] === 'clan') {
            if(paths.length === 1) {
            } else {
                if(paths[1] === 'members') {
                    this.parseClanMembers();
                }
            }
        } else if(paths[0] === 'manor') {
            this.parseManor();
        } else if(paths[0] === 'highscore') {
            this.parseHighscore();
        }

        account.setProfile(this.data);

        this.checkProfile();
        this.checkDuels();
        //this.notify('classes = ' + mainContent.classList + ', ' + JSON.encode(paths));
    },parsePlayer: function() {
        this.data.knight_id = parseInt(document.id('shieldNeutral').href.split('/')[5]);
        this.data.knight_name = document.id('life').retrieve('tip:title').split(', Stufe')[0];
        this.data.knight_level = parseInt(document.id('userLevel').get('text').trim());
        this.data.life = parseInt(document.id('lifeCount').get('text').trim());
        this.data.maxLife = g_maxHealth;
        this.data.experience = parseInt(document.id('levelCount').get('text').trim());
        this.data.silver = parseInt(document.id('silverCount').get('text').trim());
        this.data.treasury = parseInt(document.id('silver').retrieve('tip:text').split(': ')[1]);
        this.data.rubies = parseInt(document.id('rubyCount').get('text').trim());
        if(document.id(document.body).hasClass('evil')) this.data.knight_course = 'evil';
        else this.data.knight_course = 'good';
    },parseProfile: function(kid) {
        var player = {};
        player.knight_id = parseInt(kid);
        var container = document.id('profileImage');
        container = container.getElement('h2');
        var temp = container.get('text').trim();
        var pos = temp.indexOf(' ');
        player.knight_rang = temp.substr(0,pos);
        player.knight_name = temp.substr(pos+1);
        container = document.id('profileDetails');
        var rows = container.getElements('td');
        rows.each(function(row,index){
            var tmp = row.get('text').trim();
            switch(index) {
                case 0:
                    player.knight_level = parseInt(tmp);
                    break;
                case 1:
                    var link = row.getFirst();
                    if(link) {
                        player.clan_id = parseInt(link.pathname.split('/')[3]);
                        player.clan_tag = tmp.substr(1,tmp.length-2);
                    }
                    break;
                case 2:
                    if(player.clan_id) player.clan_rang = tmp;
                    break;
                case 3:
                    player.loot_won = parseInt(tmp);
                    break;
                case 4:
                    player.loot_lose = parseInt(tmp);
                    break;
                case 5:
                    player.turniere = parseInt(tmp);
                    break;
                case 6:
                    player.fights_won = parseInt(tmp);
                    break;
                case 7:
                    player.fights_balance = parseInt(tmp);
                    break;
                case 8:
                    player.fights_lose = parseInt(tmp);
                    break;
                default:
                    break;
            }
        },this);
        container = document.id('profileAttrib');
        rows = container.getElements('td');
        rows.each(function(row,index){
            var tmp = row.get('text').trim();
            switch(index) {
                case 0: // Stärke
                    player.strength = parseInt(tmp);
                    break;
                case 1: // Geschick
                    player.dexterity = parseInt(tmp);
                    break;
                case 2: // Konstitution
                    player.endurance = parseInt(tmp);
                    break;
                case 3: // Glück
                    player.luck = parseInt(tmp);
                    break;
                case 4: // Waffenkunst
                    player.weapon = parseInt(tmp);
                    break;
                case 5: // Verteidigungskunst
                    player.shield = parseInt(tmp);
                    break;
                case 6: // Schaden
                    tmp = tmp.split(' - ');
                    player.damage_min = tmp[0];
                    player.damage_max = tmp[1];
                    break;
                case 7: // Rüstung
                    player.armour = parseInt(tmp);
                    break;
                default:
                    break;
            }
        },this);
        account.setPlayer(player);
    },parseProposal: function() {
        var proposal = document.id('proposals');
        if(!proposal) return;
        // alle a finden (bis zu 3)
        var anchors = [];
        Array.each(proposal.getElements('a'),function(anchor){
            anchors.push(anchor.href);
        });
        //this.notify(anchors.length + ' ' + JSON.encode(anchors));
    },parseClanMembers: function() {
        var cmTable = document.id('membersTable');
        var cmBody = cmTable.getElement('tbody');
        var cmRows = cmBody.getElements('tr');
        cmRows.each(function(row){
            var cmCols = row.getElements('td');
            var player = {};
            cmCols.each(function(col){
                if(col.hasClass('memberName')) {
                    var link = col.getFirst();
                    player.knight_id = parseInt(link.pathname.split('/')[3]);
                    var temp = link.get('text');
                    var pos = temp.indexOf(' ');
                    player.knight_rang = temp.substr(0,pos);
                    player.knight_name = temp.substr(pos+1);
                } else if(col.hasClass('memberLevel')) {
                    player.knight_level = parseInt(col.get('text'));
                } else if(col.hasClass('memberSilver')) {
                    player.silver_spend = parseInt(col.get('text').replace(/\./g,''));
                } else if(col.hasClass('memberRubies')) {
                    player.rubies_spend = parseInt(col.get('text').replace(/\./g,''));
                } else if(col.hasClass('memberActivity')) {
                    var div = col.getFirst();
                    var temp = div.retrieve('tip:title');
                    var pos = temp.indexOf(': ');
                    player.last_activity = temp.substr(pos+2);
                }
            },this);
            account.setPlayer(player);
        },this);
    },parseManor: function() {
        for(i=1;i<7;++i) {
            var item = document.id('manorItem'+i);
            if(item.hasClass('manorItemActive')) {
                var days = document.id('manorDays'+i);
                this.data.manor['Item'+i] = parseInt(days.get('text'));
            }
        }
    },parseHighscore: function() {
        var hsTable = document.id('highscoreTable');
        var hsBody = hsTable.getElement('tbody');
        var hsRows = hsBody.getElements('tr');
        hsRows.each(function(row,key){
            if(!row.hasClass('userSeperator')) {
                var hsCols = row.getElements('td');
                var player = {};
                hsCols.each(function(col){
                    if(col.hasClass('highscore01')) {
                    } else if(col.hasClass('highscore02')) {
                        if(col.getFirst().hasClass('iconKarmaEvil')) player.course = 'evil';
                        else player.knight_course = 'good';
                    } else if(col.hasClass('highscore03')) {
                        var links = col.getElements('a');
                        player.knight_id = parseInt(links[0].pathname.split('/')[3]);
                        var temp = links[0].get('text');
                        var pos = temp.indexOf(' ');
                        player.knight_rang = temp.substr(0,pos);
                        player.knight_name = temp.substr(pos+1);
                        if(links.length === 2) {
                            player.clan_id = parseInt(links[1].pathname.split('/')[3]);
                            temp = links[1].get('text');
                            player.clan_tag = temp.substr(1,temp.length-2);
                        }
                    } else if(col.hasClass('highscore04')) {
                        player.knight_level = parseInt(col.get('text'));
                    } else if(col.hasClass('highscore05')) { // loot
                        player.loot_won = parseInt(col.get('text').replace(/\./g,''));
                    } else if(col.hasClass('highscore06')) { // fights
                        player.fights = parseInt(col.get('text').replace(/\./g,''));
                    } else if(col.hasClass('highscore07')) { // fights_won
                        player.fights_won = parseInt(col.get('text').replace(/\./g,''));
                    } else if(col.hasClass('highscore08')) { // fights_lose
                        player.fights_lose = parseInt(col.get('text').replace(/\./g,''));
                    }
                },this);
                account.setPlayer(player);
            }
        },this);
    },getItemData: function(item_id) {
        if(document.id(item_id)) account.setItem(document.id(item_id).retrieve('item:allAttributes'));
    },notify: function(text) {
        g_notify.alert('BattleKnight Plugin', text, {duration: 5000});
    },pling: function() {
        var audio = document.id('devAudioPling');
        if($chk(audio) && !($chk(Browser.Engine.trident))) {
            audio.play();
        }
    },buildZonesTooltip: function(knight_id) {
        var tip;
        var hitZones = account.player(knight_id,'hitZones');
        var defendZones = account.player(knight_id,'defendZones');
        if(!hitZones || !defendZones) return;
        tip = '[ ';
        JSON.decode(hitZones).each(function(zone,idx){
            if(idx > 0) tip += ', ';
            tip += targetAreas.Attack[zone][0];
        },this);
        tip += ' ]<br>[ ';
        JSON.decode(defendZones).each(function(zone,idx){
            if(idx > 0) tip += ', ';
            tip += targetAreas.Defend[zone][0];
        },this);
        tip += ' ]';
        return(tip);
    },checkProfile: function() {
        if(!account.isActive()) return;
        var ist, soll;
        // Knight's Name
        var knightName = document.id('km_knightName');
        ist = knightName.get('text');
        soll = account.profile('knight_name');
        if(soll !== ist) {
            knightName.set('text', soll);
        }
        if(!knightName.hasClass('knightToolTip')) {
            var zonesTip = this.buildZonesTooltip(this.data.knight_id);
            if(zonesTip) {
                knightName.store('tip:title', 'Trefferzonen');
                knightName.store('tip:text', zonesTip);
                knightName.addClass('knightToolTip');
                var km_knightToolTips = new Tips('.knightToolTip', {showDelay: 0,hideDelay: 10,className: 'mediumTip',onShow: function(tip) {
                        tip.fade(0.8);
                    },onHide: function(tip) {
                        tip.fade(0);
                    },text: ''});
            }
        }
        // Knight's Location
        ist = document.id('km_location').get('text');
        if(account.profile('location') && km_locations[account.profile('location')].title !== ist) {
            document.id('km_location').set('text', km_locations[account.profile('location')].title);
        }
        // Mission Points
        ist = document.id('km_missionPoints').get('text');
        soll = account.profile('missionPoints');
        if(soll !== ist) {
            document.id('km_missionPoints').set('text', soll);
        }
        // GroupMission Points
        ist = document.id('km_gmPoints').get('text');
        soll = account.profile('gmPoints');
        if(soll !== ist) {
            document.id('km_gmPoints').set('text', soll);
        }
    },buildReportRow: function(id, report) {
        if(document.id('report'+id)) return;
        var duelsTableBody = document.id('duelTable').getLast();
        var newRow = new Element('tr', {'id': 'report'+id, 'class': 'reportToolTip'});
        var newEl;
        newEl = new Element('td');
        var icon = new Element('span', {'class': 'duelIcon'}); //iconWins
        var stats;
        var opponentId;
        if(parseInt(this.data.knight_id) === parseInt(report.aggressor_id)) {
            stats = report.fight_stats.aggressor;
            opponentId = parseInt(report.defender_id);
        } else {
            stats = report.fight_stats.defender;
            opponentId = parseInt(report.aggressor_id);
            newRow.setStyle('background-color', 'rgba(165, 42, 42, 0.45)');
        }
        var tipText = this.data.knight_name;
        if(stats.fightResult === 'won') {
            icon.addClass('iconWins');
            tipText += ' erbeutet '+stats.silver+' Silber von ';
        } else {
            icon.addClass('iconLosses');
            tipText += ' verliert '+Math.abs(parseInt(stats.silver))+' Silber an ';
        }
        icon.inject(newEl);
        tipText += account.player(opponentId,'knight_name')+'.';
        tipText += '<br />'+this.buildZonesTooltip(opponentId);
        var fTime = report.fight_time;//.split(' ')[1];
        var pos = fTime.lastIndexOf(':')
        fTime = fTime.substr(0,pos);
        newRow.setAttribute('onClick', 'g_notify.alert("Duell ('+fTime+')", "'+tipText+'", {duration: 5000});');
        newRow.store('tip:title', 'Duell ('+fTime+')');
        newRow.store('tip:text', tipText);
        newEl.inject(newRow);
        newEl = new Element('td', {'align': 'right'});
        var knightLink = new Element('a', {'class': 'specialKM', 'rel': '#modal_kmKnight', 'onClick': 'window.km_currentKnight='+opponentId+';'});
        knightLink.set('text', account.player(opponentId,'knight_name'));
        knightLink.inject(newEl);
        newEl.inject(newRow);
        newRow.inject(duelsTableBody);
    },checkDuels: function() {
        var temp = account.reports(this.options.lastDuels,'duel');
        if(!temp) return;
        var duelsTable = document.id('duelTable').getLast();
        var duels = JSON.decode(temp);
        var keys = Object.keys(duels);
        keys.reverse();
        Array.each(keys,function(entry){
            var report = duels[entry];
            this.buildReportRow(entry,report);
        },this);
        var km_knightToolTips = new Tips($$('.reportToolTip'), {showDelay: 0,hideDelay: 10,className: 'mediumTip',onShow: function(tip) {
                tip.fade(0.8);
            },onHide: function(tip) {
                tip.fade(0);
            },text: ''});
        SqueezeBox.assign($$('a.specialKM'), {handler: 'rel',size: {x: 710,y: 470},classWindow: 'sbox-window-custom sbox-window-kmZones',
            onOpen: function() {
                var allDivs = $('sbox-content').getElements('div');
                allDivs.each(function(kDiv){
                    if(kDiv.hasClass('missionName')) {
                        kDiv.getFirst().set('text', account.player(window.km_currentKnight,'knight_name'));
                    }
                },this);
        }});
    }
});
