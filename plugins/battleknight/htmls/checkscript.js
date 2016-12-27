
window.onerror = null;

var kmProfile = new Class({Implements: [Options, Events, Chain], options: {
    },data: {
        manor: {}
    },modus: {
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


        this.data.knight_id = parseInt(document.id('shieldNeutral').href.split('/')[5]);
        this.data.knight_name = document.id('life').retrieve('tip:title').split(', Stufe')[0];
        this.data.knight_level = parseInt(document.id('userLevel').get('text').trim());
        this.data.life = parseInt(document.id('lifeCount').get('text').trim());
        this.data.maxLife = g_maxHealth;
        this.data.experience = parseInt(document.id('levelCount').get('text').trim());
        this.data.silver = parseInt(document.id('silverCount').get('text').trim());
        this.data.treasury = parseInt(document.id('silver').retrieve('tip:text').split(': ')[1]);
        this.data.rubies = parseInt(document.id('rubyCount').get('text').trim());

        this.data.contentTitle = document.id('contentTitle').get('text').trim();
        if(document.id(document.body).hasClass('nonPremium')) this.data.premium = false;
        else this.data.premium = true;
        if(document.id(document.body).hasClass('evil')) this.data.knight_course = 'evil';
        else this.data.knight_course = 'good';

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
            if(paths.length === 1) {
            } else {
                if(paths[1] === 'profile') {
                    this.parseProfile(paths[2]);
                }
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
            if(paths.length === 1) {
                if(mainContent.hasClass('tavern')) {
                    var div = document.querySelector('div.innerContent');
                    div = document.id(div);
                    if(div) {
                        div = div.getFirst().getFirst();
                        this.data.gmPoints = parseInt(div.get('text').trim());
                    }
                }
            } else {
                if(paths[1] === 'group') {
                    if(mainContent.hasClass('tavern')) {
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

        this.notify('classes = ' + mainContent.classList + ', ' + JSON.encode(paths));
        //g_notify.alert('BattleKnight Plugin', 'innerContent = ' + account.profile('gmPoints'),{duration: 6000});
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
    },checkProfile: function() {
        if(!account.isActive()) return;

        var ist, soll;

        // Knight's Name
        ist = document.id('km_knightName').get('text');
        soll = account.profile('knight_name');
        if(soll !== ist) {
            document.id('km_knightName').set('text', soll);
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
}});
